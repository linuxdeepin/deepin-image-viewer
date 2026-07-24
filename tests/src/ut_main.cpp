// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QGuiApplication>
#include <QCoreApplication>
#include <DLog>

// 定义日志分类，与 src/main.cpp 中的定义保持一致
// 由于测试不编译 main.cpp，需要在此处定义
Q_LOGGING_CATEGORY(logImageViewer, "org.deepin.dde.imageviewer")

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("deepin");
    QCoreApplication::setApplicationName("deepin-image-viewer");

    Dtk::Core::DLogManager::registerConsoleAppender();

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
