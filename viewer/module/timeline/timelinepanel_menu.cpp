#include "timelinepanel.h"
#include "controller/dbmanager.h"
#include "controller/exporter.h"
#include "controller/wallpapersetter.h"
#include "controller/popupdialogmanager.h"
#include "frame/deletedialog.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "timelineframe.h"
#include <QShortcut>
#include <QtConcurrent>

#include <DMenu>
#include "daction.h"

using namespace Dtk::Widget;

namespace {

const QString FAVORITES_ALBUM_NAME = "My favorites";
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";

const QString SETTINGS_GROUP = "TIMEPANEL";
const QString SETTINGS_ICON_SCALE_KEY = "IconScale";

enum MenuItemId {
    IdView,
    IdFullScreen,
    IdStartSlideShow,
    IdPrint,
    IdAddToAlbum,
    IdExport,
    IdCopy,
    IdThrowToTrash,
    IdRemoveFromTimeline,
    IdEdit,
    IdAddToFavorites,
    IdRemoveFromFavorites,
    IdRotateClockwise,
    IdRotateCounterclockwise,
    IdLabel,
    IdSetAsWallpaper,
    IdDisplayInFileManager,
    IdImageInfo,
    IdSubMenu,
    IdSeparator
};

}
using namespace Dtk::Widget;

void TimelinePanel::initShortcut()
{
    //FullScreen
    QShortcut *sc = new QShortcut(QKeySequence("F11"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = m_frame->selectedPaths();
        if (paths.isEmpty()||paths.length() > 1)
            return;
        SignalManager::ViewInfo vinfo;
        vinfo.inDatabase = true;
        vinfo.lastPanel = this;
        vinfo.path = paths.first();
        vinfo.paths = paths;
        vinfo.fullScreen = true;
        dApp->signalM->viewImage(vinfo);
    });
    //Start Slide show
    sc = new QShortcut(QKeySequence("F5"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = m_frame->selectedPaths();
        if (paths.isEmpty()) {
            return;
        }
        const QStringList viewPaths = (paths.length() == 1) ?
                    dApp->dbM->getAllPaths() : paths;
        const QString dpath = paths.first();

        SignalManager::ViewInfo vinfo;
        vinfo.inDatabase = true;
        vinfo.lastPanel = this;
        vinfo.path = dpath;
        vinfo.paths = viewPaths;
        dApp->signalM->startSlideShow(vinfo);
    });
    //Print
    sc = new QShortcut(QKeySequence("Ctrl+P"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        const QStringList paths = m_frame->selectedPaths();
        if (paths.isEmpty()) {
            return;
        }
        const QString dpath = paths.first();
        using namespace controller::popup;
        printDialog(dpath);
    });
    //Copy
    sc = new QShortcut(QKeySequence("Ctrl+C"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = m_frame->selectedPaths();
        utils::base::copyImageToClipboard(paths);
    });
    //Throw to trash
    sc = new QShortcut(QKeySequence("Delete"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = m_frame->selectedPaths();
        popupDelDialog(paths);
    });
    //Add to My favorites
    sc = new QShortcut(QKeySequence("Ctrl+K"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = m_frame->selectedPaths();
        dApp->dbM->insertIntoAlbum(FAVORITES_ALBUM_NAME, paths);
    });
    //Remove From My favorites
    sc = new QShortcut(QKeySequence("Shift+Ctrl+K"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = m_frame->selectedPaths();
        dApp->dbM->removeFromAlbum(FAVORITES_ALBUM_NAME, paths);
    });
    //Rotate
    sc = new QShortcut(QKeySequence("Ctrl+R"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = m_frame->selectedPaths();
        if (m_rotateList.isEmpty()) {
            m_rotateList = paths;
            for (QString path : paths) {
                QtConcurrent::run(this, &TimelinePanel::rotateImage, path, 90);
            }
        }
    });
    //Counter Rotate
    sc = new QShortcut(QKeySequence("Shift + Ctrl+R"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = m_frame->selectedPaths();
        if (m_rotateList.isEmpty()) {
            m_rotateList = paths;
            for (QString path : paths) {
                QtConcurrent::run(this, &TimelinePanel::rotateImage, path, -90);
            }
        }
    });
    //set as wallpaper
    sc = new QShortcut(QKeySequence("Ctrl+F8"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = m_frame->selectedPaths();
        if (paths.isEmpty()) {
            return;
        }
        const QString dpath = paths.first();
         dApp->wpSetter->setWallpaper(dpath);
    });
    //Display in file manager
    sc = new QShortcut(QKeySequence("Ctrl+D"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=]{
        const QStringList paths = m_frame->selectedPaths();
        if (paths.isEmpty()) {
            return;
        }
        const QString dpath = paths.first();
        utils::base::showInFileManager(dpath);
    });
    // Image info
    sc = new QShortcut(QKeySequence("Alt+Return"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        const QStringList paths = m_frame->selectedPaths();
        if (! paths.isEmpty()) {
            dApp->signalM->showImageInfo(paths.first());
        }
    });

    // Select all
    sc = new QShortcut(QKeySequence("Ctrl+A"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
//        m_view->selectAll();
        m_frame->selectAll();
    });
}

