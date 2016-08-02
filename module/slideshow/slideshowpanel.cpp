#include "slideshowpanel.h"
#include "slideeffectplayer.h"
#include "application.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QPainter>
#include <QResizeEvent>
#include <QShortcut>

SlideShowPanel::SlideShowPanel(QWidget *parent)
    : ModulePanel(parent),
      m_menu(new PopupMenuManager(this))
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

    if (m_lastPanel) {
        emit dApp->signalM->gotoPanel(m_lastPanel);
    }
    else {
        emit dApp->signalM->backToMainPanel();
    }

    // Clear cache
    QImage ti(width(), height(), QImage::Format_ARGB32);
    ti.fill(0);
    setImage(ti);
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

void SlideShowPanel::startSlideShow(ModulePanel *lastPanel,
                                    const QStringList &paths,
                                    const QString &path)
{
    if (paths.isEmpty()) {
        qDebug() << "Start SlideShow failed! Paths is empty!";
        return;
    }
    m_lastPanel = lastPanel;

    m_player->setImagePaths(paths);
    m_player->setCurrentImage(path);
    m_player->start();

    emit dApp->signalM->gotoPanel(this);
    showFullScreen();
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

    TIMER_SINGLESHOT(300, {
    emit dApp->signalM->hideBottomToolbar(true);
    emit dApp->signalM->hideExtensionPanel(true);
    emit dApp->signalM->hideTopToolbar(true);
    setImage(getFitImage(m_player->currentImagePath()));

                         }, this)
}

QImage SlideShowPanel::getFitImage(const QString &path)
{
    QImage ti(width(), height(), QImage::Format_ARGB32);

    QImage image(path);
    QRectF source(0.0, 0.0, image.width(), image.height());
    QRectF target;
    if (image.width() > image.height()) {
        const qreal h = 1.0 * image.height() * width() / image.width();
        target = QRectF(0.0, (height() - h) / 2, width(), h);
    }
    else {
        const qreal w = 1.0 * image.width() * height() / image.height();
        target = QRectF((width() - w) / 2, 0.0, w, height());
    }

    QPainter painter(&ti);
    painter.fillRect(0, 0, width(), height(), Qt::black);
    painter.drawImage(target, image, source);

    return ti;
}
