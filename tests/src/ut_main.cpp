// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QApplication>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <DLog>

// 定义日志分类，与 src/main.cpp 中的定义保持一致
// 由于测试不编译 main.cpp，需要在此处定义
Q_LOGGING_CATEGORY(logImageViewer, "org.deepin.dde.imageviewer")

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("deepin");
    QCoreApplication::setApplicationName("deepin-image-viewer");

    Dtk::Core::DLogManager::registerConsoleAppender();

    // 关闭该分类的 debug 输出。原因：unionimage 中的全局静态对象
    // UnionImage_Private 的析构函数会调用 qCDebug(logImageViewer)，而在进程
    // 退出阶段 spdlog/DTK 日志器已先于该静态对象析构，会触发析构顺序冲突
    // （段错误，已影响 ut_unionimage）。关闭 debug 后该析构期日志为空操作。
    QLoggingCategory::setFilterRules(QStringLiteral("org.deepin.dde.imageviewer.debug=false"));

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
