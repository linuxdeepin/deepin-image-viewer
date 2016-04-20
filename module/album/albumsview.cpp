#include "albumsview.h"
#include "albumdelegate.h"
#include <QDebug>
#include <QBuffer>

namespace {

const int ITEM_SPACING = 58;
const QSize ITEM_DEFAULT_SIZE = QSize(152, 168);

}  // namespace

AlbumsView::AlbumsView(QWidget *parent)
    : QListView(parent)
{
    setItemDelegate(new AlbumDelegate(this));
    m_itemModel = new QStandardItemModel(this);
    setModel(m_itemModel);

    setMovement(QListView::Free);
    setFrameStyle(QFrame::NoFrame);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setFlow(QListView::LeftToRight);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setUniformItemSizes(true);
    setSpacing(ITEM_SPACING);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragEnabled(false);
}

void AlbumsView::addAlbum(const DatabaseManager::AlbumInfo &info)
{
    // AlbumName ImageCount EarliestTime LatestTime Thumbnail
    QList<DatabaseManager::ImageInfo> imgInfos =
            DatabaseManager::instance()->getImageInfosByAlbum(info.name);
    if (imgInfos.isEmpty()) {
        return;
    }
    QByteArray thumbnailByteArray;
    QBuffer inBuffer( &thumbnailByteArray );
    inBuffer.open( QIODevice::WriteOnly );
    // write inPixmap into inByteArray
    if ( ! imgInfos.first().thumbnail.save( &inBuffer, "JPG" )) {
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
    m_itemModel->appendColumn(items);

    QModelIndex index = m_itemModel->index(0, m_itemModel->columnCount() - 1, QModelIndex());
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
