/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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

#include <QApplication>
#include <DLog>
#include <QTranslator>

using namespace Dtk::Core;

namespace {

const char kPlatformThemeName[] = "QT_QPA_PLATFORMTHEME";

}  // namespace

int main(int argc, char *argv[])
{
    // If platform theme name is empty, fallback to gtk2.
    // gtk2 theme is included in libqt5libqgtk2 package.

    //TODO: the Qt default theme's name is empty.
    //so i comment out the code.
    // if (qgetenv(kPlatformThemeName).length() == 0) {
    //        qDebug() << qgetenv(kPlatformThemeName);
    //      qputenv(kPlatformThemeName, "gtk2");
    //    }
   //    qDebug() << "application started" << QDateTime::currentDateTime();
    if (QFile("/usr/lib/dde-desktop/plugins/platform/libdxcb.so").exists()) {
        qDebug() << "load dxcb from local path: /usr/lib/dde-desktop/plugins/platform/libdxcb.so";
        qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", "/usr/lib/dde-desktop/plugins/platform");
    }
    Application::loadDXcbPlugin();
    Application a(argc, argv);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setAttribute(Qt::AA_EnableHighDpiScaling);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    qDebug() << "LogFile:" << DLogManager::getlogFilePath();

    if (!service::isDefaultImageViewer()) {
        qDebug() << "Set defaultImage viewer succeed:" << service::setDefaultImageViewer(true);
    } else {
        qDebug() << "Deepin Image Viewer is defaultImage!";
    }

    CommandLine *cl = CommandLine::instance();

    if (cl->processOption()) {
        if (Application::isDXcbPlatform()) {
            quick_exit(a.exec());
        } else {
            return a.exec();
        }
    }
    else {
        return 0;
    }
}
