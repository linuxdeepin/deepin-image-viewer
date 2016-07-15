#include "mainwidget.h"
#include "controller/importer.h"
#include "utils/baseutils.h"
#include "widgets/processtooltip.h"
#include "imageinfodialog.h"
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

MainWidget::MainWidget(bool manager, QWidget *parent)
    : QFrame(parent),
      m_sManager(SignalManager::instance())
{
    initStyleSheet();
    initPanelStack();
    initExtensionPanel();
    initTopToolbar();
    initBottomToolbar();

    if (manager) {
        initAlbumPanel();
        initEditPanel();
        initSlideShowPanel();
        initTimelinePanel();
    }
    initViewPanel();

    connect(m_sManager, &SignalManager::backToMainWindow, this, [=] {
        onGotoPanel(m_timelinePanel);
        emit m_sManager->showTopToolbar();
        emit m_sManager->showBottomToolbar();
        emit m_sManager->hideExtensionPanel(true);
    });
    connect(m_sManager, &SignalManager::gotoPanel,
            this, &MainWidget::onGotoPanel);
    connect(m_sManager, &SignalManager::showInFileManager,
            this, [=] (const QString &path) {
        utils::base::showInFileManager(path);
    });
    connect(m_sManager, &SignalManager::showProcessTooltip,
            this, &MainWidget::onShowProcessTooltip);
    connect(m_sManager, &SignalManager::showImageInfo,
            this, &MainWidget::onShowImageInfo);
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
    QPointer<ModulePanel> p(panel);
    if (p.isNull()) {
        return;
    }

    Importer::instance()->nap();

    m_panelStack->setCurrentWidget(panel);
    emit m_sManager->updateTopToolbarLeftContent(panel->toolbarTopLeftContent());
    emit m_sManager->updateTopToolbarMiddleContent(panel->toolbarTopMiddleContent());
    emit m_sManager->updateBottomToolbarContent(panel->toolbarBottomContent());
    emit m_sManager->updateExtensionPanelContent(panel->extensionPanelContent());
}

void MainWidget::onShowProcessTooltip(const QString &message, bool success)
{
    ProcessTooltip *t = new ProcessTooltip(this, m_panelStack);
    t->showTooltip(message, success);
    t->move((width() - t->width()) / 2, height() * 4 / 5);
}

void MainWidget::onShowImageInfo(const QString &path)
{
    ImageInfoDialog *info = new ImageInfoDialog(this, m_panelStack);
    info->setPath(path);
    info->move((width() - info->width()) / 2 +
               mapToGlobal(QPoint(0, 0)).x(),
               (window()->height() - info->sizeHint().height()) / 2 +
               mapToGlobal(QPoint(0, 0)).y());
    info->show();
    connect(info, &ImageInfoDialog::closed, info, &ImageInfoDialog::deleteLater);
    connect(m_sManager, &SignalManager::gotoPanel,
            info, &ImageInfoDialog::close);
    connect(m_sManager, &SignalManager::gotoAlbumPanel,
            info, &ImageInfoDialog::close);
    connect(m_sManager, &SignalManager::backToMainWindow,
            info, &ImageInfoDialog::close);
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
//    m_topToolbar->moveWithAnimation(0, 0);
    m_topToolbar->move(0, 0);
    connect(m_sManager, &SignalManager::updateTopToolbarLeftContent,
            this, [=](QWidget *c) {
        if (c != nullptr)
            m_topToolbar->setLeftContent(c);
    });
    connect(m_sManager, &SignalManager::updateTopToolbarMiddleContent,
            this, [=](QWidget *c) {
        if (c != nullptr)
            m_topToolbar->setMiddleContent(c);
    });
    connect(m_sManager, &SignalManager::showTopToolbar, this, [=] {
//        m_topToolbar->moveWithAnimation(0, 0);
        m_topToolbar->move(0, 0);
    });
    connect(m_sManager, &SignalManager::hideTopToolbar, this,
            [=](bool immediately) {
        Q_UNUSED(immediately)
        m_topToolbar->move(0, - TOP_TOOLBAR_HEIGHT);
//        if (immediately) {
//            m_topToolbar->move(0, - TOP_TOOLBAR_HEIGHT);
//        }
//        else {
//            m_topToolbar->moveWithAnimation(0, - TOP_TOOLBAR_HEIGHT);
//        }
    });
}

