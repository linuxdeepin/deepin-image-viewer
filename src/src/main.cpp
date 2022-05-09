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
//#define protected public
//#include <DApplication>
//#undef protected

#include <DWidgetUtil>
#include <DMainWindow>
#include <DLog>
#include <DApplicationSettings>
#include <DVtableHook>

#include <QTranslator>
#include <QDebug>
#include <QDesktopWidget>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "mainwindow/mainwindow.h"
#include "application.h"

//using namespace Dtk::Core;

// 最小宽高
#define MAINWIDGET_MINIMUN_HEIGHT 300
#define MAINWIDGET_MINIMUN_WIDTH 628

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

//判断是否是wayland
bool CheckWayland()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive))
        return true;
    else {
        return false;
    }
}
//转换路径
QUrl UrlInfo(QString path)
{
    QUrl url;
    // Just check if the path is an existing file.
    if (QFile::exists(path)) {
        url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(path));
        return url;
    }

    const auto match = QRegularExpression(QStringLiteral(":(\\d+)(?::(\\d+))?:?$")).match(path);

    if (match.isValid()) {
        // cut away line/column specification from the path.
        path.chop(match.capturedLength());
    }

    // make relative paths absolute using the current working directory
    // prefer local file, if in doubt!
    url = QUrl::fromUserInput(path, QDir::currentPath(), QUrl::AssumeLocalFile);

    // in some cases, this will fail, e.g.
    // assume a local file and just convert it to an url.
    if (!url.isValid()) {
        // create absolute file path, we will e.g. pass this over dbus to other processes
        url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(path));
    }
    return url;
}

int main(int argc, char *argv[])
{
    //for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    //判断是否是wayland
    if (CheckWayland()) {
        //默认走xdgv6,该库没有维护了，因此需要添加该代码
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    }

//    Application::loadDXcbPlugin();
    Application  a(argc, argv);
    a.setAttribute(Qt::AA_ForceRasterWidgets);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setOrganizationName("deepin");
    a.setApplicationName("deepin-image-viewer");
    a.loadTranslator();
    a.setApplicationDisplayName(QObject::tr("Image Viewer"));
    a.setProductIcon(QIcon::fromTheme("deepin-image-viewer"));
    a.setApplicationDescription(QObject::tr("Image Viewer is an image viewing tool with fashion interface and smooth performance."));
    a.loadTranslator();
    //save theme
    DApplicationSettings saveTheme;
    Q_UNUSED(saveTheme);

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
    //修改为从单例获取

    DMainWindow *mainwindow = new DMainWindow();
    MainWindow *w = new MainWindow(mainwindow);
    mainwindow->setCentralWidget(w);
    w->setDMainWindow(mainwindow);

    //初始化大小为上次关闭大小
    if (mainwindow) {
        int defaultW = 0;
        int defaultH = 0;
        QDesktopWidget *dw = QApplication::desktop();
        if (double(dw->geometry().width()) * 0.60 < MAINWIDGET_MINIMUN_WIDTH) {
            defaultW = MAINWIDGET_MINIMUN_WIDTH;
        } else {
            defaultW = int(double(dw->geometry().width()) * 0.60);
        }

        if (double(dw->geometry().height()) * 0.60 < MAINWIDGET_MINIMUN_HEIGHT) {
            defaultH = MAINWIDGET_MINIMUN_HEIGHT;
        } else {
            defaultH = int(double(dw->geometry().height()) * 0.60);
        }
        int ww = w->value(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, QVariant(defaultW)).toInt();
        int wh = w->value(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, QVariant(defaultH)).toInt();
        mainwindow->resize(ww, wh);
        mainwindow->setMinimumSize(MAINWIDGET_MINIMUN_WIDTH, MAINWIDGET_MINIMUN_HEIGHT);
    }
    QString filepath = "";
    QStringList arguments = QCoreApplication::arguments();
    for (QString path : arguments) {
        path = UrlInfo(path).toLocalFile();
        if (QFileInfo(path).isFile()) {
            bool bRet = w->slotDrogImg(QStringList(path));
            if (bRet) {
                break;
            }
        }
    }

    mainwindow->show();

    //修复窗口会一直在中间变小的问题
    if (checkOnly()) {
        Dtk::Widget::moveToCenter(mainwindow);
    }
//    Dtk::Core::DVtableHook::overrideVfptrFun(qApp, &DApplication::handleQuitAction, w, &MainWindow::quitApp);
    QObject::connect(dApp, &Application::sigQuit, w, &MainWindow::quitApp, Qt::DirectConnection);
    return a.exec();
}
