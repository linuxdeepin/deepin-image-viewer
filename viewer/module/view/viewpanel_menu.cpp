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

#include <DAction>
#include <DMenu>

using namespace Dtk::Widget;

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
    IdThrowToTrash,
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

void ViewPanel::showMenuContext(QPoint pos)
{
    ////////////////////////////////////////////////////////////////////////////
    //get addToAlbum's List
    DMenu subAlbumNameMenu;
    bool albumListIsEmpty = true;
    const QStringList albums = dApp->dbM->getAllAlbumNames();
    if (! m_infos.isEmpty()) {
        for (QString album : albums) {
            if (album == FAVORITES_ALBUM_NAME) {
                continue;
            }
            const QStringList paths = dApp->dbM->getPathsByAlbum(album);
            if (! paths.contains(m_current->filePath)) {
                albumListIsEmpty = false;
                subAlbumNameMenu.addAction(album);
            }
        }
    }
    ////////////////////////////////////////////////////////////////////////
    DMenu mainMenu;
    DAction* fullSAct = new DAction((window()->isFullScreen() ?
                        tr("Exit fullscreen") : tr("Fullscreen")), this);
    fullSAct->setData(IdFullScreen);
    mainMenu.addAction(fullSAct);

    DAction* startSlideSAct = new DAction(tr("Start slide show"), this);
    startSlideSAct->setData(IdStartSlideShow);
    mainMenu.addAction(startSlideSAct);
    DAction* printAct = new DAction(tr("Print"), this);
    printAct->setData(IdPrint);
    mainMenu.addAction(printAct);
    DAction* addToAlbumAct = new DAction(tr("Add to album"), this);
    addToAlbumAct->setData(IdAddToAlbum);
    addToAlbumAct->setMenu(&subAlbumNameMenu);
    if (m_vinfo.inDatabase && !albumListIsEmpty )
        mainMenu.addAction(addToAlbumAct);
    mainMenu.addSeparator();
    //////////////////////////////////////////////////////////////////////////
    DAction* cpyAct = new DAction(tr("Copy"), this);
    cpyAct->setData(IdCopy);
    mainMenu.addAction(cpyAct);
    DAction* delAct = new DAction(tr("Throw to Trash"), this);
    delAct->setData(IdThrowToTrash);
    mainMenu.addAction(delAct);
    ///////////////////////////////////////////////////////////////////////////
    if (! m_vinfo.album.isEmpty()) {
        mainMenu.addSeparator();
        DAction* removeAlbumAct = new DAction(tr("Remove from album"), this);
        removeAlbumAct->setData(IdRemoveFromAlbum);
        mainMenu.addAction(removeAlbumAct);
    }
    mainMenu.addSeparator();

    if (m_vinfo.inDatabase) {
        if (m_current != m_infos.constEnd() &&
                ! dApp->dbM->isImgExistInAlbum(FAVORITES_ALBUM_NAME, m_current->filePath)) {
            DAction* addFavAct = new DAction(tr("Add to My favorites"), this);
            addFavAct->setData(IdAddToFavorites);
            mainMenu.addAction(addFavAct);

        } else {
            DAction* unFavAct = new DAction(tr("Unfavorite"), this);
            unFavAct->setData(IdRemoveFromFavorites);
            mainMenu.addAction(unFavAct);
        }
        mainMenu.addSeparator();
    }
    ///////////////////////////////////////////////////////////////////////////
    if (! m_viewB->isWholeImageVisible() && m_nav->isAlwaysHidden()) {
        DAction* showNavAct = new DAction(tr("Show navigation window"), this);
        showNavAct->setData(IdShowNavigationWindow);
        mainMenu.addAction(showNavAct);
        mainMenu.addSeparator();
    } else if (! m_viewB->isWholeImageVisible() && !m_nav->isAlwaysHidden()) {
        DAction* hideNavAct = new DAction(tr("Hide navigation window"), this);
        hideNavAct->setData(IdHideNavigationWindow);
        mainMenu.addAction(hideNavAct);
        mainMenu.addSeparator();
    }

    if (utils::image::imageSupportSave(m_current->filePath))  {
        DAction* rotateAct = new DAction(tr("Rotate clockwise"),
                                         this);
        rotateAct->setData(IdRotateClockwise);
        mainMenu.addAction(rotateAct);
        DAction* antiRotateAct = new DAction(tr("Rotate "
                                                "counterclockwise"), this);
        antiRotateAct->setData(IdRotateCounterclockwise);
        mainMenu.addAction(antiRotateAct);
        mainMenu.addSeparator();
        DAction* wallpaperAct = new DAction(tr("Set as wallpaper"), this);
        wallpaperAct->setData(IdSetAsWallpaper);
        mainMenu.addAction(wallpaperAct);
        mainMenu.addSeparator();
    }

    if (m_vinfo.inDatabase) {
        DAction* displayInFileMAct = new DAction(tr("Display in file manager"),
                                                 this);
        displayInFileMAct->setData(IdDisplayInFileManager);
        mainMenu.addAction(displayInFileMAct);
    }
    DAction* imgInfoAct = new DAction(tr("Image info"), this);
    imgInfoAct->setData(IdImageInfo);
    mainMenu.addAction(imgInfoAct);

    QObject::connect(&mainMenu, &DMenu::triggered, this, [=](QAction* action) {
        if (action->data().toInt() == 0 && action->text() != (window()->isFullScreen() ?
           tr("Exit fullscreen") : tr("Fullscreen"))) {
            onMenuItemClicked(IdAddToAlbum, action->text());
        } else {
            onMenuItemClicked(action->data().toInt(), action->text());
        }
    });

    mainMenu.exec(pos);
}

