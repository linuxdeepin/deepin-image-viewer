#include "mainwidget.h"
#include "utils/baseutils.h"
#include "widgets/processtooltip.h"
#include <QFile>
#include <QDebug>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;
const int BOTTOM_TOOLBAR_HEIGHT = 24;
const int EXTENSION_PANEL_WIDTH = 240;

}  // namespace

MainWidget::MainWidget(QWidget *parent)
    : QFrame(parent)
{
    initStyleSheet();
    initPanelStack();
    initExtensionPanel();
    initTopToolbar();
    initBottomToolbar();

    initTimelinePanel();
    initAlbumPanel();
    initViewPanel();
    initEditPanel();

    connect(m_signalManager, &SignalManager::backToMainWindow, this, [=] {
        onGotoPanel(m_timelinePanel);
    });
    connect(m_signalManager, &SignalManager::gotoPanel,
            this, &MainWidget::onGotoPanel);
    connect(m_signalManager, &SignalManager::gotoPanel, this, &MainWidget::onGotoPanel);
    connect(m_signalManager, &SignalManager::showInFileManager,
            this, [=] (const QString &path) {
        utils::base::showInFileManager(path);
    });
    connect(m_signalManager, &SignalManager::showProcessTooltip,
            this, &MainWidget::onShowProcessTooltip);
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
        m_bottomToolbar->resize(width(), m_bottomToolbar->height());
        if (m_bottomToolbar->isVisible())
            m_bottomToolbar->move(0, height() - m_bottomToolbar->height());
    }
    if (m_extensionPanel) {
        m_extensionPanel->resize(m_extensionPanel->width(), height());
    }
}

void MainWidget::onGotoPanel(ModulePanel *panel)
{
    m_panelStack->setCurrentWidget(panel);
    emit m_signalManager->updateTopToolbarLeftContent(panel->toolbarTopLeftContent());
    emit m_signalManager->updateTopToolbarMiddleContent(panel->toolbarTopMiddleContent());
    emit m_signalManager->updateBottomToolbarContent(panel->toolbarBottomContent());
    emit m_signalManager->updateExtensionPanelContent(panel->extensionPanelContent());
}

void MainWidget::onShowProcessTooltip(const QString &message, bool success)
{
    ProcessTooltip *t = new ProcessTooltip(this, m_panelStack);
    t->showTooltip(message, success);
    t->move((width() - t->width()) / 2, (height() - t->height()) / 2);
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
    connect(m_signalManager, &SignalManager::hideTopToolbar, this,
            [=](bool immediately) {
        if (immediately) {
            m_topToolbar->move(0, - TOP_TOOLBAR_HEIGHT);
        }
        else {
            m_topToolbar->moveWithAnimation(0, - TOP_TOOLBAR_HEIGHT);
        }
    });
}

void MainWidget::initBottomToolbar()
{
    m_bottomToolbar = new BottomToolbar(this, m_panelStack);
    m_bottomToolbar->resize(width(), BOTTOM_TOOLBAR_HEIGHT);
    m_bottomToolbar->move(0, height() - m_bottomToolbar->height());
    connect(m_signalManager, &SignalManager::updateBottomToolbarContent,
            this, [=](QWidget *c, bool wideMode) {
        m_bottomToolbar->setContent(c);
        if (wideMode) {
            m_bottomToolbar->setFixedHeight(38);
        }
        else {
            m_bottomToolbar->setFixedHeight(24);
        }
        m_bottomToolbar->move(0, height() - m_bottomToolbar->height());
    });
    connect(m_signalManager, &SignalManager::showBottomToolbar, this, [=] {
        m_bottomToolbar->setVisible(true);
        m_bottomToolbar->moveWithAnimation(0, height() - m_bottomToolbar->height());
    });

    connect(m_signalManager, &SignalManager::hideBottomToolbar,
            this, [=](bool immediately) {
        if (immediately) {
            m_bottomToolbar->move(0, height());
            m_bottomToolbar->setVisible(false);
        }
        else {
            m_bottomToolbar->moveWithAnimation(0, height());
        }
    });
}

void MainWidget::initExtensionPanel()
{
    m_extensionPanel = new ExtensionPanel(this, m_panelStack);
    m_extensionPanel->move(- EXTENSION_PANEL_WIDTH, 0);
    connect(m_signalManager, &SignalManager::updateExtensionPanelContent,
            this, [=](QWidget *c) {
        m_extensionPanel->setContent(c);
    });
    connect(m_signalManager, &SignalManager::updateExtensionPanelRect, this, [=] {
        m_extensionPanel->updateRectWithContent();
    });
    connect(m_signalManager, &SignalManager::showExtensionPanel, this, [=] {
        // Is visible
        if (m_extensionPanel->pos() == QPoint(0, 0)) {
            m_extensionPanel->moveWithAnimation(- m_extensionPanel->width(), 0);
        }
        else {
            m_extensionPanel->moveWithAnimation(0, 0);
        }
    });
    connect(m_signalManager, &SignalManager::hideExtensionPanel, this, [=] {
        m_extensionPanel->moveWithAnimation(- qMax(m_extensionPanel->width(),
                                                   EXTENSION_PANEL_WIDTH), 0);
    });
}

void MainWidget::initStyleSheet()
{
    QFile sf(":/qss/resources/qss/frame.qss");
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
}

void MainWidget::initAlbumPanel()
{
    m_albumPanel = new AlbumPanel;
    m_panelStack->addWidget(m_albumPanel);

    connect(m_signalManager, &SignalManager::createAlbum,
            m_albumPanel, &AlbumPanel::onCreateAlbum);
    connect(m_signalManager, &SignalManager::gotoAlbumPanel,
            this, [=] (const QString &album) {
        onGotoPanel(m_albumPanel);
        if (! album.isEmpty()) {
            m_albumPanel->onOpenAlbum(album);
        }
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
