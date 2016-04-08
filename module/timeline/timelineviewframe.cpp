#include "timelineviewframe.h"
#include <QResizeEvent>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QPainter>

const int THUMBNAIL_MAX_SCALE_SIZE = 384;

TimelineViewFrame::TimelineViewFrame(const QString &timeline, bool multiselection, QWidget *parent)
    : QFrame(parent), m_multiselection(multiselection), m_iconSize(96, 96), m_timeline(timeline)
{
    QLabel *title = new QLabel(timeline);
    title->setObjectName("TimelineFrameTitle");
    QLabel *separator = new QLabel();
    separator->setObjectName("TimelineSeparator");
    separator->setFixedHeight(1);

    initListView();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(title);
    layout->addWidget(m_listView);
    layout->addWidget(separator);
}

void TimelineViewFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    m_listView->setFixedWidth(e->size().width());
}

void TimelineViewFrame::initListView()
{
    m_listView = new ThumbnailListView();
    m_listView->setIconSize(m_iconSize);
    m_listView->setModel( &m_standardModel );
    if (m_multiselection) {
        m_listView->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    else {
        m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

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

QPixmap TimelineViewFrame::generateSelectedThumanail(const QPixmap &pixmap)
{
    if (m_multiselection) {
        QPixmap target = pixmap;
        QPainter painter(&target);
        QPixmap icon(":/images/icons/resources/images/icons/item-selected.png");
        int selectIconSize = 80;
        painter.drawPixmap((target.width() - selectIconSize) / 2,
                           (target.height() - selectIconSize) / 2,
                           selectIconSize, selectIconSize, icon);

        return target;
    }
    else {
        return pixmap;
    }
}

QPixmap TimelineViewFrame::increaseThumbnail(const QPixmap &pixmap)
{
    QSize targetSize;
    if (pixmap.width() > pixmap.height()) {
        targetSize = QSize(THUMBNAIL_MAX_SCALE_SIZE,
                           (double)THUMBNAIL_MAX_SCALE_SIZE / pixmap.width() *
                           pixmap.height());
    }
    else {
        targetSize = QSize((double)THUMBNAIL_MAX_SCALE_SIZE / pixmap.height() *
                           pixmap.width(), THUMBNAIL_MAX_SCALE_SIZE);
    }
    return pixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QSize TimelineViewFrame::iconSize() const
{
    return m_listView->iconSize();
}

void TimelineViewFrame::setIconSize(const QSize &iconSize)
{
    m_listView->setIconSize(iconSize);
}

void TimelineViewFrame::insertItem(const DatabaseManager::ImageInfo &info)
{
    QStandardItem *item = new QStandardItem();
    item->setData(info.path, Qt::UserRole);
    QIcon icon;
    QPixmap thumbnail = increaseThumbnail(info.thumbnail);
    icon.addPixmap(thumbnail, QIcon::Normal);
    icon.addPixmap(generateSelectedThumanail(thumbnail), QIcon::Selected);
    item->setIcon(icon);
    item->setToolTip(info.name);

    m_standardModel.setItem(m_standardModel.rowCount(), 0, item);
}

void TimelineViewFrame::removeItem(const QString &name)
{
    Q_UNUSED(name)
}

QStringList TimelineViewFrame::selectedImages()
{
    QStringList names;;
    for (QModelIndex index : m_listView->selectionModel()->selectedIndexes()) {
        QString path = index.data(Qt::UserRole).toString();
        names << QFileInfo(path).fileName();
    }

    return names;
}

QString TimelineViewFrame::timeline() const
{
    return m_timeline;
}

QSize TimelineViewFrame::viewSize() const
{
    return m_listView->childrenRect().size();
}
