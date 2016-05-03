#include "viewpanel.h"
#include <dimagebutton.h>
#include <QBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QMenu>
#include <QDebug>
#include "controller/signalmanager.h"
#include "imageinfowidget.h"
#include <darrowrectangle.h>
#include "utils/imgutil.h"
#include "slideeffect/slideeffectplayer.h"

using namespace Dtk::Widget;

ViewPanel::ViewPanel(QWidget *parent)
    : ModulePanel(parent)
{
    m_slide = new SlideEffectPlayer(this);
    connect(m_slide, &SlideEffectPlayer::stepChanged, [this](int steps){
        m_current += steps;
    });
    connect(m_slide, &SlideEffectPlayer::currentImageChanged, [this](const QString& path){
        m_nav->setImage(QImage(path).scaled(m_slide->frameSize(), Qt::KeepAspectRatio)); //slide image size is widget size
        if (m_info)
            m_info->setImagePath(path);
    });
    connect(m_slide, &SlideEffectPlayer::frameReady, [this](const QImage& image) {
        m_view->setImage(image);
    });
    connect(SignalManager::instance(), &SignalManager::gotoPanel, [this](){
        m_slide->stop();
    });
    connect(SignalManager::instance(), &SignalManager::viewImage, [this](QString path) {
        DatabaseManager::ImageInfo info = DatabaseManager::instance()->getImageInfoByPath(path);
        m_infos = DatabaseManager::instance()->getImageInfosByTime(info.time);
        m_current = std::find_if(m_infos.cbegin(), m_infos.cend(), [&](const DatabaseManager::ImageInfo info){ return info.path == path;});
        openImage(path);
    });
    m_view = new ImageWidget();
    QHBoxLayout *hl = new QHBoxLayout();
    setLayout(hl);
    hl->addWidget(m_view);
    connect(m_view, &ImageWidget::rotated, [this](int degree) {
        const QTransform t = QTransform().rotate(degree);
        QImage img = m_view->image().transformed(t);
        utils::saveImageWithExif(img, m_current->path, m_current->path, t);
    });
    connect(m_view, &ImageWidget::fliped, [this](bool x, bool y) {
        const QTransform t = QTransform().scale(x ? -1 : 1, y ? -1 : 1);
        QImage img = m_view->image().transformed(t);
        utils::saveImageWithExif(img, m_current->path, m_current->path, t);
    });

    m_nav = new NavigationWidget(this);
    connect(m_view, &ImageWidget::transformChanged, [this](){
        // TODO: check user settings
        if (!m_nav->isAlwaysHidden())
            m_nav->setVisible(!m_view->isWholeImageVisible());
        m_nav->setRectInImage(m_view->visibleImageRect());
    });
    connect(m_view, &ImageWidget::doubleClicked, this, &ViewPanel::toggleFullScreen);
    connect(m_nav, &NavigationWidget::requestMove, [this](int x, int y){
        m_view->setImageMove(x, y);
    });
}

QWidget *ViewPanel::toolbarBottomContent()
{
    QWidget *w = new QWidget();
    QHBoxLayout *hb = new QHBoxLayout();
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(10);
    w->setLayout(hb);
    DImageButton *btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/info_normal.png");
    btn->setHoverPic(":/images/resources/images/info_hover.png");
    btn->setPressPic(":/images/resources/images/info_active.png");
    hb->addWidget(btn);
    hb->addStretch();
    connect(btn, &DImageButton::clicked, SignalManager::instance(), &SignalManager::showExtensionPanel);

    btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/collect_normal.png");
    btn->setHoverPic(":/images/resources/images/collect_hover.png");
    btn->setPressPic(":/images/resources/images/collect_active.png");
    hb->addWidget(btn);

    btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/previous_normal.png");
    btn->setHoverPic(":/images/resources/images/previous_hover.png");
    btn->setPressPic(":/images/resources/images/previous_press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, [this]() {
        m_slide->stop();
        if (m_current == m_infos.cbegin())
            return;
        --m_current;
        openImage(m_current->path);
    });

    btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/slideshow_normal.png");
    btn->setHoverPic(":/images/resources/images/slideshow_hover.png");
    btn->setPressPic(":/images/resources/images/slideshow_press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, [this](){
        if (m_slide->isRunning()) {
            m_slide->stop();
            return;
        }
        QStringList paths;
        foreach (const DatabaseManager::ImageInfo& info, m_infos) {
            paths << info.path;
        }
        m_slide->setImagePaths(paths);
        m_slide->setCurrentImage(m_current->path);
        m_slide->start();
    });

    btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/next_normal.png");
    btn->setHoverPic(":/images/resources/images/next_hover.png");
    btn->setPressPic(":/images/resources/images/next_press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, [this]() {
        m_slide->stop();
        if (m_current == m_infos.cend())
            return;
        ++m_current;
        if (m_current == m_infos.cend()) {
            --m_current;
            return;
        }
        openImage(m_current->path);
    });

    btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/edit_normal.png");
    btn->setHoverPic(":/images/resources/images/edit_hover.png");
    btn->setPressPic(":/images/resources/images/edit_press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, [this](){
        Q_EMIT SignalManager::instance()->editImage(m_current->path);
    });

    hb->addStretch();

    btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/delete_normal.png");
    btn->setHoverPic(":/images/resources/images/delete_hover.png");
    btn->setPressPic(":/images/resources/images/delete_press.png");
    hb->addWidget(btn);
    return w;
}

