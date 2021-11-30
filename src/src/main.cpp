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
#define protected public
#include <DApplication>
#undef protected
#include <DWidgetUtil>
#include <DMainWindow>
#include <DLog>
#include <DApplicationSettings>
#include <DVtableHook>

#include <QTranslator>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "mainwindow/mainwindow.h"

//using namespace Dtk::Core;


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

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

//    Application::loadDXcbPlugin();
    DApplication  a(argc, argv);
    a.setAttribute(Qt::AA_ForceRasterWidgets);
    a.loadTranslator();
    a.setOrganizationName("deepin");
    a.setApplicationName("deepin-image-viewer");
    a.setApplicationDisplayName(QObject::tr("Image Viewer"));
    a.setProductIcon(QIcon::fromTheme("deepin-image-viewer"));
    a.setApplicationDescription(QObject::tr("Image Viewer is an image viewing tool with fashion interface and smooth performance."));
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
//    w->processOption();
    w->initSize();
    for (int i = 1; i < argc; ++i) {
        QString path = argv[i];
        if (QFileInfo(path).isFile()) {
            bool bRet = w->slotDrogImg(QStringList(argv[i]));
            if (!bRet) {
                return 0;
            }
            break;
        }
    }

    mainwindow->show();

    //修复窗口会一直在中间变小的问题
    if (checkOnly()) {
        Dtk::Widget::moveToCenter(mainwindow);
    }
    Dtk::Core::DVtableHook::overrideVfptrFun(qApp, &DApplication::handleQuitAction, w, &MainWindow::quitApp);

    return a.exec();
}
