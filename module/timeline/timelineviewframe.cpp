#include "timelineviewframe.h"
#include <QResizeEvent>
#include <QDateTime>
#include <QDebug>

TimelineViewFrame::TimelineViewFrame(const QString &timeline, QWidget *parent)
    : QFrame(parent), m_timeline(timeline)
{
    QLabel *title = new QLabel(timeline);
    title->setObjectName("TimelineFrameTitle");

    initListView();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(title);
    layout->addWidget(m_listView);
}

void TimelineViewFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    m_listView->setFixedWidth(e->size().width());
}

void TimelineViewFrame::initListView()
{
    m_listView = new ThumbnailListView();
    m_listView->setIconSize(QSize(96, 96));
    m_listView->setModel( &m_standardModel );
    connect(m_listView, &ThumbnailListView::doubleClicked, this, [=] (const QModelIndex & index) {
        emit SignalManager::instance()->viewImage(index.data(Qt::UserRole).toString());
    });

    //add data
    QList<DatabaseManager::ImageInfo> list
            = DatabaseManager::instance()->getImageInfoByTime(
                QDateTime::fromString(m_timeline, DATETIME_FORMAT));
    for (DatabaseManager::ImageInfo info : list) {
        insertItem(info);
    }
}

void TimelineViewFrame::insertItem(const DatabaseManager::ImageInfo &info)
{
    QStandardItem *item = new QStandardItem();
    item->setData(info.path, Qt::UserRole);
    QIcon icon;
    icon.addPixmap(info.thumbnail, QIcon::Normal);
    icon.addPixmap(info.thumbnail, QIcon::Selected);
    item->setIcon(icon);
    item->setToolTip(info.name);

    m_standardModel.setItem(m_standardModel.rowCount(), 0, item);
//    m_standardModel.setData();
}

void TimelineViewFrame::removeItem(const QString &name)
{
    Q_UNUSED(name)
}
