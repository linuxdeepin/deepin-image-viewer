#include "timelineimageview.h"
#include "timelineviewframe.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include <QPushButton>
#include <QDebug>

TimelineImageView::TimelineImageView(QWidget *parent)
    : QScrollArea(parent), m_ascending(false)
{
    m_contentFrame = new QFrame;
    m_contentFrame->setObjectName("TimelinesContent");
    m_contentFrame->setAutoFillBackground(true);
    m_contentLayout = new QVBoxLayout(m_contentFrame);
    m_contentLayout->setContentsMargins(0, 50, 0, 10);

    setWidget(m_contentFrame);
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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
    m_contentFrame->setFixedWidth(width());
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
//    qDebug() << "New Timeline: " << timeline;
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
