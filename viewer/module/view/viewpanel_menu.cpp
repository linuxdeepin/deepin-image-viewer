#include "viewpanel.h"
#include "imagewidget.h"
#include "navigationwidget.h"
#include <controller/exporter.h>
#include <controller/popupmenumanager.h>
#include <controller/wallpapersetter.h>
#include <utils/baseutils.h>
#include <utils/imageutils.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QShortcut>

namespace {

const QString SHORTCUT_SPLIT_FLAG = "@-_-@";
const QString FAVORITES_ALBUM_NAME = "My favorites";

}  // namespace

enum MenuItemId {
    IdFullScreen,
    IdStartSlideShow,
    IdAddToAlbum,
    IdExport,
    IdCopy,
    IdMoveToTrash,
    IdRemoveFromTimeline,
    IdRemoveFromAlbum,
    IdEdit,
    IdAddToFavorites,
    IdRemoveFromFavorites,
    IdShowNavigationWindow,
    IdHideNavigationWindow,
    IdRotateClockwise,
    IdRotateCounterclockwise,
    IdLabel,
    IdSetAsWallpaper,
    IdDisplayInFileManager,
    IdImageInfo,
    IdSubMenu,
    IdSeparator
};


void ViewPanel::initPopupMenu()
{
    m_popupMenu = new PopupMenuManager(this);
    connect(this, &ViewPanel::customContextMenuRequested, this, [=] {
        if (m_infos.isEmpty()) {
            m_popupMenu->setMenuContent("");
            return;
        }
        updateMenuContent();
        m_popupMenu->showMenu();
    });
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &ViewPanel::onMenuItemClicked);
}

void createMI(QJsonArray *items,
                     const MenuItemId id,
                     const QString &text,
                     const QString &shortcut = "",
                     const bool isSeparator = false,
                     const QJsonObject &subMenu = QJsonObject())
{
    auto obj = PopupMenuManager::createItemObj(int(id), text,
                                               isSeparator, shortcut, subMenu);
    items->append(obj);
}

const QString ViewPanel::createMenuContent()
{
    QJsonArray items;

    createMI(&items, IdFullScreen, (window()->isFullScreen() ? tr("Exit fullscreen") : tr("Fullscreen")), "F11");
    createMI(&items, IdStartSlideShow, tr("Start slide show"), "F5");
    if (m_vinfo.inDatabase) {
        const QJsonObject objF = createAlbumMenuObj(false);
        if (! objF.isEmpty()) {
            createMI(&items, IdAddToAlbum, tr("Add to album"), "", false, objF);
        }
    }

    /**************************************************************************/
    createMI(&items, IdSeparator, "", "", true);

    if (m_vinfo.inDatabase) {
        createMI(&items, IdExport, tr("Export"));
    }
    createMI(&items, IdCopy, tr("Copy"), "Ctrl+C");
    createMI(&items, IdMoveToTrash, tr("Throw to Trash"), "Delete");
    if (! m_vinfo.album.isEmpty()) {
        createMI(&items, IdRemoveFromAlbum, tr("Remove from album"), "Shift+Delete");
    }

    /**************************************************************************/
    createMI(&items, IdSeparator, "", "", true);

    if (m_vinfo.inDatabase) {
        if (! dApp->databaseM->imageExistAlbum(m_view->imageName(),
                                               FAVORITES_ALBUM_NAME)) {
            createMI(&items, IdAddToFavorites, tr("Add to My favorites"), "Ctrl+K");
        } else {
            createMI(&items, IdRemoveFromFavorites, tr("Unfavorite"), "Ctrl+Shift+K");
        }
    }

    /**************************************************************************/
    createMI(&items, IdSeparator, "", "", true);

    if (m_view->scaleValue() > 1 && m_nav->isAlwaysHidden()) {
        createMI(&items, IdShowNavigationWindow, tr("Show navigation window"));
    } else if (m_view->scaleValue() > 1 && !m_nav->isAlwaysHidden()) {
        createMI(&items, IdHideNavigationWindow, tr("Hide navigation window"));
    }

    /**************************************************************************/
    createMI(&items, IdSeparator, "", "", true);

    createMI(&items, IdRotateClockwise, tr("Rotate clockwise"), "Ctrl+R");
    createMI(&items, IdRotateCounterclockwise, tr("Rotate counterclockwise"), "Ctrl+Shift+R");

    /**************************************************************************/
    createMI(&items, IdSeparator, "", "", true);

    createMI(&items, IdSetAsWallpaper, tr("Set as wallpaper"), "Ctrl+8");
    if (m_vinfo.inDatabase) {
        createMI(&items, IdDisplayInFileManager, tr("Display in file manager"), "Ctrl+D");
    }
    createMI(&items, IdImageInfo, tr("Image info"), "Alt+Enter");

    QJsonObject contentObj;
    contentObj["x"] = 0;
    contentObj["y"] = 0;
    contentObj["items"] = QJsonValue(items);

    QJsonDocument document(contentObj);

    return QString(document.toJson());
}


