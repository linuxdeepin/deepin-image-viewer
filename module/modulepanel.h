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
    ModulePanel(QWidget *parent = 0)
        :QFrame(parent),m_sManager(SignalManager::instance())
    {
        connect(m_sManager, &SignalManager::gotoPanel,
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
        emit m_sManager->updateBottomToolbarContent(toolbarBottomContent());
        emit m_sManager->updateExtensionPanelContent(extensionPanelContent());
        emit m_sManager->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit m_sManager->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
    }

private:
    SignalManager *m_sManager;
};

#endif // MODULEPANEL_H
