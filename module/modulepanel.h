#ifndef MODULEPANEL_H
#define MODULEPANEL_H

#include "application.h"
#include "controller/signalmanager.h"
#include <QDebug>
#include <QFile>
#include <QFrame>

class ModulePanel : public QFrame
{
    Q_OBJECT
public:
    ModulePanel(QWidget *parent = 0)
        :QFrame(parent)
    {
        connect(dApp->signalM, &SignalManager::gotoPanel,
                this, &ModulePanel::showPanelEvent);
    }
    virtual QWidget *toolbarTopLeftContent() = 0;
    virtual QWidget *toolbarTopMiddleContent() = 0;
    virtual QWidget *toolbarBottomContent() = 0;
    virtual QWidget *extensionPanelContent() = 0;

protected:
    virtual void showPanelEvent(ModulePanel *p) {
        if (p != this)
            return;
        emit dApp->signalM->updateBottomToolbarContent(toolbarBottomContent());
        emit dApp->signalM->updateExtensionPanelContent(extensionPanelContent());
        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
    }
};

#endif // MODULEPANEL_H
