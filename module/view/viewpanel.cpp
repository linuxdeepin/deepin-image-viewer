#include "viewpanel.h"
#include <dimagebutton.h>
#include <QBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QDebug>
#include "controller/signalmanager.h"
#include "imageinfowidget.h"

using namespace Dtk::Widget;

ViewPanel::ViewPanel(QWidget *parent)
    : ModulePanel(parent)
{
    connect(SignalManager::instance(), &SignalManager::viewImage, this, &ViewPanel::openImage);
    m_view = new ImageWidget();
    QHBoxLayout *hl = new QHBoxLayout();
    setLayout(hl);
    hl->addWidget(m_view);

    m_nav = new NavigationWidget(this);
    m_nav->resize(200, 200);
    connect(m_view, &ImageWidget::transformChanged, [this](){
        m_nav->setRectInImage(m_view->visibleImageRect());
    });
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
    btn->setNormalPic(":/images/icons/resources/images/icons/info-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/info-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/info-active.png");
    hb->addWidget(btn);
    hb->addStretch();
    connect(btn, &DImageButton::clicked, SignalManager::instance(), &SignalManager::showExtensionPanel);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/collect-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/collect-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/collect-active.png");
    hb->addWidget(btn);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/previous-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/previous-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/previous-press.png");
    hb->addWidget(btn);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/slideshow-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/slideshow-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/slideshow-press.png");
    hb->addWidget(btn);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/next-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/next-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/next-press.png");
    hb->addWidget(btn);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/edit-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/edit-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/edit-press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, [this](){
        Q_EMIT SignalManager::instance()->editImage(m_view->imagePath());
    });

    hb->addStretch();

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/delete-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/delete-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/delete-press.png");
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
    btn->setNormalPic(":/images/icons/resources/images/icons/album-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/album-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/album-active.png");
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
    btn->setNormalPic(":/images/icons/resources/images/icons/contrarotate-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/contrarotate-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/contrarotate-press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, m_view, &ImageWidget::rotateAntiClockWise);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/clockwise-rotation-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/clockwise-rotation-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/clockwise-rotation-press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, m_view, &ImageWidget::rotateClockWise);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/adapt-image-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/adapt-image-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/adapt-image-active.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, [this](){
        m_view->resetTransform();
        m_view->setScaleValue(1);
    });
    //
#if 0
    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/share-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/share-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/share-active.png");
    hb->addWidget(btn);
#endif
    hb->addStretch();
    return w;
}

QWidget *ViewPanel::extensionPanelContent()
{
    m_info = new ImageInfoWidget();
    m_info->setImagePath(m_view->imagePath());
    return m_info;
}

void ViewPanel::resizeEvent(QResizeEvent *e)
{
    m_nav->move(e->size().width() - m_nav->width() - 10, e->size().height() - m_nav->height());
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
