// SPDX-FileCopyrightText: 2020-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLICATION_H
#define APPLICATION_H

#include <DApplication>
DWIDGET_USE_NAMESPACE
using namespace Dtk::Core;

#define dApp (static_cast<Application *>(QCoreApplication::instance()))
class Application : public DApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    ~Application();

    Q_SIGNAL void sigQuit();

    enum Command {  // CLI参数类型
        None,
        NewWindow,  // 新窗口
        Config,     // 权限控制(带有Base64 json参数)
        Print,      // 快速打印图片
    };

    void parseCommandLine();
    bool isCommandSet(Command cmd, QString *param = nullptr);
    QStringList parseFileList() const;

    // 触发打印图片操作
    int triggerPrintAction();

private:
    QMap<Command, QString> cmdWithParam;  // 命令-参数映射表
    QStringList fileList;                 // 解析的待处理文件列表
};

#endif  // APPLICATION_H
