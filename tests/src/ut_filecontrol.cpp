// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_filecontrol.h"
#include "filecontrol.h"

#include <QUrl>
#include <QTemporaryFile>
#include <QImage>
#include <QStandardPaths>

void ut_filecontrol::SetUp()
{
}

void ut_filecontrol::TearDown()
{
}

// 测试构造与析构
TEST_F(ut_filecontrol, Construct)
{
    FileControl *control = new FileControl();
    ASSERT_TRUE(control != nullptr);
    delete control;
}

// 测试获取标准图片路径
TEST_F(ut_filecontrol, StandardPicturesPath)
{
    FileControl control;
    QString path = control.standardPicturesPath();
    EXPECT_FALSE(path.isEmpty());
}

// 测试获取文件名（不要求文件存在）
TEST_F(ut_filecontrol, SlotGetFileName)
{
    FileControl control;
    QString name = control.slotGetFileName("/tmp/test/image.png");
    EXPECT_FALSE(name.isEmpty());
}

// 测试获取文件后缀（需要文件存在，slotFileSuffix 内部使用 QUrl::toLocalFile）
TEST_F(ut_filecontrol, SlotFileSuffix)
{
    FileControl control;

    // 创建临时文件以测试后缀获取
    QTemporaryFile tmpFile("ut_test_suffix_XXXXXX.png");
    tmpFile.setAutoRemove(true);
    ASSERT_TRUE(tmpFile.open());

    QImage img(2, 2, QImage::Format_ARGB32);
    img.save(&tmpFile, "PNG");
    tmpFile.close();

    // slotFileSuffix 内部使用 QUrl(path).toLocalFile()，需传入 file:// URL
    QString fileUrl = QUrl::fromLocalFile(tmpFile.fileName()).toString();
    QString suffix = control.slotFileSuffix(fileUrl);
    EXPECT_FALSE(suffix.isEmpty());
}

// 测试判断是否为图片
TEST_F(ut_filecontrol, IsImage)
{
    FileControl control;
    EXPECT_TRUE(control.isImage("/tmp/test/image.jpg"));
    EXPECT_TRUE(control.isImage("/tmp/test/image.png"));
    EXPECT_FALSE(control.isImage("/tmp/test/document.txt"));
}

// 测试配置读写
TEST_F(ut_filecontrol, ConfigValue)
{
    FileControl control;
    control.setConfigValue("ut_group", "ut_key", QString("ut_value"));
    QVariant val = control.getConfigValue("ut_group", "ut_key");
    EXPECT_EQ(val.toString(), QString("ut_value"));
}
