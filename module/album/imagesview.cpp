#include "imagesview.h"
#include "widgets/thumbnaillistview.h"
#include "controller/databasemanager.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include "controller/exporter.h"
#include "controller/wallpapersetter.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "dscrollbar.h"
#include <QFileInfo>
#include <QStandardItem>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <math.h>

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
    setVerticalScrollBar(new Dtk::Widget::DScrollBar());

    initContent();
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    updateMenuContents();
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &ImagesView::onMenuItemClicked);
//    connect(m_sManager, &SignalManager::imageInserted,
//            this, [=] (const DatabaseManager::ImageInfo &info) {
//        if (info.albums.contains(m_album)) {
//            insertItem(info);
//        }
//    });
}

void ImagesView::setAlbum(const QString &album)
{
    m_album = album;

    m_view->clearData();
    QList<DatabaseManager::ImageInfo> infos =
            m_dbManager->getImageInfosByAlbum(album);
    // Load up to 100 images at initialization to accelerate rendering
    const int preloadCount = 100;
    for (int i = 0; i < qMin(infos.length(), preloadCount); i ++) {
        insertItem(infos[i]);
    }

    TIMER_SINGLESHOT(500, {
        if (infos.length() >= preloadCount) {
            for (int i = preloadCount; i < infos.length(); i ++) {
                insertItem(infos[i]);
            }
        }
    }, this, infos);

    m_topTips->setAlbum(album);
    updateTopTipsRect();
}

bool ImagesView::removeItem(const QString &name)
{
    return m_view->removeItem(name);
}

int ImagesView::count() const
{
    return m_view->count();
}

void ImagesView::initContent()
{
    m_contentWidget = new QWidget;
    m_contentWidget->setObjectName("ImagesViewContent");
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(10, 70, 6, 20);

    setWidget(m_contentWidget);

    initListView();
    initTopTips();
}

void ImagesView::initListView()
{
    m_view = new ThumbnailListView();
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contentLayout->addWidget(m_view);
    m_contentLayout->addStretch(1);

    connect(m_view, &ThumbnailListView::doubleClicked,
            this, [=] (const QModelIndex & index) {
        const QString path = m_view->itemInfo(index).path;
        emit m_sManager->viewImage(path, QStringList(), m_album);
    });
    connect(m_view, &ThumbnailListView::singleClicked, this, [=] (QMouseEvent *e) {
        if (e->button() == Qt::RightButton &&
                m_view->indexAt(e->pos()).isValid()) {
            m_popupMenu->setMenuContent(createMenuContent());
            m_popupMenu->showMenu();
        }
    });
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
    items.append(createMenuItem(IdMoveToTrash, tr("Throw to trash")));
    items.append(createMenuItem(IdRemoveFromAlbum, tr("Remove from album"),
                                false, "Delete"));

    items.append(createMenuItem(IdSeparator, "", true));
//    items.append(createMenuItem(IdEdit, tr("Edit"), false, "Ctrl+E"));
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
                                "Alt+Return"));

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

void ImagesView::insertItem(const DatabaseManager::ImageInfo &info)
{
    ThumbnailListView::ItemInfo vi;
    vi.name = info.name;
    vi.path = info.path;
    vi.tickable = false;  // TODO

    m_view->insertItem(vi);
}

void ImagesView::updateThumbnail(const QString &path)
{
    const QString name = QFileInfo(path).fileName();
    m_dbManager->updateThumbnail(name);
    m_view->updateThumbnail(name);
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

    const QStringList nList = selectedImagesNameList();
    const QStringList pList = selectedImagesPathList();
    const QStringList viewPaths = (pList.length() == 1) ? QStringList() : pList;
//    const QString cname = currentSelectOne(false);
    const QString cpath = currentSelectOne(true);
    switch (MenuItemId(menuId)) {
    case IdView:
        m_sManager->viewImage(cpath, viewPaths, m_album);
        break;
    case IdFullScreen:
        m_sManager->viewImage(cpath, viewPaths, m_album);
        m_sManager->fullScreen(cpath);
        break;
    case IdStartSlideShow:
        m_sManager->viewImage(cpath, viewPaths, m_album);
        m_sManager->startSlideShow(cpath);
        break;
    case IdExport:
        Exporter::instance()->exportImage(selectedImagesPathList());
        break;
    case IdCopy:
        utils::base::copyImageToClipboard(pList);
        break;
    case IdMoveToTrash: {
        for (QString path : pList) {
            const QString name = QFileInfo(path).fileName();
            removeItem(name);
            m_dbManager->removeImageFromAlbum(m_album, name);
            m_dbManager->removeImage(name);
            utils::base::trashFile(path);
        }
        break;
    }
    case IdRemoveFromAlbum:
        for (QString name : nList) {
            removeItem(name);
            m_dbManager->removeImageFromAlbum(m_album, name);
        }
        break;
//    case IdEdit:
//        m_sManager->editImage(cpath);
//        break;
    case IdRotateClockwise:
        for (QString path : pList) {
            utils::image::rotate(path, 90);
            updateThumbnail(path);
        }
        break;
    case IdRotateCounterclockwise:
        for (QString path : pList) {
            utils::image::rotate(path, -90);
            updateThumbnail(path);
        }
        break;
//    case IdLabel:
//        break;
    case IdSetAsWallpaper:
        WallpaperSetter::instance()->setWallpaper(cpath);
        break;
    case IdDisplayInFileManager:
        utils::base::showInFileManager(cpath);
        break;
    case IdImageInfo:
        m_sManager->showImageInfo(cpath);
        break;
    default:
        break;
    }
}

void ImagesView::updateTopTipsRect()
{
    m_topTips->move(0, TOP_TOOLBAR_HEIGHT);
    m_topTips->resize(width(), m_topTips->height());
    const int lm = m_view->count() == 0
            ? 0 : - m_view->hOffset() + m_contentLayout->contentsMargins().left();
    m_topTips->setLeftMargin(lm);
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
    return m_album;
}

QSize ImagesView::iconSize() const
{
    return m_view->iconSize();
}

void ImagesView::setIconSize(const QSize &iconSize)
{
    m_view->setIconSize(iconSize);
    updateTopTipsRect();
}

QStringList ImagesView::selectedImagesNameList() const
{
    QStringList names;
    const QList<ThumbnailListView::ItemInfo> infos =
            m_view->selectedItemInfos();
    for (ThumbnailListView::ItemInfo info : infos) {
        names << info.name;
    }

    return names;
}

QStringList ImagesView::selectedImagesPathList() const
{
    QStringList paths;
    const QList<ThumbnailListView::ItemInfo> infos =
            m_view->selectedItemInfos();
    for (ThumbnailListView::ItemInfo info : infos) {
        paths << info.path;
    }

    return paths;
}

void ImagesView::resizeEvent(QResizeEvent *e)
{
    QScrollArea::resizeEvent(e);
    updateTopTipsRect();
}
