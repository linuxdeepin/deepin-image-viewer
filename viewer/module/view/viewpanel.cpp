#include "viewpanel.h"
#include "application.h"
#include "navigationwidget.h"
#include "controller/divdbuscontroller.h"
#include "controller/signalmanager.h"
#include "contents/imageinfowidget.h"
#include "contents/ttmcontent.h"
#include "contents/ttlcontent.h"
#include "scen/imageview.h"
#include "utils/imageutils.h"
#include "utils/baseutils.h"
#include "widgets/imagebutton.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QProxyStyle>
#include <QFileSystemWatcher>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMenu>
#include <QPixmapCache>
#include <QProcess>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QtConcurrent>

#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>

using namespace Dtk::Widget;

namespace {

const int TOP_TOOLBAR_HEIGHT = 39;
const int DELAY_HIDE_CURSOR_INTERVAL = 3000;
//const QSize ICON_SIZE = QSize(48, 40);

}  // namespace

ViewPanel::ViewPanel(QWidget *parent)
    : ModulePanel(parent)
    , m_isInfoShowed(false)
    , m_hideCursorTid(0)
    , m_viewB(nullptr)
    , m_info(nullptr)
    , m_stack(nullptr)
{
    m_vinfo.inDatabase = false;
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    initStack();
    initFloatingComponent();

    initConnect();
    initShortcut();
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
            emit dApp->signalM->showTopToolbar();
            emit dApp->signalM->hideBottomToolbar(true);
        }
    });
    connect(dApp->signalM, &SignalManager::showExtensionPanel, this, [=] {
        m_isInfoShowed = true;
    });
    connect(dApp->signalM, &SignalManager::hideExtensionPanel, this, [=] {
        m_isInfoShowed = false;
    });

    qRegisterMetaType<SignalManager::ViewInfo>("SignalManager::ViewInfo");
    connect(dApp->signalM, &SignalManager::viewImage,
            this, [=](const SignalManager::ViewInfo &vinfo){

        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());

        onViewImage(vinfo);
        if (NULL == vinfo.lastPanel) {
            return;
        } else if (vinfo.lastPanel->moduleName() == "AlbumPanel" ||
                vinfo.lastPanel->moduleName() == "ViewPanel") {
            m_currentImageLastDir = vinfo.album;
            emit viewImageFrom(vinfo.album);
        } else if (vinfo.lastPanel->moduleName() == "TimelinePanel") {
            m_currentImageLastDir = tr("Timeline");
            emit viewImageFrom(tr("Timeline"));
        }
        //TODO: there will be some others panel
    });

    connect(dApp->signalM, &SignalManager::removedFromAlbum,
            this, [=] (const QString &album, const QStringList &paths) {
        if (! isVisible() || album != m_vinfo.album || m_vinfo.album.isEmpty())
            return;
        for (QString path : paths) {
            if (imageIndex(path) == imageIndex(m_current->filePath))
                removeCurrentImage();
        }
    });
    connect(dApp->signalM, &SignalManager::imagesRemoved,
            this, [=] (const DBImgInfoList &infos) {
       if (m_infos.length() > 0 && m_infos.cend() != m_current &&
               infos.length() == 1 && infos.first().filePath == m_current->filePath) {
            removeCurrentImage();
       }

       updateMenuContent();
    });
    connect(m_viewB, &ImageView::mouseHoverMoved, this, &ViewPanel::mouseMoved);
    connect(m_emptyWidget, &ThumbnailWidget::mouseHoverMoved, this, &ViewPanel::mouseMoved);
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
            sw->addPath(QFileInfo(info.path).absolutePath());
        }
    });
    connect(sw, &QFileSystemWatcher::directoryChanged, this, [=] {
        if (m_current == m_infos.cend() || m_infos.isEmpty())
            return;
        updateLocalImages();
    });
    connect(sw, &QFileSystemWatcher::fileChanged, this, [=] {
        if (m_current == m_infos.cend() || m_infos.isEmpty())
            return;
        updateLocalImages();
    });
}

void ViewPanel::updateLocalImages() {
    const QString cp = m_current->filePath;
    m_infos = getImageInfos(getFileInfos(cp));
    m_current = m_infos.cbegin();
    for (; m_current != m_infos.cend(); m_current ++) {
        if (m_current->filePath == cp) {
            return;
        }
    }
}
void ViewPanel::mousePressEvent(QMouseEvent *e)
{
    emit dApp->signalM->hideExtensionPanel();
    if (e->button() == Qt::BackButton) {
        if (window()->isFullScreen()) {
            showNormal();
        }
        else {
            backToLastPanel();
        }
    }

    ModulePanel::mousePressEvent(e);
}

void ViewPanel::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        setStyleSheet(utils::base::getFileContent(
                          ":/resources/dark/qss/view.qss"));
    } else {
        setStyleSheet(utils::base::getFileContent(
                          ":/resources/light/qss/view.qss"));
    }
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
    m_hideCursorTid = startTimer(DELAY_HIDE_CURSOR_INTERVAL);
}

