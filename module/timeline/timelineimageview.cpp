#include "timelineimageview.h"
#include "widgets/scrollbar.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include <math.h>
#include <QEvent>
#include <QPushButton>
#include <QScrollBar>
#include <QMouseEvent>
#include <QDebug>

const int SLIDER_FRAME_WIDTH = 130;
const int TOP_TOOLBAR_HEIGHT = 40;

TimelineImageView::TimelineImageView(bool multiselection, QWidget *parent)
    : QScrollArea(parent),
      m_vScrollBar(new ScrollBar),
      m_ascending(false),
      m_multiSelection(multiselection),
      m_scrollPerc(0.0),
      m_iconSize(96, 96)
{
    initSliderFrame();
    initTopTips();
    initContents();

    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setVerticalScrollBar(m_vScrollBar);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    qRegisterMetaType<DatabaseManager::ImageInfo>("DatabaseManager::ImageInfo");
    // Watching import thread
    connect(SignalManager::instance(), &SignalManager::imageInserted,
            this, &TimelineImageView::onImageInserted/*, Qt::QueuedConnection*/);
    connect(SignalManager::instance(), &SignalManager::imageRemoved,
            this, &TimelineImageView::removeImage);
    installEventFilter(this);
}

/*!
 * \brief TimelineImageView::clearImages
 * Destroy all frame and images to save memory.
 */
void TimelineImageView::clearImages()
{
    for (TimelineViewFrame * frame : m_frames.values()) {
        m_contentLayout->removeWidget(frame);
        delete frame;
    }
    m_frames.clear();
}

void TimelineImageView::onImageInserted(const DatabaseManager::ImageInfo &info)
{
    if (! isVisible()) {
        return;
    }
    const QString timeLine = utils::base::timeToString(info.time, true);
    // TimeLine frame not exist, create one
    // Note: use m_frames.keys().indexOf(timeLine) will cause[QObject::connect:
    //       Cannot queue arguments of type 'QList<QPersistentModelIndex>']
    //       and I do not know why.
    if (m_frames[timeLine] == nullptr) {
        inserFrame(timeLine);
    }
    else {
        m_frames.value(timeLine)->insertItem(info);
    }
}

void TimelineImageView::clearSelection()
{
    for (TimelineViewFrame * frame : m_frames.values()) {
        frame->clearSelection();
    }
}

void TimelineImageView::setIconSize(const QSize &iconSize)
{
    for (TimelineViewFrame * frame : m_frames.values()) {
        frame->setIconSize(iconSize);
    }

    m_iconSize = iconSize;
    updateContentRect();
}

void TimelineImageView::updateThumbnail(const QString &name)
{
    for (TimelineViewFrame * frame : m_frames.values()) {
        frame->updateThumbnail(name);
    }
}

bool TimelineImageView::isEmpty() const
{
    return m_frames.isEmpty();
}

/*!
    \fn QMap<QString, QString> TimelineImageView::selectedImages() const

    Return the name-path map of all frame's selected items.
*/
QMap<QString, QString> TimelineImageView::selectedImages() const
{
    QMap<QString, QString> images;
    for (TimelineViewFrame * frame : m_frames.values()) {
        const QMap<QString, QString> map = frame->selectedImages();
        for (QString name : map.keys()) {
            images[name] = map[name];
        }
    }

    return images;
}

QString TimelineImageView::currentMonth()
{
    return getMonthByTimeline(currentTimeline());
}

void TimelineImageView::resizeEvent(QResizeEvent *e)
{
    QScrollArea::resizeEvent(e);
    updateContentRect();
    updateSliderFrmaeRect();
    updateTopTipsRect();
}

void TimelineImageView::initSliderFrame()
{
    m_sliderFrame = new SliderFrame(this);
    connect(m_sliderFrame, &SliderFrame::valueChanged, this, [this](double perc) {
        if (m_sliderFrame->isVisible()) {
            if (m_sliderFrame->containsMouse()) {
                m_vScrollBar->setValue((1 - perc) * (m_vScrollBar->maximum() - m_vScrollBar->minimum()));
            }
        }
    });
    connect(m_vScrollBar, &ScrollBar::valueChanged, this, [this] {
//        qDebug() << m_vScrollBar->value()
//                 << m_vScrollBar->maximum()
//                 << m_vScrollBar->minimum();
        m_sliderFrame->setValue(scrollingPercent());
        QString month = currentMonth();
        m_sliderFrame->setCurrentInfo(month, DatabaseManager::instance()->getImagesCountByMonth(month));
    });
    m_sliderFrame->hide();
}

void TimelineImageView::initTopTips()
{
    m_topTips = new TopTimelineTips(this);
    connect(m_vScrollBar, &QScrollBar::valueChanged, this, [this] {
        if (scrollingPercent() == 0) {
            m_topTips->hide();
        }
        else {
            m_topTips->setText(currentTimeline());
            m_topTips->show();
        }
    });
    m_topTips->hide();
}

