#include "viewpanel.h"
#include "imageinfowidget.h"
#include "contents/ttmcontent.h"
#include "contents/ttlcontent.h"
#include "slideeffect/slideeffectplayer.h"
#include "controller/signalmanager.h"
#include "controller/popupmenumanager.h"
#include "controller/wallpapersetter.h"
#include "controller/divdbuscontroller.h"
#include "controller/databasemanager.h"
#include "controller/exporter.h"
#include "widgets/imagebutton.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
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
      m_fromFileManager(true),
      m_popupMenu(new PopupMenuManager(this)),
      m_sManager(SignalManager::instance())
{
    initStack();
    initSlider();
    initSliderEffectPlay();
    initNavigation();
    initConnect();
    initShortcut();
    initStyleSheet();
    setMouseTracking(true);

    setAcceptDrops(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    updateMenuContent();
    installEventFilter(this);
}

void ViewPanel::initConnect() {
    connect(this, &ViewPanel::customContextMenuRequested, this, [=] {
        updateMenuContent();
        m_popupMenu->showMenu();
    });
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &ViewPanel::onMenuItemClicked);

    connect(m_sManager, &SignalManager::gotoPanel,
            this, [=] (ModulePanel *p){
        if (p != this) {
            showToolbar(true);
            showToolbar(false);
        }
        else {
            emit m_sManager->hideBottomToolbar(true);
        }
        m_slide->stop();
    });
    connect(m_sManager, &SignalManager::viewImage,
    [this](const QString &path, const QString &album, bool fromFileManager ) {
        if (fromFileManager) {
            m_infos = getImageInfos(getFileInfos(path));
        }
        else {
            DatabaseManager::ImageInfo info =
                    dbManager()->getImageInfoByPath(path);
            if (album.isEmpty()) {
                m_infos = dbManager()->getImageInfosByTime(info.time);
            }
            else {
                m_infos = dbManager()->getImageInfosByAlbum(album);
            }
        }

        m_albumName = album;
        m_current = m_infos.cbegin();
        for (; m_current != m_infos.cend(); m_current ++) {
            if (m_current->path == path) {
                openImage(path, fromFileManager);
                return;
            }
        }

        // Not exist in DB, it must from FileManager
        m_current = m_infos.cbegin();
        openImage(path, fromFileManager);
    });
    connect(m_sManager, &SignalManager::fullScreen,
            this, &ViewPanel::toggleFullScreen);
    connect(m_sManager, &SignalManager::startSlideShow,
            this, &ViewPanel::toggleSlideShow);
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
        window()->showNormal();
        if (m_slide->isRunning()) {
            m_slide->stop();
            showToolbar(true);
        }
        else {
            Q_EMIT m_sManager->backToMainWindow();
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
        showNormal();
        return;
    }

    emit m_sManager->hideTopToolbar(false);
//    emit m_sManager->hideBottomToolbar(false);
    QStringList paths;
    for (const DatabaseManager::ImageInfo& info : m_infos) {
        paths << info.path;
    }
    showFullScreen();
    m_slide->setImagePaths(paths);
    m_slide->setCurrentImage(m_current->path);
    m_slide->start();
}

void ViewPanel::showToolbar(bool isTop)
{
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, this, [=] {
       if (isTop)
           emit m_sManager->showTopToolbar();
//       else
//           emit m_sManager->showBottomToolbar();
    });
    connect(t, &QTimer::timeout, t, &QTimer::deleteLater);
    t->start(SHOW_TOOLBAR_INTERVAL);
}

void ViewPanel::showNormal()
{
    window()->showNormal();
    showToolbar(true);
//    showToolbar(false);
}

