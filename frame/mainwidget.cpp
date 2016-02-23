#include "mainwidget.h"
#include <dimagebutton.h>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

using namespace Dtk::Widget;

const int TOP_TOOLBAR_HEIGHT = 40;
const int BOTTOM_TOOLBAR_HEIGHT = 24;
const int EXTENSION_PANEL_WIDTH = 240;

MainWidget::MainWidget(QWidget *parent)
    : QFrame(parent)
{
    initStyleSheet();
    QDesktopWidget dw;
    int ww = dw.geometry().width() * 0.8 < 700 ? 700 : dw.geometry().width() * 0.8;
    int wh = dw.geometry().height() * 0.8 < 500 ? 500 : dw.geometry().height() * 0.8;
    resize(ww, wh);
    setMinimumSize(700, 500);
    move((dw.geometry().width() - ww) / 2, (dw.geometry().height() - wh) / 4);

    initCenterContent();
    initExtensionPanel();
    initTopToolbar();
    initBottomToolbar();
}

MainWidget::~MainWidget()
{

}

void MainWidget::resizeEvent(QResizeEvent *)
{
    if (m_topToolbar) {
        updateTopToolbarPosition();
    }
    if (m_bottomToolbar) {
        updateBottomToolbarPosition();
    }
    if (m_extensionPanel) {
        updateExtensionPanelPosition();
    }
}

void MainWidget::initCenterContent()
{
    m_centerContent = new QWidget(this);
    m_centerContent->setObjectName("CenterContent");
    QHBoxLayout *cl = new QHBoxLayout(m_centerContent);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(0);
    connect(m_signalManager, &SignalManager::updateCenterContent, this, [=](QWidget *c) {
        QLayoutItem *child;
        if ((child = cl->takeAt(0)) != 0) {
            delete child;
        }

        cl->addWidget(c);
    });

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_centerContent);
}

void MainWidget::initTopToolbar()
{
    m_topToolbar = new TopToolbar(this, m_centerContent);
    updateTopToolbarPosition();
    connect(m_signalManager, &SignalManager::updateTopToolbarLeftContent, this, [=](QWidget *c) {
        m_topToolbar->setLeftContent(c);
    });
    connect(m_signalManager, &SignalManager::updateTopToolbarMiddleContent, this, [=](QWidget *c) {
        m_topToolbar->setMiddleContent(c);
    });
    connect(m_signalManager, &SignalManager::showTopToolbar, this, [=] {
        m_topToolbar->move(0, 0);
    });
    connect(m_signalManager, &SignalManager::hideTopToolbar, this, [=] {
        m_topToolbar->move(0, - TOP_TOOLBAR_HEIGHT);
    });
}

void MainWidget::initBottomToolbar()
{
    m_bottomToolbar = new BottomToolbar(this, m_centerContent);
    updateBottomToolbarPosition();
    connect(m_signalManager, &SignalManager::updateBottomToolbarContent, this, [=](QWidget *c) {
        m_bottomToolbar->setContent(c);
    });
    connect(m_signalManager, &SignalManager::showBottomToolbar, this, [=] {
        m_bottomToolbar->move(0, 0);
    });
    connect(m_signalManager, &SignalManager::hideBottomToolbar, this, [=] {
        m_bottomToolbar->move(0, - BOTTOM_TOOLBAR_HEIGHT);
    });
}

void MainWidget::initExtensionPanel()
{
    m_extensionPanel = new ExtensionPanel(this, m_centerContent);
    updateExtensionPanelPosition();
    connect(m_signalManager, &SignalManager::updateExtensionPanelContent, this, [=](QWidget *c) {
        m_extensionPanel->setContent(c);
    });
    connect(m_signalManager, &SignalManager::showExtensionPanel, this, [=] {
        m_extensionPanel->move(0, 0);
    });
    connect(m_signalManager, &SignalManager::hideExtensionPanel, this, [=] {
        m_extensionPanel->move(- EXTENSION_PANEL_WIDTH, 0);
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

void MainWidget::updateTopToolbarPosition()
{
    m_topToolbar->resize(width(), TOP_TOOLBAR_HEIGHT);
    m_topToolbar->move(0, 0);
}

void MainWidget::updateBottomToolbarPosition()
{
    m_bottomToolbar->resize(width(), BOTTOM_TOOLBAR_HEIGHT);
    m_bottomToolbar->move(0, height() - BOTTOM_TOOLBAR_HEIGHT);
}

void MainWidget::updateExtensionPanelPosition()
{
    m_extensionPanel->resize(EXTENSION_PANEL_WIDTH, height());
    m_extensionPanel->move(0, 0);
}