void MainWidget::initBottomToolbar()
{
    m_bottomToolbar = new BottomToolbar(this, m_panelStack);
    m_bottomToolbar->resize(width(), BOTTOM_TOOLBAR_HEIGHT);
    m_bottomToolbar->move(0, height() - m_bottomToolbar->height());
    connect(m_sManager, &SignalManager::updateBottomToolbarContent,
            this, [=](QWidget *c, bool wideMode) {
        if (c == nullptr)
            return;
        m_bottomToolbar->setContent(c);
        if (wideMode) {
            m_bottomToolbar->setFixedHeight(38);
        }
        else {
            m_bottomToolbar->setFixedHeight(24);
        }
        m_bottomToolbar->move(0, height() - m_bottomToolbar->height());
    });
    connect(m_sManager, &SignalManager::showBottomToolbar, this, [=] {
        m_bottomToolbar->setVisible(true);
        m_bottomToolbar->move(0, height() - m_bottomToolbar->height());
//        // Make the bottom toolbar always stay at the bottom after windows resize
//        m_bottomToolbar->move(0, height());
//        m_bottomToolbar->moveWithAnimation(0, height() - m_bottomToolbar->height());
    });

    connect(m_sManager, &SignalManager::hideBottomToolbar,
            this, [=](bool immediately) {
        m_bottomToolbar->move(0, height());
        m_bottomToolbar->setVisible(false);
        Q_UNUSED(immediately)
//        if (immediately) {
//            m_bottomToolbar->move(0, height());
//            m_bottomToolbar->setVisible(false);
//        }
//        else {
//            m_bottomToolbar->moveWithAnimation(0, height());
//        }
    });
}

void MainWidget::initExtensionPanel()
{
    m_extensionPanel = new ExtensionPanel(this, m_panelStack);
    m_extensionPanel->move(- EXTENSION_PANEL_WIDTH, 0);
    connect(m_sManager, &SignalManager::updateExtensionPanelContent,
            this, [=](QWidget *c) {
        if (c != nullptr)
            m_extensionPanel->setContent(c);
    });
    connect(m_sManager, &SignalManager::updateExtensionPanelRect,
            this, [=] {
        m_extensionPanel->updateRectWithContent();
    });
    connect(m_sManager, &SignalManager::showExtensionPanel, this, [=] {
        // Is visible
        if (m_extensionPanel->pos() == QPoint(0, 0)) {
            m_extensionPanel->move(- m_extensionPanel->width(), 0);
        }
        else {
            m_extensionPanel->move(0, 0);
        }
    });
    connect(m_sManager, &SignalManager::hideExtensionPanel,
            this, [=] (bool immediately) {
        m_extensionPanel->move(- qMax(m_extensionPanel->width(),
                                               EXTENSION_PANEL_WIDTH), 0);
        Q_UNUSED(immediately)
//        if (immediately) {
//            m_extensionPanel->move(- qMax(m_extensionPanel->width(),
//                                                   EXTENSION_PANEL_WIDTH), 0);
//        }
//        else {
//            m_extensionPanel->moveWithAnimation(- qMax(m_extensionPanel->width(),
//                                                   EXTENSION_PANEL_WIDTH), 0);
//        }
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

    connect(m_sManager, &SignalManager::createAlbum,
            m_albumPanel, &AlbumPanel::onCreateAlbum);
    connect(m_sManager, &SignalManager::gotoAlbumPanel,
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

void MainWidget::initSlideShowPanel()
{
    m_slideShowPanel = new SlideShowPanel();
    m_panelStack->addWidget(m_slideShowPanel);
}
