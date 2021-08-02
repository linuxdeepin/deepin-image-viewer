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

#include "module/view/homepagewidget.h"
#include "../libimage-viewer/imageviewer.h"

MainWindow::MainWindow()
{
    this->setObjectName("drawMainWindow");
    setContentsMargins(0, 0, 0, 0);
    setMinimumSize(880, 500);
    resize(880, 600);
    initUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setDMainWindow(DMainWindow *mainwidow)
{
    m_mainwidow = mainwidow;
    m_mainwidow->titlebar()->setIcon(QIcon::fromTheme("deepin-image-viewer"));
}

//初始化QStackedWidget和展示
void MainWindow::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);

    m_centerWidget = new QStackedWidget(this);
    layout->addWidget(m_centerWidget);

    m_homePageWidget = new HomePageWidget(this);
    m_centerWidget->addWidget(m_homePageWidget);

    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";
//    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeLocal, CACHE_PATH, this);
    m_centerWidget->addWidget(m_imageViewer);
    m_centerWidget->setCurrentWidget(m_homePageWidget);

    connect(m_homePageWidget, &HomePageWidget::sigOpenImage, this, &MainWindow::slotOpenImg);

    QShortcut *openFileManager = new QShortcut(QKeySequence("Ctrl+o"), this);
    openFileManager->setContext(Qt::WindowShortcut);
    connect(openFileManager, &QShortcut::activated, this, [ = ] {
        this->slotOpenImg();
    });

}

void MainWindow::slotOpenImg()
{
    bool bRet = m_imageViewer->startChooseFileDialog();
    if (bRet) {
        m_centerWidget->setCurrentWidget(m_imageViewer);
        if (m_mainwidow->titlebar()) {
            //隐藏原有DMainWindow titlebar，使用自定义标题栏
            m_mainwidow->titlebar()->setFixedHeight(0);
            m_mainwidow->titlebar()->setIcon(QIcon::fromTheme("deepin-image-viewer"));
            m_mainwidow->setTitlebarShadowEnabled(true);
        }
    }

}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    DWidget::resizeEvent(e);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    return DWidget::eventFilter(obj, event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    DWidget::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    DWidget::showEvent(event);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    DWidget::wheelEvent(event);
}
