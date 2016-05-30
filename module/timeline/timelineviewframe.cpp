#include "timelineviewframe.h"
#include "controller/popupmenumanager.h"
#include "controller/wallpapersetter.h"
#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QResizeEvent>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QJsonArray>
#include <QJsonDocument>

namespace {

const int THUMBNAIL_MAX_SCALE_SIZE = 384;
const QString FAVORITES_ALBUM_NAME = "My favorites";
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";

}  //namespace

TimelineViewFrame::TimelineViewFrame(const QString &timeline,
                                     bool multiselection,
                                     QWidget *parent)
    : QFrame(parent),
      m_multiselection(multiselection),
      m_iconSize(96, 96),
      m_timeline(timeline),
      m_popupMenu(new PopupMenuManager(parent)),
      m_dbManager(DatabaseManager::instance()),
      m_sManager(SignalManager::instance())
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

    updateMenuContents();
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &TimelineViewFrame::onMenuItemClicked);
}

void TimelineViewFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    m_listView->setFixedWidth(e->size().width());
}

void TimelineViewFrame::initListView()
{
    m_listView = new ThumbnailListView();
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listView->setIconSize(m_iconSize);
    m_listView->setModel( &m_standardModel );
    if (m_multiselection) {
        m_listView->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    else {
        m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    connect(m_listView, &ThumbnailListView::clicked,
            this, &TimelineViewFrame::clicked);
    connect(m_listView, &ThumbnailListView::doubleClicked,
            this, [=] (const QModelIndex & index) {
        emit m_sManager->viewImage(index.data(Qt::UserRole).toString());
    });
    connect(m_listView, &ThumbnailListView::customContextMenuRequested,
            [this] (const QPoint &pos) {
        if (m_listView->indexAt(pos).isValid()) {
            m_popupMenu->setMenuContent(createMenuContent());
            m_popupMenu->showMenu();
        }
    });

    //add data
    QList<DatabaseManager::ImageInfo> list
            = DatabaseManager::instance()->getImageInfosByTime(
                utils::base::stringToDateTime(m_timeline));
    for (DatabaseManager::ImageInfo info : list) {
        insertItem(info);
    }
}

DatabaseManager::ImageInfo TimelineViewFrame::imageInfo(const QString &name)
{
    return m_dbManager->getImageInfoByName(name);
}

QString TimelineViewFrame::currentSelectOne(bool isPath)
{
    const QStringList nl = selectedImagesNameList();
    const QStringList pl = selectedImagesPathList();

    if (isPath) {
        if (pl.isEmpty())
            return QString();
        else
            return pl.first();
    }
    else {
        if (nl.isEmpty())
            return QString();
        else
            return nl.first();
    }
}

QPixmap TimelineViewFrame::generateSelectedThumanail(const QPixmap &pixmap)
{
    if (m_multiselection) {
        QPixmap target = pixmap;
        QPainter painter(&target);
        QPixmap icon(":/images/resources/images/item_selected.png");
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

QString TimelineViewFrame::createMenuContent()
{
    QJsonArray items;
    items.append(createMenuItem(IdView, tr("View")));
    items.append(createMenuItem(IdFullScreen, tr("Fullscreen"),
                                false, "F11"));
    items.append(createMenuItem(IdStartSlideShow, tr("Start slide show"), false,
                                "F5"));
    const QJsonObject objF = createAlbumMenuObj();
    if (! objF.isEmpty()) {
        items.append(createMenuItem(IdAddToAlbum, tr("Add to album"),
                                    false, "", objF));
    }

    items.append(createMenuItem(IdSeparator, "", true));

    items.append(createMenuItem(IdExport, tr("Export"), false, ""));
    items.append(createMenuItem(IdCopy, tr("Copy"), false, "Ctrl+C"));
    items.append(createMenuItem(IdDelete, tr("Delete"), false, "Delete"));

    items.append(createMenuItem(IdSeparator, "", true));
    items.append(createMenuItem(IdEdit, tr("Edit"), false, "Ctrl+E"));
    if (! m_dbManager->imageExistAlbum(currentSelectOne(false),
                                       FAVORITES_ALBUM_NAME)) {
        items.append(createMenuItem(IdAddToFavorites, tr("Add to favorites"),
                                    false, "Ctrl+K"));
    } else {
        items.append(createMenuItem(IdRemoveFromFavorites,
            tr("Remove from favorites"), false, "Ctrl+Shift+K"));
    }
    items.append(createMenuItem(IdSeparator, "", true));

    items.append(createMenuItem(IdRotateClockwise, tr("Rotate clockwise"),
                                false, "Ctrl+R"));
    items.append(createMenuItem(IdRotateCounterclockwise,
        tr("Rotate counterclockwise"), false, "Ctrl+Shift+R"));

    items.append(createMenuItem(IdSeparator, "", true));

    items.append(createMenuItem(IdLabel, tr("Text tag")));
    items.append(createMenuItem(IdSetAsWallpaper, tr("Set as wallpaper"), false,
                                "Ctrl+F8"));
    items.append(createMenuItem(IdDisplayInFileManager,
        tr("Display in file manager"), false, "Ctrl+D"));
    items.append(createMenuItem(IdImageInfo, tr("Image info"), false,
                                "Alt+Enter"));

    QJsonObject contentObj;
    contentObj["x"] = 0;
    contentObj["y"] = 0;
    contentObj["items"] = QJsonValue(items);

    return QString(QJsonDocument(contentObj).toJson());
}

QJsonValue TimelineViewFrame::createMenuItem(const MenuItemId id,
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

QJsonObject TimelineViewFrame::createAlbumMenuObj()
{
    const QStringList albums = m_dbManager->getAlbumNameList();
    const QStringList selectNames = selectedImagesNameList();

    QJsonArray items;
    if (! selectNames.isEmpty()) {
        for (QString album : albums) {
            if (album == FAVORITES_ALBUM_NAME || album == "Recent imported") {
                continue;
            }
            const QStringList names = m_dbManager->getImageNamesByAlbum(album);
            if (names.indexOf(selectNames.first()) == -1) {
                items.append(createMenuItem(IdAddToAlbum, album));
            }
        }
    }

    QJsonObject contentObj;
    if (! items.isEmpty()) {
        contentObj[""] = QJsonValue(items);
    }

    return contentObj;

}

void TimelineViewFrame::updateThumbnail(const QString &name)
{
    for (int i = 0; i < m_standardModel.rowCount(); i ++) {
        if (m_standardModel.item(i, 0)->toolTip() == name) {
            DatabaseManager::ImageInfo info =
                    m_dbManager->getImageInfoByName(name);
            const QPixmap p = utils::image::getThumbnail(info.path);
            info.thumbnail = p;
            m_dbManager->updateImageInfo(info);

            QIcon icon;
            QPixmap thumbnail = increaseThumbnail(p);
            icon.addPixmap(thumbnail, QIcon::Normal);
            icon.addPixmap(generateSelectedThumanail(thumbnail), QIcon::Selected);
            m_standardModel.item(i, 0)->setIcon(icon);
            return;
        }
    }
}

void TimelineViewFrame::updateMenuContents()
{
    // For update shortcut
    m_popupMenu->setMenuContent(createMenuContent());
}

void TimelineViewFrame::onMenuItemClicked(int menuId, const QString &text)
{
    if (selectedImagesPathList().isEmpty()) {
        return;
    }

    const QString cname = currentSelectOne(false);
    const QString cpath = currentSelectOne(true);
    const DatabaseManager::ImageInfo info = m_dbManager->getImageInfoByName(cname);
    switch (MenuItemId(menuId)) {
    case IdView:
        m_sManager->viewImage(cpath);
        break;
    case IdFullScreen:
        m_sManager->viewImage(cpath);
        m_sManager->fullScreen(cpath);
        break;
    case IdStartSlideShow:
        m_sManager->viewImage(cpath);
        m_sManager->startSlideShow(cpath);
        break;
    case IdAddToAlbum:
        m_dbManager->insertImageIntoAlbum(text.split(SHORTCUT_SPLIT_FLAG).first(),
            cname, utils::base::timeToString(imageInfo(cname).time));
        break;
    case IdCopy:
        utils::base::copyImageToClipboard(cpath);
        break;
    case IdDelete:
        m_dbManager->removeImage(cname);
        break;
    case IdEdit:
        m_sManager->editImage(cpath);
        break;
    case IdAddToFavorites:
        m_dbManager->insertImageIntoAlbum(FAVORITES_ALBUM_NAME, info.name,
                                          utils::base::timeToString(info.time));
        updateMenuContents();
        break;
    case IdRemoveFromFavorites:
        m_dbManager->removeImageFromAlbum(FAVORITES_ALBUM_NAME, info.name);
        updateMenuContents();
        break;
    case IdRotateClockwise:
        utils::image::rotate(cpath, 90);
        updateThumbnail(cname);
        break;
    case IdRotateCounterclockwise:
        utils::image::rotate(cpath, -90);
        updateThumbnail(cname);
        break;
    case IdLabel:
        break;
    case IdSetAsWallpaper:
        WallpaperSetter::instance()->setWallpaper(cpath);
        break;
    case IdDisplayInFileManager:
        utils::base::showInFileManager(cpath);
        break;
    case IdImageInfo:
        break;
    default:
        break;
    }
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

bool TimelineViewFrame::removeItem(const QString &name)
{
    for (int i = 0; i < m_standardModel.rowCount(); i ++) {
        if (m_standardModel.item(i, 0)->toolTip() == name) {
            m_standardModel.removeRow(i);
            return true;
        }
    }

    return false;
}

void TimelineViewFrame::clearSelection() const
{
    m_listView->selectionModel()->clearSelection();
}

QStringList TimelineViewFrame::selectedImagesNameList()
{
    QStringList names;
    for (QModelIndex index : m_listView->selectionModel()->selectedIndexes()) {
        QString path = index.data(Qt::UserRole).toString();
        names << QFileInfo(path).fileName();
    }

    return names;
}

QStringList TimelineViewFrame::selectedImagesPathList()
{
    QStringList paths;
    for (QModelIndex index : m_listView->selectionModel()->selectedIndexes()) {
        paths << index.data(Qt::UserRole).toString();
    }

    return paths;
}

QString TimelineViewFrame::timeline() const
{
    return m_timeline;
}

bool TimelineViewFrame::isEmpty() const
{
    return m_standardModel.rowCount() == 0;
}

bool TimelineViewFrame::contain(const QModelIndex &index) const
{
    return index.model() == &m_standardModel;
}

QSize TimelineViewFrame::viewSize() const
{
    return m_listView->childrenRect().size();
}