int ViewPanel::imageIndex(const QString &path)
{
    for (int i = 0; i < m_infos.length(); i ++) {
        if (m_infos.at(i).filePath == path) {
            return i;
        }
    }

    return -1;
}

DBImgInfoList ViewPanel::getImageInfos(const QFileInfoList &infos)
{
    DBImgInfoList imageInfos;
    for (QFileInfo info : infos) {
        DBImgInfo imgInfo;
        imgInfo.fileName = info.fileName();
        imgInfo.filePath = info.absoluteFilePath();

        imageInfos << imgInfo;
    }

    return imageInfos;
}

const QStringList ViewPanel::paths() const
{
    QStringList list;
    for (DBImgInfo info : m_infos) {
        list << info.filePath;
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

    ttlc->setFixedWidth((window()->width() - 48*6)/2);

    ttlc->setCurrentDir(m_currentImageLastDir);
    connect(ttlc, &TTLContent::clicked, this, &ViewPanel::backToLastPanel);
    connect(this, &ViewPanel::viewImageFrom, ttlc, &TTLContent::setCurrentDir);
    return ttlc;
}

QWidget *ViewPanel::toolbarTopMiddleContent()
{
    TTMContent *ttmc = new TTMContent(! m_vinfo.inDatabase);
    if (! m_infos.isEmpty() && m_current != m_infos.constEnd()) {
        ttmc->setImage(m_current->filePath);
    }
    else {
        ttmc->setImage("");
    }

    connect(this, &ViewPanel::updateCollectButton,
            ttmc, &TTMContent::updateCollectButton);
    connect(this, &ViewPanel::imageChanged, ttmc, &TTMContent::setImage);
    connect(ttmc, &TTMContent::rotateClockwise, this, [=]{
        rotateImage(true);
    });
    connect(ttmc, &TTMContent::rotateCounterClockwise, this, [=]{
        rotateImage(false);
    });
    connect(ttmc, &TTMContent::removed, this, [=] {
        if (m_vinfo.inDatabase) {
            popupDelDialog(m_current->filePath);
        } else {
            const QString path = m_current->filePath;
            removeCurrentImage();
            utils::base::trashFile(path);
        }
    });
    connect(ttmc, &TTMContent::resetTransform, this, [=] (bool fitWindow) {
        if (fitWindow) {
            m_viewB->fitWindow();
        } else {
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
        m_info = new ImageInfoWidget(":/resources/dark/qss/view.qss",
                                     ":/resources/light/qss/view.qss");
    }

    l->addSpacing(0);
    l->addWidget(m_info);

    return w;
}

const SignalManager::ViewInfo ViewPanel::viewInfo() const
{
    return m_vinfo;
}

bool ViewPanel::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    if (e->type() == QEvent::Hide) {
        m_viewB->clear();
    }

    if (e->type() == QEvent::Resize && this->isVisible()) {
        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
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

    if (m_viewB->isFitWindow() || m_viewB->isFitImage()) {
        resetImageGeometry();
    }
}

void ViewPanel::timerEvent(QTimerEvent *e)
{

    if (e->timerId() == m_hideCursorTid &&
            !m_menu->isVisible() && !m_printDialogVisible) {
        dApp->setOverrideCursor(Qt::BlankCursor);
    }

    ModulePanel::timerEvent(e);
}

void ViewPanel::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    using namespace utils::image;
    QStringList paths;
    for (QUrl url : urls) {
        const QString path = url.toLocalFile();
        if (QFileInfo(path).isDir()) {
            auto finfos =  getImagesInfo(path, false);
            for (auto finfo : finfos) {
                if (imageSupportRead(finfo.absoluteFilePath()))
                    paths << finfo.absoluteFilePath();
            }
        }
        else if (imageSupportRead(path)) {
            paths << path;
        }
    }

    if (! paths.isEmpty())
        viewOnNewProcess(paths);

    event->accept();
    ModulePanel::dropEvent(event);
}

void ViewPanel::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
    ModulePanel::dragEnterEvent(event);
}

void ViewPanel::onViewImage(const SignalManager::ViewInfo &vinfo)
{
    using namespace utils::base;
    m_vinfo = vinfo;

    if (vinfo.fullScreen) {
        showFullScreen();
    }
    emit dApp->signalM->gotoPanel(this);

    // The control buttons is diffrence
    if (! vinfo.inDatabase) {
        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
    }

    // Get view range
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
                m_infos = dApp->dbM->getAllInfos();
            }
            else {
                m_infos = dApp->dbM->getInfosByAlbum(vinfo.album);
            }
        }
        else {
            m_infos = getImageInfos(getFileInfos(vinfo.path));
        }
    }

    // Get the image which need to open currently
    m_current = m_infos.cbegin();
    if (! vinfo.path.isEmpty()) {
        for (; m_current != m_infos.cend(); m_current ++) {
            if (m_current->filePath == vinfo.path) {
                break;
            }
        }
    }

    if (m_current == m_infos.cend()) {
        qWarning() << "The specify path not in view range: "
                   << vinfo.path << vinfo.paths;
        return;
    }
    openImage(m_current->filePath);
}

