#include "albumsview.h"
#include "albumdelegate.h"
#include "controller/signalmanager.h"
#include "controller/popupmenumanager.h"
#include "controller/exporter.h"
#include "controller/importer.h"
#include "utils/baseutils.h"
#include <QDebug>
#include <QBuffer>
#include <QJsonDocument>
#include <QMouseEvent>

namespace {

const QString MY_FAVORITES_ALBUM = "My favorites";
const QString RECENT_IMPORTED_ALBUM = "Recent imported";
const int ITEM_SPACING = 58;
const QSize ITEM_DEFAULT_SIZE = QSize(152, 168);

}  // namespace

AlbumsView::AlbumsView(QWidget *parent)
    : QListView(parent),
      m_dbManager(DatabaseManager::instance()),
      m_sManager(SignalManager::instance()),
      m_popupMenu(new PopupMenuManager(this))
{
    setMouseTracking(true);
    AlbumDelegate *delegate = new AlbumDelegate(this);
    connect(delegate, &AlbumDelegate::editingFinished,
            this, [=](const QModelIndex &index) {
        closePersistentEditor(index);
    });
    setItemDelegate(delegate);
    m_model = new QStandardItemModel(this);
    setModel(m_model);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setUniformItemSizes(true);
    setSpacing(ITEM_SPACING);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragEnabled(false);

    installEventFilter(this);
    // Aways has Favorites album
    m_dbManager->insertImageIntoAlbum(MY_FAVORITES_ALBUM, "", "");
    m_dbManager->insertImageIntoAlbum(RECENT_IMPORTED_ALBUM, "", "");

    connect(this, &AlbumsView::doubleClicked,
            this, &AlbumsView::onDoubleClicked);
    connect(this, &AlbumsView::customContextMenuRequested,
            this, [=] (const QPoint &pos) {
        m_popupMenu->setMenuContent(createMenuContent(indexAt(pos)));
        m_popupMenu->showMenu();
    });
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &AlbumsView::onMenuItemClicked);
    connect(m_sManager, &SignalManager::imageCountChanged,
            this, &AlbumsView::updateView);
}

QModelIndex AlbumsView::addAlbum(const DatabaseManager::AlbumInfo &info)
{
    // AlbumName ImageCount BeginTime EndTime Thumbnail
    QStringList imgNames = m_dbManager->getImageNamesByAlbum(info.name);
    if (imgNames.isEmpty()) {
        return QModelIndex();
    }
    QByteArray thumbnailByteArray;
    QBuffer inBuffer( &thumbnailByteArray );
    inBuffer.open( QIODevice::WriteOnly );
    // write inPixmap into inByteArray
    QString imageName = imgNames.first();
    if (imageName.isEmpty()) {
        for (QString name : imgNames) {
            if (! name.isEmpty()) {
                imageName = name;
                break;
            }
        }
    }
    if ( ! m_dbManager->getImageInfoByName(imageName)
         .thumbnail.save( &inBuffer, "JPG" )) {
        qDebug() << "Write pixmap to buffer error!" << info.name << imageName;
    }

    QVariantList datas;
    datas.append(QVariant(info.name));
    datas.append(QVariant(info.count));
    datas.append(QVariant(info.beginTime));
    datas.append(QVariant(info.endTime));
    datas.append(QVariant(thumbnailByteArray));

    QStandardItem *item = new QStandardItem();
    QList<QStandardItem *> items;
    items.append(item);
    m_model->appendRow(items);

    QModelIndex index = m_model->index(m_model->rowCount() - 1, 0);
    //    m_itemModel->setData(index, QVariant(info.name), Qt::EditRole);
    m_model->setData(index, QVariant(datas), Qt::DisplayRole);
    m_model->setData(index, QVariant(ITEM_DEFAULT_SIZE), Qt::SizeHintRole);

    return index;
}

QSize AlbumsView::itemSize() const
{
    return m_itemSize;
}

void AlbumsView::setItemSize(const QSize &itemSize)
{
    m_itemSize = itemSize;
    for (int column = 0; column < m_model->columnCount(); column ++) {
        QModelIndex index = m_model->index(0, column, QModelIndex());
        m_model->setData(index, QVariant(itemSize), Qt::SizeHintRole);
    }
}

bool AlbumsView::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    if (e->type() == QEvent::Hide) {
        m_model->clear();
    }
    else if (e->type() == QEvent::Show) {
        updateView();
    }
    return false;
}

void AlbumsView::mousePressEvent(QMouseEvent *e)
{
    if (! indexAt(e->pos()).isValid()) {
        this->selectionModel()->clearSelection();
    }
    else {
        m_popupMenu->setMenuContent(createMenuContent(indexAt(e->pos())));
    }

    QListView::mousePressEvent(e);
}

QString AlbumsView::getAlbumName(const QModelIndex &index)
{
    QString albumName = "";
    QList<QVariant> datas = index.model()->data(index, Qt::DisplayRole).toList();
    if (! datas.isEmpty()) {
        albumName = datas[0].toString();
    }

    return albumName;
}