void ViewPanel::onMenuItemClicked(int menuId, const QString &text)
{
    using namespace utils::base;
    using namespace utils::image;

    const QStringList mtl = text.split(SHORTCUT_SPLIT_FLAG);
    const QString path = m_current->filePath;
    const QString albumName = mtl.isEmpty() ? "" : mtl.first();

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
    case IdAddToAlbum:
        dApp->dbM->insertIntoAlbum(albumName, QStringList(path));
        break;
    case IdExport:
        dApp->exporter->exportImage(QStringList(path));
        break;
    case IdCopy:
        copyImageToClipboard(QStringList(path));
        break;
    case IdThrowToTrash: {
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

}

void ViewPanel::initShortcut()
{
    //FullScreen
    QShortcut *sc = new QShortcut(QKeySequence("F11"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        toggleFullScreen();
    });
    //Start Slide show
    sc = new QShortcut(QKeySequence("F5"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QString path = m_current->filePath;
        auto vinfo = m_vinfo;
        vinfo.fullScreen = window()->isFullScreen();
        vinfo.lastPanel = this;
        vinfo.path = path;
        vinfo.paths = paths();
        emit dApp->signalM->startSlideShow(vinfo);
    });
    //Print
    sc = new QShortcut(QKeySequence("Ctrl+P"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        const QString path = m_current->filePath;
        using namespace controller::popup;
        printDialog(path);
    });
    //Copy
    sc = new QShortcut(QKeySequence("Ctrl+C"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QString path = m_current->filePath;
        utils::base::copyImageToClipboard(QStringList(path));
    });
    //Throw to trash
    sc = new QShortcut(QKeySequence("Delete"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QString path = m_current->filePath;
        popupDelDialog(path);
    });
    //Add to My favorites
    sc = new QShortcut(QKeySequence("Ctrl+K"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QString path = m_current->filePath;
        dApp->dbM->insertIntoAlbum(FAVORITES_ALBUM_NAME, QStringList(path));
        emit updateCollectButton();
    });
    //Remove From My favorites
    sc = new QShortcut(QKeySequence("Shift+Ctrl+K"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QString path = m_current->filePath;
        dApp->dbM->removeFromAlbum(FAVORITES_ALBUM_NAME, QStringList(path));
        emit updateCollectButton();
    });
    //Rotate
    sc = new QShortcut(QKeySequence("Ctrl+R"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        rotateImage(true);
    });
    //Counter Rotate
    sc = new QShortcut(QKeySequence("Shift + Ctrl+R"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        rotateImage(false);
    });
    //set as wallpaper
    sc = new QShortcut(QKeySequence("Ctrl+F8"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QString path = m_current->filePath;
        dApp->wpSetter->setWallpaper(path);;
    });
    //Display in file manager
    sc = new QShortcut(QKeySequence("Ctrl+D"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QString path = m_current->filePath;
        emit dApp->signalM->showInFileManager(path);
    });
    // Image info
    sc = new QShortcut(QKeySequence("Alt+Return"), this);
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
        m_viewB->setScaleValue(1.1);
    });

    // Zoom in
    sc = new QShortcut(QKeySequence(Qt::Key_Down), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
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
