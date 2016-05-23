#include "viewpanel.h"
#include "imageinfowidget.h"
#include "contents/tbcontent.h"
#include "contents/ttmcontent.h"
#include "contents/ttlcontent.h"
#include "slideeffect/slideeffectplayer.h"
#include "controller/signalmanager.h"
#include "controller/popupmenumanager.h"
#include "controller/wallpapersetter.h"
#include "controller/divdbuscontroller.h"
#include "widgets/imagebutton.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QFileInfo>
#include <QResizeEvent>
#include <QRegularExpression>
#include <QShortcut>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QTimer>
#include <QDebug>

#include <ddialog.h>

using namespace Dtk::Widget;

namespace {

const QString SHORTCUT_SPLIT_FLAG = "@-_-@";
const int TOP_TOOLBAR_HEIGHT = 40;
const int BOTTOM_TOOLBAR_HEIGHT = 24;
const int SHOW_TOOLBAR_INTERVAL = 200;

const QString FAVORITES_ALBUM_NAME = "My favorites";

}  // namespace

ViewPanel::ViewPanel(QWidget *parent)
    : ModulePanel(parent),
      m_popupMenu(new PopupMenuManager(this)),
      m_signalManager(SignalManager::instance()),
      m_dbManager(DatabaseManager::instance())
{
    initViewContent();
    initSlider();
    initNavigation();
    initConnect();
    initShortcut();
    initStyleSheet();
    setMouseTracking(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    updateMenuContent();
}

void ViewPanel::initConnect() {
    connect(this, &ViewPanel::customContextMenuRequested, this, [=] {
        updateMenuContent();
        m_popupMenu->showMenu();
    });
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &ViewPanel::onMenuItemClicked);

    connect(m_signalManager, &SignalManager::gotoPanel,
            this, [=] (ModulePanel *p){
        if (p != this) {
            emit m_signalManager->showTopToolbar();
            emit m_signalManager->showBottomToolbar();
        }
    });
    connect(m_signalManager, &SignalManager::gotoPanel, [this](){
        m_slide->stop();
    });
    connect(m_signalManager, &SignalManager::viewImage,
    [this](const QString &path, const QString &album, bool fromFileManager ) {
        if (fromFileManager) {
            m_infos = readImageInfosFromDir(path);
        }
        else {
            DatabaseManager::ImageInfo info =
                    m_dbManager->getImageInfoByPath(path);
            if (album.isEmpty()) {
                m_infos = m_dbManager->getImageInfosByTime(info.time);
            }
            else {
                m_infos = m_dbManager->getImageInfosByAlbum(album);
            }
        }

        m_current = std::find_if(m_infos.cbegin(), m_infos.cend(),
                                 [&](const DatabaseManager::ImageInfo info){
            return info.path == path;}
        );
        m_albumName = album;
        openImage(path, true);
    });
}

void ViewPanel::initShortcut()
{
    // Previous
    QShortcut *sc = new QShortcut(QKeySequence(Qt::Key_Left), this);
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
        qreal v = m_view->scaleValue() + 0.01;
        m_view->setScaleValue(qMin(v, 1.0));
    });

    // Zoom in
    sc = new QShortcut(QKeySequence(Qt::Key_Down), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        qreal v = m_view->scaleValue() - 0.01;
        m_view->setScaleValue(qMax(v, 0.1));
    });

    // Esc
    sc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        if (m_slide->isRunning()) {
            m_slide->stop();
        }
        else {
            Q_EMIT m_signalManager->backToMainWindow();
        }
    });
}

void ViewPanel::initStyleSheet()
{
    QFile sf(":/qss/resources/qss/view.qss");
    if (sf.open(QIODevice::ReadOnly)) {
        setStyleSheet(sf.readAll());
        sf.close();
    }
    else {
        qDebug() << "Set style sheet fot view panel error!";
    }
}

void ViewPanel::updateMenuContent()
{
    // For update shortcut
    m_popupMenu->setMenuContent(createMenuContent());
}

