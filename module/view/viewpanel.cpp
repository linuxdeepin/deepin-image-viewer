#include "viewpanel.h"
#include "imageinfowidget.h"
#include "contents/ttmcontent.h"
#include "contents/ttlcontent.h"
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
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QFileInfo>
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
      m_popupMenu(new PopupMenuManager(this)),
      m_sManager(SignalManager::instance())
{
    m_vinfo.inDatabase = false;

    initStack();
    initNavigation();
    initSlider();

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
        if (m_infos.isEmpty()) {
            m_popupMenu->setMenuContent("");
            return;
        }
        updateMenuContent();
        m_popupMenu->showMenu();
    });
    connect(m_popupMenu, &PopupMenuManager::menuItemClicked,
            this, &ViewPanel::onMenuItemClicked);

    connect(m_sManager, &SignalManager::gotoPanel,
            this, [=] (ModulePanel *p){
        showToolbar(true);
        if (p != this) {
            showToolbar(false);
        }
        else {
            emit m_sManager->hideBottomToolbar(true);
        }
    });
    qRegisterMetaType<SignalManager::ViewInfo>("SignalManager::ViewInfo");
    connect(m_sManager, &SignalManager::viewImage,
            this, &ViewPanel::onViewImage);
    connect(m_sManager, &SignalManager::removeFromAlbum,
            this, [=] (QString album, QString name) {
        if (isVisible()
                && ! m_vinfo.album.isEmpty()
                && album == m_vinfo.album
                && imageIndex(name) == imageIndex(m_current->name))
            removeCurrentImage();
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
        m_view->setScaleValue(qMin(v, 10.0));
    });

    // Zoom in
    sc = new QShortcut(QKeySequence(Qt::Key_Down), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [=] {
        qreal v = m_view->scaleValue() - 0.1;
        m_view->setScaleValue(qMax(v, 0.5));
    });

    // Esc
    m_esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    m_esc->setContext(Qt::WindowShortcut);
    connect(m_esc, &QShortcut::activated, this, [=] {
        if (window()->isFullScreen()) {
            showNormal();
        }
        else {
            backToLastPanel();
        }
    });
}

void ViewPanel::mousePressEvent(QMouseEvent *e) {
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
    if (isTop)
        emit m_sManager->showTopToolbar();
    //       else
    //           emit m_sManager->showBottomToolbar();
                     }, this, isTop);
}

void ViewPanel::showNormal()
{
    if (m_isMaximized)
        window()->showMaximized();
    else
        window()->showNormal();
    showToolbar(true);
//    showToolbar(false);
}

void ViewPanel::showFullScreen()
{
    m_isMaximized = window()->isMaximized();
    // Full screen then hide bars because hide animation depends on height()
    window()->showFullScreen();

    TIMER_SINGLESHOT(300,
    {Q_EMIT m_sManager->hideExtensionPanel(true);
     Q_EMIT m_sManager->hideTopToolbar(true);}, this);
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
//        imgInfo.time = utils::image::getCreateDateTime(imgInfo.path);
//        imgInfo.albums = QStringList();
//        imgInfo.labels = QStringList();
//        imgInfo.thumbnail = utils::image::getThumbnail(imgInfo.path);

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
    TTLContent *ttlc = new TTLContent(m_vinfo.inDatabase);
    connect(ttlc, &TTLContent::clicked, this, &ViewPanel::backToLastPanel);
    return ttlc;
}

QWidget *ViewPanel::toolbarTopMiddleContent()
{
    TTMContent *ttmc = new TTMContent(! m_vinfo.inDatabase);
    ttmc->onImageChanged(m_view->imageName(), m_view->imagePath());
    connect(this, &ViewPanel::updateCollectButton,
            ttmc, &TTMContent::updateCollectButton);
    connect(this, &ViewPanel::imageChanged, ttmc, &TTMContent::onImageChanged);
    connect(ttmc, &TTMContent::rotateClockwise, this, [=]{
        m_view->rotateClockWise();
        m_nav->setImage(m_view->image());
        // Remove cache force view's delegate reread thumbnail
        QPixmapCache::remove(m_current->name);
    });
    connect(ttmc, &TTMContent::rotateCounterClockwise, this, [=]{
        m_view->rotateCounterclockwise();
        m_nav->setImage(m_view->image());
        // Remove cache force view's delegate reread thumbnail
        QPixmapCache::remove(m_current->name);
    });
    connect(ttmc, &TTMContent::removed, this, [=] {
        dbManager()->removeImage(m_current->name);
        utils::base::trashFile(m_current->path);
        removeCurrentImage();
    });
    connect(ttmc, &TTMContent::resetTransform, this, [=] (bool fitWindow) {
        m_view->resetTransform();
        if (fitWindow) {
            m_view->setScaleValue(1 / m_view->windowRelativeScale());
        }

        m_scaleSlider->setCurrentValue(m_view->scaleValue()*100);;
    });
    return ttmc;
}

