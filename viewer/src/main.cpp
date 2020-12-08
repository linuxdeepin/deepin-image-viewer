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
#include "accessibility/acobjectlist.h"

#include <QApplication>
#include <DLog>
#include <QTranslator>
#include <DApplicationSettings>

using namespace Dtk::Core;

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

//    Application::loadDXcbPlugin();
    Application::instance(argc,argv);

    dApp->m_app->setAttribute(Qt::AA_ForceRasterWidgets);
    dApp->m_app->installEventFilter(dApp);
#ifdef INSTALLACCESSIBLEFACTORY
    QAccessible::installFactory(accessibleFactory);
#endif
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    qDebug() << "LogFile:" << DLogManager::getlogFilePath();

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