void ViewPanel::toggleSlideShow()
{
    if (m_slide->isRunning()) {
        m_slide->stop();
        return;
    }
    QStringList paths;
    for (const DatabaseManager::ImageInfo& info : m_infos) {
        paths << info.path;
    }
    m_slide->setImagePaths(paths);
    m_slide->setCurrentImage(m_current->path);
    m_slide->start();
}

void ViewPanel::showToolbar(bool isTop)
{
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, this, [=] {
       if (isTop)
           emit m_signalManager->showTopToolbar();
       else
           emit m_signalManager->showBottomToolbar();
    });
    connect(t, &QTimer::timeout, t, &QTimer::deleteLater);
    t->start(SHOW_TOOLBAR_INTERVAL);
}

bool ViewPanel::mouseContainsByTopToolbar(const QPoint &pos)
{
    const QRect rect(0, 0, width(), TOP_TOOLBAR_HEIGHT);
    return rect.contains(pos);
}

bool ViewPanel::mouseContainsByBottomToolbar(const QPoint &pos)
{
    const QRect rect(0, height() - BOTTOM_TOOLBAR_HEIGHT, width(),
                     BOTTOM_TOOLBAR_HEIGHT);
    return rect.contains(pos);
}

QList<DatabaseManager::ImageInfo> ViewPanel::readImageInfosFromDir(const QString &path)
{
    QDir dir = QFileInfo(path).dir();
    QStringList ol = utils::image::supportImageTypes();
    QStringList nl;
    for (QString type : ol) {
        nl << "*." + type;
    }

    QList<DatabaseManager::ImageInfo> imageInfos;
    const QFileInfoList infos = dir.entryInfoList(nl, QDir::Files);
    for (int i = 0; i < infos.length(); i++) {
        DatabaseManager::ImageInfo imgInfo;
        imgInfo.name = infos.at(i).fileName();
        imgInfo.path = infos.at(i).absoluteFilePath();
        imgInfo.time = utils::image::getCreateDateTime(path);
        imgInfo.albums = QStringList();
        imgInfo.labels = QStringList();
        imgInfo.thumbnail = utils::image::getThumbnail(path);

        imageInfos << imgInfo;
    }

    return imageInfos;
}

QWidget *ViewPanel::toolbarBottomContent()
{
    TBContent *tbc = new TBContent;
    tbc->setStyleSheet(this->styleSheet());
    connect(this, &ViewPanel::updateCollectButton,
            tbc, &TBContent::updateCollectButton);
    connect(this, &ViewPanel::imageChanged, tbc, &TBContent::onImageChanged);
    connect(tbc, &TBContent::popupDeleteDialog,
            this, &ViewPanel::popupDeleteDialog);
    connect(tbc, &TBContent::showNext, this, &ViewPanel::showNext);
    connect(tbc, &TBContent::showPrevious, this, &ViewPanel::showPrevious);
    connect(tbc, &TBContent::toggleSlideShow, this, &ViewPanel::toggleSlideShow);

    return tbc;
}

QWidget *ViewPanel::toolbarTopLeftContent()
{
    TTLContent *ttlc = new TTLContent;
    connect(ttlc, &TTLContent::backToMain, this, [=] {
        m_slide->stop();
        // Use dbus interface to make sure it will always back to the main process
        DIVDBusController().backToMainWindow();
    });
    return ttlc;
}

QWidget *ViewPanel::toolbarTopMiddleContent()
{
    TTMContent *ttmc = new TTMContent;
    connect(ttmc, &TTMContent::resetTransform, this, [=] (bool fitWindow) {
        m_view->resetTransform();
        if (fitWindow) {
            m_view->setScaleValue(1);
        }
    });
    connect(ttmc, &TTMContent::rotateClockWise,
            m_view, &ImageWidget::rotateClockWise);
    connect(ttmc, &TTMContent::rotateCounterclockwise,
            m_view, &ImageWidget::rotateCounterclockwise);
    return ttmc;
}

QWidget *ViewPanel::extensionPanelContent()
{
    m_info = new ImageInfoWidget();
    m_info->setStyleSheet(styleSheet());
    return m_info;
}

void ViewPanel::resizeEvent(QResizeEvent *e)
{
    m_nav->move(e->size().width() - m_nav->width() - 10,
                e->size().height() - m_nav->height() -10);
    m_slide->setFrameSize(e->size().width(), e->size().height());
}

