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
#include <QMimeData>
#include <QSettings>
#include <QCommandLineParser>
#include <QCoreApplication>

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
#include "../libimage-viewer/imageengine.h"
#include "shortcut.h"

const QString CONFIG_PATH =   QDir::homePath() +
                              "/.config/deepin/deepin-image-viewer/config.conf";

MainWindow::MainWindow()
{
    this->setObjectName("MainWindow");
    setContentsMargins(0, 0, 0, 0);
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

void MainWindow::setValue(const QString &group, const QString &key, const QVariant &value)
{
    if (!m_settings) {
        m_settings = new QSettings(CONFIG_PATH, QSettings::IniFormat, this);
    }
    m_settings->beginGroup(group);
    m_settings->setValue(key, value);
    m_settings->endGroup();
}

QVariant MainWindow::value(const QString &group, const QString &key, const QVariant &defaultValue)
{
    if (!m_settings) {
        m_settings = new QSettings(CONFIG_PATH, QSettings::IniFormat, this);
    }
    QVariant value;
    m_settings->beginGroup(group);
    value = m_settings->value(key, defaultValue);
    m_settings->endGroup();

    return value;
}

void MainWindow::processOption()
{
    QCommandLineParser m_cmdParser;
    if (!m_cmdParser.parse(QCoreApplication::arguments())) {
        return ;
    }
    QStringList names = m_cmdParser.optionNames();
    for (QString name : names) {
    }

    QStringList pas = m_cmdParser.positionalArguments();
    for (QString path : pas) {
        if (QFileInfo(path).isFile()) {
            slotDrogImg(QStringList(path));
        }
    }
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

    connect(m_homePageWidget, &HomePageWidget::sigDrogImage, this, &MainWindow::slotDrogImg);

    //ImageEngine::instance()->sigPicCountIsNull()
    connect(ImageEngine::instance(), &ImageEngine::sigPicCountIsNull, this, [ = ] {
        m_centerWidget->setCurrentWidget(m_homePageWidget);
        if (m_mainwidow->titlebar())
        {
            //隐藏原有DMainWindow titlebar，使用自定义标题栏
            int height =  m_mainwidow->titlebar()->height();
            m_mainwidow->titlebar()->setFixedHeight(50);
            m_mainwidow->titlebar()->setIcon(QIcon::fromTheme("deepin-image-viewer"));
            m_mainwidow->setTitlebarShadowEnabled(true);
            m_mainwidow->resize(this->width(), this->height() + height);
            //防止标题栏出现横杠
            m_mainwidow->resize(this->width(), this->height());
        }
    });

    QShortcut *openFileManager = new QShortcut(QKeySequence("Ctrl+o"), this);
    openFileManager->setContext(Qt::WindowShortcut);
    connect(openFileManager, &QShortcut::activated, this, [ = ] {
        this->slotOpenImg();
    });

    QShortcut *scViewShortcut = new QShortcut(QKeySequence("Ctrl+Shift+/"), this);
    scViewShortcut->setObjectName(SC_VIEW_SHORTCUT);
    // connect(scE, SIGNAL(activated()), dApp, SLOT(quit()));
    connect(scViewShortcut, &QShortcut::activated, this, [ = ] {
        qDebug() << "receive Ctrl+Shift+/";
        QRect rect = window()->geometry();
        QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
        Shortcut sc;
        QStringList shortcutString;
        QString param1 = "-j=" + sc.toStr();
        QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
        shortcutString << "-b" << param1 << param2;
        qDebug() << shortcutString;
        QProcess::startDetached("deepin-shortcut-viewer", shortcutString);
    });

}

void MainWindow::slotOpenImg()
{
    bool bRet = m_imageViewer->startChooseFileDialog();
    if (bRet) {
        m_centerWidget->setCurrentWidget(m_imageViewer);
        if (m_mainwidow->titlebar()) {
            //隐藏原有DMainWindow titlebar，使用自定义标题栏
            int height =  m_mainwidow->titlebar()->height();
            m_mainwidow->titlebar()->setFixedHeight(0);
            m_mainwidow->titlebar()->setIcon(QIcon::fromTheme("deepin-image-viewer"));
            m_mainwidow->setTitlebarShadowEnabled(true);
            m_mainwidow->resize(this->width(), this->height() + height);
            //防止标题栏出现横杠
            m_mainwidow->resize(this->width(), this->height());
        }
    }

}

void MainWindow::slotDrogImg(const QStringList &paths)
{
    bool bRet = m_imageViewer->startdragImage(paths);
    if (bRet) {
        m_centerWidget->setCurrentWidget(m_imageViewer);
        if (m_mainwidow->titlebar()) {
            //隐藏原有DMainWindow titlebar，使用自定义标题栏
            int height =  m_mainwidow->titlebar()->height();
            m_mainwidow->titlebar()->setFixedHeight(0);
            m_mainwidow->titlebar()->setIcon(QIcon::fromTheme("deepin-image-viewer"));
            m_mainwidow->setTitlebarShadowEnabled(true);
            m_mainwidow->resize(this->width(), this->height() + height);
            //防止标题栏出现横杠
            m_mainwidow->resize(this->width(), this->height());
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if (!isMaximized() && !window()->isFullScreen() && !window()->isMaximized()) {
        setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, width());
        setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, height());
    }
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

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
    event->acceptProposedAction();
    DWidget::dragEnterEvent(event);
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }
    QStringList paths;
    for (QUrl url : urls) {
        //修复style问题，取消了path
        //lmh0901判断是否是图片
        paths << url.toLocalFile();
    }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    DWidget::wheelEvent(event);
}
