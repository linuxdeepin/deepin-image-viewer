#include <math.h>
#include "timelineimageview.h"
#include "timelineviewframe.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include <QPushButton>
#include <QScrollBar>
#include <QDebug>

TimelineImageView::TimelineImageView(QWidget *parent)
    : QScrollArea(parent), m_ascending(false), m_iconSize(96, 96)
{
    m_sliderFrame = new SliderFrame(this);
    connect(m_sliderFrame, &SliderFrame::valueChanged, this, [this](double perc) {
        verticalScrollBar()->setValue((1 - perc) * (verticalScrollBar()->maximum() - verticalScrollBar()->minimum()));
    });
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value){
        m_sliderFrame->setValue((double)value /
                                (verticalScrollBar()->maximum() - verticalScrollBar()->minimum()));
    });

    m_contentFrame = new QFrame;
    m_contentFrame->setObjectName("TimelinesContent");
    m_contentFrame->setAutoFillBackground(true);
    m_contentLayout = new QVBoxLayout(m_contentFrame);
    m_contentLayout->setContentsMargins(0, 50, 0, 10);

    setWidget(m_contentFrame);
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setMaximum(25);
    verticalScrollBar()->setMinimum(0);

    // Read all timelines for initialization
    QStringList timelines = DatabaseManager::instance()->getTimeLineList(m_ascending);
    for (QString timeline : timelines) {
        inserFrame(timeline);
    }

    qRegisterMetaType<DatabaseManager::ImageInfo>("DatabaseManager::ImageInfo");
    // Watching import thread
    connect(SignalManager::instance(), &SignalManager::imageInserted,
            this, [=] (const DatabaseManager::ImageInfo &info) {
        const QString timeLine = info.time.toString(DATETIME_FORMAT);
        // TimeLine frame not exist, create one
        if (m_frames.keys().indexOf(timeLine) == -1) {
            inserFrame(timeLine);
        }
        m_frames.value(timeLine)->insertItem(info);
    }, Qt::QueuedConnection);
}

void TimelineImageView::setIconSize(const QSize &iconSize)
{
    for (TimelineViewFrame * frame : m_frames.values()) {
        frame->setIconSize(iconSize);
    }

    m_iconSize = iconSize;
    updateContentRect();
}

QStringList TimelineImageView::selectedImages()
{
    QStringList names;
    for (TimelineViewFrame * frame : m_frames.values()) {
        names << frame->selectedImages();
    }

    return names;
}

void TimelineImageView::resizeEvent(QResizeEvent *e)
{
    QScrollArea::resizeEvent(e);
    updateContentRect();
    updateSliderFrmaeRect();
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
    TimelineViewFrame *frame = new TimelineViewFrame(timeline);
    m_frames.insert(timeline, frame);
    QStringList timelines = m_frames.keys();
    if (!m_ascending) {
        timelines = reversed(timelines);
    }
    m_contentLayout->insertWidget(timelines.indexOf(timeline), frame);
}

void TimelineImageView::removeFrame(const QString &timeline)
{
    Q_UNUSED(timeline)
}

void TimelineImageView::updateSliderFrmaeRect()
{
    m_sliderFrame->move(0, 0);
    m_sliderFrame->resize(130, height());
}

void TimelineImageView::updateContentRect()
{
    int hMargin = (width() - getMinContentsWidth()) / 2;
    m_contentLayout->setContentsMargins(hMargin, 50, hMargin, 10);
    m_contentFrame->setFixedWidth(width());
}

int TimelineImageView::getMinContentsWidth()
{
    int itemSpacing = 10;
    int viewHMargin = 14 * 2;
    int holdCount = floor((double)(width() - itemSpacing - viewHMargin) / (m_iconSize.width() + itemSpacing));
    return (m_iconSize.width() + itemSpacing) * holdCount + itemSpacing + viewHMargin;
}
