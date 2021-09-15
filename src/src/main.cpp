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

#ifdef CMAKE_BUILD
#include "config.h"
#endif
#include <DApplication>
#include <DWidgetUtil>
#include <DMainWindow>
#include <DLog>
#include <DApplicationSettings>

#include <QTranslator>
#include <QDesktopWidget>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "mainwindow/mainwindow.h"

//using namespace Dtk::Core;
const int MAINWIDGET_MINIMUN_HEIGHT = 335;
const int MAINWIDGET_MINIMUN_WIDTH = 730;//增加了ocr，最小宽度为630到现在730


DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

bool checkOnly()
{
    //single
    QString userName = QDir::homePath().section("/", -1, -1);
    std::string path = ("/home/" + userName + "/.cache/deepin/deepin-image-viewer/").toStdString();
    QDir tdir(path.c_str());
    if (!tdir.exists()) {
        bool ret =  tdir.mkpath(path.c_str());
        qDebug() << ret ;
    }

    path += "single";
    int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    int flock = lockf(fd, F_TLOCK, 0);

    if (fd == -1) {
        perror("open lockfile/n");
        return false;
    }
    if (flock == -1) {
        perror("lock file error/n");
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    //for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

//    Application::loadDXcbPlugin();
    DApplication  a(argc, argv);
    a.loadTranslator();
    a.setApplicationDisplayName(QObject::tr("Image Viewer"));
    a.setProductIcon(QIcon::fromTheme("deepin-image-viewer"));
    a.setApplicationDescription(QObject::tr("Image Viewer is an image viewing tool with fashion interface and smooth performance."));

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    qDebug() << "LogFile:" << DLogManager::getlogFilePath();
    a.setApplicationVersion("1.0.0");
#ifdef CMAKE_BUILD
    //增加版本号
//    a.setApplicationVersion(DApplication::buildVersion(VERSION));
    a.setApplicationVersion(VERSION);
#endif

    //主窗体应该new出来,不应该是static变量
    MainWindow *w = new MainWindow();  //修改为从单例获取

    DMainWindow *mainwindow = new DMainWindow();
    mainwindow->setCentralWidget(w);
    w->setDMainWindow(mainwindow);
//    w->processOption();
    for (int i = 1; i < argc; ++i) {
        QString path = argv[i];
        if (QFileInfo(path).isFile()) {
            w->slotDrogImg(QStringList(argv[i]));
            break;
        }
    }
    //初始化大小为上次关闭大小
    QDesktopWidget dw;
    const int defaultW = dw.geometry().width() * 0.60 < MAINWIDGET_MINIMUN_WIDTH
                         ? MAINWIDGET_MINIMUN_WIDTH
                         : dw.geometry().width() * 3 / 5;
    const int defaultH = dw.geometry().height() * 0.60 < MAINWIDGET_MINIMUN_HEIGHT
                         ? MAINWIDGET_MINIMUN_HEIGHT
                         : dw.geometry().height() * 3 / 5;
    const int ww =
        w->value(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, QVariant(defaultW)).toInt();
    const int wh =
        w->value(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, QVariant(defaultH)).toInt();
    mainwindow->resize(ww, wh);
    mainwindow->setMinimumSize(MAINWIDGET_MINIMUN_WIDTH, MAINWIDGET_MINIMUN_HEIGHT);
    mainwindow->show();

    //修复窗口会一直在中间变小的问题
    if (checkOnly()) {
        Dtk::Widget::moveToCenter(mainwindow);
    }

    return a.exec();
}