QWidget *ViewPanel::extensionPanelContent()
{
    QWidget *w = new QWidget;
    w->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout *l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 40);

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

    m_scaleSlider->move(this->rect().right() - m_scaleSlider->width() - 20,
        (this->rect().height() - m_scaleSlider->height() + TOP_TOOLBAR_HEIGHT) / 2);

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
    }
    emit m_sManager->gotoPanel(this);

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
                m_infos = dbManager()->getAllImageInfos();
            }
            else {
                m_infos = dbManager()->getImageInfosByAlbum(vinfo.album);
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
    TIMER_SINGLESHOT(500, {openImage(m_view->imagePath(), m_vinfo.inDatabase);
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
            emit imageChanged("", "");
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
        if (!dbManager()->imageExistAlbum(m_current->name,
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

void ViewPanel::backToLastPanel()
{
    if (window()->isFullScreen()) {
        showNormal();
    }
    if (m_vinfo.lastPanel) {
        emit m_sManager->gotoPanel(m_vinfo.lastPanel);
        emit m_sManager->hideExtensionPanel(true);
        emit m_sManager->showBottomToolbar();
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
        emit m_sManager->startSlideShow(this, paths(), path);
        break;
    case IdAddToAlbum:
        dbManager()->insertImageIntoAlbum(albumName, name, time);
        break;
    case IdExport:
    {
        QStringList exportFile;
        exportFile << path;
        Exporter::instance()->exportImage(exportFile);
        break;
    }
    case IdCopy:
        copyImageToClipboard(QStringList(path));
        break;
    case IdMoveToTrash:
        dbManager()->removeImage(name);
        trashFile(path);
        removeCurrentImage();
        break;
    case IdRemoveFromTimeline:
        dbManager()->removeImage(name);
        removeCurrentImage();
        break;
    case IdRemoveFromAlbum:
        dbManager()->removeImageFromAlbum(m_vinfo.album, name);
        break;
    case IdEdit:
        m_sManager->editImage(m_view->imagePath());
        break;
    case IdAddToFavorites:
        dbManager()->insertImageIntoAlbum(FAVORITES_ALBUM_NAME, name, time);
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
        m_nav->setImage(m_view->image());
        // Remove cache force view's delegate reread thumbnail
        QPixmapCache::remove(name);
        break;
    case IdRotateCounterclockwise:
        m_view->rotateCounterclockwise();
        m_nav->setImage(m_view->image());
        // Remove cache force view's delegate reread thumbnail
        QPixmapCache::remove(name);
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
    QTimer* hideTimer;
    hideTimer = new QTimer(this);
    hideTimer->setInterval(2000);
    hideTimer->setSingleShot(true);

    m_scaleSlider = new ImageSliderFrame(this);
    m_scaleSlider->hide();

    connect(m_view, &ImageWidget::scaleValueChanged, [this](qreal value) {
        m_scaleSlider->setCurrentValue(value*100);
    });

    connect(m_scaleSlider, &ImageSliderFrame::valueChanged, [=](double perc){
        m_view->setScaleValue(perc*950/100);
        m_scaleSlider->show();
        hideTimer->start();

    });

    connect(hideTimer, &QTimer::timeout, [this](){
        m_scaleSlider->hide();
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
        toggleFullScreen();
        m_scaleSlider->hide();
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
    m_nav->setImage(m_view->image());

    m_scaleSlider->setCurrentValue(m_view->scaleValue()*100);

    if (m_info) {
        m_info->setImagePath(path);
    }

    m_stack->setCurrentIndex(0);

    emit imageChanged(m_view->imageName(), m_view->imagePath());
    if (! inDB) {
        emit m_sManager->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit m_sManager->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
        emit m_sManager->updateExtensionPanelContent(extensionPanelContent());
        emit m_sManager->showTopToolbar();
        emit m_sManager->hideBottomToolbar(true);
    }
    else {
        emit updateCollectButton();
    }
}
