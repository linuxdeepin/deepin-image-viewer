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

// 单实例
Q_GLOBAL_STATIC(ImageSourceModel, ImageSourceModelInstance)

// 此文件是QML应用的启动文件，一般无需修改
// 请在LauncherPlugin::main()中实现所需功能
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

    const QString uri("org.deepin.image.viewer");
    qmlRegisterType<ImageInfo>(uri.toUtf8().data(), 1, 0, "ImageInfo");
    qmlRegisterUncreatableType<Types>(uri.toUtf8().data(), 1, 0, "Types", "Types only use for define");

    ImageSourceModel *imageSourceModel = ImageSourceModelInstance();
    qmlRegisterSingletonType<ImageSourceModel>(
        uri.toUtf8().data(), 1, 0, "ImageSourceModel", [](QQmlEngine *, QJSEngine *) -> QObject * {
            return ImageSourceModelInstance();
        });

    GlobalControl control;
    control.setGlobalModel(imageSourceModel);
    engine.rootContext()->setContextProperty("GControl", &control);
    GlobalStatus status;
    engine.rootContext()->setContextProperty("GStatus", &status);

    // 请在此处注册需要导入到QML中的C++类型
    // 例如： engine.rootContext()->setContextProperty("Utils", new Utils);
    // 后端缩略图加载，使用 QMLEngine 管理生命周期
    MultiImageLoad *multiImagaLoad = new MultiImageLoad;
    engine.addImageProvider(QLatin1String("Multiimage"), multiImagaLoad);

    FileControl *fileControl = new FileControl();
    engine.rootContext()->setContextProperty("fileControl", fileControl);
    // 关联文件处理（需要保证优先处理，onImageFileChanged已做多线程安全）
    //    QObject::connect(
    //        fileControl, &FileControl::requestImageFileChanged, load, [&](const QString &filePath, bool isMultiImage, bool
    //        isExist) {
    //            // 更新缓存信息
    //            load->onImageFileChanged(filePath, isMultiImage, isExist);
    //            // 处理完成后加载图片
    //            emit fileControl->imageFileChanged(filePath, isMultiImage, isExist);
    //        });

    // 光标位置查询工具
    CursorTool *cursorTool = new CursorTool();
    engine.rootContext()->setContextProperty("cursorTool", cursorTool);
    // OCR分析工具
    auto liveTextAnalyzer = new LiveTextAnalyzer;
    engine.rootContext()->setContextProperty("liveTextAnalyzer", liveTextAnalyzer);
    engine.addImageProvider(QLatin1String("liveTextAnalyzer"), liveTextAnalyzer);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    // 设置DBus接口
    ApplicationAdaptor adaptor(fileControl);
    QDBusConnection::sessionBus().registerService("com.deepin.imageViewer");
    QDBusConnection::sessionBus().registerObject("/", fileControl);

    return app->exec();
}
