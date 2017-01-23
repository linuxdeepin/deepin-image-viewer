#include "viewpanel.h"
#include "application.h"
#include "contents/imageinfowidget.h"
#include "controller/configsetter.h"
#include "controller/wallpapersetter.h"
#include "navigationwidget.h"
#include "scen/imageview.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/dialogs/filedeletedialog.h"
#include <QMenu>
#include <QKeySequence>
#include <QJsonArray>
#include <QJsonDocument>
#include <QShortcut>
#include <QStyleFactory>

#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>

namespace {

const int SWITCH_IMAGE_DELAY = 300;
const QString SHORTCUTVIEW_GROUP = "SHORTCUTVIEW";
const QString FAVORITES_ALBUM_NAME = "My favorite";

QString ss(const QString &text)
{
    QString str = dApp->setter->value(SHORTCUTVIEW_GROUP, text).toString();
    str.replace(" ", "");
    return str;
}

enum MenuItemId {
    IdFullScreen,
    IdExitFullScreen,
    IdStartSlideShow,
    IdPrint,
    IdAddToAlbum,
    IdCopy,
    IdCopyToClipboard,
    IdMoveToTrash,
    IdRemoveFromTimeline,
    IdRemoveFromAlbum,
    IdAddToFavorites,
    IdRemoveFromFavorites,
    IdShowNavigationWindow,
    IdHideNavigationWindow,
    IdRotateClockwise,
    IdRotateCounterclockwise,
    IdSetAsWallpaper,
    IdDisplayInFileManager,
    IdImageInfo,
    IdSubMenu,
};

}  // namespace

void ViewPanel::initPopupMenu()
{
    m_menu = new QMenu;
    m_menu->setStyle(QStyleFactory::create("dlight"));
    connect(this, &ViewPanel::customContextMenuRequested, this, [=] {
        if (! m_infos.isEmpty()) {
            updateMenuContent();
            dApp->setOverrideCursor(Qt::ArrowCursor);
            m_menu->popup(QCursor::pos());
        }
    });
    connect(m_menu, &QMenu::aboutToHide, this, [=] {
        dApp->restoreOverrideCursor();
    });
    connect(m_menu, &QMenu::triggered, this, &ViewPanel::onMenuItemClicked);
    connect(dApp->setter, &ConfigSetter::valueChanged, this, [=] {
        if (this && this->isVisible()) {
            updateMenuContent();
        }
    });
}

void ViewPanel::appendAction(int id, const QString &text, const QString &shortcut)
{
    QAction *ac = new QAction(m_menu);
    addAction(ac);
    ac->setText(text);
    ac->setProperty("MenuID", id);
    ac->setShortcut(QKeySequence(shortcut));
    m_menu->addAction(ac);
}

QMenu *ViewPanel::createAlbumMenu()
{
    if (m_infos.isEmpty() || m_current == m_infos.constEnd() || ! m_vinfo.inDatabase) {
        return nullptr;
    }

    QMenu *am = new QMenu(tr("Add to album"));
    am->setStyle(QStyleFactory::create("dlight"));
    QStringList albums = dApp->dbM->getAllAlbumNames();
    albums.removeAll(FAVORITES_ALBUM_NAME);

    QAction *ac = new QAction(am);
    ac->setProperty("MenuID", IdAddToAlbum);
    ac->setText(tr("Add to new album"));
    ac->setData(QString("Add to new album"));
    am->addAction(ac);
    am->addSeparator();
    for (QString album : albums) {
        const QStringList paths = dApp->dbM->getPathsByAlbum(album);
        if (! paths.contains(m_current->filePath)) {
            QAction *ac = new QAction(am);
            ac->setProperty("MenuID", IdAddToAlbum);
            ac->setText(fontMetrics().elidedText(QString(album).replace("&", "&&"), Qt::ElideMiddle, 200));
            ac->setData(album);
            am->addAction(ac);
        }
    }

    return am;
}

void ViewPanel::onMenuItemClicked(QAction *action)
{
    using namespace utils::base;
    using namespace utils::image;

    const QString path = m_current->filePath;
    const int id = action->property("MenuID").toInt();

    switch (MenuItemId(id)) {
    case IdFullScreen:
    case IdExitFullScreen:
        toggleFullScreen();
        break;
    case IdStartSlideShow: {
        auto vinfo = m_vinfo;
        vinfo.fullScreen = window()->isFullScreen();
        vinfo.lastPanel = this;
        vinfo.path = path;
        vinfo.paths = paths();
        emit dApp->signalM->startSlideShow(vinfo, m_vinfo.inDatabase);
        break;
    }
    case IdPrint: {
        showPrintDialog(QStringList(path));
        break;
    }
    case IdAddToAlbum: {
        const QString album = action->data().toString();
        if (album != "Add to new album") {
            dApp->dbM->insertIntoAlbum(album, QStringList(path));
        } else {
            dApp->signalM->createAlbum(QStringList(path));
        }
        break;
    }
    case IdCopy:
        copyImageToClipboard(QStringList(path));
        break;
    case IdCopyToClipboard:
        copyOneImageToClipboard(path);
        break;
    case IdMoveToTrash:
        if (m_vinfo.inDatabase) {
            popupDelDialog(path);
        } else {
            removeCurrentImage();
            utils::base::trashFile(path);
        }
        break;
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
        if (m_isInfoShowed)
            emit dApp->signalM->hideExtensionPanel();
        else
            emit dApp->signalM->showExtensionPanel();
        // Update panel info
        m_info->setImagePath(path);
        break;
    default:
        break;
    }

    updateMenuContent();
}

