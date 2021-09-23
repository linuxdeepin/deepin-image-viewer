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
#include "application.h"
#include "controller/commandline.h"
#include "service/defaultimageviewer.h"
#include "accessibility/acobjectlist.h"
#ifdef CMAKE_BUILD
#include "config.h"
#endif
#include <QApplication>
#include <DLog>
#include <QTranslator>
#include <DApplicationSettings>
#include <DVtableHook>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
using namespace Dtk::Core;
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
    //判断是否是wayland
    if (CheckWayland()) {
        //默认走xdgv6,该库没有维护了，因此需要添加该代码
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    }

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

//    Application::loadDXcbPlugin();
    Application::instance(argc, argv);

    dApp->m_app->setAttribute(Qt::AA_ForceRasterWidgets);
    dApp->m_app->installEventFilter(dApp);
#ifdef CMAKE_BUILD
    //设置版本号
    dApp->m_app->setApplicationVersion(DApplication::buildVersion(VERSION));
#endif
    DVtableHook::overrideVfptrFun(dApp->m_app, &DApplication::handleQuitAction, dApp, &Application::quitApp);
#ifdef INSTALLACCESSIBLEFACTORY
    QAccessible::installFactory(accessibleFactory);
#endif
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    qDebug() << "LogFile:" << DLogManager::getlogFilePath();

    if (dApp->isPanelDev()) {
        //将时间写入QDataStream
        QDateTime wstime = QDateTime::currentDateTime();
        bool newflag = true;

        if (!checkOnly()) {
            newflag = false;
        }
        //save theme
        DApplicationSettings saveTheme;
        CommandLine *cl = CommandLine::instance();

        qDebug() << "133行";
        if (cl->processOption(wstime, newflag)) {
            qDebug() << "135行dApp->m_app->exec()";
            return dApp->m_app->exec();
        } else {
            qDebug() << "138行return0";
            return 0;
        }

    } else {
#ifndef LITE_DIV
        if (!service::isDefaultImageViewer()) {
            qDebug() << "Set defaultImage viewer succeed:" << service::setDefaultImageViewer(true);
        } else {
            qDebug() << "Deepin Image Viewer is defaultImage!";
        }
#endif
        //save theme
        DApplicationSettings saveTheme;
        CommandLine *cl = CommandLine::instance();
        if (cl->processOption()) {
            return dApp->m_app->exec();
        } else {
            return 0;
        }
    }

}