void TimelinePanel::showMenuContext(QPoint pos) {
    auto paths = m_frame->selectedPaths();
    auto supportPath = std::find_if_not(paths.cbegin(), paths.cend(),
                                        utils::image::imageSupportSave);
    bool canSave = supportPath == paths.cend();

    if (paths.isEmpty()) {
        return;
    }
    ////////////////////////////////////////////////////////////////////////////
    //get addToAlbum's List
    DMenu subAlbumNameMenu;
    const QStringList albums = dApp->dbM->getAllAlbumNames();
    bool albumListIsEmpty = true;
    for (QString album : albums) {
        if (album == FAVORITES_ALBUM_NAME) {
            continue;
        }
        const QStringList aps = dApp->dbM->getPathsByAlbum(album);
        for (QString path : paths) {
            if (aps.indexOf(path) == -1) {
                albumListIsEmpty = false;
                subAlbumNameMenu.addAction(album);
                break;
            }
        }
    }
    ////////////////////////////////////////////////////////////////////////
    DMenu mainMenu;

    if (paths.length() == 1) {
        DAction* viewAct = new DAction(tr("View"), this);
        viewAct->setData(IdView);
        mainMenu.addAction(viewAct);
        DAction* fullSAct = new DAction(tr("Fullscreen"), this);
        fullSAct->setData(IdFullScreen);
        mainMenu.addAction(fullSAct);
    }
    DAction* startSlideSAct = new DAction(tr("Start slide show"), this);
    startSlideSAct->setData(IdStartSlideShow);
    mainMenu.addAction(startSlideSAct);
    DAction* printAct = new DAction(tr("Print"), this);
    printAct->setData(IdPrint);
    mainMenu.addAction(printAct);
    DAction* addToAlbumAct = new DAction(tr("Add to album"), this);
    addToAlbumAct->setData(IdAddToAlbum);
    addToAlbumAct->setMenu(&subAlbumNameMenu);
    if (!albumListIsEmpty)
        mainMenu.addAction(addToAlbumAct);
    mainMenu.addSeparator();
    //////////////////////////////////////////////////////////////////////////
    DAction* cpyAct = new DAction(tr("Copy"), this);
    cpyAct->setData(IdCopy);
    mainMenu.addAction(cpyAct);
    DAction* delAct = new DAction(tr("Throw to Trash"), this);
    delAct->setData(IdThrowToTrash);
    mainMenu.addAction(delAct);
    mainMenu.addSeparator();
    /////////////////////////////////////////////////////////////////////////
    DAction* addToFavAct = new DAction(tr("Add to My favorites"), this);
    addToFavAct->setData(IdAddToFavorites);
    if (paths.length() == 1) {
        DAction* removeFavAct = new DAction(tr("Unfavorite"), this);
        removeFavAct->setData(IdRemoveFromFavorites);
        if (!dApp->dbM->isImgExistInAlbum(FAVORITES_ALBUM_NAME, paths.first())) {

            mainMenu.addAction(addToFavAct);
        } else {
            mainMenu.addAction(removeFavAct);
        }
    } else {
        bool v = false;
        for (QString img : paths) {
            if (!dApp->dbM->isImgExistInAlbum(FAVORITES_ALBUM_NAME, img)) {
                v = true;
                break;
            }
        }
        if (v)
            mainMenu.addAction(addToFavAct);
    }
    ////////////////////////////////////////////////////////////////////////
    if (paths.length() == 1) {
        if (canSave) {
            mainMenu.addSeparator();
            DAction* rotateAct = new DAction(tr("Rotate clockwise"),
                                                      this);
            rotateAct->setData(IdRotateClockwise);
            mainMenu.addAction(rotateAct);
            DAction* antiRotateAct = new DAction(tr("Rotate "
                                            "counterclockwise"), this);
            antiRotateAct->setData(IdRotateCounterclockwise);
            mainMenu.addAction(antiRotateAct);
        }
    }
    mainMenu.addSeparator();
    if (paths.length() == 1) {
        if (canSave) {
            DAction* wallpaperAct = new DAction(tr("Set as wallpaper"), this);
            wallpaperAct->setData(IdSetAsWallpaper);
            mainMenu.addAction(wallpaperAct);
        }
        DAction* displayInFileMAct = new DAction(tr("Display in file manager"),
                                                 this);
        displayInFileMAct->setData(IdDisplayInFileManager);
        mainMenu.addAction(displayInFileMAct);
        DAction* imgInfoAct = new DAction(tr("Image info"), this);
        imgInfoAct->setData(IdImageInfo);
        mainMenu.addAction(imgInfoAct);
    }

    QObject::connect(&mainMenu, &DMenu::triggered, this, [=](QAction* action) {
        if (action->data().toInt() == 0 && action->text() != tr("View")) {
            onMenuItemClicked(IdAddToAlbum, action->text());
        } else {
            onMenuItemClicked(action->data().toInt(), action->text());
        }
    });
    mainMenu.exec(pos);
}