void ViewPanel::updateMenuContent()
{
    m_menu->clear();
    qDeleteAll(this->actions());

    if (m_infos.isEmpty()) {
        return;
    }

    if (window()->isFullScreen()) {
        appendAction(IdExitFullScreen, tr("Exit fullscreen"), ss("Fullscreen"));
    }
    else {
        appendAction(IdFullScreen, tr("Fullscreen"), ss("Fullscreen"));
    }
    appendAction(IdStartSlideShow, tr("Start slideshow"), ss("Start slideshow"));
    appendAction(IdPrint, tr("Print"), ss("Print"));
    if (m_vinfo.inDatabase) {
        QMenu *am = createAlbumMenu();
        if (am) {
            m_menu->addMenu(am);
        }
    }
    m_menu->addSeparator();
    /**************************************************************************/
    appendAction(IdCopy, tr("Copy"), ss("Copy"));
    appendAction(IdCopyToClipboard, tr("Copy to clipboard"), ss("Copy to clipboard"));
    appendAction(IdMoveToTrash, tr("Throw to trash"), ss("Throw to trash"));
    if (! m_vinfo.album.isEmpty()) {
        appendAction(IdRemoveFromAlbum,
                     tr("Remove from album"), ss("Remove from album"));
    }
    m_menu->addSeparator();
    /**************************************************************************/
    if (m_vinfo.inDatabase) {
        if (m_current != m_infos.constEnd() &&
                ! dApp->dbM->isImgExistInAlbum(FAVORITES_ALBUM_NAME,
                                               m_current->filePath)) {
            appendAction(IdAddToFavorites,
                         tr("Add to my favorite"), ss("Add to my favorite"));
        } else {
            appendAction(IdRemoveFromFavorites,
                         tr("Remove from my favorite"),
                         ss("Remove from my favorite"));
        }
    }
    m_menu->addSeparator();
    /**************************************************************************/
    if (! m_viewB->isWholeImageVisible() && m_nav->isAlwaysHidden()) {
        appendAction(IdShowNavigationWindow,
                     tr("Show navigation window"), ss("Show navigation window"));
    }
    else if (! m_viewB->isWholeImageVisible() && !m_nav->isAlwaysHidden()) {
        appendAction(IdHideNavigationWindow,
                     tr("Hide navigation window"), ss("Hide navigation window"));
    }
    /**************************************************************************/
    if (utils::image::imageSupportSave(m_current->filePath)) {
        m_menu->addSeparator();
        appendAction(IdRotateClockwise,
                     tr("Rotate clockwise"), ss("Rotate clockwise"));
        appendAction(IdRotateCounterclockwise,
                     tr("Rotate counterclockwise"), ss("Rotate counterclockwise"));
    }
    /**************************************************************************/
    if (utils::image::imageSupportSave(m_current->filePath))  {
        appendAction(IdSetAsWallpaper,
                     tr("Set as wallpaper"), ss("Set as wallpaper"));
    }
    if (m_vinfo.inDatabase) {
        appendAction(IdDisplayInFileManager,
                     tr("Display in file manager"), ss("Display in file manager"));
    }
    appendAction(IdImageInfo, tr("Image info"), ss("Image info"));
}

void ViewPanel::initShortcut()
{
    // Delay image toggle
    QTimer *dt = new QTimer(this);
    dt->setSingleShot(true);
    dt->setInterval(SWITCH_IMAGE_DELAY);
    // Previous
    QShortcut *sc = new QShortcut(QKeySequence(Qt::Key_Left), this);
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
        emit dApp->signalM->hideExtensionPanel(true);
    });
    //1:1 size
    QShortcut *adaptImage = new QShortcut(QKeySequence("Ctrl+0"), this);
    adaptImage->setContext(Qt::WindowShortcut);
    connect(adaptImage, &QShortcut::activated, this, [=]{
        m_viewB->fitImage();
    });
}

void ViewPanel::popupDelDialog(const QString path) {
    const QStringList paths(path);
    FileDeleteDialog *fdd = new FileDeleteDialog(paths);
    fdd->show();
}