QWidget *ViewPanel::toolbarTopLeftContent()
{
    QWidget *w = new QWidget();
    QHBoxLayout *hb = new QHBoxLayout();
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(0);
    w->setLayout(hb);
    DImageButton *btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/album_normal.png");
    btn->setHoverPic(":/images/resources/images/album_hover.png");
    btn->setPressPic(":/images/resources/images/album_active.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, SignalManager::instance(), &SignalManager::backToMainWindow);
    return w;
}

QWidget *ViewPanel::toolbarTopMiddleContent()
{
    QWidget *w = new QWidget();
    QHBoxLayout *hb = new QHBoxLayout();
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(10);
    w->setLayout(hb);
    hb->addStretch();
    DImageButton *btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/contrarotate_normal.png");
    btn->setHoverPic(":/images/resources/images/contrarotate_hover.png");
    btn->setPressPic(":/images/resources/images/contrarotate_press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, m_view, &ImageWidget::rotateAntiClockWise);

    btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/clockwise_rotation_normal.png");
    btn->setHoverPic(":/images/resources/images/clockwise_rotation_hover.png");
    btn->setPressPic(":/images/resources/images/clockwise_rotation_press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, m_view, &ImageWidget::rotateClockWise);

    btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/adapt_image_normal.png");
    btn->setHoverPic(":/images/resources/images/adapt_image_hover.png");
    btn->setPressPic(":/images/resources/images/adapt_image_active.png");
    btn->setToolTip(tr("1:1 Size"));
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, [this](){
        m_view->resetTransform();
        m_view->setScaleValue(1);
    });
    //
#if 0
    btn = new DImageButton();
    btn->setNormalPic(":/images/resources/images/share_normal.png");
    btn->setHoverPic(":/images/resources/images/share_hover.png");
    btn->setPressPic(":/images/resources/images/share_active.png");
    hb->addWidget(btn);
#endif
    hb->addStretch();
    return w;
}

QWidget *ViewPanel::extensionPanelContent()
{
    m_info = new ImageInfoWidget();
    m_info->setImagePath(m_current->path);
    return m_info;
}

void ViewPanel::resizeEvent(QResizeEvent *e)
{
    m_nav->move(e->size().width() - m_nav->width() - 10, e->size().height() - m_nav->height() -10);
    m_slide->setFrameSize(e->size().width(), e->size().height());
}

void ViewPanel::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu m;
    m.addAction(window()->isFullScreen() ? tr("Exit fullscreen") : tr("Fullscreen"), this, SLOT(toggleFullScreen()));
    m.exec(e->globalPos());
}

void ViewPanel::toggleFullScreen()
{
    if (window()->isFullScreen()) {
        window()->showNormal();
        Q_EMIT SignalManager::instance()->showBottomToolbar();
        Q_EMIT SignalManager::instance()->showTopToolbar();
    } else {
        window()->showFullScreen(); //full screen then hide bars because hide animation depends on height()
        Q_EMIT SignalManager::instance()->hideBottomToolbar();
        Q_EMIT SignalManager::instance()->hideExtensionPanel();
        Q_EMIT SignalManager::instance()->hideTopToolbar();
    }
}

void ViewPanel::openImage(const QString &path)
{
    Q_EMIT SignalManager::instance()->gotoPanel(this);
    m_view->setImage(path);
    m_nav->setImage(m_view->image());
    qDebug() << "view path: " << m_view->imagePath();
    if (m_info)
        m_info->setImagePath(path);
}