void TimelineImageView::initContents()
{
    m_contentFrame = new QFrame;
    m_contentFrame->setObjectName("TimelinesContent");
    m_contentFrame->setAutoFillBackground(true);
    m_contentLayout = new QVBoxLayout(m_contentFrame);
    m_contentLayout->setContentsMargins(0, 50, 0, 10);

    setWidget(m_contentFrame);
}

void TimelineImageView::insertReadyFrames()
{
    if (! isVisible() || ! m_frames.isEmpty()) {
        return;
    }

    // Read all timelines for initialization
    QStringList timelines = DatabaseManager::instance()->getTimeLineList(m_ascending);
    for (QString timeline : timelines) {
        inserFrame(timeline, m_multiSelection);
    }
    // Delay for viewport expanded
    QTimer::singleShot(100, this, [=] {
        m_vScrollBar->setValue(m_vScrollBar->maximum() * m_scrollPerc);
    });
}

bool TimelineImageView::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    if (e->type() == QEvent::Hide) {
        m_scrollPerc = 1.0 * m_vScrollBar->value() / m_vScrollBar->maximum();
        // Make sure the other module is ready(eg.view)
        QMetaObject::invokeMethod(this, "clearImages", Qt::QueuedConnection);
    }
    else if (e->type() == QEvent::Show) {
        // Fix me: if not delay insert, the scroller will always scroll back to
        // top after scrolled by m_sliderFrame
        if (m_frames.isEmpty())
            QTimer::singleShot(100, this, SLOT(insertReadyFrames()));
    }
    return false;
}

template <typename T>
QList<T> reversed( const QList<T> & in ) {
    QList<T> result;
    result.reserve( in.size() );
    std::reverse_copy( in.begin(), in.end(), std::back_inserter( result ) );
    return result;
}

void TimelineImageView::inserFrame(const QString &timeline, bool multiselection)
{
    TimelineViewFrame *frame = new TimelineViewFrame(timeline, multiselection, this);
    connect(frame, &TimelineViewFrame::mousePress, this, [=] (QMouseEvent *e){
        if (e->button() != Qt::LeftButton)
            return;
        for (TimelineViewFrame *frame : m_frames) {
            if (! multiselection && frame != sender()) {
                frame->clearSelection();
            }
        }
    });
    connect(frame, &TimelineViewFrame::customContextMenuRequested,
            this, &TimelineImageView::customContextMenuRequested);
    m_frames.insert(timeline, frame);
    QStringList timelines = m_frames.keys();
    if (!m_ascending) {
        timelines = reversed(timelines);
    }
    m_contentLayout->insertWidget(timelines.indexOf(timeline), frame);
}

void TimelineImageView::removeFrame(const QString &timeline)
{
    QWidget *w = m_frames.value(timeline);
    m_contentLayout->removeWidget(w);
    m_frames.remove(timeline);
    w->deleteLater();
}

void TimelineImageView::removeImage(const QString &name)
{
    const QStringList timelines = m_frames.keys();
    for (QString t : timelines) {
        if (m_frames.value(t)->removeItem(name)) {
            // Check if timeline is empty
            if (m_frames.value(t)->isEmpty()) {
                removeFrame(t);
            }
            break;
        }
    }
}

void TimelineImageView::updateSliderFrmaeRect()
{
    m_sliderFrame->move(0, 0);
    m_sliderFrame->resize(SLIDER_FRAME_WIDTH, height());
}

void TimelineImageView::updateContentRect()
{
    int hMargin = (width() - getMinContentsWidth()) / 2;
    m_contentLayout->setContentsMargins(hMargin, 50, hMargin, 10);
    m_contentFrame->setFixedWidth(width());
}

void TimelineImageView::updateTopTipsRect()
{
    m_topTips->move(0, TOP_TOOLBAR_HEIGHT);
    m_topTips->resize(width(), m_topTips->height());
}

int TimelineImageView::getMinContentsWidth()
{
    int itemSpacing = 10;
    int viewHMargin = 14 * 2;
    int holdCount = floor((double)(width() - itemSpacing - viewHMargin) / (m_iconSize.width() + itemSpacing));
    return (m_iconSize.width() + itemSpacing) * holdCount + itemSpacing + viewHMargin;
}

QString TimelineImageView::currentTimeline()
{
    if (m_frames.isEmpty())
        return QString();
    int currentY = verticalScrollBar()->maximum() * scrollingPercent() + contentsMargins().top();
    QString timeline = m_frames.last()->timeline();
    for (TimelineViewFrame *frame : m_frames) {
        if (frame->geometry().contains(frame->x(), currentY)) {
            timeline = frame->timeline();
            break;
        }
    }
    return timeline;
}

QString TimelineImageView::getMonthByTimeline(const QString &timeline)
{
    return utils::base::stringToDateTime(timeline).toString("yyyy.MM");
}

double TimelineImageView::scrollingPercent()
{
    return (double)(verticalScrollBar()->value())
            / (verticalScrollBar()->maximum() - verticalScrollBar()->minimum());
}
