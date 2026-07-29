// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_ocrinterface.h"
#include "ocrinterface.h"

#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QDBusMessage>
#include <QImage>
#include <QVariant>

#include "stub.h"

// 桩：拦截 QDBusAbstractInterface::doCall，避免真实 DBus 调用（open* 方法的
// 内联 call() 模板最终都委托给私有 doCall）。
static QDBusMessage ut_ocr_stub_doCall(QDBusAbstractInterface *, QDBus::CallMode,
                                       const QString &, const QVariant *, size_t)
{
    return QDBusMessage();
}

void ut_ocrinterface::SetUp() {}
void ut_ocrinterface::TearDown() {}

// ==================== staticInterfaceName ====================

TEST_F(ut_ocrinterface, StaticInterfaceName_ReturnsComDeepinOcr)
{
    EXPECT_STREQ(OcrInterface::staticInterfaceName(), "com.deepin.Ocr");
}

// ==================== 构造函数 ====================

// 构造：基础属性 service/path/interface 正确设置
TEST_F(ut_ocrinterface, Construct_SetsServicePathInterface)
{
    OcrInterface iface("com.deepin.Ocr.Fake",
                       "/com/deepin/Ocr/Fake",
                       QDBusConnection::sessionBus());
    EXPECT_EQ(iface.service(), QString("com.deepin.Ocr.Fake"));
    EXPECT_EQ(iface.path(), QString("/com/deepin/Ocr/Fake"));
    EXPECT_EQ(iface.interface(), QString("com.deepin.Ocr"));
}

// 构造：public 成员 dbus 默认绑定到 sessionBus
TEST_F(ut_ocrinterface, Construct_PublicDbusMemberIsSessionBus)
{
    OcrInterface iface("com.deepin.Ocr.Fake",
                       "/com/deepin/Ocr/Fake",
                       QDBusConnection::sessionBus());
    EXPECT_EQ(iface.dbus.name(), QDBusConnection::sessionBus().name());
}

// 构造：使用 systemBus 不崩溃
TEST_F(ut_ocrinterface, Construct_WithSystemBus_NoCrash)
{
    OcrInterface iface("com.deepin.Ocr.Fake",
                       "/com/deepin/Ocr/Fake",
                       QDBusConnection::systemBus());
    EXPECT_EQ(iface.interface(), QString("com.deepin.Ocr"));
}

// ==================== 析构函数 ====================

TEST_F(ut_ocrinterface, Destruct_DoesNotCrash)
{
    OcrInterface *iface = new OcrInterface("com.deepin.Ocr.Fake",
                                           "/com/deepin/Ocr/Fake",
                                           QDBusConnection::sessionBus());
    delete iface;
    SUCCEED();
}

// ==================== openFile ====================

// openFile: 不触发真实 DBus，返回非空 reply（不崩溃）
TEST_F(ut_ocrinterface, OpenFile_ReturnsPendingReply_NoCrash)
{
    Stub stub;
    stub.set(ADDR(QDBusAbstractInterface, doCall), ut_ocr_stub_doCall);
    OcrInterface iface("com.deepin.Ocr.Fake",
                       "/com/deepin/Ocr/Fake",
                       QDBusConnection::sessionBus());
    QDBusPendingReply<> reply = iface.openFile("/tmp/ut_ocr_test.png");
    (void)reply;
    SUCCEED();
}

// ==================== openImage ====================

// openImage: 有效图片经 PNG 编码、压缩、base64 后发出 call
TEST_F(ut_ocrinterface, OpenImage_ValidImage_NoCrash)
{
    Stub stub;
    stub.set(ADDR(QDBusAbstractInterface, doCall), ut_ocr_stub_doCall);
    OcrInterface iface("com.deepin.Ocr.Fake",
                       "/com/deepin/Ocr/Fake",
                       QDBusConnection::sessionBus());
    QImage img(20, 20, QImage::Format_RGB32);
    img.fill(Qt::red);
    QDBusPendingReply<> reply = iface.openImage(img);
    (void)reply;
    SUCCEED();
}

// openImage: null 图片时 save 失败，data 保持空，仍发出 call（else 分支）
TEST_F(ut_ocrinterface, OpenImage_NullImage_NoCrash)
{
    Stub stub;
    stub.set(ADDR(QDBusAbstractInterface, doCall), ut_ocr_stub_doCall);
    OcrInterface iface("com.deepin.Ocr.Fake",
                       "/com/deepin/Ocr/Fake",
                       QDBusConnection::sessionBus());
    QImage nullImg;
    QDBusPendingReply<> reply = iface.openImage(nullImg);
    (void)reply;
    SUCCEED();
}

// ==================== openImageAndName ====================

// openImageAndName: 有效图片 + 名称
TEST_F(ut_ocrinterface, OpenImageAndName_ValidImageAndName_NoCrash)
{
    Stub stub;
    stub.set(ADDR(QDBusAbstractInterface, doCall), ut_ocr_stub_doCall);
    OcrInterface iface("com.deepin.Ocr.Fake",
                       "/com/deepin/Ocr/Fake",
                       QDBusConnection::sessionBus());
    QImage img(20, 20, QImage::Format_RGB32);
    img.fill(Qt::blue);
    QDBusPendingReply<> reply = iface.openImageAndName(img, "ut_image.png");
    (void)reply;
    SUCCEED();
}

// openImageAndName: null 图片（save 失败分支）
TEST_F(ut_ocrinterface, OpenImageAndName_NullImage_NoCrash)
{
    Stub stub;
    stub.set(ADDR(QDBusAbstractInterface, doCall), ut_ocr_stub_doCall);
    OcrInterface iface("com.deepin.Ocr.Fake",
                       "/com/deepin/Ocr/Fake",
                       QDBusConnection::sessionBus());
    QImage nullImg;
    QDBusPendingReply<> reply = iface.openImageAndName(nullImg, "ut_null.png");
    (void)reply;
    SUCCEED();
}
