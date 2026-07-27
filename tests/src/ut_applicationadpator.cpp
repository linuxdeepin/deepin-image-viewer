// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_applicationadpator.h"
#include "applicationadpator.h"
#include "filecontrol.h"

#include <QSignalSpy>
#include "stub.h"

// 可配置返回值的桩函数, 控制 FileControl::isCanReadable 的行为
static bool g_ut_adpator_canReadable = true;
static bool ut_adpator_stub_isCanReadable(FileControl *, const QString &)
{
    return g_ut_adpator_canReadable;
}

// 可配置返回值的桩函数, 控制 FileControl::isImage 的行为
static bool g_ut_adpator_isImage = true;
static bool ut_adpator_stub_isImage(FileControl *, const QString &)
{
    return g_ut_adpator_isImage;
}

void ut_applicationadpator::SetUp()
{
    g_ut_adpator_canReadable = true;
    g_ut_adpator_isImage = true;
}

void ut_applicationadpator::TearDown() {}

// 构造函数: 传入有效 FileControl, 内部 fileControl 应被保存
TEST_F(ut_applicationadpator, Construct_WithFileControl)
{
    FileControl control;
    ApplicationAdaptor adaptor(&control);
    // 通过行为间接验证 fileControl 已被保存: 调用 openImageFile 不应崩溃
    Stub stub;
    g_ut_adpator_canReadable = false;
    g_ut_adpator_isImage = false;
    stub.set(ADDR(FileControl, isCanReadable), ut_adpator_stub_isCanReadable);
    stub.set(ADDR(FileControl, isImage), ut_adpator_stub_isImage);
    EXPECT_FALSE(adaptor.openImageFile("/tmp/nonexistent.png"));
}

// 构造函数: 传入 nullptr, openImageFile 应安全返回 false
TEST_F(ut_applicationadpator, Construct_WithNullptr_FileControlIsNull)
{
    ApplicationAdaptor adaptor(nullptr);
    EXPECT_FALSE(adaptor.openImageFile("/tmp/whatever.png"));
}

// openImageFile: fileControl 为空时返回 false, 不访问桩函数
TEST_F(ut_applicationadpator, OpenImageFile_NullFileControl_ReturnsFalse)
{
    ApplicationAdaptor adaptor(nullptr);
    EXPECT_FALSE(adaptor.openImageFile("/tmp/any.png"));
}

// openImageFile: 不可读时返回 false, 不发射 openImageFile 信号
TEST_F(ut_applicationadpator, OpenImageFile_NotReadable_ReturnsFalse)
{
    FileControl control;
    ApplicationAdaptor adaptor(&control);
    QSignalSpy spy(&control, &FileControl::openImageFile);

    Stub stub;
    g_ut_adpator_canReadable = false;
    g_ut_adpator_isImage = true;
    stub.set(ADDR(FileControl, isCanReadable), ut_adpator_stub_isCanReadable);
    stub.set(ADDR(FileControl, isImage), ut_adpator_stub_isImage);

    EXPECT_FALSE(adaptor.openImageFile("/tmp/notreadable.png"));
    EXPECT_EQ(spy.count(), 0);
}

// openImageFile: 可读但非图片时返回 false, 不发射信号
TEST_F(ut_applicationadpator, OpenImageFile_ReadableButNotImage_ReturnsFalse)
{
    FileControl control;
    ApplicationAdaptor adaptor(&control);
    QSignalSpy spy(&control, &FileControl::openImageFile);

    Stub stub;
    g_ut_adpator_canReadable = true;
    g_ut_adpator_isImage = false;
    stub.set(ADDR(FileControl, isCanReadable), ut_adpator_stub_isCanReadable);
    stub.set(ADDR(FileControl, isImage), ut_adpator_stub_isImage);

    EXPECT_FALSE(adaptor.openImageFile("/tmp/notimage.png"));
    EXPECT_EQ(spy.count(), 0);
}

// openImageFile: 可读且为图片, 传入普通本地路径, 应发射信号并返回 true
TEST_F(ut_applicationadpator, OpenImageFile_ReadableAndImage_PlainPath_ReturnsTrue)
{
    FileControl control;
    ApplicationAdaptor adaptor(&control);
    QSignalSpy spy(&control, &FileControl::openImageFile);

    Stub stub;
    g_ut_adpator_canReadable = true;
    g_ut_adpator_isImage = true;
    stub.set(ADDR(FileControl, isCanReadable), ut_adpator_stub_isCanReadable);
    stub.set(ADDR(FileControl, isImage), ut_adpator_stub_isImage);

    EXPECT_TRUE(adaptor.openImageFile("/tmp/image.png"));
    EXPECT_EQ(spy.count(), 1);
    // 传入普通路径时, 内部会转换为 file:// URL
    EXPECT_EQ(spy.takeFirst().at(0).toString(), QString("file:///tmp/image.png"));
}

// openImageFile: 可读且为图片, 传入 file:// URL, 同样应成功
TEST_F(ut_applicationadpator, OpenImageFile_ReadableAndImage_FileUrl_ReturnsTrue)
{
    FileControl control;
    ApplicationAdaptor adaptor(&control);
    QSignalSpy spy(&control, &FileControl::openImageFile);

    Stub stub;
    g_ut_adpator_canReadable = true;
    g_ut_adpator_isImage = true;
    stub.set(ADDR(FileControl, isCanReadable), ut_adpator_stub_isCanReadable);
    stub.set(ADDR(FileControl, isImage), ut_adpator_stub_isImage);

    EXPECT_TRUE(adaptor.openImageFile("file:///tmp/url_image.png"));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), QString("file:///tmp/url_image.png"));
}
