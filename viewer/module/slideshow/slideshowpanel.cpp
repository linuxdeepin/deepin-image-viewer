#include "slideshowpanel.h"
#include "slideeffectplayer.h"
#include "application.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include "module/view/viewpanel.h"
#include "utils/baseutils.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QPainter>
#include <QResizeEvent>
#include <QShortcut>

namespace {

const int DELAY_START_INTERVAL = 500;
const int DELAY_HIDE_CURSOR_INTERVAL = 3000;

}  // namespace

SlideShowPanel::SlideShowPanel(QWidget *parent)
    : ModulePanel(parent)
    , m_hideCursorTid(0)
    , m_startTid(0)
    , m_menu(new PopupMenuManager(this))
{
    initeffectPlay();
    initMenu();
    initShortcut();

    connect(dApp->signalM, &SignalManager::startSlideShow,
            this, &SlideShowPanel::startSlideShow);
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
    p.setRenderHint(QPainter::Antialiasing);
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
    m_menu = new PopupMenuManager(this);
    m_menu->setMenuContent(menuContent());
    connect(m_menu, &PopupMenuManager::menuItemClicked,
            this, &SlideShowPanel::backToLastPanel);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &SlideShowPanel::customContextMenuRequested,
            m_menu, &PopupMenuManager::showMenu);
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
             !m_menu->menuIsVisible()) {
        dApp->setOverrideCursor(Qt::BlankCursor);
    }

    ModulePanel::timerEvent(event);
}

const QString SlideShowPanel::menuContent()
{
    QJsonArray items;
    items.append(QJsonValue(m_menu->createItemObj(10000,
                                                  tr("Stop slide show"),
                                                  false,
                                                  "F5",
                                                  QJsonObject())));
    QJsonObject contentObj;
    contentObj["x"] = 0;
    contentObj["y"] = 0;
    contentObj["items"] = QJsonValue(items);

    QJsonDocument document(contentObj);

    return QString(document.toJson());
}

void SlideShowPanel::setImage(const QImage &img)
{
    m_img = img;
    update();
}

void SlideShowPanel::startSlideShow(const SignalManager::ViewInfo &vinfo)
{
    if (vinfo.paths.isEmpty()) {
        qDebug() << "Start SlideShow failed! Paths is empty!";
        return;
    }

    m_vinfo = vinfo;
    m_player->setImagePaths(vinfo.paths);
    m_player->setCurrentImage(vinfo.path);

    m_startTid = startTimer(DELAY_START_INTERVAL);
    if (!m_menu->menuIsVisible())
        dApp->setOverrideCursor(Qt::BlankCursor);
    m_hideCursorTid = startTimer(DELAY_HIDE_CURSOR_INTERVAL);
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
    painter.fillRect(0, 0, dww, dwh, QColor(27, 27, 27));
    painter.drawImage(target, image, source);

    return ti;
}
