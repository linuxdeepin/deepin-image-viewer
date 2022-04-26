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

#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <DApplication>
#include "src/filecontrol.h"

#include "launcherplugin.h"

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

LauncherPlugin::LauncherPlugin(QObject *parent)
    : QObject(parent)
{

}

LauncherPlugin::~LauncherPlugin()
{

}

#include <QQuickWindow>

int LauncherPlugin::main(QGuiApplication *app, QQmlApplicationEngine *engine)
{
    // 请在此处注册需要导入到QML中的C++类型
    // 例如： engine->rootContext()->setContextProperty("Utils", new Utils);
    engine->rootContext()->setContextProperty("fileControl", new FileControl());
    engine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine->rootObjects().isEmpty())
        return -1;
//    auto window = qobject_cast<QQuickWindow *>(engine->rootObjects().first());
//    if (window) {
//        window->setIcon(QIcon("qrc:/icon/deepin-image-viewer.svg"));
//    }
    app->setWindowIcon(QIcon::fromTheme("deepin-image-viewer"));
    app->setApplicationDisplayName(QObject::tr("Image Viewer"));
//    app->setApplicationDescription(QObject::tr("Image Viewer is an image viewing tool with fashion interface and smooth performance."));
    app->setApplicationName("deepin-image-viewer");
    app->setApplicationDisplayName(QObject::tr("Image Viewer"));
    return app->exec();
}

QGuiApplication *LauncherPlugin::createApplication(int &argc, char **argv)
{
    // 与 DAppLoader 的关系为：该函数会由 DAppLoader::createApplication() 调用，
    // 然后返回值作为 DAppLoader::exec() 的一个参数，接着 DAppLoader::exec() 会调用
    // LauncherPlugin::main()，最终启动整个程序显示界面。
    // 重写此接口的目的：
    // 1.可以使用自己创建的 QGuiApplication 对象；
    // 2.可以在创建 QGuiApplication 之前为程序设置一些属性（如使用
    //   QCoreApplication::setAttribute 禁用屏幕缩放）；
    // 3.可以添加一些在 QGuiApplication 构造过程中才需要的环境变量；

    // TODO: 无 XDG_CURRENT_DESKTOP 变量时，将不会加载 deepin platformtheme 插件，会导致
    // 查找图标的接口无法调用 qt5integration 提供的插件，后续应当把图标查找相关的功能移到 dtkgui
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    // 注意:请不要管理 QGuiApplication 对象的生命周期！
    DApplication *a = new DApplication(argc, argv);
    a->loadTranslator();
    a->setApplicationLicense("GPLV3");
    a->setApplicationVersion("1.0.0");
    a->setOrganizationName("deepin");
    a->setApplicationName("deepin-image-viewer");
    a->setApplicationDisplayName(QObject::tr("Image Viewer"));
    a->setProductIcon(QIcon::fromTheme("deepin-image-viewer"));
    a->setApplicationDescription(QObject::tr("Image Viewer is an image viewing tool with fashion interface and smooth performance."));
    // a->setApplicationName("Launch-plugin");
    a->loadTranslator();

    return a;
}