void ViewPanel::mouseMoveEvent(QMouseEvent *e)
{
    if (m_view->isMoving()) {
        ModulePanel::mouseMoveEvent(e);
        return;
    }

    if (mouseContainsByTopToolbar(e->pos())) {
        showToolbar(true);
    }
    else if (mouseContainsByBottomToolbar(e->pos())) {
        showToolbar(false);
    }

    ModulePanel::mouseMoveEvent(e);
}

void ViewPanel::enterEvent(QEvent *e)
{
    // Leave from toolbar and enter inside panel
    Q_UNUSED(e);
    Q_EMIT m_signalManager->hideBottomToolbar();
    Q_EMIT m_signalManager->hideTopToolbar();
}

void ViewPanel::toggleFullScreen()
{
    if (window()->isFullScreen()) {
        window()->showNormal();
    } else {
        // Full screen then hide bars because hide animation depends on height()
        window()->showFullScreen();
        window()->setFixedSize(qApp->desktop()->screenGeometry().size());
        m_view->setFullScreen(window()->size());

        Q_EMIT m_signalManager->hideExtensionPanel();
        Q_EMIT m_signalManager->hideTopToolbar(true);
        Q_EMIT m_signalManager->hideBottomToolbar(true);
    }
}

void ViewPanel::showPrevious()
{
    m_slide->stop();
    if (m_current == m_infos.cbegin())
        return;
    --m_current;
    openImage(m_current->path);
}

void ViewPanel::showNext()
{
    m_slide->stop();
    if (m_current == m_infos.cend())
        return;
    ++m_current;
    if (m_current == m_infos.cend()) {
        --m_current;
        return;
    }
    openImage(m_current->path);
}

