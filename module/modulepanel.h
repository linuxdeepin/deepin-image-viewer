#ifndef MODULEPANEL_H
#define MODULEPANEL_H

#include <QFrame>
#include "controller/signalmanager.h"

class ModulePanel : public QFrame
{
    Q_OBJECT
public:
    ModulePanel(QWidget *parent = 0) :QFrame(parent){
        connect(this, &ModulePanel::hideBottomToolbar,
                SignalManager::instance(), &SignalManager::hideBottomToolbar);
        connect(this, &ModulePanel::showBottomToolbar,
                SignalManager::instance(), &SignalManager::showBottomToolbar);
        connect(this, &ModulePanel::hideTopToolbar,
                SignalManager::instance(), &SignalManager::hideTopToolbar);
        connect(this, &ModulePanel::showTopToolbar,
                SignalManager::instance(), &SignalManager::showTopToolbar);
        connect(this, &ModulePanel::hideExtensionPanel,
                SignalManager::instance(), &SignalManager::hideExtensionPanel);
        connect(this, &ModulePanel::showExtensionPanel,
                SignalManager::instance(), &SignalManager::showExtensionPanel);
        connect(this, &ModulePanel::backToMainWindow,
                SignalManager::instance(), &SignalManager::backToMainWindow);

        connect(this, &ModulePanel::updateTopToolbarLeftContent,
                SignalManager::instance(), &SignalManager::updateTopToolbarLeftContent);
        connect(this, &ModulePanel::updateTopToolbarMiddleContent,
                SignalManager::instance(), &SignalManager::updateTopToolbarMiddleContent);
        connect(this, &ModulePanel::updateBottomToolbarContent,
                SignalManager::instance(), &SignalManager::updateBottomToolbarContent);
        connect(this, &ModulePanel::updateExtensionPanelContent,
                SignalManager::instance(), &SignalManager::updateExtensionPanelContent);
    }
    virtual QWidget *toolbarTopLeftContent() = 0;
    virtual QWidget *toolbarTopMiddleContent() = 0;
    virtual QWidget *toolbarBottomContent() = 0;
    virtual QWidget *extensionPanelContent() = 0;

signals:
    void hideTopToolbar();
    void showTopToolbar();
    void hideBottomToolbar();
    void showBottomToolbar();
    void hideExtensionPanel();
    void showExtensionPanel();
    void backToMainWindow();
    void updateTopToolbarLeftContent(QWidget *content);
    void updateTopToolbarMiddleContent(QWidget *content);
    void updateBottomToolbarContent(QWidget *content);
    void updateExtensionPanelContent(QWidget *content);
};

#endif // MODULEPANEL_H
