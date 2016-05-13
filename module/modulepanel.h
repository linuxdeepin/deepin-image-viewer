#ifndef MODULEPANEL_H
#define MODULEPANEL_H

#include <QFrame>
#include <QDebug>
#include <QFile>
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
    }
    virtual QWidget *toolbarTopLeftContent() = 0;
    virtual QWidget *toolbarTopMiddleContent() = 0;
    virtual QWidget *toolbarBottomContent() = 0;
    virtual QWidget *extensionPanelContent() = 0;

signals:
    void hideTopToolbar(bool immediately = false);
    void showTopToolbar();
    void hideBottomToolbar(bool immediately = false);
    void showBottomToolbar();
    void hideExtensionPanel();
    void showExtensionPanel();
    void backToMainWindow();
};

#endif // MODULEPANEL_H
