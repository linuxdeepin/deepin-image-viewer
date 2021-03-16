/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
        : QFrame(parent)
    {
        connect(dApp->signalM, &SignalManager::gotoPanel,
                this, &ModulePanel::showPanelEvent);
    }
    virtual bool isMainPanel() {return false;}
    virtual QString moduleName() = 0;
    virtual QWidget *extensionPanelContent() = 0;
    virtual QWidget *toolbarBottomContent() = 0;
    virtual QWidget *toolbarTopMiddleContent() = 0;
    virtual QWidget *toolbarTopLeftContent() = 0;

protected:
    virtual void showPanelEvent(ModulePanel *p)
    {
        if (p != this)
            return;

        emit dApp->signalM->updateBottomToolbarContent(toolbarBottomContent());
        emit dApp->signalM->updateExtensionPanelContent(extensionPanelContent());
        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
    }
};

#endif // MODULEPANEL_H