void ViewPanel::showFullScreen()
{
    // Full screen then hide bars because hide animation depends on height()
    window()->showFullScreen();
    window()->setFixedSize(qApp->desktop()->screenGeometry().size());

    Q_EMIT m_sManager->hideExtensionPanel();
    Q_EMIT m_sManager->hideTopToolbar(true);
//    Q_EMIT m_sManager->hideBottomToolbar(true);
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

QList<DatabaseManager::ImageInfo> ViewPanel::getImageInfos(
        const QFileInfoList &infos)
{
    QList<DatabaseManager::ImageInfo> imageInfos;
    for (int i = 0; i < infos.length(); i++) {
        DatabaseManager::ImageInfo imgInfo;
        imgInfo.name = infos.at(i).fileName();
        imgInfo.path = infos.at(i).absoluteFilePath();
        imgInfo.time = utils::image::getCreateDateTime(imgInfo.path);
        imgInfo.albums = QStringList();
        imgInfo.labels = QStringList();
        imgInfo.thumbnail = utils::image::getThumbnail(imgInfo.path);

        imageInfos << imgInfo;
    }

    return imageInfos;
}

QFileInfoList ViewPanel::getFileInfos(const QString &path)
{
    QFileInfo pinfo(path);
    QDir dir = pinfo.isDir() ? QDir(path) : pinfo.dir();
    QStringList ol = utils::image::supportImageTypes();
    QStringList nl;
    for (QString type : ol) {
        nl << "*." + type;
    }

    return dir.entryInfoList(nl, QDir::Files);
}

DatabaseManager *ViewPanel::dbManager() const
{
    // Use database will cause db file lock. There is no need to access the db
    // if view image from file-manager
    return DatabaseManager::instance();
}

QWidget *ViewPanel::toolbarBottomContent()
{
    return nullptr;
}

QWidget *ViewPanel::toolbarTopLeftContent()
{
    TTLContent::ImageSource source;
    if (m_fromFileManager) {
        source = TTLContent::FromFileManager;
    }
    else if (m_albumName.isEmpty()) {
        source = TTLContent::FromTimeline;
    }
    else {
        source = TTLContent::FromAlbum;
    }
    TTLContent *ttlc = new TTLContent(source);
    connect(ttlc, &TTLContent::clicked, this, [=] (TTLContent::ImageSource s) {
        m_slide->stop();
        if (window()->isFullScreen())
            showNormal();
        switch (s) {
        case TTLContent::FromFileManager:
        case TTLContent::FromTimeline:
            // Use dbus interface to make sure it will always back to the
            // main process
            DIVDBusController().backToMainWindow();
            break;
        case TTLContent::FromAlbum:
            emit m_sManager->gotoAlbumPanel(m_albumName);
            break;
        default:
            break;
        }
    });
    return ttlc;
}

QWidget *ViewPanel::toolbarTopMiddleContent()
{
    TTMContent *ttmc = new TTMContent(m_fromFileManager);
    connect(this, &ViewPanel::updateCollectButton,
            ttmc, &TTMContent::updateCollectButton);
    connect(this, &ViewPanel::imageChanged, ttmc, &TTMContent::onImageChanged);
    connect(ttmc, &TTMContent::showNext, this, &ViewPanel::showNext);
    connect(ttmc, &TTMContent::showPrevious, this, &ViewPanel::showPrevious);
    connect(ttmc, &TTMContent::removed, this, [=] {
        dbManager()->removeImage(m_current->name);
        utils::base::trashFile(m_current->path);
        removeCurrentImage();
    });
    connect(ttmc, &TTMContent::resetTransform, this, [=] (bool fitWindow) {
        m_view->resetTransform();
        if (fitWindow) {
            m_view->setScaleValue(1);
        }
    });
    return ttmc;
}

QWidget *ViewPanel::extensionPanelContent()
{
    QWidget *w = new QWidget;
    w->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout *l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 40);

    m_info = new ImageInfoWidget();
    m_info->setStyleSheet(styleSheet());

    l->addSpacing(TOP_TOOLBAR_HEIGHT);
    l->addWidget(m_info);

    return w;
}

bool ViewPanel::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    if (e->type() == QEvent::Hide) {
        m_view->setImage("");
        m_slide->setImagePaths(QStringList());
    }
    return false;
}

