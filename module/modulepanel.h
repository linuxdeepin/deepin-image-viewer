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
    ModulePanel(QWidget *parent = 0) :QFrame(parent){}
    virtual QWidget *toolbarTopLeftContent() = 0;
    virtual QWidget *toolbarTopMiddleContent() = 0;
    virtual QWidget *toolbarBottomContent() = 0;
    virtual QWidget *extensionPanelContent() = 0;
};

#endif // MODULEPANEL_H
