// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/filecontrol.h"
#include "src/thumbnailload.h"
#include "src/cursortool.h"
#include "src/ocr/livetextanalyzer.h"
#include "config.h"

#include <DApplication>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QScopedPointer>
#include <QQmlContext>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

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
    app->setApplicationDescription(QObject::tr("Image Viewer is an image viewing tool with fashion interface and smooth performance."));
    app->setWindowIcon(QIcon::fromTheme("deepin-image-viewer"));
    
    QQmlApplicationEngine engine;
    // 请在此处注册需要导入到QML中的C++类型
    // 例如： engine.rootContext()->setContextProperty("Utils", new Utils);
    // 后端缩略图加载
    LoadImage *load = new LoadImage();
    engine.rootContext()->setContextProperty("CodeImage", load);
    engine.addImageProvider(QLatin1String("ThumbnailImage"), load->m_pThumbnail);
    engine.addImageProvider(QLatin1String("viewImage"), load->m_viewLoad);
    // 后端多页图加载
    engine.addImageProvider(QLatin1String("multiimage"), load->m_multiLoad);

    FileControl *fileControl = new FileControl();
    engine.rootContext()->setContextProperty("fileControl", fileControl);
    // 关联文件处理（需要保证优先处理，onImageFileChanged已做多线程安全）
    QObject::connect(fileControl, &FileControl::requestImageFileChanged,
                     load, [&](const QString &filePath, bool isMultiImage, bool isExist){
        // 更新缓存信息
        load->onImageFileChanged(filePath, isMultiImage, isExist);
        // 处理完成后加载图片
        emit fileControl->imageFileChanged(filePath, isMultiImage, isExist);
    });

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

    return app->exec();
}
