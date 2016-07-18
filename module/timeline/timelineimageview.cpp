#include "timelineimageview.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include "controller/importer.h"
#include "controller/configsetter.h"
#include "utils/baseutils.h"
#include "widgets/scrollbar.h"
#include <math.h>
#include <QEvent>
#include <QPushButton>
#include <QScrollBar>
#include <QMouseEvent>
#include <QDebug>

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;

const int MIN_ICON_SIZE = 96;
const QString SETTINGS_GROUP = "TIMEPANEL";
const QString SETTINGS_ICON_SCALE_KEY = "IconScale";

}  //namespace

TimelineImageView::TimelineImageView(bool multiselection, QWidget *parent)
    : QScrollArea(parent),
      m_ascending(false),
      m_multiSelection(multiselection),
      m_iconSize(96, 96)
{
    setVerticalScrollBar(new ScrollBar(this));

//    initSliderFrame();
    initTopTips();
    initContents();

    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
//    if (! isVisible()) {
//        return;
//    }
    const QString timeLine = utils::base::timeToString(info.time, true);
    // TimeLine frame not exist, create one
    // Note: use m_frames.keys().indexOf(timeLine) will cause[QObject::connect:
    //       Cannot queue arguments of type 'QList<QPersistentModelIndex>']
    //       and I do not know why.
    if (m_frames[timeLine] == nullptr) {
        inserFrame(timeLine);
        m_frames[timeLine]->setIconSize(m_iconSize);
    }
    m_frames[timeLine]->insertItem(info);
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
    updateTopTipsRect();
}

void TimelineImageView::setTickable(bool v)
{
    for (TimelineViewFrame * frame : m_frames.values()) {
        frame->setTickable(v);
    }
}

void TimelineImageView::setMultiSelection(bool multiple)
{
    for (TimelineViewFrame * frame : m_frames.values()) {
        frame->setMultiSelection(multiple);
    }
}

void TimelineImageView::updateThumbnail(const QString &name)
{
    for (TimelineViewFrame * frame : m_frames.values()) {
        frame->updateThumbnail(name);
    }
}

bool TimelineImageView::isMultiSelection() const
{
    if (m_frames.isEmpty())
        return false;
    else
        return m_frames.first()->isMultiSelection();
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
    // FIXME
    // m_contentLayout's ContentsMargins would be change after resize
//    m_contentLayout->setContentsMargins(0, 50, 0, 10);
}

void TimelineImageView::initSliderFrame()
{
//    m_sliderFrame = new SliderFrame(this);
//    connect(m_sliderFrame, &SliderFrame::valueChanged, this, [this](double perc) {
//        if (m_sliderFrame->isVisible()) {
//            if (m_sliderFrame->containsMouse()) {
//                verticalScrollBar()->setValue(
//                            (1 - perc) * (verticalScrollBar()->maximum()
//                                          - verticalScrollBar()->minimum()));
//            }
//        }
//    });
//    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this] {
////        qDebug() << m_vScrollBar->value()
////                 << m_vScrollBar->maximum()
////                 << m_vScrollBar->minimum();
//        m_sliderFrame->setValue(scrollingPercent());
//        QString month = currentMonth();
//        m_sliderFrame->setCurrentInfo(month, DatabaseManager::instance()->getImagesCountByMonth(month));
//    });
//    m_sliderFrame->hide();
}

void TimelineImageView::initTopTips()
{
    m_topTips = new TopTimelineTips(this);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this] {
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
    m_stretchFrame = new QFrame;

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

    const QList<DatabaseManager::ImageInfo> infos =
            DatabaseManager::instance()->getAllImageInfos();
    // Load up to 100 images at initialization to accelerate rendering
    for (int i = 0; i < qMin(infos.length(), 100); i ++) {
        onImageInserted(infos[i]);
    }

    TIMER_SINGLESHOT(500, {
        if (infos.length() >= 100) {
            for (int i = 100; i < infos.length(); i ++) {
                onImageInserted(infos[i]);
            }
        }
    }, this, infos);

    const int iconSize = ConfigSetter::instance()->value(
                SETTINGS_GROUP,
                SETTINGS_ICON_SCALE_KEY,
                QVariant(0)).toInt() * 32 + MIN_ICON_SIZE;
    setIconSize(QSize(iconSize, iconSize));
}

bool TimelineImageView::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    if (e->type() == QEvent::Hide) {
//        m_scrollPerc = 1.0 * m_vScrollBar->value() / m_vScrollBar->maximum();
//        // Make sure the other module is ready(eg.view)
//        QMetaObject::invokeMethod(this, "clearImages", Qt::QueuedConnection);
    }
    else if (e->type() == QEvent::Show) {
        if (m_frames.isEmpty())
            insertReadyFrames();
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

void TimelineImageView::inserFrame(const QString &timeline)
{
    TimelineViewFrame *frame = new TimelineViewFrame(timeline, this);
    connect(frame, &TimelineViewFrame::singleClicked, this, [=] (QMouseEvent *e){
        TimelineViewFrame *f = qobject_cast<TimelineViewFrame *>(sender());
        if (e->button() == Qt::LeftButton ||
                (e->button() == Qt::RightButton && ! posInSelected(e->pos(), f))) {
            for (TimelineViewFrame *frame : m_frames) {
                if (frame != f) {
                    frame->clearSelection();
                }
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

    // Aways put the stretch frame at the end of layout
    QLayoutItem *item = m_contentLayout->takeAt(
                m_contentLayout->indexOf(m_stretchFrame));
    m_contentLayout->insertWidget(timelines.indexOf(timeline), frame);
    m_contentLayout->addWidget(m_stretchFrame, 1, Qt::AlignTop);
    delete item;
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
//    m_sliderFrame->move(0, 0);
//    m_sliderFrame->resize(SLIDER_FRAME_WIDTH, height());
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
    if (! m_frames.isEmpty()) {
        TIMER_SINGLESHOT(100, {
            m_topTips->setLeftMargin(- m_frames.first()->hOffset() +
                                    m_contentLayout->contentsMargins().left());
                         },this)
    }
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

bool TimelineImageView::posInSelected(const QPoint &pos, TimelineViewFrame *frame)
{
    for (TimelineViewFrame *f : m_frames) {
        if (frame == f && f->posInSelected(pos)) {
            return true;
        }
    }
    return false;
}
