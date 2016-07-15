#include "slideshowpanel.h"
#include "slideeffectplayer.h"
#include "controller/popupmenumanager.h"
#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include <QDebug>
#include <QPainter>
#include <QResizeEvent>
#include <QShortcut>

SlideShowPanel::SlideShowPanel(QWidget *parent)
    : ModulePanel(parent),
      m_menu(new PopupMenuManager(this)),
      m_sManager(SignalManager::instance())
{
    initeffectPlay();
    initMenu();
    initShortcut();

    connect(m_sManager, &SignalManager::startSlideShow,
            this, &SlideShowPanel::startSlideShow);
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
        emit m_sManager->gotoPanel(m_lastPanel);
    }
    else {
        emit m_sManager->backToMainWindow();
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
    QShortcut *sc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, &SlideShowPanel::backToLastPanel);
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

    emit m_sManager->gotoPanel(this);
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
//    window()->resize(qApp->desktop()->screenGeometry().size());


    TIMER_SINGLESHOT(300, {
    emit m_sManager->hideBottomToolbar(true);
    emit m_sManager->hideExtensionPanel(true);
    emit m_sManager->hideTopToolbar(true);
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
