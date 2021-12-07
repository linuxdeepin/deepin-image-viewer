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
#include "./accessibility/ac-desktop-define.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QDesktopWidget>
#include <QShortcut>
#include <QDir>
#include <QMimeData>
#include <QSettings>
#include <QCommandLineParser>
#include <QStandardPaths>

#include <DTableView>
#include <DFileDialog>

#include "module/view/homepagewidget.h"
#include "../libimageviewer/imageviewer.h"
#include "../libimageviewer/imageengine.h"

const int MAINWIDGET_MINIMUN_HEIGHT = 335;
const int MAINWIDGET_MINIMUN_WIDTH = 730;//增加了ocr，最小宽度为630到现在730

const QString CONFIG_PATH =   QDir::homePath() +
                              "/.config/deepin/deepin-image-viewer/config.conf";

MainWindow::MainWindow(QWidget *parent)
    : DWidget(parent)
{
    this->setObjectName("MainWindow");
    setContentsMargins(0, 0, 0, 0);

    //初始化UI
    initUI();


}

MainWindow::~MainWindow()
{

}

void MainWindow::setDMainWindow(DMainWindow *mainwidow)
{
    if (mainwidow) {
        m_mainwidow = mainwidow;
        m_mainwidow->titlebar()->setIcon(QIcon::fromTheme("deepin-image-viewer"));
        m_mainwidow->installEventFilter(this);
    }
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

QJsonObject MainWindow::createShorcutJson()
{

    QJsonObject shortcut1;
    shortcut1.insert("name", tr("Fullscreen"));
    shortcut1.insert("value", "F11");

    QJsonObject shortcut2;
    shortcut2.insert("name", tr("Exit fullscreen"));
    shortcut2.insert("value", "Esc");

    QJsonObject shortcut3;
    shortcut3.insert("name", tr("Extract text"));
    shortcut3.insert("value", "Alt + O");

    QJsonObject shortcut4;
    shortcut4.insert("name", tr("Slide show"));
    shortcut4.insert("value", "F5");

    QJsonObject shortcut5;
    shortcut5.insert("name", tr("Rename"));
    shortcut5.insert("value", "F2");

    QJsonObject shortcut6;
    shortcut6.insert("name", tr("Copy"));
    shortcut6.insert("value", "Ctrl + C");

    QJsonObject shortcut7;
    shortcut7.insert("name", tr("Delete"));
    shortcut7.insert("value", "Delete");

    QJsonObject shortcut8;
    shortcut8.insert("name", tr("Rotate clockwise"));
    shortcut8.insert("value", "Ctrl + R");

    QJsonObject shortcut9;
    shortcut9.insert("name", tr("Rotate counterclockwise"));
    shortcut9.insert("value", "Ctrl + Shift + R");

    QJsonObject shortcut10;
    shortcut10.insert("name", tr("Set as wallpaper"));
    shortcut10.insert("value", "Ctrl + F9");

    QJsonObject shortcut11;
    shortcut11.insert("name", tr("Display in file manager"));
    shortcut11.insert("value", "Alt + D");

    QJsonObject shortcut12;
    shortcut12.insert("name", tr("Image info"));
    shortcut12.insert("value", "Ctrl + I");

    QJsonObject shortcut13;
    shortcut13.insert("name", tr("Previous"));
    shortcut13.insert("value", "Left");

    QJsonObject shortcut14;
    shortcut14.insert("name", tr("Next"));
    shortcut14.insert("value", "Right");

    QJsonObject shortcut15;
    shortcut15.insert("name", tr("Zoom in"));
    shortcut15.insert("value", "Ctrl+ '+'");

    QJsonObject shortcut16;
    shortcut16.insert("name", tr("Zoom out"));
    shortcut16.insert("value", "Ctrl+ '-'");

    QJsonObject shortcut17;
    shortcut17.insert("name", tr("Open"));
    shortcut17.insert("value", "Ctrl+O");

    QJsonObject shortcut18;
    shortcut18.insert("name", " ");
    shortcut18.insert("value", "  ");

    QJsonArray shortcutArray1;
    shortcutArray1.append(shortcut1);
    shortcutArray1.append(shortcut2);
    shortcutArray1.append(shortcut3);
    shortcutArray1.append(shortcut4);
    shortcutArray1.append(shortcut5);
    shortcutArray1.append(shortcut6);
    shortcutArray1.append(shortcut7);
    shortcutArray1.append(shortcut8);
    shortcutArray1.append(shortcut9);
    shortcutArray1.append(shortcut10);
    shortcutArray1.append(shortcut11);
    shortcutArray1.append(shortcut12);
    shortcutArray1.append(shortcut13);
    shortcutArray1.append(shortcut14);
    shortcutArray1.append(shortcut15);
    shortcutArray1.append(shortcut16);
    shortcutArray1.append(shortcut17);
    shortcutArray1.append(shortcut18);

    QJsonObject shortcut_group1;
    shortcut_group1.insert("groupName", tr("Image Viewing"));
    shortcut_group1.insert("groupItems", shortcutArray1);

    QJsonObject shortcut19;
    shortcut19.insert("name", tr("Help"));
    shortcut19.insert("value", "F1");

    QJsonObject shortcut20;
    shortcut20.insert("name", tr("Display shortcuts"));
    shortcut20.insert("value", "Ctrl + Shift + ?");

    QJsonArray shortcutArray2;
    shortcutArray2.append(shortcut19);
    shortcutArray2.append(shortcut20);

    QJsonObject shortcut_group2;
    shortcut_group2.insert("groupName", tr("Settings"));
    shortcut_group2.insert("groupItems", shortcutArray2);

    QJsonArray shortcutArrayall;
    shortcutArrayall.append(shortcut_group1);
    shortcutArrayall.append(shortcut_group2);

    QJsonObject main_shortcut;
    main_shortcut.insert("shortcut", shortcutArrayall);

    return main_shortcut;
}

void MainWindow::initSize()
{
    //初始化大小为上次关闭大小
    if (m_mainwidow) {
        int defaultW = 0;
        int defaultH = 0;
        QDesktopWidget dw;
        if (double(dw.geometry().width()) * 0.60 < MAINWIDGET_MINIMUN_WIDTH) {
            defaultW = MAINWIDGET_MINIMUN_WIDTH;
        } else {
            defaultW = int(double(dw.geometry().width()) * 0.60);
        }

        if (double(dw.geometry().height()) * 0.60 < MAINWIDGET_MINIMUN_HEIGHT) {
            defaultH = MAINWIDGET_MINIMUN_HEIGHT;
        } else {
            defaultH = int(double(dw.geometry().height()) * 0.60);
        }


        int ww = value(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, QVariant(defaultW)).toInt();
        int wh = value(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, QVariant(defaultH)).toInt();
        m_mainwidow->resize(ww, wh);
        m_mainwidow->setMinimumSize(MAINWIDGET_MINIMUN_WIDTH, MAINWIDGET_MINIMUN_HEIGHT);
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

    m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeLocal, CACHE_PATH, nullptr, this);
    m_centerWidget->addWidget(m_imageViewer);

    m_centerWidget->setCurrentWidget(m_homePageWidget);

    connect(m_homePageWidget, &HomePageWidget::sigOpenImage, this, &MainWindow::slotOpenImg);

    connect(m_homePageWidget, &HomePageWidget::sigDrogImage, this, &MainWindow::slotDrogImg);

    //ImageEngine::instance()->sigPicCountIsNull()
    connect(ImageEngine::instance(), &ImageEngine::sigPicCountIsNull, this, [ = ] {
        m_centerWidget->setCurrentWidget(m_homePageWidget);
        if (m_mainwidow && m_mainwidow->titlebar())
        {
            //需要全屏切回普通窗口
            m_mainwidow->showNormal();
            //隐藏原有DMainWindow titlebar，使用自定义标题栏
            int height =  m_mainwidow->titlebar()->height();
            m_mainwidow->titlebar()->setFixedHeight(50);
            m_mainwidow->titlebar()->setIcon(QIcon::fromTheme("deepin-image-viewer"));
            m_mainwidow->setTitlebarShadowEnabled(true);
            int normalheight = m_mainwidow->height() + 1;
            m_mainwidow->resize(m_mainwidow->width(), normalheight);
            m_mainwidow->resize(m_mainwidow->width(), normalheight - 1);
        }
    });

    QShortcut *openFileManager = new QShortcut(QKeySequence("Ctrl+o"), this);
    openFileManager->setContext(Qt::WindowShortcut);
    connect(openFileManager, &QShortcut::activated, this, &MainWindow::slotOpenImg);

    QShortcut *scViewShortcut = new QShortcut(QKeySequence("Ctrl+Shift+/"), this);
    scViewShortcut->setObjectName(SC_VIEW_SHORTCUT);
    scViewShortcut->setContext(Qt::ApplicationShortcut);
    // connect(scE, SIGNAL(activated()), dApp, SLOT(quit()));
    connect(scViewShortcut, &QShortcut::activated, this, &MainWindow::showShortCut);

}

void MainWindow::slotOpenImg()
{
#ifdef NOUSE_TEST
    bool bRet = m_imageViewer->startChooseFileDialog();
    if (bRet) {
#else
    {
#endif
        if (m_imageViewer)
        {
            m_centerWidget->setCurrentWidget(m_imageViewer);
        }
        if (m_mainwidow && m_mainwidow->titlebar())
        {
            //隐藏原有DMainWindow titlebar，使用自定义标题栏
            int height =  m_mainwidow->titlebar()->height();
            m_mainwidow->titlebar()->setFixedHeight(0);
            m_mainwidow->titlebar()->setIcon(QIcon::fromTheme("deepin-image-viewer"));
            m_mainwidow->setTitlebarShadowEnabled(true);
            int normalheight = m_mainwidow->height() + 1;
            m_mainwidow->resize(m_mainwidow->width(), normalheight);
            m_mainwidow->resize(m_mainwidow->width(), normalheight - 1);
        }
    }

}

bool MainWindow::slotDrogImg(const QStringList &paths)
{
    bool bRet = true;
#ifdef NOUSE_TEST
    if (!m_homePageWidget->checkMinePaths(paths)) {
        bRet = false;
    }
    if (bRet) {
        bRet = m_imageViewer->startdragImage(paths);
    }
    if (bRet) {
#else
    Q_UNUSED(paths);
    bRet = true;
    {
#endif
        if (m_imageViewer) {
            m_centerWidget->setCurrentWidget(m_imageViewer);
        }
        if (m_mainwidow && m_mainwidow->titlebar()) {
            //隐藏原有DMainWindow titlebar，使用自定义标题栏
            int height =  m_mainwidow->titlebar()->height();
            m_mainwidow->titlebar()->setFixedHeight(0);
            m_mainwidow->titlebar()->setIcon(QIcon::fromTheme("deepin-image-viewer"));
            m_mainwidow->setTitlebarShadowEnabled(true);
            int normalheight = m_mainwidow->height() + 1;
            m_mainwidow->resize(m_mainwidow->width(), normalheight);
            m_mainwidow->resize(m_mainwidow->width(), normalheight - 1);
        }
    }
    return bRet;
}

void MainWindow::quitApp()
{
    this->close();
    //程序退出
    qApp->exit();
}

void MainWindow::showShortCut()
{
    qDebug() << "receive Ctrl+Shift+/";
    QRect rect = window()->geometry();
    QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);

    QStringList shortcutString;
    QJsonObject json = createShorcutJson();
    QString param1 = "-j=" + QString(QJsonDocument(json).toJson());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;//之前是 shortcutString << "-b" << param1 << param2; 增加-b在wayland下存在会重复创建的问题，就会出现闪烁

    //换用相册的方式打开deepin-shortcut-viewer
    QProcess *shortcutViewProcess = new QProcess(this);
    shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProcess, SIGNAL(finished(int)), shortcutViewProcess, SLOT(deleteLater()));
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if (!isMaximized() && !window()->isFullScreen() && !window()->isMaximized() && m_mainwidow) {
        setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, m_mainwidow->width());
        setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, m_mainwidow->height());
    }
    DWidget::resizeEvent(e);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Close) {
        //监控到mainwindow关闭，则关闭m_imageViewer
        if (m_imageViewer) {
            m_imageViewer->deleteLater();
            m_imageViewer = nullptr;
        }
    }
    return DWidget::eventFilter(obj, event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
    event->acceptProposedAction();
    DWidget::dragEnterEvent(event);
}

