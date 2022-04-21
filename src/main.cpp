/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QScopedPointer>
#include <QQmlContext>

#include "launcherplugin.h"
#include "thumbnailload.h"

// 此文件是QML应用的启动文件，一般无需修改
// 请在LauncherPlugin::main()中实现所需功能
int main(int argc, char *argv[])
{
    qputenv("D_POPUP_MODE", "embed");
    LauncherPlugin plugin;
    QScopedPointer<QGuiApplication> app(plugin.createApplication(argc, argv));
    QQmlApplicationEngine engine;

    //后端缩略图加载
    LoadImage *load = new LoadImage();
    engine.rootContext()->setContextProperty("CodeImage", load);
    engine.addImageProvider(QLatin1String("CodeImg"), load->m_pThumbnail);

    return plugin.main(app.get(), &engine);
}
