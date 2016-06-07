#include "timelineimageview.h"
#include "timelineviewframe.h"
#include "widgets/scrollbar.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include <math.h>
#include <QPushButton>
#include <QScrollBar>
#include <QDebug>

const int SLIDER_FRAME_WIDTH = 130;
const int TOP_TOOLBAR_HEIGHT = 40;

TimelineImageView::TimelineImageView(bool multiselection, QWidget *parent)
    : QScrollArea(parent),
      m_vScrollBar(new ScrollBar),
      m_ascending(false),
      m_multiSelection(multiselection),
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
            this, [=] (const DatabaseManager::ImageInfo &info) {
        const QString timeLine = utils::base::timeToString(info.time);
        // TimeLine frame not exist, create one
        if (m_frames.keys().indexOf(timeLine) == -1) {
            inserFrame(timeLine);
        }
        else {
            m_frames.value(timeLine)->insertItem(info);
        }
    }, Qt::QueuedConnection);
    connect(SignalManager::instance(), &SignalManager::imageRemoved,
            this, &TimelineImageView::removeImage);
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
            if (m_sliderFrame->pressed()) {
                verticalScrollBar()->setValue((1 - perc) * (verticalScrollBar()->maximum() - verticalScrollBar()->minimum()));
            }
            QString month = currentMonth();
            m_sliderFrame->setCurrentInfo(month, DatabaseManager::instance()->getImagesCountByMonth(month));
        }
    });
    connect(m_vScrollBar, &QScrollBar::valueChanged, this, [this] {
        m_sliderFrame->setValue(scrollingPercent());
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
    // Read all timelines for initialization
    QStringList timelines = DatabaseManager::instance()->getTimeLineList(m_ascending);
    for (QString timeline : timelines) {
        inserFrame(timeline, m_multiSelection);
    }
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
    connect(frame, &TimelineViewFrame::mousePress, this, [=] {
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
