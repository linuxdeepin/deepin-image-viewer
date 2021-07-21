/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangYong <zhangyong@uniontech.com>
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
#include "mainwindow.h"
#include "config.h"
#include "ac-desktop-define.h"

#include <QGraphicsDropShadowEffect>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QDesktopWidget>
#include <QShortcut>
#include <QDir>

#include <dgiovolumemanager.h>
#include <dgiofile.h>
#include <dgiofileinfo.h>
#include <dgiovolume.h>
#include <DTableView>
#include <DApplicationHelper>
#include <DFileDialog>
#include <DApplication>
#include <DMessageManager>
#include <DFloatingMessage>
#include <DWidgetUtil>
#include <DStandardPaths>

//#include <thumbna>

MainWindow::MainWindow()
{
    this->setObjectName("drawMainWindow");
    setMinimumSize(880, 500);
    resize(1300, 848);
}
//初始化QStackedWidget和展示
void MainWindow::initUI()
{
    m_pCenterWidget = new QStackedWidget(this);
    this->setCentralWidget(m_pCenterWidget);
//    ThumbnailWidget *Widget = new ThumbnailWidget(m_pCenterWidget);
}

MainWindow::~MainWindow()
{
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    DMainWindow::resizeEvent(e);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    return DMainWindow::eventFilter(obj, event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    DMainWindow::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    DMainWindow::showEvent(event);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    DMainWindow::wheelEvent(event);
}
