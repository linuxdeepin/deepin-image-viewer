// SPDX-FileCopyrightText: 2020-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"

#include <libimageviewer/image-viewer_global.h>
#ifdef IMAGEVIEWER_CLASS_QUICKPRINT
#include <libimageviewer/quickprint.h>
#endif
#include <QtDebug>
#include <QCommandLineParser>

Application::Application(int &argc, char **argv)
    : DApplication(argc, argv)
{
}

Application::~Application()
{
    Q_EMIT sigQuit();
}

/**
   @brief 解析命令行参数
 */
void Application::parseCommandLine()
{
    QCommandLineParser parser;
    QCommandLineOption newWindowOption("new-window");
    QCommandLineOption configOption("config", "permission config", "base64 param");
    QCommandLineOption printOption("print");

    parser.addOption(newWindowOption);
    parser.addOption(configOption);
    parser.addOption(printOption);

    if (!parser.parse(arguments())) {
        qWarning() << qPrintable("Parse param error:") << parser.errorText();
    }

    if (parser.isSet(newWindowOption)) {
        cmdWithParam.insert(NewWindow, {});
    }

    if (parser.isSet(configOption)) {
        cmdWithParam.insert(Config, parser.value(configOption));
    }

    if (parser.isSet(printOption)) {
        cmdWithParam.insert(Print, parser.value(printOption));
    }

    fileList = parser.positionalArguments();
}

/**
   @brief 判断是否设置了 \a cmd 指令，若指令存在参数，且 \a param 不为 nullptr , 将写入参数数据
 */
bool Application::isCommandSet(Application::Command cmd, QString *param)
{
    if (!cmdWithParam.contains(cmd)) {
        return false;
    }

    if (param) {
        *param = cmdWithParam.value(cmd);
    }
    return true;
}

/**
   @brief 返回解析的文件列表
 */
QStringList Application::parseFileList() const
{
    return fileList;
}

/**
   @brief 触发打印图片操作
 */
int Application::triggerPrintAction()
{
#ifdef IMAGEVIEWER_CLASS_QUICKPRINT
    qInfo() << "Trigger print pictures";

    // 调用异步打印，打印完成后退出
    QuickPrint print;
    connect(&print, &QuickPrint::printFinished, this, &Application::quit);
    if (!print.showPrintDialogAsync(fileList)) {
        return 1;
    }

    return exec();
#else
    return 1;
#endif
}
