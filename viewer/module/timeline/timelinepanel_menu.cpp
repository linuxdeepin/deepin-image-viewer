#include "timelinepanel.h"
#include "timelineframe.h"
#include "controller/dbmanager.h"
#include "controller/exporter.h"
#include "controller/popupmenumanager.h"
#include "controller/wallpapersetter.h"
#include "controller/popupdialogmanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/dialogs/filedeletedialog.h"
#include <QShortcut>
#include <QtConcurrent>

namespace {

const QString FAVORITES_ALBUM_NAME = "My favorites";
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";

const QString SETTINGS_GROUP = "TIMEPANEL";
const QString SETTINGS_ICON_SCALE_KEY = "IconScale";

//album_map<key, value>: key is albumName's ellipsis form like NewAlbum...XX,
// and the value is albumName's fullName like NewAlbumXXXXXXXXXXXXX;
QMap<QString, QString> album_map;
enum MenuItemId {
    IdView,
    IdFullScreen,
    IdStartSlideShow,
    IdPrint,
    IdAddToAlbum,
    IdExport,
    IdCopy,
    IdMoveToTrash,
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

}  // namespace

void TimelinePanel::initPopupMenu()
{
    m_popupMenu = new PopupMenuManager(this);
    updateMenuContents();
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &TimelinePanel::onMenuItemClicked);
}