void ViewPanel::resizeEvent(QResizeEvent *e)
{
    m_view->resetTransform();

    m_imageSlider->move(this->rect().right() - m_imageSlider->width() - 20,
        (m_view->height() - m_imageSlider->height()) / 2 + TOP_TOOLBAR_HEIGHT);

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
//    else if (mouseContainsByBottomToolbar(e->pos())) {
//        showToolbar(false);
//    }

    ModulePanel::mouseMoveEvent(e);
}

void ViewPanel::enterEvent(QEvent *e)
{
    // Leave from toolbar and enter inside panel
    Q_UNUSED(e);
    if (m_slide->isRunning() || window()->isFullScreen()) {
//        Q_EMIT m_sManager->hideBottomToolbar();
        Q_EMIT m_sManager->hideTopToolbar();
    }
}

void ViewPanel::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    if (urls.length() == 1) {  // Single file or directory
        const QString path = urls.first().toLocalFile();
        m_infos = getImageInfos(getFileInfos(path));
    }
    else {  // Multi file or directory
        m_infos.clear();
        for (QUrl url : urls) {
            const QString path = url.toLocalFile();
            if (QFileInfo(path).isDir()) {
                m_infos << getImageInfos(getFileInfos(path));
            }
            else {
                if (utils::image::imageIsSupport(path)) {
                    QFileInfoList finfos;
                    finfos << QFileInfo(path);
                    m_infos << getImageInfos(finfos);
                }
            }
        }
    }

    event->accept();
    if (m_infos.isEmpty()) {
        return;
    }

    m_current = m_infos.cbegin();
    m_albumName = "";
    openImage(m_current->path);
}