void ViewPanel::toggleFullScreen()
{
    if (window()->isFullScreen()) {
        showNormal();
        killTimer(m_hideCursorTid);
        m_hideCursorTid = 0;
        dApp->setOverrideCursor(Qt::OpenHandCursor);
    } else {
        showFullScreen();
        if (!m_menu->isVisible())
            dApp->setOverrideCursor(Qt::BlankCursor);
        m_hideCursorTid = startTimer(DELAY_HIDE_CURSOR_INTERVAL);
    }
}

bool ViewPanel::showPrevious()
{
    if (m_infos.isEmpty())
        return false;
    if (m_current == m_infos.cbegin())
        m_current = m_infos.cend();
    --m_current;

    openImage(m_current->filePath, m_vinfo.inDatabase);
    return true;
}

bool ViewPanel::showNext()
{
    if (m_infos.isEmpty())
        return false;
    if (m_current == m_infos.cend()) {
        m_current = m_infos.cbegin();
    }
    ++m_current;
    if (m_current == m_infos.cend())
        m_current = m_infos.cbegin();

    openImage(m_current->filePath, m_vinfo.inDatabase);
    return true;
}

void ViewPanel::removeCurrentImage()
{
    if (m_infos.isEmpty())
         return;

    m_infos.removeAt(imageIndex(m_current->filePath));
    if (! showNext()) {
        if (! showPrevious()) {
            qDebug() << "No images to show!";
            m_current = m_infos.cend();
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

void ViewPanel::viewOnNewProcess(const QStringList &paths)
{
    const QString pro = "deepin-image-viewer";
    QProcess * p = new QProcess;
    connect(p, SIGNAL(finished(int)), p, SLOT(deleteLater()));

    QStringList options;
    for (QString path : paths) {
        options << "-o" << path;
    }
    p->start(pro, options);
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
    m_emptyWidget = new ThumbnailWidget(":/resources/dark/qss/thumbnailwidget.qss",
                                        ":/resources/light/qss/thumbnailwidget.qss");

    m_stack->addWidget(m_viewB);
    m_stack->addWidget(m_emptyWidget);
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
    m_info->updateInfo();

    emit imageChanged(m_current->filePath);
}

void ViewPanel::initViewContent()
{
    m_viewB = new ImageView;

    connect(m_viewB, &ImageView::doubleClicked, [this]() {
        toggleFullScreen();
    });
    connect(m_viewB, &ImageView::clicked, this, [=] {
        dApp->signalM->hideExtensionPanel();
    });
    connect(m_viewB, &ImageView::imageChanged, this, [=] (QString path) {
        emit imageChanged(path);
        // Pixmap is cache in thread, make sure the size would correct after
        // cache is finish
        resetImageGeometry();
    });
}

void ViewPanel::openImage(const QString &path, bool inDB)
{
//    if (! QFileInfo(path).exists()) {
        // removeCurrentImage() will cause timerEvent be trigered again by
        // showNext() or showPrevious(), so delay to remove current image
        // to break the loop
//        TIMER_SINGLESHOT(100, {removeCurrentImage();}, this);
//        return;
//    }

    if (inDB) {
        // TODO
        // Check whether the thumbnail is been rotated in outside
//        QtConcurrent::run(utils::image::removeThumbnail, path);
    }

    m_viewB->setImage(path);

    updateMenuContent();
    resetImageGeometry();

    if (m_info) {
        m_info->setImagePath(path);
    }

    if (!QFileInfo(path).exists()) {
        m_emptyWidget->setThumbnailImage(utils::image::getThumbnail(path));
        m_stack->setCurrentIndex(1);
    } else {
        m_stack->setCurrentIndex(0);
    }
    if (inDB) {
        emit updateCollectButton();
    }
}

void  ViewPanel::showPrintDialog(const QStringList &paths) {
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    QPixmap img;

    QPrintDialog* printDialog = new QPrintDialog(&printer, this);
    printDialog->resize(400, 300);
    m_printDialogVisible = true;
    if (printDialog->exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        QRectF drawRectF; QRect wRect;
        QList<QString>::const_iterator i;
        for(i = paths.begin(); i!= paths.end(); ++i){
            if (!img.load(*i)) {
                qDebug() << "img load failed" << *i;
                continue;
            }
            wRect = printer.pageRect();
            if (img.width() > wRect.width() || img.height() > wRect.height()) {
                img = img.scaled(wRect.size(), Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
            }
            drawRectF = QRectF(qreal(wRect.width() - img.width())/2,
                               qreal(wRect.height() - img.height())/2,
                              img.width(), img.height());
            painter.drawPixmap(drawRectF.x(), drawRectF.y(), img.width(),
                               img.height(), img);
            if (i != paths.end() - 1)
                printer.newPage();
        }
        painter.end();
        qDebug() << "print succeed!";
        return;
    }


    QObject::connect(printDialog, &QPrintDialog::finished,  this, [=]{
        printDialog->deleteLater();
        m_printDialogVisible =  false;
    });

    qDebug() << "print failed!";
}