QString AlbumsView::getNewAlbumName() const
{
    const QString nan = "New Album";
    const QStringList albums = m_dbManager->getAlbumNameList();
    QStringList tmpList;
    for (QString album : albums) {
        if (album.startsWith("New Album")) {
            tmpList << album;
        }
    }

    if (tmpList.isEmpty()) {
        return nan;
    }
    else if (tmpList.length() == 1) {
        return nan + QString::number(1);
    }
    else {
        qSort(tmpList.begin(), tmpList.end());
        for (int i = tmpList.length() - 1; i > 0; i ++) {
            const int count
                    = QString(QString(tmpList.at(i)).split(nan).last()).toInt();
            if (count >= 0) {
                return nan + QString::number(i + 1);
            }
        }

        return nan;
    }
}

QString AlbumsView::createMenuContent(const QModelIndex &index)
{
    QJsonArray items;
    if (index.isValid()) {
        bool isSpecial = false;
        QList<QVariant> datas =
                index.model()->data(index, Qt::DisplayRole).toList();
        if (! datas.isEmpty()) {
            const QString albumName = datas[0].toString();
            if (albumName == MY_FAVORITES_ALBUM
                    || albumName == RECENT_IMPORTED_ALBUM) {
                isSpecial = true;
            }
        }

        items.append(createMenuItem(IdView, tr("View")));
        items.append(createMenuItem(IdStartSlideShow, tr("Start slide show"),
                                    false, "F5"));
        items.append(createMenuItem(IdSeparator, "", true));
        if (! isSpecial)
            items.append(createMenuItem(IdRename, tr("Rename")));
        items.append(createMenuItem(IdExport, tr("Export")));
        items.append(createMenuItem(IdCopy, tr("Copy"), false, "Ctrl+C"));
        if (! isSpecial)
            items.append(createMenuItem(IdDelete, tr("Delete"), false,
                                        "Delete"));
        items.append(createMenuItem(IdSeparator, "", true));
        items.append(createMenuItem(IdAlbumInfo, tr("Album info"), false,
                                    "Ctrl+Alt+Enter"));
    }
    else {
        items.append(createMenuItem(IdCreate, tr("Create Album")));
    }

    QJsonObject contentObj;
    contentObj["x"] = 0;
    contentObj["y"] = 0;
    contentObj["items"] = QJsonValue(items);

    QJsonDocument document(contentObj);

    return QString(document.toJson());
}

QJsonValue AlbumsView::createMenuItem(const MenuItemId id,
                                      const QString &text,
                                      const bool isSeparator,
                                      const QString &shortcut,
                                      const QJsonObject &subMenu)
{
    return QJsonValue(m_popupMenu->createItemObj(id,
                                                 text,
                                                 isSeparator,
                                                 shortcut,
                                                 subMenu));
}

void AlbumsView::onMenuItemClicked(int menuId)
{
    const QString albumName = getAlbumName(currentIndex());
    switch (MenuItemId(menuId)) {
    case IdCreate:
        createAlbum();
        break;
    case IdView:
        emit openAlbum(albumName);
        break;
    case IdStartSlideShow:
    {
        const QList<DatabaseManager::ImageInfo> infos =
                m_dbManager->getImageInfosByAlbum(albumName);
        if (! infos.isEmpty()) {
            emit m_sManager->viewImage(infos.first().path);
            emit m_sManager->startSlideShow(infos.first().path);
        }
        break;
    }
    case IdRename:
        openPersistentEditor(this->currentIndex());
        break;
    case IdExport:
        Exporter::instance()->exportAlbum(albumName);
        break;
    case IdCopy:
    {
        const QList<DatabaseManager::ImageInfo> infos =
                m_dbManager->getImageInfosByAlbum(albumName);
        QStringList paths;
        for (int i = 0; i < infos.length(); i ++) {
            paths << infos[i].path;
        }
        utils::base::copyImageToClipboard(paths);
        break;
    }
    case IdDelete:
        if (albumName != MY_FAVORITES_ALBUM
                && albumName != RECENT_IMPORTED_ALBUM) {
            m_dbManager->removeAlbum(albumName);
            m_model->removeRow(currentIndex().row());
        }
        break;
    case IdAlbumInfo:
        break;
    default:
        break;
    }
}

void AlbumsView::onDoubleClicked(const QModelIndex &index)
{
    emit openAlbum(getAlbumName(index));
}

void AlbumsView::createAlbum()
{
    const QString name = getNewAlbumName();
    m_dbManager->insertImageIntoAlbum(name, "", "");
    QModelIndex index = addAlbum(m_dbManager->getAlbumInfo(name));
    openPersistentEditor(index);
}

void AlbumsView::updateView()
{
    // DO NOT update during import
    if (Importer::instance()->getProgress() != 1 || ! isVisible())
        return;

    m_model->clear();

    // Make those special album always show at front
    addAlbum(m_dbManager->getAlbumInfo(MY_FAVORITES_ALBUM));
    addAlbum(m_dbManager->getAlbumInfo(RECENT_IMPORTED_ALBUM));
    QStringList albums = m_dbManager->getAlbumNameList();
    albums.removeAll(MY_FAVORITES_ALBUM);
    albums.removeAll(RECENT_IMPORTED_ALBUM);
    for (const QString name : albums) {
        addAlbum(m_dbManager->getAlbumInfo(name));
    }
}
