// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QApplication>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QThreadPool>
#include <DLog>

#include "thumbnailcache.h"

// 定义日志分类，与 src/main.cpp 中的定义保持一致
// 由于测试不编译 main.cpp，需要在此处定义
Q_LOGGING_CATEGORY(logImageViewer, "org.deepin.dde.imageviewer")

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("deepin");
    QCoreApplication::setApplicationName("deepin-image-viewer");

    Dtk::Core::DLogManager::registerConsoleAppender();

    // 测试期间开启 debug 输出以提高行覆盖率（覆盖 qCDebug 行）。
    QLoggingCategory::setFilterRules(QStringLiteral("org.deepin.dde.imageviewer.debug=true"));

    testing::InitGoogleTest(&argc, argv);

    int result = RUN_ALL_TESTS();

    // 显式清理 ThumbnailCache 单例：imageprovider/imageinfo 的后台加载流程
    // (QtConcurrent::run) 会往单例缓存中 add QImage。
    // 必须先等待全局线程池完成所有后台任务，否则后台线程可能在 clear()
    // 之后再次 add，导致 LSan 在进程退出时报告 ThumbnailCache 泄漏。
    QThreadPool::globalInstance()->waitForDone();
    ThumbnailCache::instance()->clear();

    // 测试结束后关闭 debug 输出。原因：unionimage 中的全局静态对象
    // UnionImage_Private 的析构函数会调用 qCDebug(logImageViewer)，而在进程
    // 退出阶段 spdlog/DTK 日志器已先于该静态对象析构，会触发析构顺序冲突
    // （段错误）。关闭 debug 后该析构期日志为空操作。
    QLoggingCategory::setFilterRules(QStringLiteral("org.deepin.dde.imageviewer.debug=false"));

    return result;
}