const QJsonObject ViewPanel::createAlbumMenuObj(bool isRemove)
{
    if (! m_vinfo.inDatabase) {
        return QJsonObject();
    }
    const QStringList albums = dApp->databaseM->getAlbumNameList();
    const QString name = m_view->imageName();

    QJsonArray items;
    if (! m_infos.isEmpty()) {
        for (QString album : albums) {
            if (album == FAVORITES_ALBUM_NAME || album == "Recent imported") {
                continue;
            }
            const QStringList names = dApp->databaseM->getImageNamesByAlbum(album);
            if (isRemove) {
                if (names.indexOf(name) != -1) {
                    album = tr("Remove from <<%1>>").arg(album);
                    createMI(&items, IdRemoveFromAlbum, album);
                }
            }
            else {
                if (names.indexOf(name) == -1) {
                    createMI(&items, IdAddToAlbum, album);
                }
            }
        }
    }

    QJsonObject contentObj;
    if (! items.isEmpty()) {
        contentObj[""] = QJsonValue(items);
    }

    return contentObj;
}

void ViewPanel::onMenuItemClicked(int menuId, const QString &text)
{
    using namespace utils::base;
    using namespace utils::image;

    const QStringList mtl = text.split(SHORTCUT_SPLIT_FLAG);
    const QString name = m_current->name;
    const QString path = m_current->path;
    const QString time = timeToString(getCreateDateTime(path));
    QString albumName = mtl.isEmpty() ? "" : mtl.first();

    switch (MenuItemId(menuId)) {
    case IdFullScreen:
        toggleFullScreen();
        break;
    case IdStartSlideShow:
        emit dApp->signalM->startSlideShow(this, paths(), path);
        break;
    case IdAddToAlbum:
        dApp->databaseM->insertImageIntoAlbum(albumName, name, time);
        break;
    case IdExport:
        dApp->exporter->exportImage(QStringList() << path);
        break;
    case IdCopy:
        copyImageToClipboard(QStringList(path));
        break;
    case IdMoveToTrash:
        dApp->databaseM->removeImages(QStringList(name));
        trashFile(path);
        removeCurrentImage();
        break;
    case IdRemoveFromAlbum:
        dApp->databaseM->removeImageFromAlbum(m_vinfo.album, name);
        break;
    case IdEdit:
        dApp->signalM->editImage(m_view->imagePath());
        break;
    case IdAddToFavorites:
        dApp->databaseM->insertImageIntoAlbum(FAVORITES_ALBUM_NAME, name, time);
        emit updateCollectButton();
        break;
    case IdRemoveFromFavorites:
        dApp->databaseM->removeImageFromAlbum(FAVORITES_ALBUM_NAME, name);
        emit updateCollectButton();
        break;
    case IdShowNavigationWindow:
        m_nav->setAlwaysHidden(false);
        break;
    case IdHideNavigationWindow:
        m_nav->setAlwaysHidden(true);
        break;
    case IdRotateClockwise:
        rotateImage(true);
        break;
    case IdRotateCounterclockwise:
        rotateImage(false);
        break;
    case IdSetAsWallpaper:
        dApp->wpSetter->setWallpaper(path);
        break;
    case IdDisplayInFileManager:
        emit dApp->signalM->showInFileManager(path);
        break;
    case IdImageInfo:
        emit dApp->signalM->showExtensionPanel();
        break;
    default:
        break;
    }

    updateMenuContent();
}

void ViewPanel::updateMenuContent()
{
    if (m_infos.isEmpty()) {
        m_popupMenu->setMenuContent("");
        return;
    }
    // For update shortcut
    m_popupMenu->setMenuContent(createMenuContent());
}

void ViewPanel::initShortcut()
{
    // Image info
    QShortcut *sc = new QShortcut(QKeySequence("Alt+Return"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated,
            dApp->signalM, &SignalManager::showExtensionPanel);

    // Previous
    sc = new QShortcut(QKeySequence(Qt::Key_Left), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, &ViewPanel::showPrevious);

    // Next
    sc = new QShortcut(QKeySequence(Qt::Key_Right), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, &ViewPanel::showNext);

    // Zoom out (Ctrl++ Not working, This is a confirmed bug in Qt 5.5.0)
    sc = new QShortcut(QKeySequence(Qt::Key_Up), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        qreal v = m_view->scaleValue() + 0.1;
        m_view->setScaleValue(qMin(v, 20.0));
    });

    // Zoom in
    sc = new QShortcut(QKeySequence(Qt::Key_Down), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        qreal v = m_view->scaleValue() - 0.1;
        m_view->setScaleValue(qMax(v, 0.02));
    });

    // Esc
    QShortcut *esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    esc->setContext(Qt::WindowShortcut);
    connect(esc, &QShortcut::activated, this, [=] {
        if (window()->isFullScreen()) {
            toggleFullScreen();
        }
        else {
            if (m_vinfo.inDatabase) {
                backToLastPanel();
            }
            else {
                dApp->quit();
            }
        }
    });
}
