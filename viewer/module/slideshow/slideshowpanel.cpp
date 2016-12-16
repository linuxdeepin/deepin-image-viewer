#include "slideshowpanel.h"
#include "slideeffectplayer.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/signalmanager.h"
#include "module/view/viewpanel.h"
#include "utils/baseutils.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QMenu>
#include <QPainter>
#include <QResizeEvent>
#include <QShortcut>
#include <QStyleFactory>

namespace {

const int DELAY_START_INTERVAL = 500;
const int DELAY_HIDE_CURSOR_INTERVAL = 3000;

}  // namespace

SlideShowPanel::SlideShowPanel(QWidget *parent)
    : ModulePanel(parent)
    , m_hideCursorTid(0)
    , m_startTid(0)
{
    initeffectPlay();
    initMenu();
    initShortcut();
    initFileSystemMonitor();

    connect(dApp->signalM, &SignalManager::startSlideShow,
            this, &SlideShowPanel::startSlideShow);
    connect(dApp->signalM, &SignalManager::imagesRemoved, [=](
            const DBImgInfoList &infos){
        foreach (DBImgInfo info, infos) {
            if (m_vinfo.paths.contains(info.filePath)) {
                m_vinfo.paths.removeOne(info.filePath);
            }
        }

        m_player->setImagePaths(m_vinfo.paths);
    });
}

QString SlideShowPanel::moduleName()
{
    return "SlideshowPanel";
}

QWidget *SlideShowPanel::toolbarBottomContent()
{
    return NULL;
}

QWidget *SlideShowPanel::toolbarTopLeftContent()
{
    return NULL;
}

QWidget *SlideShowPanel::toolbarTopMiddleContent()
{
    return NULL;
}

QWidget *SlideShowPanel::extensionPanelContent()
{
    return NULL;
}

void SlideShowPanel::paintEvent(QPaintEvent *e)
{
    ModulePanel::paintEvent(e);

    if (m_img.isNull())
        return;

    QPainter p(this);
//    p.setRenderHint(QPainter::Antialiasing);
    p.drawPixmap(this->rect(), QPixmap::fromImage(m_img));
}

void SlideShowPanel::resizeEvent(QResizeEvent *e)
{
    m_player->setFrameSize(e->size().width(), e->size().height());
    ModulePanel::resizeEvent(e);
}

void SlideShowPanel::backToLastPanel()
{
    m_player->stop();
    showNormal();

    if (m_vinfo.lastPanel) {
        ViewPanel *vp = dynamic_cast<ViewPanel *>(m_vinfo.lastPanel);
        if (vp) {
            m_vinfo.path = m_player->currentImagePath();
            m_vinfo.lastPanel = vp->viewInfo().lastPanel;
            emit dApp->signalM->viewImage(m_vinfo);

            if (m_vinfo.fullScreen) {
                emit dApp->signalM->hideTopToolbar(true);
            }
        }
        else {
            emit dApp->signalM->gotoPanel(m_vinfo.lastPanel);
            emit dApp->signalM->showBottomToolbar();
        }
    }
    else {
        emit dApp->signalM->backToMainPanel();
    }

    // Clear cache
    QImage ti(width(), height(), QImage::Format_ARGB32);
    ti.fill(0);
    setImage(ti);

    dApp->setOverrideCursor(Qt::ArrowCursor);
    killTimer(m_hideCursorTid);
    m_hideCursorTid = 0;
}

void SlideShowPanel::initeffectPlay()
{
    m_player = new SlideEffectPlayer(this);
    connect(m_player, &SlideEffectPlayer::frameReady,
            this, &SlideShowPanel::setImage);
}

void SlideShowPanel::initMenu()
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    m_menu = new QMenu;
    m_menu->setStyle(QStyleFactory::create("dlight"));
    QAction *ac = new QAction(m_menu);
    ac->setText(tr("Stop slideshow"));
    ac->setShortcut(QKeySequence(dApp->setter->value("SHORTCUTVIEW", "Start slideshow").toString()));
    m_menu->addAction(ac);
    this->addAction(ac);

    connect(m_menu, &QMenu::triggered, this, &SlideShowPanel::backToLastPanel);
    connect(this, &SlideShowPanel::customContextMenuRequested, this, [=] {
        m_menu->popup(QCursor::pos());
    });
}

