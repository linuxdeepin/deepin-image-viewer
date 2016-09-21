#include "viewpanel.h"
#include "application.h"
#include "navigationwidget.h"
#include "controller/divdbuscontroller.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include "contents/imageinfowidget.h"
#include "contents/ttmcontent.h"
#include "contents/ttlcontent.h"
#include "scen/imageview.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
#include "frame/deletedialog.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QPixmapCache>
#include <QProcess>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QtConcurrent>

using namespace Dtk::Widget;

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;
const int OPEN_IMAGE_DELAY_INTERVAL = 500;

}  // namespace

ViewPanel::ViewPanel(QWidget *parent)
    : ModulePanel(parent)
    , m_viewB(nullptr)
    , m_info(nullptr)
    , m_stack(nullptr)
{
    m_vinfo.inDatabase = false;

    initStack();
    initFloatingComponent();

    initConnect();
    initShortcut();
    initStyleSheet();
    initFileSystemWatcher();

    initPopupMenu();

    setAcceptDrops(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    installEventFilter(this);
}

QString ViewPanel::moduleName()
{
    return "ViewPanel";
}

void ViewPanel::initConnect() {
    connect(dApp->signalM, &SignalManager::gotoPanel,
            this, [=] (ModulePanel *p){
        if (p != this) {
            emit dApp->signalM->showTopToolbar();
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

//    connect(dApp->signalM, &SignalManager::windowStatesChanged,
//            this, [=] (const Qt::WindowStates state) {
//        Q_UNUSED(state);
//    });
}

void ViewPanel::initFileSystemWatcher()
{
    // Watch the local file changed if it open from file manager
    QFileSystemWatcher *sw = new QFileSystemWatcher(this);
    connect(dApp->signalM, &SignalManager::viewImage,
            this, [=](const SignalManager::ViewInfo &info) {
        if (!sw->directories().isEmpty())
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
    setStyleSheet(utils::base::getFileContent(":/qss/resources/qss/view.qss"));
}


void ViewPanel::updateThumbnail(const QString &name)
{
    dApp->databaseM->updateThumbnail(name);
    // For view's thumbnail update
    QPixmapCache::remove(name);
}

void ViewPanel::showNormal()
{
    if (m_isMaximized) {
        window()->showMaximized();
    }
    else {
        window()->showNormal();
    }

    emit dApp->signalM->showTopToolbar();
}

void ViewPanel::showFullScreen()
{
    m_isMaximized = window()->isMaximized();
    window()->showFullScreen();
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
        popupDelDialog(m_current->path, m_current->name);
    });
    connect(ttmc, &TTMContent::resetTransform, this, [=] (bool fitWindow) {
        if (fitWindow) {
            m_viewB->fitWindow();
        }
        else {
            m_viewB->fitImage();
        }
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
        m_viewB->setImage("");
    }
    else if (m_infos.length() > 0
             &&  m_current != m_infos.constEnd()
             && e->type() == QEvent::Show) {
        // After slide show
        m_openTid = startTimer(m_openTid == 0 ? 0 : OPEN_IMAGE_DELAY_INTERVAL);
    }

    return false;
}

void ViewPanel::resizeEvent(QResizeEvent *e)
{
    ModulePanel::resizeEvent(e);

    // There will be several times the size change during switch to full process
    // So correct it every times
    if (window()->isFullScreen()) {
        Q_EMIT dApp->signalM->hideExtensionPanel(true);
        Q_EMIT dApp->signalM->hideTopToolbar(true);
    }

    if (m_viewB->isFitWindow()) {
        resetImageGeometry();
    }
}

void ViewPanel::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != m_openTid ||
            m_infos.isEmpty() ||
            m_current == m_infos.cend()) {
        return;
    }

    openImage(m_current->path, m_vinfo.inDatabase);
    killTimer(m_openTid);
    m_openTid = 0;
}

void ViewPanel::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    using namespace utils::image;
    QString paths;
    for (QUrl url : urls) {
        const QString path = url.toLocalFile();
        if (QFileInfo(path).isDir()) {
            auto finfos =  getImagesInfo(path, false);
            for (auto finfo : finfos) {
                if (isImageSupported(finfo.absoluteFilePath()))
                    paths += finfo.absoluteFilePath() + ",";
            }
        }
        else if (isImageSupported(path)) {
            paths += path + ",";
        }
    }

    if (! paths.isEmpty())
        viewOnNewProcess(paths);

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

    if (vinfo.fullScreen) {
        showFullScreen();
    }
    emit dApp->signalM->gotoPanel(this);

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
    if (! vinfo.path.isEmpty()) {
        for (; m_current != m_infos.cend(); m_current ++) {
            if (m_current->path == vinfo.path) {
                break;
            }
        }
    }

    m_openTid = startTimer(m_openTid == 0 ? 0 : OPEN_IMAGE_DELAY_INTERVAL);
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
    if (m_infos.isEmpty())
        return false;
    if (m_current == m_infos.cbegin())
        m_current = m_infos.cend();
    --m_current;

    killTimer(m_openTid);
    m_openTid = startTimer(m_openTid == 0 ? 0 : OPEN_IMAGE_DELAY_INTERVAL);
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

    killTimer(m_openTid);
    m_openTid = startTimer(m_openTid == 0 ? 0 : OPEN_IMAGE_DELAY_INTERVAL);
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
            emit imageChanged("");
            m_stack->setCurrentIndex(1);
        }
    }
}

