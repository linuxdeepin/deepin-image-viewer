#include "viewpanel.h"
#include "navigationwidget.h"
#include "contents/imageinfowidget.h"
#include "frame/deletedialog.h"
#include "scen/imageview.h"

#include <controller/exporter.h>
#include <controller/popupmenumanager.h>
#include <controller/wallpapersetter.h>
#include <controller/popupdialogmanager.h>
#include <utils/baseutils.h>
#include <utils/imageutils.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QShortcut>

#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>

namespace {

const QString SHORTCUT_SPLIT_FLAG = "@-_-@";
const QString FAVORITES_ALBUM_NAME = "My favorites";

}  // namespace

enum MenuItemId {
    IdFullScreen,
    IdStartSlideShow,
    IdPrint,
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
    createMI(&items, IdPrint, tr("Print"), "Ctrl+P");
    if (m_vinfo.inDatabase) {
        const QJsonObject objF = createAlbumMenuObj(false);
        if (! objF.isEmpty()) {
            createMI(&items, IdAddToAlbum, tr("Add to album"), "", false, objF);
        }
    }

    /**************************************************************************/
    createMI(&items, IdSeparator, "", "", true);
    //Hide the export function
    //if (m_vinfo.inDatabase) {
    //    createMI(&items, IdExport, tr("Export"));
    //}
    createMI(&items, IdCopy, tr("Copy"), "Ctrl+C");
    createMI(&items, IdMoveToTrash, tr("Throw to Trash"), "Delete");
    if (! m_vinfo.album.isEmpty()) {
        createMI(&items, IdRemoveFromAlbum, tr("Remove from album"), "Shift+Delete");
    }

    /**************************************************************************/
    createMI(&items, IdSeparator, "", "", true);

    if (m_vinfo.inDatabase) {
        if (m_current != m_infos.constEnd() &&
                ! dApp->dbM->isImgExistInAlbum(FAVORITES_ALBUM_NAME, m_current->filePath)) {
            createMI(&items, IdAddToFavorites, tr("Add to My favorites"), "Ctrl+K");
        } else {
            createMI(&items, IdRemoveFromFavorites, tr("Unfavorite"), "Ctrl+Shift+K");
        }
    }

    /**************************************************************************/
    createMI(&items, IdSeparator, "", "", true);

    if (! m_viewB->isWholeImageVisible() && m_nav->isAlwaysHidden()) {
        createMI(&items, IdShowNavigationWindow, tr("Show navigation window"));
    } else if (! m_viewB->isWholeImageVisible() && !m_nav->isAlwaysHidden()) {
        createMI(&items, IdHideNavigationWindow, tr("Hide navigation window"));
    }

    /**************************************************************************/
    if (utils::image::imageSupportSave(m_current->filePath)) {
    createMI(&items, IdSeparator, "", "", true);

    createMI(&items, IdRotateClockwise, tr("Rotate clockwise"), "Ctrl+R");
    createMI(&items, IdRotateCounterclockwise, tr("Rotate counterclockwise"), "Ctrl+Shift+R");
    }
    /**************************************************************************/
    createMI(&items, IdSeparator, "", "", true);

    if (utils::image::imageSupportSave(m_current->filePath))  {
    createMI(&items, IdSetAsWallpaper, tr("Set as wallpaper"), "Ctrl+F8");
    }
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
    if (m_current == m_infos.constEnd() || ! m_vinfo.inDatabase) {
        return QJsonObject();
    }
    const QStringList albums = dApp->dbM->getAllAlbumNames();