void SlideShowPanel::initShortcut()
{
    // Esc
    m_sEsc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    m_sEsc->setContext(Qt::WindowShortcut);
    connect(m_sEsc, &QShortcut::activated, this, &SlideShowPanel::backToLastPanel);
}

void SlideShowPanel::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::BackButton)
        m_sEsc->activated();
}

void SlideShowPanel::initFileSystemMonitor() {
    m_fileSystemMonitor = new QFileSystemWatcher(this);

    connect(m_fileSystemMonitor, &QFileSystemWatcher::fileChanged, [=]
            (const QString&path){
        if (!QFileInfo(path).exists()) {
            if (m_vinfo.paths.contains(path)) {
                m_vinfo.paths.removeOne(path);
                m_fileSystemMonitor->removePath(path);
            }
        }

        m_player->setImagePaths(m_vinfo.paths);
    });
}

void SlideShowPanel::timerEvent(QTimerEvent *event)
{
    // Delay to avoid fast switching causes the system to get stuck
    if (event->timerId() == m_startTid) {
        killTimer(m_startTid);
        m_startTid = 0;

        m_player->start();
        emit dApp->signalM->gotoPanel(this);
        showFullScreen();
    }
    else if (event->timerId() == m_hideCursorTid &&
             !m_menu->isVisible()) {
        dApp->setOverrideCursor(Qt::BlankCursor);
    }

    ModulePanel::timerEvent(event);
}

void SlideShowPanel::setImage(const QImage &img)
{
    m_img = img;
    update();
}

void SlideShowPanel::startSlideShow(const SignalManager::ViewInfo &vinfo,
                                    bool inDB)
{
    if (vinfo.paths.isEmpty()) {
        qDebug() << "Start SlideShow failed! Paths is empty!";
        return;
    }

    m_vinfo = vinfo;
    m_player->setImagePaths(vinfo.paths);
    m_player->setCurrentImage(vinfo.path);

    m_startTid = startTimer(DELAY_START_INTERVAL);
    if (!m_menu->isVisible())
        dApp->setOverrideCursor(Qt::BlankCursor);
    m_hideCursorTid = startTimer(DELAY_HIDE_CURSOR_INTERVAL);

    if (!inDB) {
        qDebug() << "startSlideShow fileMonitor";
        m_fileSystemMonitor->addPaths(m_vinfo.paths);
    }
}

void SlideShowPanel::showNormal()
{
    if (m_isMaximized)
        window()->showMaximized();
    else
        window()->showNormal();
}

void SlideShowPanel::showFullScreen()
{
    m_isMaximized = window()->isMaximized();
    // Full screen then hide bars because hide animation depends on height()
    window()->showFullScreen();

    setImage(getFitImage(m_player->currentImagePath()));
    emit dApp->signalM->hideBottomToolbar(true);
    emit dApp->signalM->hideExtensionPanel(true);
    emit dApp->signalM->hideTopToolbar(true);
}

/*!
 * \brief SlideShowPanel::getFitImage
 * Compound pixmap to fill the blank after started.
 * \param path
 * \return
 */
QImage SlideShowPanel::getFitImage(const QString &path)
{
    QDesktopWidget *dw = dApp->desktop();
    const int dww = dw->screenGeometry(window()).width();
    const int dwh = dw->screenGeometry(window()).height();

    QImage ti(dww, dwh, QImage::Format_ARGB32);
    QImage image(path);
    QRectF source(0.0, 0.0, image.width(), image.height());
    QRectF target;
    if (1.0 * dww / dwh > 1.0 * image.width() / image.height()) {
        const qreal w = 1.0 * image.width() * dwh / image.height();
        target = QRectF((dww - w) / 2, 0.0, w, dwh);
    }
    else {
        const qreal h = 1.0 * image.height() * dww / image.width();
        target = QRectF(0.0, (dwh - h) / 2, dww, h);
    }

    QPainter painter(&ti);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(0, 0, dww, dwh, QColor(27, 27, 27));
    painter.drawImage(target, image, source);

    return ti;
}