void TimelinePanel::onMenuItemClicked(int menuId, const QString &text) {
    const QStringList paths = m_frame->selectedPaths();
    if (paths.isEmpty()) {
        return;
    }

    const QStringList viewPaths = (paths.length() == 1) ?
                dApp->dbM->getAllPaths() : paths;
    const QString dpath = paths.first();

    SignalManager::ViewInfo vinfo;
    vinfo.inDatabase = true;
    vinfo.lastPanel = this;
    vinfo.path = dpath;
    vinfo.paths = viewPaths;

    switch (MenuItemId(menuId)) {
    case IdView:
        dApp->signalM->viewImage(vinfo);
        break;
    case IdFullScreen:
        vinfo.fullScreen = true;
        dApp->signalM->viewImage(vinfo);
        break;
    case IdStartSlideShow:
        dApp->signalM->startSlideShow(vinfo);
        break;
    case IdPrint: {
        using namespace controller::popup;
        printDialog(dpath);
        break;
    }
    case IdAddToAlbum: {
        const QString album = text;
        dApp->dbM->insertIntoAlbum(album, paths);
        break;
    }
    case IdCopy:
        utils::base::copyImageToClipboard(paths);
        break;
    case IdThrowToTrash: {
        popupDelDialog(paths);
        break;
    }
    case IdAddToFavorites:
        dApp->dbM->insertIntoAlbum(FAVORITES_ALBUM_NAME, paths);
        break;
    case IdRemoveFromFavorites:
        dApp->dbM->removeFromAlbum(FAVORITES_ALBUM_NAME, paths);
        break;
    case IdRotateClockwise:
        if (m_rotateList.isEmpty()) {
            m_rotateList = paths;
            for (QString path : paths) {
                QtConcurrent::run(this, &TimelinePanel::rotateImage, path, 90);
            }
        }
        break;
    case IdRotateCounterclockwise:
        if (m_rotateList.isEmpty()) {
            m_rotateList = paths;
            for (QString path : paths) {
                QtConcurrent::run(this, &TimelinePanel::rotateImage, path, -90);
            }
        }
        break;
    case IdSetAsWallpaper:
        dApp->wpSetter->setWallpaper(dpath);
        break;
    case IdDisplayInFileManager:
        utils::base::showInFileManager(dpath);
        break;
    case IdImageInfo:
        dApp->signalM->showImageInfo(dpath);
        break;
    default:
        break;
    }
}

void TimelinePanel::rotateImage(const QString &path, int degree)
{
    utils::image::rotate(path, degree);
    m_rotateList.removeAll(path);
    if (m_rotateList.isEmpty()) {
        qDebug() << "Rotate finish!";
//        m_view->updateThumbnails();
        m_frame->updateThumbnails();
    }
}

void TimelinePanel::popupDelDialog(const QStringList &paths)
{
    DeleteDialog* dialog = new DeleteDialog(paths, false, this);
    dialog->show();
    dialog->moveToCenter();
    connect(dialog, &DeleteDialog::buttonClicked, [=](int index){
        dialog->hide();

        if (index == 1) {
            dApp->dbM->removeImgInfos(paths);
            utils::base::trashFiles(paths);
        }
    });
    connect(dialog, &DeleteDialog::closed, dialog, &DeleteDialog::deleteLater);
}
