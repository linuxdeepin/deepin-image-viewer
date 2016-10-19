#include "timelinepanel.h"
#include "timelineimageview.h"
#include "controller/exporter.h"
#include "controller/popupmenumanager.h"
#include "controller/wallpapersetter.h"
#include "frame/deletedialog.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QShortcut>
#include <QtConcurrent>

namespace {

const QString FAVORITES_ALBUM_NAME = "My favorites";
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";

const QString SETTINGS_GROUP = "TIMEPANEL";
const QString SETTINGS_ICON_SCALE_KEY = "IconScale";

enum MenuItemId {
    IdView,
    IdFullScreen,
    IdStartSlideShow,
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

const DatabaseManager::ImageInfo imageInfo(const QString &name)
{
    return dApp->databaseM->getImageInfoByName(name);
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
    // Image info
    QShortcut *sc = new QShortcut(QKeySequence("Alt+Return"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        const QStringList paths = m_view->selectedImages().values();
        if (! paths.isEmpty())
            dApp->signalM->showImageInfo(paths.first());
    });

    // Select all
    sc = new QShortcut(QKeySequence("Ctrl+A"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        m_view->selectAll();
    });
}

void TimelinePanel::updateMenuContents()
{
    // For update shortcut
    m_popupMenu->setMenuContent(createMenuContent());
}

void TimelinePanel::onMenuItemClicked(int menuId, const QString &text)
{
    QMap<QString, QString> images = m_view->selectedImages();
    if (images.isEmpty()) {
        return;
    }

    const QStringList names = images.keys();
    const QStringList paths = images.values();
    const QStringList viewPaths = (paths.length() == 1) ?
                dApp->databaseM->getAllImagesPath() : paths;
    const QString cname = names.first();
    const QString cpath = paths.first();

    SignalManager::ViewInfo vinfo;
    vinfo.inDatabase = true;
    vinfo.lastPanel = this;
    vinfo.path = cpath;
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
    case IdAddToAlbum: {
        const QString album = text.split(SHORTCUT_SPLIT_FLAG).first();
        for (QString name : names) {
            dApp->databaseM->insertImageIntoAlbum(
                album, name, utils::base::timeToString(imageInfo(name).time));
        }
        break;
    }
    case IdExport:
        dApp->exporter->exportImage(paths);
        break;
    case IdCopy:
        utils::base::copyImageToClipboard(paths);
        break;
    case IdMoveToTrash: {
        popupDelDialog(paths, names);
        break;
    }
    case IdAddToFavorites:
        for(QString name : names) {
        dApp->databaseM->insertImageIntoAlbum(FAVORITES_ALBUM_NAME, name,
            utils::base::timeToString(imageInfo(cname).time));
        }
        updateMenuContents();
        break;
    case IdRemoveFromFavorites:
        dApp->databaseM->removeImageFromAlbum(FAVORITES_ALBUM_NAME, cname);
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
        dApp->wpSetter->setWallpaper(cpath);
        break;
    case IdDisplayInFileManager:
        utils::base::showInFileManager(cpath);
        break;
    case IdImageInfo:
        dApp->signalM->showImageInfo(cpath);
        break;
    default:
        break;
    }
}

QString TimelinePanel::createMenuContent()
{
    auto images = m_view->selectedImages();
    const QStringList paths = images.values();
    auto supportPath = std::find_if_not(paths.cbegin(), paths.cend(),
                                        utils::image::imageSupportSave);
    bool canSave = supportPath == paths.cend();

    QJsonArray items;

    ////////////////////////////////////////////////////////////////////////////
    if (images.count() == 1) {
        createMI(&items, IdView, tr("View"));
        createMI(&items, IdFullScreen, tr("Fullscreen"), "F11");
    }
    createMI(&items, IdStartSlideShow, tr("Start slide show"), "F5");
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
    if (images.count() == 1) {
        if (! dApp->databaseM->imageExistAlbum(images.firstKey(),
                                               FAVORITES_ALBUM_NAME)) {
            createMI(&items, IdAddToFavorites, tr("Add to My favorites"),
                     "Ctrl+K");
        }
        else {
            createMI(&items, IdRemoveFromFavorites, tr("Unfavorite"),
                     "Ctrl+Shift+K");
        }
    } else {
        bool v = false;
        for (QString img : images) {
            if (! dApp->databaseM->imageExistAlbum(images.key(img),
                                                   FAVORITES_ALBUM_NAME)) {
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
    if (images.count() == 1) {
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
    const QStringList albums = dApp->databaseM->getAlbumNameList();
    const QStringList names = m_view->selectedImages().keys();

    QJsonArray items;
    if (! names.isEmpty()) {
        for (QString album : albums) {
            if (album == FAVORITES_ALBUM_NAME || album == "Recent imported") {
                continue;
            }
            const QStringList names =
                    dApp->databaseM->getImageNamesByAlbum(album);
            for (QString name : names) {
                if (names.indexOf(name) == -1) {
                    createMI(&items, IdAddToAlbum, album);
                    break;
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

void TimelinePanel::rotateImage(const QString &path, int degree)
{
    utils::image::rotate(path, degree);
    m_rotateList.removeAll(path);
    if (m_rotateList.isEmpty()) {
        qDebug() << "Rotate finish!";
        m_view->updateThumbnails();
    }
}

void TimelinePanel::popupDelDialog(const QStringList paths, const QStringList names)
{
    DeleteDialog* dialog = new DeleteDialog(paths, false, this);
    dialog->show();
    dialog->moveToCenter();
    connect(dialog, &DeleteDialog::buttonClicked, [=](int index){
        if (index == 1) {
            dApp->databaseM->removeImages(names);
            utils::base::trashFiles(paths);
        }
    });
    connect(dialog, &DeleteDialog::closed, dialog, &DeleteDialog::deleteLater);

}
