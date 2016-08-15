#include "viewpanel.h"
#include "application.h"
#include "contents/ttmcontent.h"
#include "contents/ttlcontent.h"
#include "controller/divdbuscontroller.h"
#include "controller/databasemanager.h"
#include "controller/exporter.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include "controller/wallpapersetter.h"
#include "imageinfowidget.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
#include "widgets/imagebutton.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QPixmapCache>
#include <QProcess>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QShortcut>
#include <QTimer>

#include <ddialog.h>

using namespace Dtk::Widget;

namespace {

const QString SHORTCUT_SPLIT_FLAG = "@-_-@";
const int TOP_TOOLBAR_HEIGHT = 40;
const int BOTTOM_TOOLBAR_HEIGHT = 24;
const int SHOW_TOOLBAR_INTERVAL = 200;
const int DELAY_VIEW_INTERVAL = 100;
const int BUTTON_PADDING = 15;
const QString FAVORITES_ALBUM_NAME = "My favorites";

}  // namespace

ViewPanel::ViewPanel(QWidget *parent)
    : ModulePanel(parent),
      m_popupMenu(new PopupMenuManager(this))
{
    m_vinfo.inDatabase = false;

    initStack();
    initFloatBtns();
    initNavigation();

    initConnect();
    initShortcut();
    initStyleSheet();
    initFileSystemWatcher();

    setMouseTracking(true);
    setAcceptDrops(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    updateMenuContent();
    installEventFilter(this);
}

QString ViewPanel::moduleName()
{
    return "ViewPanel";
}

void ViewPanel::initConnect() {
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

    connect(dApp->signalM, &SignalManager::gotoPanel,
            this, [=] (ModulePanel *p){
        showToolbar(true);
        if (p != this) {
            showToolbar(false);
        }
        else {
            emit dApp->signalM->hideBottomToolbar(true);
        }
    });
    qRegisterMetaType<SignalManager::ViewInfo>("SignalManager::ViewInfo");
    connect(dApp->signalM, &SignalManager::viewImage,
            this, &ViewPanel::onViewImage);
    connect(dApp->signalM, &SignalManager::removedFromAlbum,
            this, [=] (const QString &album, const QStringList &names) {
        if (! isVisible() || album != m_vinfo.album || m_vinfo.album.isEmpty())
            return;
        for (QString name : names) {
            if (imageIndex(name) == imageIndex(m_current->name))
                removeCurrentImage();
        }
    });

    connect(m_previousBtn, &ImageButton::clicked, this, &ViewPanel::showPrevious);
    connect(m_nextBtn, &ImageButton::clicked, this, &ViewPanel::showNext);
    connect(m_view, &ImageWidget::switchImgBtnVisible, this, [=](bool vi) {
        if (vi) {
            m_previousBtn->show();
            m_nextBtn->show();
        } else {
            m_previousBtn->hide();
            m_nextBtn->hide();
        }
    });

}

void ViewPanel::initFileSystemWatcher()
{
    // Watch the local file changed if it open from file manager
    QFileSystemWatcher *sw = new QFileSystemWatcher(this);
    connect(dApp->signalM, &SignalManager::viewImage,
            this, [=](const SignalManager::ViewInfo &info) {
        sw->removePaths(sw->directories());
        if (! info.inDatabase) {
            sw->addPath(QFileInfo(info.path).dir().absolutePath());
        }
    });
    connect(sw, &QFileSystemWatcher::directoryChanged, this, [=] {
        if (m_current == m_infos.cend())
            return;
        const QString cp = m_current->path;
        m_infos = getImageInfos(getFileInfos(cp));
        m_current = m_infos.cbegin();
        for (; m_current != m_infos.cend(); m_current ++) {
            if (m_current->path == cp) {
                return;
            }
        }
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
            showNormal();
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

void ViewPanel::mousePressEvent(QMouseEvent *e) {
    emit dApp->signalM->hideExtensionPanel();
    if (e->button() == Qt::BackButton) {
        if (window()->isFullScreen()) {
            showNormal();
        }
        else {
            backToLastPanel();
        }
    }
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

void ViewPanel::showToolbar(bool isTop)
{
    TIMER_SINGLESHOT(SHOW_TOOLBAR_INTERVAL, {
    if (isTop && ! window()->isFullScreen())
        emit dApp->signalM->showTopToolbar();
    //       else
    //           emit dApp->signalM->showBottomToolbar();
                     }, this, isTop);
}

void ViewPanel::showNormal()
{
    if (m_isMaximized) {
        window()->showMaximized();
//        // FIXME the window-manager will alway start the growing-
//        // animation(expand from topleft to bottomright) when change the
//        // window's state to Qt::WindowMaximized, so change the flag temporarily
//        // to avoid the animation run
//        auto flags = window()->windowFlags();
//        window()->setWindowFlags(Qt::SplashScreen);
//        window()->show();
//        window()->setWindowFlags(flags);
//        TIMER_SINGLESHOT(50, {window()->showMaximized();}, this)
    }
    else {
        window()->showNormal();
    }

    showToolbar(true);
}

void ViewPanel::showFullScreen()
{
    m_isMaximized = window()->isMaximized();
    window()->showFullScreen();
//    // FIXME the window-manager will alway start the growing-
//    // animation(expand from topleft to bottomright) when change the
//    // window's state to Qt::WindowMaximized, so change the flag temporarily
//    // to avoid the animation run
//    auto flags = window()->windowFlags();
//    window()->setWindowFlags(Qt::SplashScreen);
//    window()->show();
//    window()->setWindowFlags(flags);
//    TIMER_SINGLESHOT(50, {window()->showFullScreen();}, this)

    // Full screen then hide bars because hide animation depends on height()
    TIMER_SINGLESHOT(300,
    {Q_EMIT dApp->signalM->hideExtensionPanel(true);
     Q_EMIT dApp->signalM->hideTopToolbar(true);}, this);
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

int ViewPanel::imageIndex(const QString &name)
{
    for (int i = 0; i < m_infos.length(); i ++) {
        if (m_infos.at(i).name == name) {
            return i;
        }
    }

    return -1;
}

QList<DatabaseManager::ImageInfo> ViewPanel::getImageInfos(
        const QFileInfoList &infos)
{
    QList<DatabaseManager::ImageInfo> imageInfos;
    for (int i = 0; i < infos.length(); i++) {
        DatabaseManager::ImageInfo imgInfo;
        imgInfo.name = infos.at(i).fileName();
        imgInfo.path = infos.at(i).absoluteFilePath();

        imageInfos << imgInfo;
    }

    return imageInfos;
}

const QStringList ViewPanel::paths() const
{
    QStringList list;
    for (DatabaseManager::ImageInfo info : m_infos) {
        list << info.path;
    }
    return list;
}

QFileInfoList ViewPanel::getFileInfos(const QString &path)
{
    return utils::image::getImagesInfo(QFileInfo(path).path(), false);
}

QWidget *ViewPanel::toolbarBottomContent()
{
    return nullptr;
}

QWidget *ViewPanel::toolbarTopLeftContent()
{
    TTLContent *ttlc = new TTLContent(m_vinfo.inDatabase);
    connect(ttlc, &TTLContent::clicked, this, &ViewPanel::backToLastPanel);
    return ttlc;
}

QWidget *ViewPanel::toolbarTopMiddleContent()
{
    TTMContent *ttmc = new TTMContent(! m_vinfo.inDatabase);
    connect(this, &ViewPanel::updateCollectButton,
            ttmc, &TTMContent::updateCollectButton);
    connect(this, &ViewPanel::imageChanged, ttmc, &TTMContent::onImageChanged);
    connect(ttmc, &TTMContent::rotateClockwise, this, [=]{
        rotateImage(true);
    });
    connect(ttmc, &TTMContent::rotateCounterClockwise, this, [=]{
        rotateImage(false);
    });
    connect(ttmc, &TTMContent::removed, this, [=] {
        dApp->databaseM->removeImages(QStringList(m_current->name));
        utils::base::trashFile(m_current->path);
        removeCurrentImage();
    });
    connect(ttmc, &TTMContent::resetTransform, this, [=] (bool fitWindow) {
        m_view->resetTransform();
        if (fitWindow) {
            m_view->setScaleValue(1 / m_view->windowRelativeScale());
        }

        m_scaleLabel->setText(QString("%1%").arg(int(m_view->scaleValue()*100)));
    });

    return ttmc;
}

QWidget *ViewPanel::extensionPanelContent()
{
    QWidget *w = new QWidget;
    w->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout *l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);

    if (! m_info) {
        m_info = new ImageInfoWidget();
        m_info->setStyleSheet(styleSheet());
    }

    l->addSpacing(TOP_TOOLBAR_HEIGHT);
    l->addWidget(m_info);

    return w;
}

bool ViewPanel::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    if (e->type() == QEvent::Hide) {
        m_view->setImage("");
    }
    else if (m_infos.length() > 0 &&  m_current != m_infos.constEnd()
             && e->type() == QEvent::Show) {
        // After slide show
        TIMER_SINGLESHOT(DELAY_VIEW_INTERVAL + 50,
        {openImage(m_current->path, m_vinfo.inDatabase);}, this);
    }
    return false;
}

void ViewPanel::resizeEvent(QResizeEvent *e)
{
    m_nav->move(e->size().width() - m_nav->width() - 60,
                e->size().height() - m_nav->height() -10);

    m_scaleLabel->move((this->rect().width() - m_scaleLabel->width()) / 2,
        (this->rect().height() - m_scaleLabel->height() + TOP_TOOLBAR_HEIGHT) / 2);

    m_previousBtn->move(x() + BUTTON_PADDING,
                        (this->rect().height() - m_previousBtn->height() + TOP_TOOLBAR_HEIGHT) / 2);


    m_nextBtn->move(this->rect().right() - m_nextBtn->width() - BUTTON_PADDING,
                    (this->rect().height() - m_nextBtn->height() + TOP_TOOLBAR_HEIGHT) / 2);
    //FIXME for reset transform after toggle fullscreen etc.
    if (! m_view->imagePath().isEmpty()) {
        m_view->setImage(QString(m_view->imagePath()));
    }
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
    ModulePanel::mouseMoveEvent(e);
}

void ViewPanel::enterEvent(QEvent *e)
{
    // Leave from toolbar and enter inside panel
    Q_UNUSED(e);
    if (window()->isFullScreen()) {
//        Q_EMIT dApp->signalM->hideBottomToolbar();
        Q_EMIT dApp->signalM->hideTopToolbar();
    }
}

void ViewPanel::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    QFileInfoList finfos;
    for (QUrl url : urls) {
        const QString path = url.toLocalFile();
        if (QFileInfo(path).isDir()) {
            finfos <<  utils::image::getImagesInfo(path, false);
        }
        else
            finfos << QFileInfo(path);
    }
    for (QFileInfo info : finfos) {
        if (utils::image::imageIsSupport(info.absoluteFilePath())) {
            viewOnNewProcess(info.absoluteFilePath());
        }
    }

    event->accept();
}

void ViewPanel::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void ViewPanel::onViewImage(const SignalManager::ViewInfo &vinfo)
{
    m_vinfo = vinfo;

    m_nav->setImage(QImage());
    if (vinfo.fullScreen) {
        showFullScreen();
        emit dApp->signalM->hideTopToolbar(true);
    }
    emit dApp->signalM->gotoPanel(this);

    TIMER_SINGLESHOT(DELAY_VIEW_INTERVAL, {

    openImage(vinfo.path, vinfo.inDatabase);

    if (! vinfo.paths.isEmpty()) {
        QFileInfoList list;
        for (QString path : vinfo.paths) {
            list << QFileInfo(path);
        }
        m_infos = getImageInfos(list);
    }
    else {
        if (vinfo.inDatabase) {
            if (vinfo.album.isEmpty()) {
                m_infos = dApp->databaseM->getAllImageInfos();
            }
            else {
                m_infos = dApp->databaseM->getImageInfosByAlbum(vinfo.album);
            }
        }
        else {
            m_infos = getImageInfos(getFileInfos(vinfo.path));
        }
    }

    m_current = m_infos.cbegin();
    for (; m_current != m_infos.cend(); m_current ++) {
        if (m_current->path == vinfo.path) {
            return;
        }
    }

    // Not exist in DB, it must from FileManager
    m_current = m_infos.cbegin();

                         }, this, vinfo)
}

void ViewPanel::toggleFullScreen()
{
    if (window()->isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }

    //FIXME For the position correction after fullscreen changed
    TIMER_SINGLESHOT(1000, {
    // If image's size is smaller than window's size, set to 1:1 size
    m_view->resetTransform();
    if (m_view->windowRelativeScale() > 1 && ! window()->isFullScreen()) {
        m_view->setScaleValue(1 / m_view->windowRelativeScale());
    }
    }, this)
}

bool ViewPanel::showPrevious()
{
    if (m_infos.isEmpty())
        return false;
    if (m_current == m_infos.cbegin())
        m_current = m_infos.cend();
    --m_current;

    openImage(m_current->path, m_vinfo.inDatabase);
    return true;
}

bool ViewPanel::showNext()
{
    if (m_infos.isEmpty())
        return false;
    if (m_current == m_infos.cend())
        m_current = m_infos.cbegin();
    ++m_current;
    if (m_current == m_infos.cend())
        m_current = m_infos.cbegin();

    openImage(m_current->path, m_vinfo.inDatabase);
    return true;
}

void ViewPanel::removeCurrentImage()
{
    if (m_infos.isEmpty())
        return;

    m_infos.removeAt(imageIndex(m_current->name));
    if (! showNext()) {
        if (! showPrevious()) {
            qDebug() << "No images to show!";
            m_nav->hide();
            emit imageChanged("", true);
            m_stack->setCurrentIndex(1);
        }
    }
}

void ViewPanel::viewOnNewProcess(const QString &path)
{
    const QString pro = "deepin-image-viewer";
    const QStringList args(path);
    QProcess * p = new QProcess;
    connect(p, SIGNAL(finished(int)), p, SLOT(deleteLater()));
    p->start(pro, args);
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
    connect(dApp->signalM, &SignalManager::showTopToolbar, this, [=] {
        vl->setContentsMargins(0, TOP_TOOLBAR_HEIGHT, 0, 0);
    });
    connect(dApp->signalM, &SignalManager::hideTopToolbar, this, [=] {
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

void ViewPanel::initFloatBtns() {
    m_previousBtn = new ImageButton(this);
    m_previousBtn->setFixedSize(36, 36);
    m_previousBtn->setToolTip(tr("Previous"));
    m_previousBtn->setObjectName("PreviousButton");
    m_previousBtn->setNormalPic(":/images/resources/images/previous_hover.png");
    m_previousBtn->setHoverPic(":/images/resources/images/previous_hover.png");
    m_previousBtn->setPressPic(":/images/resources/images/previous_press.png");

    m_nextBtn = new ImageButton(this);
    m_nextBtn->setFixedSize(36, 36);
    m_nextBtn->setToolTip(tr("Next"));
    m_nextBtn->setObjectName("NextButton");
    m_nextBtn->setNormalPic(":/images/resources/images/next_hover.png");
    m_nextBtn->setHoverPic(":/images/resources/images/next_hover.png");
    m_nextBtn->setPressPic(":/images/resources/images/next_press.png");

    m_previousBtn->move(x() + BUTTON_PADDING,
                        (this->rect().height() - m_previousBtn->height() + TOP_TOOLBAR_HEIGHT) / 2);
    m_nextBtn->move(this->rect().right() - m_nextBtn->width()/2- BUTTON_PADDING,
                    (this->rect().height() - m_nextBtn->height() + TOP_TOOLBAR_HEIGHT) / 2);

    m_previousBtn->hide();
    m_nextBtn->hide();

    m_scaleLabel = new QLabel(this);
    m_scaleLabel->setObjectName("ScaleLabel");
    m_scaleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_scaleLabel->setFixedSize(66, 32);
    m_scaleLabel->move(this->rect().center());
    m_scaleLabel->setAlignment(Qt::AlignCenter);
    m_scaleLabel->setText(QString("%1%").arg(100));
    m_scaleLabel->hide();
    //hideTime is delay to hide m_scaleLabel
    QTimer *hideTimer = new QTimer(this);
    hideTimer->setInterval(2000);
    hideTimer->setSingleShot(true);

    connect(hideTimer, &QTimer::timeout, [this](){
        m_scaleLabel->hide();
    });

    connect(m_view, &ImageWidget::scaleValueChanged, [this, hideTimer](qreal value) {
        m_scaleLabel->show();
        m_scaleLabel->setText(QString("%1%").arg(int(value*100)));
        hideTimer->start();
    });
}

QString ViewPanel::createMenuContent()
{
    QJsonArray items;


    if (window()->isFullScreen()) {
        items.append(createMenuItem(IdFullScreen, tr("Exit fullscreen"),
                                    false, "F11"));
    }
    else {
        items.append(createMenuItem(IdFullScreen, tr("Fullscreen"),
                                    false, "F11"));
    }

    items.append(createMenuItem(IdStartSlideShow, tr("Start slide show"),
                                false, "F5"));

    if (m_vinfo.inDatabase) {
        const QJsonObject objF = createAlbumMenuObj(false);
        if (! objF.isEmpty()) {
            items.append(createMenuItem(IdAddToAlbum, tr("Add to album"),
                                        false, "", objF));
        }
    }

    items.append(createMenuItem(IdSeparator, "", true));

    if (m_vinfo.inDatabase) {
        items.append(createMenuItem(IdExport, tr("Export"), false, ""));
    }

    items.append(createMenuItem(IdCopy, tr("Copy"), false, "Ctrl+C"));
    items.append(createMenuItem(IdMoveToTrash, tr("Throw to Trash"), false,
                                "Delete"));
    if (! m_vinfo.album.isEmpty()) {
        items.append(createMenuItem(IdRemoveFromAlbum,
            tr("Remove from album"), false, "Shift+Delete"));
    }

    items.append(createMenuItem(IdSeparator, "", true));
    //        items.append(createMenuItem(IdEdit, tr("Edit"), false, "Ctrl+E"));
    if (m_vinfo.inDatabase) {
        if (!dApp->databaseM->imageExistAlbum(m_current->name,
                                          FAVORITES_ALBUM_NAME)) {
            items.append(createMenuItem(IdAddToFavorites,
                                        tr("Add to My favorites"), false, "Ctrl+K"));
        } else {
            items.append(createMenuItem(IdRemoveFromFavorites,
                                        tr("Unfavorite"), false, "Ctrl+Shift+K"));
        }
    }
    items.append(createMenuItem(IdSeparator, "", true));

    if (m_view->scaleValue() > 1 && m_nav->isAlwaysHidden()) {
        items.append(createMenuItem(IdShowNavigationWindow,
                                    tr("Show navigation window")));
    } else if (m_view->scaleValue() > 1 && !m_nav->isAlwaysHidden()) {
        items.append(createMenuItem(IdHideNavigationWindow,
                                    tr("Hide navigation window")));
    }

    items.append(createMenuItem(IdSeparator, "", true));

    items.append(createMenuItem(IdRotateClockwise, tr("Rotate clockwise"),
                                false, "Ctrl+R"));
    items.append(createMenuItem(IdRotateCounterclockwise,
                                tr("Rotate counterclockwise"), false, "Ctrl+Shift+R"));

    items.append(createMenuItem(IdSeparator, "", true));

    //        items.append(createMenuItem(IdLabel, tr("Text tag")));
    items.append(createMenuItem(IdSetAsWallpaper, tr("Set as wallpaper"),
                                false, "Ctrl+F8"));
    if (m_vinfo.inDatabase)
        items.append(createMenuItem(IdDisplayInFileManager,
                                    tr("Display in file manager"), false, "Ctrl+D"));
    items.append(createMenuItem(IdImageInfo, tr("Image info"), false,
                                "Alt+Return"));

    QJsonObject contentObj;
    contentObj["x"] = 0;
    contentObj["y"] = 0;
    contentObj["items"] = QJsonValue(items);

    QJsonDocument document(contentObj);

    return QString(document.toJson());
}

QJsonObject ViewPanel::createAlbumMenuObj(bool isRemove)
{
    if (! m_vinfo.inDatabase) {
        return QJsonObject();
    }
    const QStringList albums = dApp->databaseM->getAlbumNameList();

    QJsonArray items;
    if (! m_infos.isEmpty()) {
        for (QString album : albums) {
            if (album == FAVORITES_ALBUM_NAME || album == "Recent imported") {
                continue;
            }
            const QStringList names = dApp->databaseM->getImageNamesByAlbum(album);
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

void ViewPanel::backToLastPanel()
{
    if (window()->isFullScreen()) {
        showNormal();
    }
    if (m_vinfo.lastPanel) {
        emit dApp->signalM->gotoPanel(m_vinfo.lastPanel);
        emit dApp->signalM->hideExtensionPanel(true);
        emit dApp->signalM->showBottomToolbar();
    }
    else {
        // Use dbus interface to make sure it will always back to the
        // main process
        DIVDBusController().backToMainWindow();
    }
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
    {
        QStringList exportFile;
        exportFile << path;
        dApp->exporter->exportImage(exportFile);
        break;
    }
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
        updateMenuContent();
        break;
    case IdRemoveFromFavorites:
        dApp->databaseM->removeImageFromAlbum(FAVORITES_ALBUM_NAME, name);
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
        rotateImage(true);
        break;
    case IdRotateCounterclockwise:
        rotateImage(false);
        break;
    case IdLabel:
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
}

void ViewPanel::rotateImage(bool clockWise)
{
    if (clockWise)
        m_view->rotateClockWise();
    else
        m_view->rotateCounterclockwise();
    m_nav->setImage(m_view->image());
    // Remove cache force view's delegate reread thumbnail
    QPixmapCache::remove(m_current->name);
    // Update the thumbnail for in DB
    if (m_vinfo.inDatabase) {
        dApp->databaseM->updateThumbnail(m_current->name);
    }
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
        toggleFullScreen();
        m_scaleLabel->hide();
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
        if (!m_nav->isAlwaysHidden()) {
            m_nav->setVisible(m_view->scaleValue() > 1);
        }
        m_nav->setRectInImage(m_view->visibleImageRect());
    });
}

void ViewPanel::openImage(const QString &path, bool inDB)
{
    if (! QFileInfo(path).exists()) {
        removeCurrentImage();
        return;
    }

    m_view->setImage(path);

    // If image's size is smaller than window's size, set to 1:1 size
    if (m_view->windowRelativeScale() > 1 && ! window()->isFullScreen()) {
        m_view->setScaleValue(1 / m_view->windowRelativeScale());
    }
    m_scaleLabel->hide();

    m_nav->setImage(m_view->image());

    m_scaleLabel->setText(QString("%1%").arg(int(m_view->scaleValue()*100)));

    if (m_info) {
        m_info->setImagePath(path);
    }

    m_stack->setCurrentIndex(0);

    emit imageChanged(m_view->imagePath(), m_view->scaleValue() == 1);
    if (! inDB) {
        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
        emit dApp->signalM->updateExtensionPanelContent(extensionPanelContent());
        emit dApp->signalM->showTopToolbar();
        emit dApp->signalM->hideBottomToolbar(true);
    }
    else {
        emit updateCollectButton();
    }
}
