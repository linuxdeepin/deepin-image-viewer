// SPDX-FileCopyrightText: 2020 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/filecontrol.h"
#include "src/cursortool.h"
#include "src/ocr/livetextanalyzer.h"
#include "src/dbus/applicationadpator.h"
#include "src/globalcontrol.h"
#include "src/globalstatus.h"
#include "src/types.h"
#include "src/imagedata/imageinfo.h"
#include "src/imagedata/imagesourcemodel.h"
#include "src/imagedata/multiimageload.h"
#include "config.h"

#include <DApplication>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QScopedPointer>
#include <QQmlContext>
#include <QIcon>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    qputenv("D_POPUP_MODE", "embed");
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    // 注意:请不要管理 QGuiApplication 对象的生命周期！
    DApplication *app = new DApplication(argc, argv);
    app->loadTranslator();
    app->setApplicationLicense("GPLV3");
    app->setApplicationVersion(VERSION);
    app->setOrganizationName("deepin");
    app->setApplicationName("deepin-image-viewer");
    app->setApplicationDisplayName(QObject::tr("Image Viewer"));
    app->setProductIcon(QIcon::fromTheme("deepin-image-viewer"));
    app->setApplicationDescription(
        QObject::tr("Image Viewer is an image viewing tool with fashion interface and smooth performance."));
    app->setWindowIcon(QIcon::fromTheme("deepin-image-viewer"));

    QQmlApplicationEngine engine;

    // 请在此处注册需要导入到QML中的C++类型
    // 例如： engine.rootContext()->setContextProperty("Utils", new Utils);
    const QString uri("org.deepin.image.viewer");
    qmlRegisterType<ImageInfo>(uri.toUtf8().data(), 1, 0, "ImageInfo");
    qmlRegisterType<ImageSourceModel>(uri.toUtf8().data(), 1, 0, "ImageSourceModel");
    qmlRegisterUncreatableType<Types>(uri.toUtf8().data(), 1, 0, "Types", "Types only use for define");

    // 全局单例
    GlobalControl control;
    engine.rootContext()->setContextProperty("GControl", &control);
    GlobalStatus status;
    engine.rootContext()->setContextProperty("GStatus", &status);
    FileControl fileControl;
    engine.rootContext()->setContextProperty("fileControl", &fileControl);
    QObject::connect(&fileControl, &FileControl::imageRenamed, &control, &GlobalControl::renameImage);
    status.setenableNavigation(fileControl.isEnableNavigation());
    QObject::connect(
        &status, &GlobalStatus::enableNavigationChanged, [&]() { fileControl.setEnableNavigation(status.enableNavigation()); });
    // 光标位置查询工具
    CursorTool cursorTool;
    engine.rootContext()->setContextProperty("cursorTool", &cursorTool);

    // 后端缩略图加载，由 QMLEngine 管理生命周期
    MultiImageLoad *multiImagaLoad = new MultiImageLoad;
    engine.addImageProvider(QLatin1String("Multiimage"), multiImagaLoad);

    // OCR分析工具
    auto liveTextAnalyzer = new LiveTextAnalyzer;
    engine.rootContext()->setContextProperty("liveTextAnalyzer", liveTextAnalyzer);
    engine.addImageProvider(QLatin1String("liveTextAnalyzer"), liveTextAnalyzer);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    // 设置DBus接口
    ApplicationAdaptor adaptor(&fileControl);
    QDBusConnection::sessionBus().registerService("com.deepin.imageViewer");
    QDBusConnection::sessionBus().registerObject("/", &fileControl);

    return app->exec();
}
