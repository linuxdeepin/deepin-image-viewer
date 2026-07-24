// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_imageinfo.h"
#include "imageinfo.h"

#include <QUrl>
#include <QSignalSpy>

void ut_imageinfo::SetUp()
{
}

void ut_imageinfo::TearDown()
{
}

// 测试默认构造
TEST_F(ut_imageinfo, DefaultConstruct)
{
    ImageInfo info;
    EXPECT_EQ(info.status(), ImageInfo::Null);
    EXPECT_EQ(info.frameIndex(), 0);
    // 默认 frameCount 为 1（空图片按单帧处理）
    EXPECT_EQ(info.frameCount(), 1);
    // 默认 width/height 为 -1（未加载）
    EXPECT_EQ(info.width(), -1);
    EXPECT_EQ(info.height(), -1);
}

// 测试设置 source
TEST_F(ut_imageinfo, SetSource)
{
    ImageInfo info;
    QSignalSpy spy(&info, &ImageInfo::sourceChanged);

    QUrl testUrl("file:///tmp/nonexistent_test_image.jpg");
    info.setSource(testUrl);

    EXPECT_EQ(info.source(), testUrl);
}

// 测试设置 frameIndex
TEST_F(ut_imageinfo, SetFrameIndex)
{
    ImageInfo info;
    QSignalSpy spy(&info, &ImageInfo::frameIndexChanged);

    info.setFrameIndex(0);
    EXPECT_EQ(info.frameIndex(), 0);
}

// 测试运行时属性默认值
TEST_F(ut_imageinfo, RuntimeProperties)
{
    ImageInfo info;

    // 运行时属性默认值
    EXPECT_EQ(info.x(), 0);
    EXPECT_EQ(info.y(), 0);
}
