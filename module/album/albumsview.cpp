#include "albumsview.h"
#include "albumdelegate.h"
#include "controller/signalmanager.h"
#include <QDebug>
#include <QBuffer>

namespace {

const int ITEM_SPACING = 58;
const QSize ITEM_DEFAULT_SIZE = QSize(152, 168);

}  // namespace

AlbumsView::AlbumsView(QWidget *parent)
    : QListView(parent)
{
    setMouseTracking(true);
    setItemDelegate(new AlbumDelegate(this));
    m_itemModel = new QStandardItemModel(this);
    setModel(m_itemModel);

    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setUniformItemSizes(true);
    setSpacing(ITEM_SPACING);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragEnabled(false);

    // Aways has Favorites album
    DatabaseManager::instance()->insertIntoAlbum("Favorites", "", "");

    connect(this, &AlbumsView::doubleClicked, this, &AlbumsView::onDoubleClicked);
}

void AlbumsView::addAlbum(const DatabaseManager::AlbumInfo &info)
{
    // AlbumName ImageCount BeginTime EndTime Thumbnail
    QStringList imgNames =
            DatabaseManager::instance()->getImageNamesByAlbum(info.name);
    if (imgNames.isEmpty()) {
        return;
    }
    QByteArray thumbnailByteArray;
    QBuffer inBuffer( &thumbnailByteArray );
    inBuffer.open( QIODevice::WriteOnly );
    // write inPixmap into inByteArray
    if ( ! DatabaseManager::instance()->getImageInfoByName(imgNames.first())
         .thumbnail.save( &inBuffer, "JPG" )) {
        qDebug() << "Write pixmap to buffer error!" << info.name;
    }

    QVariantList datas;
    datas.append(QVariant(info.name));
    datas.append(QVariant(info.count));
    datas.append(QVariant(info.earliestTime));
    datas.append(QVariant(info.latestTime));
    datas.append(QVariant(thumbnailByteArray));

    QStandardItem *item = new QStandardItem();
    QList<QStandardItem *> items;
    items.append(item);
    m_itemModel->appendRow(items);

    QModelIndex index = m_itemModel->index(m_itemModel->rowCount() - 1, 0, QModelIndex());
//    m_itemModel->setData(index, QVariant(info.name), Qt::EditRole);
    m_itemModel->setData(index, QVariant(datas), Qt::DisplayRole);
    m_itemModel->setData(index, QVariant(ITEM_DEFAULT_SIZE), Qt::SizeHintRole);
}

QSize AlbumsView::itemSize() const
{
    return m_itemSize;
}

void AlbumsView::setItemSize(const QSize &itemSize)
{
    m_itemSize = itemSize;
    for (int column = 0; column < m_itemModel->columnCount(); column ++) {
        QModelIndex index = m_itemModel->index(0, column, QModelIndex());
        m_itemModel->setData(index, QVariant(itemSize), Qt::SizeHintRole);
    }
}

void AlbumsView::onDoubleClicked(const QModelIndex &index)
{
    QString albumName = "";
    QList<QVariant> datas = index.model()->data(index, Qt::DisplayRole).toList();
    if (! datas.isEmpty()) {
        albumName = datas[0].toString();
    }

    emit openAlbum(albumName);
}
