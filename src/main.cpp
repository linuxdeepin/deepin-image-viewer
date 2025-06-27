// SPDX-FileCopyrightText: 2020 ~ 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/filecontrol.h"
#include "src/cursortool.h"
#include "src/ocr/livetextanalyzer.h"
#include "src/dbus/applicationadpator.h"
#include "src/declarative/mousetrackitem.h"
#include "src/declarative/pathviewrangehandler.h"
#include "src/globalcontrol.h"
#include "src/globalstatus.h"
#include "src/types.h"
#include "src/imagedata/imageinfo.h"
#include "src/imagedata/imagesourcemodel.h"
#include "src/imagedata/imageprovider.h"
#include "src/utils/filetrashhelper.h"
#include "src/commandparser.h"
#include "config.h"

#include <DApplication>
#include <DLog>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QScopedPointer>
#include <QQmlContext>
#include <QIcon>

Q_LOGGING_CATEGORY(logImageViewer, "org.deepin.dde.imageviewer")
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    qCDebug(logImageViewer) << "Application starting...";
    qputenv("D_POPUP_MODE", "embed");
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qCDebug(logImageViewer) << "XDG_CURRENT_DESKTOP is empty, setting to Deepin.";
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    DApplication app(argc, argv);
    app.loadTranslator();
    qCDebug(logImageViewer) << "Translator loaded.";
    app.setApplicationLicense("GPLV3");
    app.setApplicationVersion(VERSION);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-image-viewer");
    app.setApplicationDisplayName(QObject::tr("Image Viewer"));
    app.setProductIcon(QIcon::fromTheme("deepin-image-viewer"));
    app.setApplicationDescription(
            QObject::tr("Image Viewer is an image viewing tool with fashion interface and smooth performance."));
    app.setWindowIcon(QIcon::fromTheme("deepin-image-viewer"));

    // LOG
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    DLogManager::registerJournalAppender();
    qCInfo(logImageViewer) << QString("%1 start, PID: %2, Version: %3")
                       .arg(app.applicationName())
                       .arg(app.applicationPid())
                       .arg(app.applicationVersion());
    qCDebug(logImageViewer) << "LogFile:" << DLogManager::getlogFilePath();

    // command
    qCDebug(logImageViewer) << "CommandParser processing...";
    CommandParser::instance()->process();
    if (CommandParser::instance()->isSet("h")) {
        qCDebug(logImageViewer) << "Help option detected, exiting application.";
        return app.exec();
    } else if (CommandParser::instance()->isSet("print")) {
        qCDebug(logImageViewer) << "Print option detected, performing quick print.";
        CommandParser::instance()->quickPrint();
        return 0;
    }

    QQmlApplicationEngine engine;
    qCDebug(logImageViewer) << "QQmlApplicationEngine created.";

    // 请在此处注册需要导入到QML中的C++类型
    // 例如： engine.rootContext()->setContextProperty("Utils", new Utils);
    // @uri org.deepin.image.viewer
    const QString uri("org.deepin.image.viewer");
    qmlRegisterType<ImageInfo>(uri.toUtf8().data(), 1, 0, "ImageInfo");
    qCDebug(logImageViewer) << "ImageInfo registered.";
    qmlRegisterUncreatableType<ImageSourceModel>(uri.toUtf8().data(), 1, 0, "ImageSourceModel", "Use for global data");
    qCDebug(logImageViewer) << "ImageSourceModel registered.";
    qmlRegisterUncreatableType<PathViewProxyModel>(uri.toUtf8().data(), 1, 0, "PathViewProxyModel", "Use for view data");
    qCDebug(logImageViewer) << "PathViewProxyModel registered.";
    qmlRegisterType<MouseTrackItem>(uri.toUtf8().data(), 1, 0, "MouseTrackItem");
    qCDebug(logImageViewer) << "MouseTrackItem registered.";
    qmlRegisterType<PathViewRangeHandler>(uri.toUtf8().data(), 1, 0, "PathViewRangeHandler");
    qCDebug(logImageViewer) << "PathViewRangeHandler registered.";

    qmlRegisterUncreatableType<Types>(uri.toUtf8().data(), 1, 0, "Types", "Types only use for define");
    qCDebug(logImageViewer) << "Types registered.";
    // 文件回收站处理
    qmlRegisterType<FileTrashHelper>(uri.toUtf8().data(), 1, 0, "FileTrashHelper");
    qCDebug(logImageViewer) << "FileTrashHelper registered.";

    // QML全局单例
    GlobalControl control;
    qmlRegisterSingletonInstance<GlobalControl>(uri.toUtf8().data(), 1, 0, "GControl", &control);
    qCDebug(logImageViewer) << "GlobalControl singleton registered.";
    GlobalStatus status;
    qmlRegisterSingletonInstance<GlobalStatus>(uri.toUtf8().data(), 1, 0, "GStatus", &status);
    qCDebug(logImageViewer) << "GlobalStatus singleton registered.";
    FileControl fileControl;
    qmlRegisterSingletonInstance<FileControl>(uri.toUtf8().data(), 1, 0, "FileControl", &fileControl);
    qCDebug(logImageViewer) << "FileControl singleton registered.";
    // 光标位置查询工具
    CursorTool cursorTool;
    qmlRegisterSingletonInstance<CursorTool>(uri.toUtf8().data(), 1, 0, "CursorTool", &cursorTool);
    qCDebug(logImageViewer) << "CursorTool singleton registered.";

    // 解析命令行参数
    QString cliParam = fileControl.parseCommandlineGetPath();
    qCDebug(logImageViewer) << "Commandline parameter parsed: " << cliParam;

    // 后端缩略图加载，由 QMLEngine 管理生命周期
    // 部分平台支持线程数较低时，使用同步加载
    ProviderCache *providerCache = nullptr;
    if (!GlobalControl::enableMultiThread()) {
        qCDebug(logImageViewer) << "Multi-threading disabled, using ImageProvider (sync).";
        ImageProvider *imageProvider = new ImageProvider;
        engine.addImageProvider(QLatin1String("ImageLoad"), imageProvider);

        providerCache = static_cast<ProviderCache *>(imageProvider);
    } else {
        qCDebug(logImageViewer) << "Multi-threading enabled, using AsyncImageProvider (async).";
        AsyncImageProvider *asyncImageProvider = new AsyncImageProvider;
        engine.addImageProvider(QLatin1String("ImageLoad"), asyncImageProvider);

        providerCache = static_cast<ProviderCache *>(asyncImageProvider);

        if (!cliParam.isEmpty()) {
            qCDebug(logImageViewer) << "Preloading image from commandline parameter.";
            asyncImageProvider->preloadImage(cliParam);
        }
    }

    ThumbnailProvider *multiImageLoad = new ThumbnailProvider;
    engine.addImageProvider(QLatin1String("ThumbnailLoad"), multiImageLoad);
    qCDebug(logImageViewer) << "ThumbnailProvider registered.";

    // 关联各组件
    // 图片旋转时更新图像缓存
    QObject::connect(&control, &GlobalControl::requestRotateCacheImage, [&]() {
        qCDebug(logImageViewer) << "Rotating image cache.";
        providerCache->rotateImageCached(control.currentRotation(), control.currentSource().toLocalFile());
    });
    qCDebug(logImageViewer) << "Connect signal requestRotateCacheImage.";

    status.setEnableNavigation(fileControl.isEnableNavigation());
    qCDebug(logImageViewer) << "Enable navigation set to: " << status.enableNavigation();
    QObject::connect(
            &status, &GlobalStatus::enableNavigationChanged, [&]() {
        qCDebug(logImageViewer) << "Enable navigation changed to: " << status.enableNavigation();
        fileControl.setEnableNavigation(status.enableNavigation());
    });
    qCDebug(logImageViewer) << "Connect signal enableNavigationChanged.";
    QObject::connect(&fileControl, &FileControl::imageRenamed, &control, [&](const QUrl &oldName, const QUrl &newName) {
        qCDebug(logImageViewer) << "Image renamed from " << oldName.toLocalFile() << " to " << newName.toLocalFile();
        providerCache->renameImageCache(oldName.toLocalFile(), newName.toLocalFile());
        control.renameImage(oldName, newName);
    });
    qCDebug(logImageViewer) << "Connect signal imageRenamed.";
    // 文件变更时清理缓存
    QObject::connect(&fileControl, &FileControl::imageFileChanged, [&](const QString &fileName) {
        qCDebug(logImageViewer) << "Image file changed: " << fileName << ", removing from cache.";
        providerCache->removeImageCache(fileName);
    });
    qCDebug(logImageViewer) << "Connect signal imageFileChanged.";

    // OCR分析工具
    auto liveTextAnalyzer = new LiveTextAnalyzer;
    engine.rootContext()->setContextProperty("liveTextAnalyzer", liveTextAnalyzer);
    engine.addImageProvider(QLatin1String("liveTextAnalyzer"), liveTextAnalyzer);
    qCDebug(logImageViewer) << "LiveTextAnalyzer registered.";

    // 判断命令行数据，在 QML 前优先加载
    if (!cliParam.isEmpty()) {
        qCDebug(logImageViewer) << "Commandline parameter is not empty, processing initial image.";
        QStringList filePaths = fileControl.getDirImagePath(cliParam);
        if (!filePaths.isEmpty()) {
            qCDebug(logImageViewer) << "Found image files: " << filePaths.count();
            control.setImageFiles(filePaths, cliParam);
            fileControl.resetImageFiles(filePaths);

            status.setStackPage(Types::ImageViewPage);
            qCDebug(logImageViewer) << "Set stack page to ImageViewPage.";
        } else {
            qCDebug(logImageViewer) << "No image files found for the given commandline parameter.";
        }
    } else {
        qCDebug(logImageViewer) << "Commandline parameter is empty, no initial image to load.";
    }

    qCDebug(logImageViewer) << "Loading QML file: qrc:/qt/qml/IVModule/qml/main.qml";
    engine.load(QUrl("qrc:/qt/qml/IVModule/qml/main.qml"));
    if (engine.rootObjects().isEmpty()) {
        qCCritical(logImageViewer) << "Failed to load QML file, root objects empty.";
        return -1;
    }
    qCDebug(logImageViewer) << "QML file loaded successfully.";

    // 设置DBus接口
    ApplicationAdaptor adaptor(&fileControl);
    QDBusConnection::sessionBus().registerService("com.deepin.imageViewer");
    qCDebug(logImageViewer) << "DBus service 'com.deepin.imageViewer' registered.";
    QDBusConnection::sessionBus().registerObject("/", &fileControl);
    qCDebug(logImageViewer) << "DBus object '/' registered.";

    return app.exec();
}