void ViewPanel::popupDeleteDialog() {
    DDialog deleteDialog;
    deleteDialog.setWindowFlags(Qt::Dialog | deleteDialog.windowFlags());
    deleteDialog.setWindowModality(Qt::WindowModal);

    deleteDialog.setTitle(QString(tr("Are you sure to delete this image ?")));
    QPixmap imageThumbnail = utils::image::getThumbnail(m_view->imagePath());
    deleteDialog.setIconPixmap(imageThumbnail.scaled(QSize(60, 48),
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    QString message = tr("This operation cannot be restored.");
    deleteDialog.setMessage(message);
    qDebug() << "m_current album:" << m_albumName;
    if (m_albumName.isEmpty()) {
        QStringList delImageFromTimelineBtns;
        delImageFromTimelineBtns << tr("Cancel") << tr("Delete");
        deleteDialog.addButtons(delImageFromTimelineBtns);

        connect(&deleteDialog, &DDialog::buttonClicked, [&](int clickedResult) {
            if (clickedResult == 0) {
                return;
            }

            QString deleteImageName = m_view->imageName();
            m_dbManager->removeImage(deleteImageName);
        });
    } else {
        QStringList delImageFromAlbumBtns;
        delImageFromAlbumBtns << tr("Cancel") << tr("Remove") << tr("Delete");
        deleteDialog.addButtons(delImageFromAlbumBtns);

        connect(&deleteDialog, &DDialog::buttonClicked, [&](int clickedResult) {
            QString imageName = m_view->imageName();
            if (clickedResult == 0) {
                return;
            } else if (clickedResult == 1) {
                m_dbManager->removeImageFromAlbum(m_albumName, imageName);
            } else {
                m_dbManager->removeImage(imageName);
            }
        });
    }

    deleteDialog.exec();
}

QString ViewPanel::createMenuContent()
{
    QJsonArray items;
    items.append(createMenuItem(IdFullScreen, tr("Fullscreen"),
                                false, "F11"));
    items.append(createMenuItem(IdStartSlideShow,
                                m_slide->isRunning() ? tr("Stop slide show")
                                                     : tr("Start slide show"),
                                false, "F5"));
    const QJsonObject objF = createAlbumMenuObj(false);
    if (! objF.isEmpty()) {
        items.append(createMenuItem(IdAddToAlbum, tr("Add to album"),
                                    false, "", objF));
    }

    items.append(createMenuItem(IdSeparator, "", true));

//    items.append(createMenuItem(IdCopy, tr("Export"), false, "Ctrl+C"));
    items.append(createMenuItem(IdCopy, tr("Copy"), false, "Ctrl+C"));
    items.append(createMenuItem(IdDelete, tr("Delete"), false, "Delete"));
    if (! m_albumName.isEmpty()) {
        items.append(createMenuItem(IdRemoveFromAlbum, tr("Remove from album"),
                                    false, "Shift+Delete"));
    }

    items.append(createMenuItem(IdSeparator, "", true));
    items.append(createMenuItem(IdEdit, tr("Edit"), false, "Ctrl+E"));
    //Call on m_current->name will be crashed
    if (!m_dbManager->imageExistAlbum(m_view->imageName(), FAVORITES_ALBUM_NAME)) {
        items.append(createMenuItem(IdAddToFavorites, tr("Add to favorites"),
                                    false, "Ctrl+K"));
    } else {
        items.append(createMenuItem(IdRemoveFromFavorites,
            tr("Remove from favorites"), false, "Ctrl+Shift+K"));
    }
    items.append(createMenuItem(IdSeparator, "", true));

    if (!m_view->isWholeImageVisible() && m_nav->isAlwaysHidden()) {
        items.append(createMenuItem(IdShowNavigationWindow,
            tr("Show navigation window")));
    } else if (!m_view->isWholeImageVisible() && !m_nav->isAlwaysHidden()) {
        items.append(createMenuItem(IdHideNavigationWindow,
            tr("Hide navigation window")));
    }

    items.append(createMenuItem(IdSeparator, "", true));

    items.append(createMenuItem(IdRotateClockwise, tr("Rotate clockwise"),
                                false, "Ctrl+R"));
    items.append(createMenuItem(IdRotateCounterclockwise,
        tr("Rotate counterclockwise"), false, "Ctrl+Shift+R"));

    items.append(createMenuItem(IdSeparator, "", true));

    items.append(createMenuItem(IdLabel, tr("Text tag")));
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

    QJsonDocument document(contentObj);

    return QString(document.toJson());
}

QJsonObject ViewPanel::createAlbumMenuObj(bool isRemove)
{
    const QStringList albums = m_dbManager->getAlbumNameList();

    QJsonArray items;
    if (! m_infos.isEmpty()) {
        for (QString album : albums) {
            if (album == FAVORITES_ALBUM_NAME || album == "Recent imported") {
                continue;
            }
            const QStringList names = m_dbManager->getImageNamesByAlbum(album);
            if (isRemove) {
                if (names.indexOf(m_current->name) != -1) {
                    album = tr("Remove from <<%1>>").arg(album);
                    items.append(createMenuItem(IdRemoveFromAlbum, album));
                }
            }
            else {
                if (names.indexOf(m_current->name) == -1) {
                    items.append(createMenuItem(IdAddToAlbum, album));
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

QJsonValue ViewPanel::createMenuItem(const ViewPanel::MenuItemId id,
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

void ViewPanel::onMenuItemClicked(int menuId, const QString &text)
{
    const QStringList mtl = text.split(SHORTCUT_SPLIT_FLAG);
    QString albumName = mtl.isEmpty() ? "" : mtl.first();

    switch (MenuItemId(menuId)) {
    case IdFullScreen:
        toggleFullScreen();
        break;
    case IdStartSlideShow:
        toggleSlideShow();
        break;
    case IdAddToAlbum:
        m_dbManager->insertImageIntoAlbum(albumName, m_current->name,
            utils::base::timeToString(m_current->time));
        break;
//    case IdExport:
//        break;
    case IdCopy:
        utils::base::copyImageToClipboard(m_current->path);
        break;
    case IdDelete:
        popupDeleteDialog();
        break;
    case IdRemoveFromAlbum:
        m_dbManager->removeImageFromAlbum(m_albumName, m_current->name);
        break;
    case IdEdit:
        m_signalManager->editImage(m_view->imagePath());
        break;
    case IdAddToFavorites:
        m_dbManager->insertImageIntoAlbum(FAVORITES_ALBUM_NAME, m_current->name,
            utils::base::timeToString(m_current->time));
        emit updateCollectButton();
        updateMenuContent();
        break;
    case IdRemoveFromFavorites:
        m_dbManager->removeImageFromAlbum(FAVORITES_ALBUM_NAME, m_current->name);
        emit updateCollectButton();
        updateMenuContent();
        break;
    case IdShowNavigationWindow:
        m_nav->setAlwaysHidden(false);
        updateMenuContent();
        break;
    case IdHideNavigationWindow:
        m_nav->setAlwaysHidden(true);
        updateMenuContent();
        break;
    case IdRotateClockwise:
        m_view->rotateClockWise();
        break;
    case IdRotateCounterclockwise:
        m_view->rotateCounterclockwise();
        break;
    case IdLabel:
        break;
    case IdSetAsWallpaper:
        WallpaperSetter::instance()->setWallpaper(m_view->imagePath());
        break;
    case IdDisplayInFileManager:
        emit m_signalManager->showInFileManager(m_view->imagePath());
        break;
    case IdImageInfo:
        emit m_signalManager->showExtensionPanel();
        break;
    default:
        break;
    }
}

void ViewPanel::initSlider()
{
    m_slide = new SlideEffectPlayer(this);
    connect(m_slide, &SlideEffectPlayer::stepChanged, [this](int steps){
        m_current += steps;
    });
    connect(m_slide, &SlideEffectPlayer::currentImageChanged,
            [this](const QString& path){
        // Slide image size is widget size
        m_nav->setImage(QImage(path).scaled(m_slide->frameSize(),
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation));
        if (m_info) {
            m_info->setImagePath(path);
        }
    });
    connect(m_slide, &SlideEffectPlayer::frameReady,
            [this](const QImage& image) {
        m_view->setImage(image);
    });
}

void ViewPanel::initViewContent()
{
    m_view = new ImageWidget();

    connect(m_view, &ImageWidget::rotated, [this](int degree) {
        const QTransform t = QTransform().rotate(degree);
        QImage img = m_view->image().transformed(t);
        utils::image::saveImageWithExif(img, m_current->path, m_current->path, t);
    });
    connect(m_view, &ImageWidget::fliped, [this](bool x, bool y) {
        const QTransform t = QTransform().scale(x ? -1 : 1, y ? -1 : 1);
        QImage img = m_view->image().transformed(t);
        utils::image::saveImageWithExif(img, m_current->path, m_current->path, t);
    });
    connect(m_view, &ImageWidget::doubleClicked,
            this, &ViewPanel::toggleFullScreen);

    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->addWidget(m_view);
}

void ViewPanel::initNavigation()
{
    m_nav = new NavigationWidget(this);
    m_nav->setVisible(! m_nav->isAlwaysHidden());
    connect(m_nav, &NavigationWidget::requestMove, [this](int x, int y){
        m_view->setImageMove(x, y);
    });
    connect(m_view, &ImageWidget::transformChanged, [this](){
        if (!m_nav->isAlwaysHidden())
            m_nav->setVisible(!m_view->isWholeImageVisible());
        m_nav->setRectInImage(m_view->visibleImageRect());
    });
}

void ViewPanel::openImage(const QString &path, bool fromOutside)
{
    Q_EMIT m_signalManager->gotoPanel(this);
    Q_EMIT m_signalManager->updateBottomToolbarContent(toolbarBottomContent(),
                                                       true);

    if (fromOutside) {
        Q_EMIT m_signalManager->updateTopToolbarLeftContent(toolbarTopLeftContent());
        Q_EMIT m_signalManager->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
        Q_EMIT m_signalManager->updateExtensionPanelContent(extensionPanelContent());
    }

    if (! mouseContainsByBottomToolbar(mapFromGlobal(QCursor::pos()))) {
        Q_EMIT m_signalManager->hideBottomToolbar();
    }
    Q_EMIT m_signalManager->hideTopToolbar();

    m_view->setImage(path);
    m_nav->setImage(m_view->image());

    if (m_info) {
        m_info->setImagePath(path);
    }

    emit imageChanged(m_current->name, m_current->path);
    emit updateCollectButton();
}
