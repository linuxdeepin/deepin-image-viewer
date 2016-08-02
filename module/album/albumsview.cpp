#include "albumsview.h"
#include "albumdelegate.h"
#include "application.h"
#include "controller/popupmenumanager.h"
#include "controller/exporter.h"
#include "controller/importer.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
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
      m_itemSize(ITEM_DEFAULT_SIZE),
      m_popupMenu(new PopupMenuManager(this))
{
    setMouseTracking(true);
    AlbumDelegate *delegate = new AlbumDelegate(this);
    connect(delegate, &AlbumDelegate::editingFinished,
            this, [=](const QModelIndex &index) {
        closePersistentEditor(index);
        emit albumCreated();
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
    // Aways has Favorites and RecentImport album
    dApp->databaseM->insertImageIntoAlbum(MY_FAVORITES_ALBUM, "", "");
    dApp->databaseM->insertImageIntoAlbum(RECENT_IMPORTED_ALBUM, "", "");

    connect(this, &AlbumsView::doubleClicked,
            this, &AlbumsView::onDoubleClicked);
    connect(this, &AlbumsView::customContextMenuRequested,
            this, [=] (const QPoint &pos) {
        m_popupMenu->setMenuContent(createMenuContent(indexAt(pos)));
        m_popupMenu->showMenu();
    });
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &AlbumsView::onMenuItemClicked);
    connect(Importer::instance(), &Importer::importProgressChanged,
            this, [=] (double progress) {
        if (progress == 1)
            updateView();
    });
}

QModelIndex AlbumsView::addAlbum(const DatabaseManager::AlbumInfo &info)
{
    // AlbumName ImageCount BeginTime EndTime Thumbnail
    QStringList imgNames = dApp->databaseM->getImageNamesByAlbum(info.name);
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
    auto imgInfo = dApp->databaseM->getImageInfoByName(imageName);
    if (! imageName.isEmpty() &&  ! imgInfo.thumbnail.save( &inBuffer, "JPG" )){
        QPixmap p = utils::image::getThumbnail(imgInfo.path);
        if (! p.save(&inBuffer, "JPG")) {
            qWarning() << "Can't get thumbnail for album: " << info.name;
        }
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
    m_model->setData(index, QVariant(m_itemSize), Qt::SizeHintRole);

    return index;
}

QSize AlbumsView::itemSize() const
{
    return m_itemSize;
}

void AlbumsView::setItemSizeMultiple(int multiple)
{
    const int w = ITEM_DEFAULT_SIZE.width() + multiple * 32;
    const int h = (1.0 *ITEM_DEFAULT_SIZE.height() / ITEM_DEFAULT_SIZE.width()) * w;
    m_itemSize = QSize(w, h);
    updateView();
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

const QStringList AlbumsView::paths(const QString &album) const
{
    const auto infos = dApp->databaseM->getImageInfosByAlbum(album);
    if (! infos.isEmpty()) {
        QStringList list;
        for (auto info : infos) {
            list << info.path;
        }
        return list;
    }
    return QStringList();
}

const QString AlbumsView::getAlbumName(const QModelIndex &index) const
{
    QString albumName = "";
    //TODO: if index isn't valid app will crashed,
    //index need be valid to access.
    if (!index.isValid())
        return "";

    QList<QVariant> datas =
            index.model()->data(index, Qt::DisplayRole).toList();
    if (! datas.isEmpty()) {
        albumName = datas[0].toString();
    }

    return albumName;
}

const QString AlbumsView::getNewAlbumName() const
{
    const QString nan = tr("Unnamed");
    const QStringList albums = dApp->databaseM->getAlbumNameList();
    QList<int> countList;
    for (QString album : albums) {
        if (album.startsWith(nan)) {
            countList << QString(album.split(nan).last()).toInt();
        }
    }

    if (countList.isEmpty()) {
        return nan;
    }
    else if (countList.length() == 1) {
        return nan + QString::number(2);
    }
    else {
        qSort(countList.begin(), countList.end());
        if (countList.first() != 0)
            return nan;
        for (int c : countList) {
            if (c == 0) {
                // Index start from 2.
                c = 1;
            }
            if (countList.indexOf(c + 1) == -1) {
                return nan + QString::number(c + 1);
            }
        }

        return nan;
    }
}

const QString AlbumsView::createMenuContent(const QModelIndex &index)
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
            items.append(createMenuItem(IdRename, tr("Rename"), false, "F2"));
        items.append(createMenuItem(IdExport, tr("Export")));
        items.append(createMenuItem(IdCopy, tr("Copy"), false, "Ctrl+C"));
        if (! isSpecial)
            items.append(createMenuItem(IdDelete, tr("Delete"), false,
                                        "Delete"));
        items.append(createMenuItem(IdSeparator, "", true));
        //        items.append(createMenuItem(IdAlbumInfo, tr("Album info"), false,
        //                                    "Ctrl+Alt+Return"));
    }
    else {
        items.append(createMenuItem(IdCreate, tr("New album")));
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
        emit startSlideShow(paths(albumName));
        break;
    case IdRename:
        openPersistentEditor(this->currentIndex());
        break;
    case IdExport:
        dApp->exporter->exportAlbum(albumName);
        break;
    case IdCopy:
    {
        const auto infos = dApp->databaseM->getImageInfosByAlbum(albumName);
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
            dApp->databaseM->removeAlbum(albumName);
            m_model->removeRow(currentIndex().row());
            emit albumRemoved();
        }
        break;
        //    case IdAlbumInfo:
        //        break;
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
    dApp->databaseM->insertImageIntoAlbum(name, "", "");
    QModelIndex index = addAlbum(dApp->databaseM->getAlbumInfo(name));
    openPersistentEditor(index);
    scrollTo(index);
}

void AlbumsView::updateView()
{
    if (! isVisible())
        return;

    m_model->clear();

    // Make those special album always show at front
    addAlbum(dApp->databaseM->getAlbumInfo(MY_FAVORITES_ALBUM));
    addAlbum(dApp->databaseM->getAlbumInfo(RECENT_IMPORTED_ALBUM));
    QStringList albums = dApp->databaseM->getAlbumNameList();
    albums.removeAll(MY_FAVORITES_ALBUM);
    albums.removeAll(RECENT_IMPORTED_ALBUM);
    for (const QString name : albums) {
        addAlbum(dApp->databaseM->getAlbumInfo(name));
    }
}
