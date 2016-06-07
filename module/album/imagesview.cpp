#include "imagesview.h"
#include "widgets/thumbnaillistview.h"
#include "controller/databasemanager.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include "controller/wallpapersetter.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QFileInfo>
#include <QStandardItem>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";

}  // namespace

ImagesView::ImagesView(QWidget *parent)
    : QScrollArea(parent),
      m_popupMenu(new PopupMenuManager(this)),
      m_dbManager(DatabaseManager::instance()),
      m_sManager(SignalManager::instance())
{
    initContent();
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    updateMenuContents();
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &ImagesView::onMenuItemClicked);
}

void ImagesView::setAlbum(const QString &album)
{
    m_model.clear();
    QList<DatabaseManager::ImageInfo> infos
            = m_dbManager->getImageInfosByAlbum(album);
    for (DatabaseManager::ImageInfo info : infos) {
        QStandardItem *item = new QStandardItem();
        item->setData(info.path, Qt::UserRole);
        QIcon icon;
        QPixmap thumbnail = m_view->increaseThumbnail(info.thumbnail);
        icon.addPixmap(thumbnail, QIcon::Normal);
        item->setIcon(icon);
        item->setToolTip(info.name);

        m_model.setItem(m_model.rowCount(), 0, item);
    }

    m_topTips->setAlbum(album);
    m_currentAlbum = album;
}

void ImagesView::updateView()
{
    setAlbum(m_currentAlbum);
}

void ImagesView::initContent()
{
    m_contentWidget = new QWidget();
    m_contentWidget->setObjectName("ImagesViewContent");
    m_contentLayout = new QVBoxLayout(m_contentWidget);

    setWidget(m_contentWidget);

    initListView();
    initTopTips();
}

void ImagesView::initListView()
{
    m_view = new ThumbnailListView(this);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setIconSize(QSize(96, 96));
    m_view->setModel( &m_model );
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_contentLayout->addWidget(m_view);

    connect(m_view, &ThumbnailListView::doubleClicked, this, [=] (const QModelIndex & index) {
        emit m_sManager->viewImage(index.data(Qt::UserRole).toString(), m_currentAlbum);
    });
    connect(m_view, &ThumbnailListView::customContextMenuRequested, [this] {
        m_popupMenu->setMenuContent(createMenuContent());
        m_popupMenu->showMenu();
    });
}

void ImagesView::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Control) {
        m_view->setSelectionMode(QAbstractItemView::MultiSelection);
    }
}

void ImagesView::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Control) {
        m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    }
}

void ImagesView::initTopTips()
{
    m_topTips = new TopAlbumTips(this);
}

QString ImagesView::currentSelectOne(bool isPath)
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

QString ImagesView::createMenuContent()
{
    QJsonArray items;
    items.append(createMenuItem(IdView, tr("View")));
    items.append(createMenuItem(IdFullScreen, tr("Fullscreen"),
                                false, "F11"));
    items.append(createMenuItem(IdStartSlideShow, tr("Start slide show"), false,
                                "F5"));

    items.append(createMenuItem(IdSeparator, "", true));

    items.append(createMenuItem(IdExport, tr("Export"), false, ""));
    items.append(createMenuItem(IdCopy, tr("Copy"), false, "Ctrl+C"));
    items.append(createMenuItem(IdDelete, tr("Delete"), false, "Delete"));

    items.append(createMenuItem(IdSeparator, "", true));
    items.append(createMenuItem(IdEdit, tr("Edit"), false, "Ctrl+E"));
    items.append(createMenuItem(IdSeparator, "", true));

    items.append(createMenuItem(IdRotateClockwise, tr("Rotate clockwise"),
                                false, "Ctrl+R"));
    items.append(createMenuItem(IdRotateCounterclockwise,
        tr("Rotate counterclockwise"), false, "Ctrl+Shift+R"));

    items.append(createMenuItem(IdSeparator, "", true));

//    items.append(createMenuItem(IdLabel, tr("Text tag")));
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

QJsonValue ImagesView::createMenuItem(const MenuItemId id,
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

void ImagesView::updateThumbnail(const QString &name)
{
    for (int i = 0; i < m_model.rowCount(); i ++) {
        if (m_model.item(i, 0)->toolTip() == name) {
            DatabaseManager::ImageInfo info =
                    m_dbManager->getImageInfoByName(name);
            const QPixmap p = utils::image::getThumbnail(info.path);
            info.thumbnail = p;
            m_dbManager->updateImageInfo(info);

            QIcon icon;
            QPixmap thumbnail = m_view->increaseThumbnail(p);
            icon.addPixmap(thumbnail, QIcon::Normal);
            m_model.item(i, 0)->setIcon(icon);
            return;
        }
    }
}

void ImagesView::updateMenuContents()
{
    m_popupMenu->setMenuContent(createMenuContent());
}

void ImagesView::onMenuItemClicked(int menuId)
{
    if (currentSelectOne().isEmpty()) {
        return;
    }

    const QString cname = currentSelectOne(false);
    const QString cpath = currentSelectOne(true);
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
    case IdCopy:
        utils::base::copyImageToClipboard(QStringList(cpath));
        break;
    case IdDelete:
        m_dbManager->removeImage(cname);
        break;
    case IdEdit:
        m_sManager->editImage(cpath);
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

void ImagesView::updateContentRect()
{
    int minWidth = getMinContentsWidth();
    int hMargin = (width() - minWidth) / 2;
    m_contentLayout->setContentsMargins(hMargin, 60, hMargin, 10);
    m_contentWidget->setFixedWidth(width());
    m_view->setFixedWidth(width());
}

void ImagesView::updateTopTipsRect()
{
    m_topTips->move(0, TOP_TOOLBAR_HEIGHT);
    m_topTips->resize(width(), m_topTips->height());
}

int ImagesView::getMinContentsWidth()
{
    int itemSpacing = 10;
    int viewHMargin = 14 * 2;
    int holdCount = floor((double)(width() - itemSpacing - viewHMargin)
                          / (iconSize().width() + itemSpacing));
    return (iconSize().width() + itemSpacing) * holdCount + itemSpacing + viewHMargin;
}

QString ImagesView::getCurrentAlbum() const
{
    return m_currentAlbum;
}

QSize ImagesView::iconSize() const
{
    return m_view->iconSize();
}

void ImagesView::setIconSize(const QSize &iconSize)
{
    m_view->setIconSize(iconSize);
}

QStringList ImagesView::selectedImagesNameList() const
{
    QStringList names;
    for (QModelIndex index : m_view->selectionModel()->selectedIndexes()) {
        QString path = index.data(Qt::UserRole).toString();
        names << QFileInfo(path).fileName();
    }

    return names;
}

QStringList ImagesView::selectedImagesPathList() const
{
    QStringList paths;
    for (QModelIndex index : m_view->selectionModel()->selectedIndexes()) {
        paths << index.data(Qt::UserRole).toString();
    }

    return paths;
}

void ImagesView::resizeEvent(QResizeEvent *e)
{
    QScrollArea::resizeEvent(e);
    updateContentRect();
    updateTopTipsRect();
}
