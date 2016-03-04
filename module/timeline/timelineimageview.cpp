#include "timelineimageview.h"
#include "timelineviewframe.h"
#include "controller/databasemanager.h"
#include <QPushButton>
#include <QDebug>

TimelineImageView::TimelineImageView(QWidget *parent)
    : QScrollArea(parent)
{
    m_contentFrame = new QFrame;
    m_contentFrame->setStyleSheet("QFrame{background: transparent;border:none;}");
    m_contentFrame->setAutoFillBackground(true);
    m_contentLayout = new QVBoxLayout(m_contentFrame);
    m_contentLayout->setContentsMargins(0, 50, 0, 10);

    setWidget(m_contentFrame);
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QStringList timelines = DatabaseManager::instance()->getTimeLineList();
    for (QString timeline : timelines) {
        inserFrame(timeline);
    }
}

void TimelineImageView::resizeEvent(QResizeEvent *e)
{
    QScrollArea::resizeEvent(e);
    m_contentFrame->setFixedWidth(width());
}

void TimelineImageView::inserFrame(const QString &timeline)
{
    qDebug() << "New Timeline: " << timeline;
    TimelineViewFrame *frame = new TimelineViewFrame(timeline);
    m_contentLayout->addWidget(frame);
}

void TimelineImageView::removeFrame(const QString &tileline)
{

}