void TimelinePanel::initShortcut()
{
//    // View
//    QShortcut *sc = new QShortcut(QKeySequence("Return"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=]{
//        const QStringList paths = m_frame->selectedPaths();
//        if (paths.isEmpty()||paths.length() > 1)
//            return;
//        SignalManager::ViewInfo vinfo;
//        vinfo.inDatabase = true;
//        vinfo.lastPanel = this;
//        vinfo.path = paths.first();
//        vinfo.paths = paths;
//        vinfo.fullScreen = false;
//        dApp->signalM->viewImage(vinfo);
//    });

//    //FullScreen
//    sc = new QShortcut(QKeySequence("F11"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=]{
//        const QStringList paths = m_frame->selectedPaths();
//        if (paths.isEmpty()||paths.length() > 1)
//            return;
//        SignalManager::ViewInfo vinfo;
//        vinfo.inDatabase = true;
//        vinfo.lastPanel = this;
//        vinfo.path = paths.first();
//        vinfo.paths = paths;
//        vinfo.fullScreen = true;
//        dApp->signalM->viewImage(vinfo);
//    });
//    //Start Slide show
//    sc = new QShortcut(QKeySequence("F5"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=]{
//        const QStringList paths = m_frame->selectedPaths();
//        if (paths.isEmpty()) {
//            return;
//        }
//        const QStringList viewPaths = (paths.length() == 1) ?
//                    dApp->dbM->getAllPaths() : paths;
//        const QString dpath = paths.first();

//        SignalManager::ViewInfo vinfo;
//        vinfo.inDatabase = true;
//        vinfo.lastPanel = this;
//        vinfo.path = dpath;
//        vinfo.paths = viewPaths;
//        dApp->signalM->startSlideShow(vinfo);
//    });
//    //Print
//    sc = new QShortcut(QKeySequence("Ctrl+P"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=] {
//        const QStringList paths = m_frame->selectedPaths();
//        if (paths.isEmpty()) {
//            return;
//        }
//        const QString dpath = paths.first();
//        using namespace controller::popup;
//        printDialog(dpath);
//    });
//    //Copy
//    sc = new QShortcut(QKeySequence("Ctrl+C"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=]{
//        const QStringList paths = m_frame->selectedPaths();
//        utils::base::copyImageToClipboard(paths);
//    });
//    //Throw to trash
//    sc = new QShortcut(QKeySequence("Delete"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=]{
//        const QStringList paths = m_frame->selectedPaths();
//        popupDelDialog(paths);
//    });
//    //Add to My favorites
//    sc = new QShortcut(QKeySequence("Ctrl+K"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=]{
//        const QStringList paths = m_frame->selectedPaths();
//        dApp->dbM->insertIntoAlbum(FAVORITES_ALBUM_NAME, paths);
//    });
//    //Remove From My favorites
//    sc = new QShortcut(QKeySequence("Shift+Ctrl+K"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=]{
//        const QStringList paths = m_frame->selectedPaths();
//        dApp->dbM->removeFromAlbum(FAVORITES_ALBUM_NAME, paths);
//    });
//    //Rotate
//    sc = new QShortcut(QKeySequence("Ctrl+R"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=]{
//        const QStringList paths = m_frame->selectedPaths();
//        if (m_rotateList.isEmpty()) {
//            m_rotateList = paths;
//            for (QString path : paths) {
//                QtConcurrent::run(this, &TimelinePanel::rotateImage, path, 90);
//            }
//        }
//    });
//    //Counter Rotate
//    sc = new QShortcut(QKeySequence("Shift + Ctrl+R"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=]{
//        const QStringList paths = m_frame->selectedPaths();
//        if (m_rotateList.isEmpty()) {
//            m_rotateList = paths;
//            for (QString path : paths) {
//                QtConcurrent::run(this, &TimelinePanel::rotateImage, path, -90);
//            }
//        }
//    });
//    //set as wallpaper
//    sc = new QShortcut(QKeySequence("Ctrl+F8"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=]{
//        const QStringList paths = m_frame->selectedPaths();
//        if (paths.isEmpty()) {
//            return;
//        }
//        const QString dpath = paths.first();
//        dApp->wpSetter->setWallpaper(dpath);
//    });
//    //Display in file manager
//    sc = new QShortcut(QKeySequence("Ctrl+D"), this);
//    sc->setContext(Qt::WindowShortcut);
//    connect(sc, &QShortcut::activated, this, [=]{
//        const QStringList paths = m_frame->selectedPaths();
//        if (paths.isEmpty()) {
//            return;
//        }
//        const QString dpath = paths.first();
//        utils::base::showInFileManager(dpath);
//    });
    // Image info
    QShortcut *sc = new QShortcut(QKeySequence("Alt+Return"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        const QStringList paths = m_frame->selectedPaths();
        if (! paths.isEmpty()) {
            dApp->signalM->showImageInfo(paths.first());
        }
    });
    sc = new QShortcut(QKeySequence("Alt+Enter"), this);
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

void TimelinePanel::updateMenuContents()
{
    // For update shortcut
    m_popupMenu->setMenuContent(createMenuContent());
}

void TimelinePanel::onMenuItemClicked(int menuId, const QString &text)
{
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
        const QString albumKey = text.split(SHORTCUT_SPLIT_FLAG).first();
        if (albumKey != tr("Add to new album")) {
            const QString album = album_map.value(albumKey);
            dApp->dbM->insertIntoAlbum(album, paths);
        }else {
            dApp->signalM->createAlbum(paths);
        }
        break;
    }
    case IdCopy:
        utils::base::copyImageToClipboard(paths);
        break;
    case IdMoveToTrash: {
        FileDeleteDialog *fdd = new FileDeleteDialog(paths);
        fdd->show();
        break;
    }
    case IdAddToFavorites:
        dApp->dbM->insertIntoAlbum(FAVORITES_ALBUM_NAME, paths);
        updateMenuContents();
        break;
    case IdRemoveFromFavorites:
        dApp->dbM->removeFromAlbum(FAVORITES_ALBUM_NAME, paths);
        updateMenuContents();
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

QString TimelinePanel::createMenuContent()
{
    auto paths = m_frame->selectedPaths();
    auto supportPath = std::find_if_not(paths.cbegin(), paths.cend(),
                                        utils::image::imageSupportSave);
    bool canSave = supportPath == paths.cend();

    QJsonArray items;

    ////////////////////////////////////////////////////////////////////////////
    if (paths.length() == 1) {
        createMI(&items, IdView, tr("View"));
        createMI(&items, IdFullScreen, tr("Fullscreen"), "F11");
    }
    createMI(&items, IdStartSlideShow, tr("Start slide show"), "F5");
    createMI(&items, IdPrint, tr("Print"), "Ctrl+P");
    const QJsonObject objF = createAlbumMenuObj();
    if (! objF.isEmpty()) {
        createMI(&items, IdAddToAlbum, tr("Add to album"), "", false, objF);
    }
    createMI(&items, IdSeparator, "", "", true);

    ////////////////////////////////////////////////////////////////////////////
    createMI(&items, IdCopy, tr("Copy"), "Ctrl+C");
    createMI(&items, IdMoveToTrash, tr("Throw to Trash"), "Delete");
    createMI(&items, IdSeparator, "", "", true);

    ////////////////////////////////////////////////////////////////////////////
    if (paths.length() == 1) {
        if (! dApp->dbM->isImgExistInAlbum(FAVORITES_ALBUM_NAME, paths.first())) {
            createMI(&items, IdAddToFavorites, tr("Add to My favorites"),
                     "Ctrl+K");
        }
        else {
            createMI(&items, IdRemoveFromFavorites, tr("Unfavorite"),
                     "Ctrl+Shift+K");
        }
    } else {
        bool v = false;
        for (QString img : paths) {
            if (! dApp->dbM->isImgExistInAlbum(FAVORITES_ALBUM_NAME, img)) {
                v = true;
                break;
            }
        }
        if (v) {
            createMI(&items, IdAddToFavorites, tr("Add to My favorites"), "Ctrl+K");
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    if (canSave) {
        createMI(&items, IdSeparator, "", "", true);
        createMI(&items, IdRotateClockwise, tr("Rotate clockwise"), "Ctrl+R");
        createMI(&items, IdRotateCounterclockwise, tr("Rotate counterclockwise"), "Ctrl+Shift+R");
    }
    createMI(&items, IdSeparator, "", "", true);

    ////////////////////////////////////////////////////////////////////////////
    if (paths.length() == 1) {
        if (canSave) {
            createMI(&items, IdSetAsWallpaper, tr("Set as wallpaper"), "Ctrl+F8");
        }
        createMI(&items, IdDisplayInFileManager, tr("Display in file manager"), "Ctrl+D");
        createMI(&items, IdImageInfo, tr("Image info"), "Alt+Enter");
    }

    QJsonObject contentObj;
    contentObj["x"] = 0;
    contentObj["y"] = 0;
    contentObj["items"] = QJsonValue(items);

    return QString(QJsonDocument(contentObj).toJson());
}

QJsonObject TimelinePanel::createAlbumMenuObj()
{
    const QStringList albums = dApp->dbM->getAllAlbumNames();
    const QStringList paths = m_frame->selectedPaths();

    QJsonArray items;
    bool isAddNewAlbum = false;
    album_map.clear();
    if (! paths.isEmpty()) {
        for (QString album : albums) {
            if (album == FAVORITES_ALBUM_NAME) {
                continue;
            }
            const QStringList aps = dApp->dbM->getPathsByAlbum(album);
            for (QString path : paths) {
                if (aps.indexOf(path) == -1) {
                    if (!isAddNewAlbum) {
                        isAddNewAlbum = true;
                        createMI(&items, IdAddToAlbum, tr("Add to new album"));
                        createMI(&items, IdSeparator, "", "", true);
                    }

                    QString albumKey = this->fontMetrics().elidedText(album,
                        Qt::ElideMiddle, 255);
                    album_map.insert(albumKey, album);
                    createMI(&items, IdAddToAlbum, albumKey);
                    break;
                }
            }
        }

        if (!isAddNewAlbum) {
            createMI(&items, IdAddToAlbum, tr("Add to new album"));
        }
    }

    QJsonObject contentObj;
    if (! items.isEmpty()) {
        contentObj[""] = QJsonValue(items);
    }

    return contentObj;
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
