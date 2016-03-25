#include "EditPanel.h"
#include <dimagebutton.h>
#include <QBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QStackedWidget>
#include <dimagebutton.h>
#include <dtextbutton.h>
#include "controller/signalmanager.h"
#include "FilterSetup.h"
#include "filters/FilterObj.h"
#include "Cut.h"

using namespace Dtk::Widget;

EditPanel::EditPanel(QWidget *parent)
    : ModulePanel(parent)
{
    m_stack = new QStackedWidget(this); // why need a parent to avoid cutwidget crash?
    connect(SignalManager::instance(), &SignalManager::editImage, this, &EditPanel::openImage);
    m_view = new ImageWidget();
    m_cut = new CutWidget();
    m_stack->addWidget(m_view);
    m_stack->addWidget(m_cut);
    m_stack->setCurrentWidget(m_view);
    QHBoxLayout *hl = new QHBoxLayout();
    setLayout(hl);
    hl->addWidget(m_stack);
    m_filterSetup = new FilterSetup(this);
    connect(m_filterSetup, &FilterSetup::filterIdChanged, this, &EditPanel::setFilterId);
    connect(m_filterSetup, &FilterSetup::filterIndensityChanged, this, &EditPanel::setFilterIndensity);
}

QWidget *EditPanel::toolbarBottomContent()
{
    return NULL;
}

QWidget *EditPanel::toolbarTopLeftContent()
{
    QWidget *w = new QWidget();
    QHBoxLayout *hb = new QHBoxLayout();
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(0);
    w->setLayout(hb);
    DImageButton *btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/previous-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/previous-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/previous-press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, SignalManager::instance(), &SignalManager::backToMainWindow);
    DTextButton *btn1 = new DTextButton(tr("Back"));
    hb->addWidget(btn1);
    connect(btn1, &DTextButton::clicked, SignalManager::instance(), &SignalManager::backToMainWindow);
    btn1 = new DTextButton(tr("Revert"));
    hb->addWidget(btn1);
    hb->addStretch();
    return w;
}

QWidget *EditPanel::toolbarTopMiddleContent()
{
    QWidget *w = new QWidget();
    QHBoxLayout *hb = new QHBoxLayout();
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(0);
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
    btn->setNormalPic(":/images/icons/resources/images/icons/filter-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/filter-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/filter-active.png");
    hb->addWidget(btn);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/cutting-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/cutting-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/cutting-active.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, [this](){
        if (m_stack->currentWidget() == m_view) {
            m_cut->setImage(m_image);
            m_stack->setCurrentWidget(m_cut);
        } else {
            m_stack->setCurrentWidget(m_view);
        }
    });

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/flip-horizontal-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/flip-horizontal-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/flip-horizontal-press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, m_view, &ImageWidget::flipX);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/flip-vertical-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/flip-vertical-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/flip-vertical-press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, m_view, &ImageWidget::flipY);


    hb->addStretch();
    return w;
}

QWidget *EditPanel::extensionPanelContent()
{
    return NULL;
}

void EditPanel::setFilterId(int value)
{
    if (m_filterId == value)
        return;
    m_filterId = value;
    if (m_filter) {
        delete m_filter;
        m_filter = 0;
    }
    qDebug("filter id: %d", m_filterId);
    m_filter = filter2d::FilterObj::create(m_filterId);
    applyFilter();
}

void EditPanel::setFilterIndensity(qreal value)
{
    if (m_filterIndensity == value)
        return;
    m_filterIndensity = value;
    applyFilter();
}

void EditPanel::applyFilter()
{
    if (!m_filter)
        return;
    m_filter->setProperty("brightness", 0.6);
    m_filter->setProperty("hue", 0.6);
    m_filter->setProperty("contrast", 0.6);
    m_filter->setProperty("saturation", 0.6);
    qDebug("set indensity: %.3f", m_filterIndensity);
    m_filter->setIndensity(m_filterIndensity);
    QImage img(m_image);
    if (img.isNull())
        return;
    m_view->setImage(m_filter->apply(img));
}

void EditPanel::openImage(const QString &path)
{
    m_path = path;
    m_image = QImage(path);
    Q_EMIT SignalManager::instance()->gotoPanel(this);
    m_stack->setCurrentWidget(m_view);
    m_view->setImage(m_image);
    m_filterSetup->resize(240, height() - 30);
    m_filterSetup->show();
    m_filterSetup->setImage(path);
}