    QJsonArray items;
    if (! m_infos.isEmpty()) {
        for (QString album : albums) {
            if (album == FAVORITES_ALBUM_NAME) {
                continue;
            }
            const QStringList paths = dApp->dbM->getPathsByAlbum(album);
            if (! paths.contains(m_current->filePath)) {
                createMI(&items, IdAddToAlbum, album);
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

    const QString path = m_current->filePath;
    const QString albumName =text;

    switch (MenuItemId(menuId)) {
    case IdFullScreen:
        toggleFullScreen();
        break;
    case IdStartSlideShow: {
        auto vinfo = m_vinfo;
        vinfo.fullScreen = window()->isFullScreen();
        vinfo.lastPanel = this;
        vinfo.path = path;
        vinfo.paths = paths();
        emit dApp->signalM->startSlideShow(vinfo);
        break;
    }
    case IdPrint: {
        using namespace controller::popup;
        printDialog(path);
        break;
    }
    case IdAddToAlbum: {
        if (albumName != tr("Add to new album")) {
            dApp->dbM->insertIntoAlbum(albumName, QStringList(path));
        } else {
            dApp->signalM->createAlbum(QStringList(path));
        }
    }
        break;
    case IdExport:
        dApp->exporter->exportImage(QStringList(path));
        break;
    case IdCopy:
        copyImageToClipboard(QStringList(path));
        break;
    case IdMoveToTrash: {
        popupDelDialog(path);
        break;
    }
    case IdRemoveFromAlbum:
        dApp->dbM->removeFromAlbum(m_vinfo.album, QStringList(path));
        break;
    case IdAddToFavorites:
        dApp->dbM->insertIntoAlbum(FAVORITES_ALBUM_NAME, QStringList(path));
        emit updateCollectButton();
        break;
    case IdRemoveFromFavorites:
        dApp->dbM->removeFromAlbum(FAVORITES_ALBUM_NAME, QStringList(path));
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
        // Update panel info
        TIMER_SINGLESHOT(100, {m_info->setImagePath(path);}, this, path);
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
    connect(sc, &QShortcut::activated, this, [=] {
        dApp->signalM->showExtensionPanel();
        // Update panel info
        TIMER_SINGLESHOT(100, {m_info->setImagePath(m_current->filePath);}, this);
    });

    // Delay image toggle
    QTimer *dt = new QTimer(this);
    dt->setSingleShot(true);
    dt->setInterval(300);
    // Previous
    sc = new QShortcut(QKeySequence(Qt::Key_Left), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        if (! dt->isActive()) {
            dt->start();
            showPrevious();
        }
    });
    // Next
    sc = new QShortcut(QKeySequence(Qt::Key_Right), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        if (! dt->isActive()) {
            dt->start();
            showNext();
        }
    });

    // Zoom out (Ctrl++ Not working, This is a confirmed bug in Qt 5.5.0)
    sc = new QShortcut(QKeySequence(Qt::Key_Up), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        qDebug() << "Qt::Key_Up:";
        m_viewB->setScaleValue(1.1);
    });

    // Zoom in
    sc = new QShortcut(QKeySequence(Qt::Key_Down), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        qDebug() << "Qt::Key_Down:";
        m_viewB->setScaleValue(0.9);
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
    //1:1 size
    QShortcut *adaptImage = new QShortcut(QKeySequence("Ctrl+0"), this);
    adaptImage->setContext(Qt::WindowShortcut);
    connect(adaptImage, &QShortcut::activated, this, [=]{
        m_viewB->fitImage();
    });
}

void ViewPanel::popupDelDialog(const QString path) {
    using namespace utils::base;
    if (m_vinfo.inDatabase) {
        DeleteDialog* delDialog = new DeleteDialog(QStringList(path));
        delDialog->show();
        delDialog->moveToCenter();
        connect(delDialog, &DeleteDialog::buttonClicked, [=](int index){
            delDialog->hide();

            if (index == 1) {
                dApp->dbM->removeImgInfos(QStringList(path));
                trashFile(path);
            }
        });

        connect(delDialog, &DeleteDialog::closed,
                delDialog, &DeleteDialog::deleteLater);
    } else {
        dApp->dbM->removeImgInfos(QStringList(path));
        trashFile(path);
        removeCurrentImage();
    }
}
