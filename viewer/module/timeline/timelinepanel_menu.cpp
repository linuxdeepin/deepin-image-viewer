#include "timelinepanel.h"
#include "timelineframe.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/dbmanager.h"
#include "controller/exporter.h"
#include "controller/wallpapersetter.h"
#include "controller/popupdialogmanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/dialogs/filedeletedialog.h"
#include <QMenu>
#include <QShortcut>
#include <QStyleFactory>
#include <QtConcurrent>

namespace {

const QString FAVORITES_ALBUM_NAME = "My favorites";

const QString SHORTCUTVIEW_GROUP = "SHORTCUTVIEW";
const QString SETTINGS_GROUP = "TIMEPANEL";
const QString SETTINGS_ICON_SCALE_KEY = "IconScale";

QString ss(const QString &text)
{
    return dApp->setter->value(SHORTCUTVIEW_GROUP, text).toString();
}

enum MenuItemId {
    IdView,
    IdFullScreen,
    IdStartSlideShow,
    IdPrint,
    IdAddToAlbum,
    IdCopy,
    IdCopyToClipboard,
    IdMoveToTrash,
    IdRemoveFromTimeline,
    IdAddToFavorites,
    IdRemoveFromFavorites,
    IdRotateClockwise,
    IdRotateCounterclockwise,
    IdSetAsWallpaper,
    IdDisplayInFileManager,
    IdImageInfo,
};

}  // namespace

void TimelinePanel::initPopupMenu()
{
    m_menu = new QMenu;
    m_menu->setStyle(QStyleFactory::create("dlight"));
    connect(m_menu, &QMenu::triggered, this, &TimelinePanel::onMenuItemClicked);
}

void TimelinePanel::appendAction(int id, const QString &text, const QString &shortcut)
{
    QAction *ac = new QAction(m_menu);
    addAction(ac);
    ac->setText(text);
    ac->setProperty("MenuID", id);
    const QString ss = dApp->setter->value(SHORTCUTVIEW_GROUP, text, shortcut).toString();
    ac->setShortcut(QKeySequence(ss));
    m_menu->addAction(ac);
}

QMenu *TimelinePanel::createAlbumMenu()
{
    QMenu *am = new QMenu(tr("Add to album"));
    am->setStyle(QStyleFactory::create("dlight"));
    QStringList albums = dApp->dbM->getAllAlbumNames();
    albums.removeAll(FAVORITES_ALBUM_NAME);
    const QStringList sps = m_frame->selectedPaths();

    QAction *ac = new QAction(am);
    ac->setProperty("MenuID", IdAddToAlbum);
    ac->setText(tr("Add to new album"));
    ac->setData(QString("Add to new album"));
    am->addAction(ac);
    am->addSeparator();
    for (QString album : albums) {
        const QStringList paths = dApp->dbM->getPathsByAlbum(album);
        for (QString sp : sps) {
            if (! paths.contains(sp)) {
                QAction *ac = new QAction(am);
                ac->setProperty("MenuID", IdAddToAlbum);
                ac->setText(fontMetrics().elidedText(album, Qt::ElideMiddle, 200));
                ac->setData(album);
                am->addAction(ac);
                break;
            }
        }
    }

    return am;
}

void TimelinePanel::updateMenuContents()
{
    auto paths = m_frame->selectedPaths();
    if (paths.isEmpty())
        return;

    m_menu->clear();
    qDeleteAll(this->actions());

    auto supportPath = std::find_if_not(paths.cbegin(), paths.cend(),
                                        utils::image::imageSupportSave);
    bool canSave = supportPath == paths.cend();

    if (paths.length() == 1) {
        appendAction(IdView, tr("View"), ss("View"));
        appendAction(IdFullScreen, tr("Fullscreen"), ss("Fullscreen"));
    }
    appendAction(IdStartSlideShow, tr("Start slideshow"), ss("Start slideshow"));
    appendAction(IdPrint, tr("Print"), ss("Print"));
    QMenu *am = createAlbumMenu();
    if (am) {
        m_menu->addMenu(am);
    }
    m_menu->addSeparator();
    /**************************************************************************/
    appendAction(IdCopy, tr("Copy Image Path"), ss("Copy Image Path"));
    if (paths.length() == 1)
        appendAction(IdCopyToClipboard, tr("Copy Image"), ss("Copy Image"));
    appendAction(IdMoveToTrash, tr("Throw to trash"), ss("Throw to trash"));
    m_menu->addSeparator();
    /**************************************************************************/
    bool isCollected = false, unFavor = false;
    for (QString img : paths) {
        if (dApp->dbM->isImgExistInAlbum(FAVORITES_ALBUM_NAME, img)) {
            isCollected = true;
            continue;
        } else {
            unFavor = true;
        }
    }

    //Multi-select don't support Unfavorites
    if (!unFavor && paths.length() == 1) {
        appendAction(IdRemoveFromFavorites, tr("Unfavorite"), ss("Unfavorite"));
    } else if (!isCollected || unFavor) {
        appendAction(IdAddToFavorites,
                     tr("Add to my favorite"), ss("Add to my favorite"));
    }

    m_menu->addSeparator();
    /**************************************************************************/
    if (canSave) {
        m_menu->addSeparator();
        appendAction(IdRotateClockwise,
                     tr("Rotate clockwise"), ss("Rotate clockwise"));
        appendAction(IdRotateCounterclockwise,
                     tr("Rotate counterclockwise"), ss("Rotate counterclockwise"));
    }
    /**************************************************************************/
    if (paths.length() == 1)  {
        if (canSave) {
            appendAction(IdSetAsWallpaper,
                         tr("Set as wallpaper"), ss("Set as wallpaper"));
        }
        appendAction(IdDisplayInFileManager,
                     tr("Display in file manager"), ss("Display in file manager"));
    }
    appendAction(IdImageInfo, tr("Image info"), ss("Image info"));
}

void TimelinePanel::onMenuItemClicked(QAction *action)
{
    QStringList paths = m_frame->selectedPaths();
    paths.removeAll(QString(""));
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

    const int id = action->property("MenuID").toInt();
    switch (id) {
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
        const QString album = action->data().toString();
        if (album != "Add to new album") {
            dApp->dbM->insertIntoAlbum(album, paths);
        }else {
            dApp->signalM->createAlbum(paths);
        }
        break;
    }
    case IdCopy:
        utils::base::copyImageToClipboard(paths);
        break;
    case IdCopyToClipboard:
        utils::base::copyOneImageToClipboard(dpath);
        break;
    case IdMoveToTrash: {
        FileDeleteDialog *fdd = new FileDeleteDialog(paths);
        fdd->show();
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

    updateMenuContents();
}

void TimelinePanel::rotateImage(const QString &path, int degree)
{
    utils::image::rotate(path, degree);
    utils::image::generateThumbnail(path);
    m_rotateList.removeAll(path);
    m_frame->updateThumbnails(path);
    if (m_rotateList.isEmpty()) {
        qDebug() << "Rotate finish!";
        m_frame->updateScrollRange();
    }
}
