#include "mainwidget.h"
#include <dimagebutton.h>
#include <QFile>
#include <QDebug>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

using namespace Dtk::Widget;

const int TOP_TOOLBAR_HEIGHT = 40;
const int BOTTOM_TOOLBAR_HEIGHT = 24;
const int EXTENSION_PANEL_WIDTH = 240;
const int MAINWIDGET_MINIMUN_WIDTH = 700;
const int MAINWIDGET_MINIMUN_HEIGHT = 500;

MainWidget::MainWidget(QWidget *parent)
    : QFrame(parent)
{
    initStyleSheet();
    QDesktopWidget dw;
    int ww = dw.geometry().width() * 0.8 < 700 ? 700 : dw.geometry().width() * 0.8;
    int wh = dw.geometry().height() * 0.8 < 500 ? 500 : dw.geometry().height() * 0.8;
    resize(ww, wh);
    setMinimumSize(MAINWIDGET_MINIMUN_WIDTH, MAINWIDGET_MINIMUN_HEIGHT);
    move((dw.geometry().width() - ww) / 2, (dw.geometry().height() - wh) / 4);

    initPanelStack();
    initExtensionPanel();
    initTopToolbar();
    initBottomToolbar();

    initTimelinePanel();
    initAlbumPanel();
    initViewPanel();
    initEditPanel();
    connect(m_signalManager, &SignalManager::backToMainWindow, this, [=] {
        m_panelStack->setCurrentWidget(m_timelinePanel);
    });
    connect(m_signalManager, &SignalManager::gotoPanel, [this](ModulePanel* panel){
        m_panelStack->setCurrentWidget(panel);
    });
}

MainWidget::~MainWidget()
{

}

void MainWidget::resizeEvent(QResizeEvent *)
{
    if (m_topToolbar) {
        m_topToolbar->resize(width(), TOP_TOOLBAR_HEIGHT);
//        m_topToolbar->move(0, 0);
    }
    if (m_bottomToolbar) {
        m_bottomToolbar->resize(width(), BOTTOM_TOOLBAR_HEIGHT);
        m_bottomToolbar->move(0, height() - BOTTOM_TOOLBAR_HEIGHT);
    }
    if (m_extensionPanel) {
        m_extensionPanel->resize(EXTENSION_PANEL_WIDTH, height());
    }
}

void MainWidget::initPanelStack()
{
    m_panelStack = new QStackedWidget(this);
    m_panelStack->setObjectName("PanelStack");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_panelStack);
}

void MainWidget::initTopToolbar()
{
    m_topToolbar = new TopToolbar(this, m_panelStack);
    m_topToolbar->resize(width(), TOP_TOOLBAR_HEIGHT);
    m_topToolbar->moveWithAnimation(0, 0);
    connect(m_signalManager, &SignalManager::updateTopToolbarLeftContent, this, [=](QWidget *c) {
        m_topToolbar->setLeftContent(c);
    });
    connect(m_signalManager, &SignalManager::updateTopToolbarMiddleContent, this, [=](QWidget *c) {
        m_topToolbar->setMiddleContent(c);
    });
    connect(m_signalManager, &SignalManager::showTopToolbar, this, [=] {
        m_topToolbar->moveWithAnimation(0, 0);
    });
    connect(m_signalManager, &SignalManager::hideTopToolbar, this, [=] {
        m_topToolbar->moveWithAnimation(0, - TOP_TOOLBAR_HEIGHT);
    });
}

void MainWidget::initBottomToolbar()
{
    m_bottomToolbar = new BottomToolbar(this, m_panelStack);
    m_bottomToolbar->resize(width(), BOTTOM_TOOLBAR_HEIGHT);
    m_bottomToolbar->move(0, height() - BOTTOM_TOOLBAR_HEIGHT);
    connect(m_signalManager, &SignalManager::updateBottomToolbarContent, this, [=](QWidget *c) {
        m_bottomToolbar->setContent(c);
    });
    connect(m_signalManager, &SignalManager::showBottomToolbar, this, [=] {
        m_bottomToolbar->moveWithAnimation(0, height() - BOTTOM_TOOLBAR_HEIGHT);
    });
    connect(m_signalManager, &SignalManager::hideBottomToolbar, this, [=] {
        m_bottomToolbar->moveWithAnimation(0, height() + BOTTOM_TOOLBAR_HEIGHT);
    });
}

void MainWidget::initExtensionPanel()
{
    m_extensionPanel = new ExtensionPanel(this, m_panelStack);
    m_extensionPanel->move(- EXTENSION_PANEL_WIDTH, 0);
    connect(m_signalManager, &SignalManager::updateExtensionPanelContent, this, [=](QWidget *c) {
        m_extensionPanel->setContent(c);
    });
    connect(m_signalManager, &SignalManager::showExtensionPanel, this, [=] {
        m_extensionPanel->moveWithAnimation(0, 0);
    });
    connect(m_signalManager, &SignalManager::hideExtensionPanel, this, [=] {
        m_extensionPanel->moveWithAnimation(- EXTENSION_PANEL_WIDTH, 0);
    });
}

void MainWidget::initStyleSheet()
{
    QFile sf(":/qss/resources/qss/default.qss");
    if (!sf.open(QIODevice::ReadOnly)) {
        qWarning() << "Open style-sheet file error:" << sf.errorString();
        return;
    }

    qApp->setStyleSheet(QString(sf.readAll()));
    sf.close();
}

void MainWidget::initTimelinePanel()
{
    m_timelinePanel = new TimelinePanel;
    m_panelStack->addWidget(m_timelinePanel);

    connect(m_timelinePanel, &TimelinePanel::needGotoAlbumPanel, this, [=] {
        emit m_signalManager->gotoPanel(m_albumPanel);
        emit m_signalManager->updateBottomToolbarContent(m_albumPanel->toolbarBottomContent());
        emit m_signalManager->updateTopToolbarLeftContent(m_albumPanel->toolbarTopLeftContent());
        emit m_signalManager->updateTopToolbarMiddleContent(m_albumPanel->toolbarTopMiddleContent());
    });
}

void MainWidget::initAlbumPanel()
{
    m_albumPanel = new AlbumPanel;
    m_panelStack->addWidget(m_albumPanel);

    connect(m_albumPanel, &AlbumPanel::needGotoTimelinePanel, this, [=] {
        emit m_signalManager->gotoPanel(m_timelinePanel);
        emit m_signalManager->updateBottomToolbarContent(m_timelinePanel->toolbarBottomContent());
        emit m_signalManager->updateTopToolbarLeftContent(m_timelinePanel->toolbarTopLeftContent());
        emit m_signalManager->updateTopToolbarMiddleContent(m_timelinePanel->toolbarTopMiddleContent());
    });
}

void MainWidget::initViewPanel()
{
    m_viewPanel = new ViewPanel();
    m_panelStack->addWidget(m_viewPanel);
}

void MainWidget::initEditPanel()
{
    m_editPanel = new EditPanel();
    m_panelStack->addWidget(m_editPanel);
}
