// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_globalcontrol.h"
#include "globalcontrol.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QImage>
#include <QFile>
#include <QStandardPaths>

void ut_globalcontrol::SetUp()
{
}

void ut_globalcontrol::TearDown()
{
}

// 测试构造与析构
TEST_F(ut_globalcontrol, Construct)
{
    GlobalControl *control = new GlobalControl();
    ASSERT_TRUE(control != nullptr);
    delete control;
}

// 测试默认索引值
TEST_F(ut_globalcontrol, DefaultIndex)
{
    GlobalControl control;
    EXPECT_EQ(control.currentIndex(), 0);
    EXPECT_EQ(control.currentFrameIndex(), 0);
    EXPECT_EQ(control.imageCount(), 0);
}

// 在临时目录中创建指定数量的测试图片文件，由 QTemporaryDir 自动清理
static QStringList createTestImageFiles(const QTemporaryDir &dir, int count)
{
    QStringList files;
    for (int i = 0; i < count; ++i) {
        QString path = dir.filePath(QString("ut_test_img_%1.png").arg(i));
        QImage img(10, 10, QImage::Format_ARGB32);
        img.fill(Qt::red);
        if (img.save(path, "PNG")) {
            files << path;
        }
    }
    return files;
}

// 测试设置图片文件列表
TEST_F(ut_globalcontrol, SetImageFiles)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 3);
    ASSERT_EQ(files.size(), 3);

    control.setImageFiles(files, files.value(1));
    EXPECT_EQ(control.imageCount(), 3);
}

// 测试设置当前索引
TEST_F(ut_globalcontrol, CurrentIndex)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 3);
    ASSERT_EQ(files.size(), 3);

    control.setImageFiles(files, files.value(0));
    EXPECT_EQ(control.imageCount(), 3);

    QSignalSpy spy(&control, &GlobalControl::currentIndexChanged);
    control.setCurrentIndex(2);
    EXPECT_EQ(control.currentIndex(), 2);
    EXPECT_EQ(spy.count(), 1);
}

// 测试旋转角度设置
TEST_F(ut_globalcontrol, CurrentRotation)
{
    GlobalControl control;
    EXPECT_EQ(control.currentRotation(), 0);

    QSignalSpy spy(&control, &GlobalControl::currentRotationChanged);
    control.setCurrentRotation(90);
    EXPECT_EQ(control.currentRotation(), 90);
    EXPECT_EQ(spy.count(), 1);
}