void ViewPanel::resetImageGeometry()
{
    // If image's size is smaller than window's size, set to 1:1 size
    if (m_viewB->windowRelativeScale() > 1) {
        m_viewB->fitImage();
    }
    else {
        m_viewB->fitWindow();
    }
}

void ViewPanel::viewOnNewProcess(const QString &paths)
{
    const QString pro = "deepin-image-viewer";
    QProcess * p = new QProcess;
    connect(p, SIGNAL(finished(int)), p, SLOT(deleteLater()));
    p->start(pro, QStringList() << "--view" << paths);
}

void ViewPanel::initStack()
{
    m_stack = new QStackedWidget;
    m_stack->setMouseTracking(true);
    m_stack->setContentsMargins(0, 0, 0, 0);

    // View frame
    initViewContent();
    QVBoxLayout *vl = new QVBoxLayout(this);
    connect(dApp->signalM, &SignalManager::showTopToolbar, this, [=] {
        vl->setContentsMargins(0, TOP_TOOLBAR_HEIGHT, 0, 0);
    });
    connect(dApp->signalM, &SignalManager::hideTopToolbar, this, [=] {
        vl->setContentsMargins(0, 0, 0, 0);
    });
    vl->addWidget(m_stack);

    // Empty frame
    QFrame *emptyFrame = new QFrame;
    emptyFrame->setMouseTracking(true);
    emptyFrame->setAttribute(Qt::WA_TranslucentBackground);
    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(":/images/resources/images/no_picture.png"));
    QHBoxLayout *il = new QHBoxLayout(emptyFrame);
    il->setContentsMargins(0, 0, 0, 0);
    il->addWidget(icon, 0, Qt::AlignCenter);

    m_stack->addWidget(m_viewB);
    m_stack->addWidget(emptyFrame);
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

void ViewPanel::rotateImage(bool clockWise)
{
    if (clockWise) {
        m_viewB->rotateClockWise();
    }
    else {
        m_viewB->rotateCounterclockwise();
    }

    resetImageGeometry();

    // Remove cache force view's delegate reread thumbnail
    QPixmapCache::remove(m_current->name);
    // Update the thumbnail for in DB
    if (m_vinfo.inDatabase) {
        dApp->databaseM->updateThumbnail(m_current->name);
    }

    emit imageChanged(m_viewB->path());
}

void ViewPanel::initViewContent()
{
    m_viewB = new ImageView;
    connect(m_viewB, &ImageView::doubleClicked, [this]() {
        toggleFullScreen();
    });
}

void ViewPanel::openImage(const QString &path, bool inDB)
{
    if (! QFileInfo(path).exists()) {
        removeCurrentImage();
        return;
    }

    if (inDB) {
        // Check whether the thumbnail is been rotated in outside
        QtConcurrent::run(this, &ViewPanel::updateThumbnail,
                          QFileInfo(path).fileName());
    }

    m_viewB->setImage(path);

    updateMenuContent();
    resetImageGeometry();

    if (m_info) {
        m_info->setImagePath(path);
    }

    m_stack->setCurrentIndex(0);

    emit imageChanged(m_viewB->path());
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