void ViewPanel::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void ViewPanel::toggleFullScreen()
{
    if (window()->isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

bool ViewPanel::showPrevious()
{
    if (m_slide->isRunning())
        return false;
    m_slide->stop();
    if (m_infos.isEmpty())
        return false;
    if (m_current == m_infos.cbegin())
        m_current = m_infos.cend();
    --m_current;
    openImage(m_current->path, m_fromFileManager);
    return true;
}

bool ViewPanel::showNext()
{
    if (m_slide->isRunning())
        return false;
    m_slide->stop();
    if (m_infos.isEmpty())
        return false;
    if (m_current == m_infos.cend())
        m_current = m_infos.cbegin();
    ++m_current;
    if (m_current == m_infos.cend())
        m_current = m_infos.cbegin();

    openImage(m_current->path, m_fromFileManager);
    return true;
}

void ViewPanel::removeCurrentImage()
{
    const QString name = m_current->name;
    for (int i = 0; i < m_infos.length(); i ++) {
        if (m_infos.at(i).name == name) {
            m_infos.removeAt(i);
        }
    }
    if (! showNext()) {
        if (! showPrevious()) {
            qDebug() << "No images to show!";
            m_stack->setCurrentIndex(1);
        }
    }
}

void ViewPanel::initStack()
{
    m_stack = new QStackedWidget;
    m_stack->setMouseTracking(true);
    m_stack->setContentsMargins(0, 0, 0, 0);

    // View frame
    initViewContent();
    QFrame *vf = new QFrame;
    QVBoxLayout *vl = new QVBoxLayout(vf);
    connect(m_sManager, &SignalManager::showTopToolbar, this, [=] {
        vl->setContentsMargins(0, TOP_TOOLBAR_HEIGHT, 0, 0);
    });
    connect(m_sManager, &SignalManager::hideTopToolbar, this, [=] {
        vl->setContentsMargins(0, 0, 0, 0);
    });
    vl->addWidget(m_view);
    m_stack->addWidget(vf);

    // Empty frame
    QFrame *emptyFrame = new QFrame;
    emptyFrame->setMouseTracking(true);
    emptyFrame->setAttribute(Qt::WA_TranslucentBackground);
    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(":/images/resources/images/empty_box.png"));
    QHBoxLayout *il = new QHBoxLayout(emptyFrame);
    il->setContentsMargins(0, 0, 0, 0);
    il->addWidget(icon, 0, Qt::AlignCenter);
    m_stack->addWidget(emptyFrame);

    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->addWidget(m_stack);
}

QString ViewPanel::createMenuContent()
{
    QJsonArray items;
    if (m_slide->isRunning()) {
        items.append(createMenuItem(IdStartSlideShow, tr("Stop slide show"),
                                    false, "F5"));
    }
    else {
        items.append(createMenuItem(IdFullScreen, tr("Fullscreen"),
                                    false, "F11"));
        items.append(createMenuItem(IdStartSlideShow, tr("Start slide show"),
                                    false, "F5"));

        if (! m_fromFileManager) {
            const QJsonObject objF = createAlbumMenuObj(false);
            if (! objF.isEmpty()) {
                items.append(createMenuItem(IdAddToAlbum, tr("Add to album"),
                                            false, "", objF));
            }
        }

        items.append(createMenuItem(IdSeparator, "", true));

        items.append(createMenuItem(IdExport, tr("Export"), false, ""));
        items.append(createMenuItem(IdCopy, tr("Copy"), false, "Ctrl+C"));
        items.append(createMenuItem(IdMoveToTrash, tr("Move to trash"), false,
                                    "Delete"));
        items.append(createMenuItem(IdRemoveFromTimeline,
                                    tr("Remove from timeline")));
        if (! m_albumName.isEmpty()) {
            items.append(createMenuItem(IdRemoveFromAlbum,
                                        tr("Remove from album")));
        }

        items.append(createMenuItem(IdSeparator, "", true));
        items.append(createMenuItem(IdEdit, tr("Edit"), false, "Ctrl+E"));
        if (! m_fromFileManager) {
            if (!dbManager()->imageExistAlbum(m_current->name,
                                              FAVORITES_ALBUM_NAME)) {
                items.append(createMenuItem(IdAddToFavorites,
                    tr("Add to favorites"), false, "Ctrl+K"));
            } else {
                items.append(createMenuItem(IdRemoveFromFavorites,
                    tr("Remove from favorites"), false, "Ctrl+Shift+K"));
            }
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
        items.append(createMenuItem(IdSetAsWallpaper, tr("Set as wallpaper"),
                                    false, "Ctrl+F8"));
        items.append(createMenuItem(IdDisplayInFileManager,
            tr("Display in file manager"), false, "Ctrl+D"));
        items.append(createMenuItem(IdImageInfo, tr("Image info"), false,
                                    "Alt+Enter"));
    }

    QJsonObject contentObj;
    contentObj["x"] = 0;
    contentObj["y"] = 0;
    contentObj["items"] = QJsonValue(items);

    QJsonDocument document(contentObj);

    return QString(document.toJson());
}

QJsonObject ViewPanel::createAlbumMenuObj(bool isRemove)
{
    if (m_fromFileManager) {
        return QJsonObject();
    }
    const QStringList albums = dbManager()->getAlbumNameList();

    QJsonArray items;
    if (! m_infos.isEmpty()) {
        for (QString album : albums) {
            if (album == FAVORITES_ALBUM_NAME || album == "Recent imported") {
                continue;
            }
            const QStringList names = dbManager()->getImageNamesByAlbum(album);
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
    const QString name = m_current->name;
    const QString path = m_current->path;
    QString albumName = mtl.isEmpty() ? "" : mtl.first();

    switch (MenuItemId(menuId)) {
    case IdFullScreen:
        toggleFullScreen();
        break;
    case IdStartSlideShow:
        toggleSlideShow();
        break;
    case IdAddToAlbum:
        dbManager()->insertImageIntoAlbum(albumName, name,
            utils::base::timeToString(m_current->time));
        break;
    case IdExport:
    {
        QStringList exportFile;
        exportFile << path;
        Exporter::instance()->exportImage(exportFile);
        break;
    }
    case IdCopy:
        utils::base::copyImageToClipboard(QStringList(path));
        break;
    case IdMoveToTrash:
        dbManager()->removeImage(name);
        utils::base::trashFile(path);
        removeCurrentImage();
        break;
    case IdRemoveFromTimeline:
        dbManager()->removeImage(name);
        removeCurrentImage();
        break;
    case IdRemoveFromAlbum:
        dbManager()->removeImageFromAlbum(m_albumName, name);
        m_albumName = "";
        break;
    case IdEdit:
        m_sManager->editImage(m_view->imagePath());
        break;
    case IdAddToFavorites:
        dbManager()->insertImageIntoAlbum(FAVORITES_ALBUM_NAME, name,
            utils::base::timeToString(m_current->time));
        emit updateCollectButton();
        updateMenuContent();
        break;
    case IdRemoveFromFavorites:
        dbManager()->removeImageFromAlbum(FAVORITES_ALBUM_NAME, name);
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
        WallpaperSetter::instance()->setWallpaper(path);
        break;
    case IdDisplayInFileManager:
        emit m_sManager->showInFileManager(path);
        break;
    case IdImageInfo:
        emit m_sManager->showExtensionPanel();
        break;
    default:
        break;
    }
}

void ViewPanel::initSlider() {
    m_imageSlider = new ImageSliderFrame(this);

    m_imageSlider->hide();
    connect(m_view, &ImageWidget::scaleValueChanged, [this](qreal value) {
        m_imageSlider->setCurrentValue(value*100);
    });

    connect(m_imageSlider, &ImageSliderFrame::valueChanged, [this](double perc) {
        m_view->setScaleValue(perc*950/100);
        m_imageSlider->show();
        m_hideSlider->start();
    });

    m_hideSlider = new QTimer(this);
    m_hideSlider->setInterval(2000);
    m_hideSlider->setSingleShot(true);

    connect(m_hideSlider, &QTimer::timeout, [this](){
        m_imageSlider->hide();
    });
}

void ViewPanel::initSliderEffectPlay()
{
    m_slide = new SlideEffectPlayer(this);
    connect(m_slide, &SlideEffectPlayer::stepChanged, [this](int steps){
        m_current += steps;
        if (m_current == m_infos.cend())
            m_current = m_infos.cbegin();
    });
    connect(m_slide, &SlideEffectPlayer::currentImageChanged,
            [this](const QString& path){
        if (! m_nav->isVisible())
            return;
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

    connect(m_view, &ImageWidget::fliped, [this](bool x, bool y) {
        const QTransform t = QTransform().scale(x ? -1 : 1, y ? -1 : 1);
        QImage img = m_view->image().transformed(t);
        utils::image::saveImageWithExif(img, m_current->path, m_current->path, t);
    });
    connect(m_view, &ImageWidget::doubleClicked, [this]() {
        if (! m_slide->isRunning()) {
            this->toggleFullScreen();
            m_imageSlider->hide();
        }
    });
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
    // TODO signal should emit when mainwidget's panel changed
    m_fromFileManager = fromOutside;
    Q_EMIT m_sManager->gotoPanel(this);

    if (fromOutside) {
        Q_EMIT m_sManager->updateTopToolbarLeftContent(toolbarTopLeftContent());
        Q_EMIT m_sManager->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
        Q_EMIT m_sManager->updateExtensionPanelContent(extensionPanelContent());
    }

//    if (! mouseContainsByBottomToolbar(mapFromGlobal(QCursor::pos()))) {
//        Q_EMIT m_sManager->hideBottomToolbar();
//    }
//    Q_EMIT m_sManager->hideTopToolbar();

    m_view->setImage(path);

    m_nav->setImage(m_view->image());

    if (m_info) {
        m_info->setImagePath(path);
    }

    m_stack->setCurrentIndex(0);

    emit imageChanged(m_current->name, m_current->path);
    emit updateCollectButton();
}
