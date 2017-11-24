/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "bottomtoolbar.h"
#include "extensionpanel.h"
#include "toptoolbar.h"
#include "controller/signalmanager.h"
#include "widgets/separator.h"

#include <QFrame>
#include <QStackedWidget>

class MainWidget : public QFrame
{
    Q_OBJECT

public:
    MainWidget(bool manager, QWidget *parent = 0);
    ~MainWidget();

protected:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void onGotoPanel(ModulePanel *panel);
    void onImported(const QString &message, bool success);
    void onShowImageInfo(const QString &path);

private:
    void initBottomToolbar();
    void initExtensionPanel();
    void initTopToolbar();

    void initConnection();
    void initPanelStack(bool manager);
    void initStyleSheet();

private:
    QStringList m_infoShowingList;

    QStackedWidget  *m_panelStack;

    bool m_manager;
    BottomToolbar   *m_bottomToolbar;
    ExtensionPanel  *m_extensionPanel;
    TopToolbar      *m_topToolbar;
    QLabel          *m_topSeparatorLine;
    QLabel          *m_btmSeparatorLine;
};

#endif // MAINWIDGET_H
