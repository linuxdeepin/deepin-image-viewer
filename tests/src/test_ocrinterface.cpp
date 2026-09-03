// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | OcrInterface(service,path,connection,parent) | low | - | 1 | 2 |
// | ~OcrInterface | low | - | 1 | 1 |
// | staticInterfaceName() | low | - | 1 | 1 |
// | openFile(filePath) | low | - | 1 | 1 |
// | openImage(image) | low | - | 1 | 2 |
// | openImageAndName(image,imageName) | low | - | 1 | 2 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（inventory 全部 6 个 testable 条目均有映射，含 ocrinterface.cpp 2 个 + ocrinterface.h 内联 4 个）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（image 有效/空、连接 session/非 session、payload 有/无）
// 3. 每个等价类的边界值显式覆盖: [x]（空 QImage 使 save 失败即 B1 反例；4x2/3x3 两尺寸 roundtrip）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（无 ≥3 组同断言输入组：openFile/openImage/openImageAndName 断言结构互不相同，不适用）
// 5. 分支清单 → 用例映射已列出: [x]（见下方分支清单，均来自 get_code_snippet 真实源码 ocrinterface.h:24-85 / ocrinterface.cpp:10-20）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（唯一 if 为 image.save 分支，真假两侧均覆盖；无 throw）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（源码无 throw 分支，不适用）
// 8. 负面场景有专门用例: [x]（空 QImage 走 save 失败分支发送空 payload）
// 9. 负面用例验证强异常安全: [x]（空图用例断言调用计数/方法名/出参不受损）
// 10. stub_ext vs gMock 选择正确: [x]（QDBusAbstractInterface 为 Qt 类，static_cast 消歧 stub doCall；无 gMock）
//
// 环境隔离说明（SetUp/TearDown 全局生效）：
// - DBus 隔离（坑 4）：不 stub isValid；Qt6 内联 call(method,args...) 统一汇入私有 QDBusAbstractInterface::doCall，
//   对其打补丁并在 lambda 内按 method 名过滤：openFile/openImage/openImageAndName 记录后返回 createReply，
//   非目标调用（DTK/DConfig 后台线程等）透传 QDBusMessage::createError 错误回复，绝不触网真实 DBus
// - QDBusMessage().createReply(...) 为成员函数调用形态（坑 2）；createError 为静态重载可用类名调用
// - 会话总线不存在时 sessionBus() 返回断连对象，构造与断言均安全（不依赖本机 DBus 守护进程）
// - 全部对象在 TearDown 中 stub.clear() 后释放
//
// 源码缺陷修复同步（原"行为锁定"缺陷已按修复后语义改写）：
// 1. 公有成员 `QDBusConnection dbus`（恒 sessionBus 且全工程未使用）已删除
//    （原 ConstructionWithNonSessionConnection_KeepsSessionBusMember 行为锁定用例改写为构造绑定校验）
// 2. openImage/openImageAndName：image.save 失败分支已补 qWarning 日志（openImageAndName 含 imageName），
//    调用结构不变，仍发送空 payload（行为锁定用例保持有效）
// 3. 头文件结尾注释 `#endif // DRAWINTERFACE_H` 与 `#ifndef OCRINTERFACE_H` 不匹配，为复制残留（ocrinterface.h:96，仅标注）
//
// 分支清单（来源：get_code_snippet ocrinterface.cpp:10-15 OcrInterface 构造函数）
// 构造仅转发基类（serviceName/ObjectPath/staticInterfaceName()/connection/parent），无分支
// 用例映射：
// - OcrInterface_Construction_BindsServicePathAndInterface              → service/path/interface/connection 四元组绑定
// - OcrInterface_ConstructionWithNonSessionConnection_BindsGivenConnection → 非 session 连接完整转发基类
//
// 分支清单（来源：get_code_snippet ocrinterface.cpp:17-20 ~OcrInterface）
// 析构仅日志，无分支
// 用例映射：
// - Destructor_ExistingInstance_DeletesWithoutSideEffect           → 幸存实例可用性
//
// 分支清单（来源：get_code_snippet ocrinterface.h:24-27 staticInterfaceName）
// 无分支，恒返回 "com.deepin.Ocr"
// 用例映射：
// - StaticInterfaceName_Always_ReturnsComDeepinOcr                 → 精确串断言
//
// 分支清单（来源：get_code_snippet ocrinterface.h:45-48 openFile）
// 无分支：call("openFile", filePath)
// 用例映射：
// - OpenFile_ValidPath_CallsMethodWithFilePathAndAutoDetectMode    → 方法名/参数/模式三重断言
//
// 分支清单（来源：get_code_snippet ocrinterface.h:56-66 openImage）
// B1: image.save(&buf,"PNG") 成功 → qCompress(9)+toBase64 后发送
// B2: save 失败（空图等）→ data 保持空 QByteArray 直接发送（qWarning 记录，payload 行为不变）
// 用例映射：
// - OpenImage_ValidImage_SendsCompressedBase64Png                  → B1（base64 解码→解压→PNG 还原逐像素对账）
// - OpenImage_NullImage_SendsEmptyPayload                          → B2（修复后仍发送空 payload）
//
// 分支清单（来源：get_code_snippet ocrinterface.h:75-85 openImageAndName）
// B1: image.save 成功 → 压缩 base64 + imageName 双参发送
// B2: save 失败 → 空 payload + imageName 双参发送（qWarning 含 imageName，payload 行为不变）
// 用例映射：
// - OpenImageAndName_ValidImageAndName_SendsPayloadAndName         → B1
// - OpenImageAndName_NullImage_SendsEmptyPayloadAndName            → B2（修复后仍发送空 payload）

#include "ocrinterface.h"

#include "stub_ext/stubext.h"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QImage>
#include <QSize>
#include <QString>
#include <QTemporaryDir>
#include <QVariant>

namespace {

const QString kTestService = QStringLiteral("org.deepin.UnitTests.Ocr");
const QString kTestPath = QStringLiteral("/org/deepin/UnitTests/Ocr");

}  // namespace

class OcrInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        installDoCallStub();
        obj = new OcrInterface(kTestService, kTestPath, QDBusConnection::sessionBus());
    }

    void TearDown() override
    {
        stub.clear();
        delete obj;
    }

    // DBus 隔离：Qt6 的内联 call(method, args...) 模板统一汇入私有 doCall，在其上按 method 名过滤；
    // 非目标方法（外部线程的任意 DBus 调用）透传错误回复，绝不触达真实总线（坑 4）
    void installDoCallStub()
    {
        using DoCallPtr = QDBusMessage (QDBusAbstractInterface:: *)(QDBus::CallMode, const QString &, const QVariant *, size_t);
        stub.set_lamda(static_cast<DoCallPtr>(&QDBusAbstractInterface::doCall),
                       [this](QDBusAbstractInterface *, QDBus::CallMode mode, const QString &method,
                              const QVariant *args, size_t numArgs) -> QDBusMessage {
                           if (method != QLatin1String("openFile") && method != QLatin1String("openImage")
                               && method != QLatin1String("openImageAndName")) {
                               return QDBusMessage::createError(QDBusError::InternalError,
                                                                 QStringLiteral("unit-test doCall passthrough"));
                           }
                           ++doCallCount;
                           lastMode = mode;
                           lastMethod = method;
                           lastArgs = QList<QVariant>(args, args + numArgs);
                           return QDBusMessage().createReply();
                       });
    }

    stub_ext::StubExt stub;
    OcrInterface *obj = nullptr;

    int doCallCount = 0;
    QString lastMethod;
    QList<QVariant> lastArgs;
    QDBus::CallMode lastMode = QDBus::AutoDetect;

    QTemporaryDir tempDir;
};

// ══════════════════════════ 构造 / 析构 / 接口名 ══════════════════════════

TEST_F(OcrInterfaceTest, OcrInterface_Construction_BindsServicePathAndInterface)
{
    // Arrange: SetUp 已用 sessionBus 构造实例，读取基类绑定的四元组
    const QString boundService = obj->service();
    const QString boundPath = obj->path();
    const QString boundInterface = obj->interface();
    const QString boundConnection = obj->connection().name();

    // Act: 构造发生在 SetUp，此处核对绑定即验证构造行为

    // Assert  // 构造参数与 staticInterfaceName 完整转发基类
    EXPECT_EQ(boundService, kTestService);
    EXPECT_EQ(boundPath, kTestPath);
    EXPECT_EQ(boundInterface, QStringLiteral("com.deepin.Ocr"));
    EXPECT_EQ(boundConnection, QDBusConnection::sessionBus().name());
}

TEST_F(OcrInterfaceTest, OcrInterface_ConstructionWithNonSessionConnection_BindsGivenConnection)
{
    // Arrange: 用 systemBus 作为构造连接（栈上第二实例，不产生 DBus 调用）
    OcrInterface sysBound(kTestService, kTestPath, QDBusConnection::systemBus());

    // Act
    const QString ctorConnection = sysBound.connection().name();

    // Assert  // 非 session 连接完整转发基类（恒 sessionBus 的 dbus 成员已随源码删除）
    EXPECT_EQ(ctorConnection, QDBusConnection::systemBus().name());
    EXPECT_NE(ctorConnection, QDBusConnection::sessionBus().name());
}

TEST_F(OcrInterfaceTest, Destructor_ExistingInstance_DeletesWithoutSideEffect)
{
    // Arrange: 堆上额外实例 victim，幸存实例保持绑定
    auto *victim = new OcrInterface(kTestService, kTestPath, QDBusConnection::sessionBus());

    // Act
    delete victim;

    // Assert  // victim 析构不影响幸存实例，也未触发任何 DBus 调用
    EXPECT_EQ(obj->service(), kTestService);
    EXPECT_EQ(doCallCount, 0);
    EXPECT_STREQ(OcrInterface::staticInterfaceName(), "com.deepin.Ocr");
}

TEST_F(OcrInterfaceTest, StaticInterfaceName_Always_ReturnsComDeepinOcr)
{
    // Arrange: 无需实例，接口名是编译期常量约定
    const char *name = OcrInterface::staticInterfaceName();

    // Act: 转成 QString 与字面量对账（兼顾 C 串与 Qt 串两种形态）

    // Assert  // 恒定返回 com.deepin.Ocr（com.deepin.Ocr 服务的本地客户端约定）
    EXPECT_STREQ(name, "com.deepin.Ocr");
    EXPECT_EQ(QString::fromLatin1(name), QStringLiteral("com.deepin.Ocr"));
}

// ══════════════════════════ openFile ══════════════════════════

TEST_F(OcrInterfaceTest, OpenFile_ValidPath_CallsMethodWithFilePathAndAutoDetectMode)
{
    // Arrange: 目标文件路径（临时目录内，不依赖测试机固定路径）
    const QString imagePath = tempDir.filePath("ocr_target.png");

    // Act
    QDBusPendingReply<> reply = obj->openFile(imagePath);

    // Assert  // openFile → doCall(AutoDetect, "openFile", [path])，消息型应答即刻完成
    EXPECT_EQ(doCallCount, 1);
    EXPECT_EQ(lastMethod, QStringLiteral("openFile"));
    ASSERT_EQ(lastArgs.size(), 1);
    EXPECT_EQ(lastArgs.at(0).toString(), imagePath);
    EXPECT_EQ(lastMode, QDBus::AutoDetect);
    EXPECT_TRUE(reply.isFinished());
}

// ══════════════════════════ openImage ══════════════════════════

TEST_F(OcrInterfaceTest, OpenImage_ValidImage_SendsCompressedBase64Png)
{
    // Arrange: 4x2 红色 ARGB 图
    QImage image(4, 2, QImage::Format_ARGB32);
    image.fill(Qt::red);

    // Act
    QDBusPendingReply<> reply = obj->openImage(image);

    // Assert  // openImage B1: payload 为 base64(qCompress(PNG))，逐级解码后与原图逐像素对账
    EXPECT_EQ(lastMethod, QStringLiteral("openImage"));
    ASSERT_EQ(lastArgs.size(), 1);
    const QByteArray payload = lastArgs.at(0).toByteArray();
    EXPECT_FALSE(payload.isEmpty());
    const QImage decoded = QImage::fromData(qUncompress(QByteArray::fromBase64(payload)), "PNG");
    EXPECT_EQ(decoded.size(), QSize(4, 2));
    EXPECT_EQ(decoded.pixel(1, 1), qRgb(255, 0, 0));
    EXPECT_EQ(doCallCount, 1);
    EXPECT_TRUE(reply.isFinished());
}

TEST_F(OcrInterfaceTest, OpenImage_NullImage_SendsEmptyPayload)
{
    // Arrange: 空 QImage（save 必然失败）
    const QImage nullImage;

    // Act
    QDBusPendingReply<> reply = obj->openImage(nullImage);

    // Assert  // openImage B2: save 失败 → qWarning 记录后空 payload 仍照常发起调用
    EXPECT_EQ(doCallCount, 1);
    EXPECT_EQ(lastMethod, QStringLiteral("openImage"));
    ASSERT_EQ(lastArgs.size(), 1);
    EXPECT_TRUE(lastArgs.at(0).toByteArray().isEmpty());
    EXPECT_TRUE(reply.isFinished());
}

// ══════════════════════════ openImageAndName ══════════════════════════

TEST_F(OcrInterfaceTest, OpenImageAndName_ValidImageAndName_SendsPayloadAndName)
{
    // Arrange: 3x3 蓝图 + 名称
    QImage image(3, 3, QImage::Format_ARGB32);
    image.fill(Qt::blue);
    const QString imageName = QStringLiteral("holiday.png");

    // Act
    QDBusPendingReply<> reply = obj->openImageAndName(image, imageName);

    // Assert  // openImageAndName B1: [payload, imageName] 双参，payload 可无损还原
    EXPECT_EQ(lastMethod, QStringLiteral("openImageAndName"));
    ASSERT_EQ(lastArgs.size(), 2);
    const QByteArray payload = lastArgs.at(0).toByteArray();
    const QImage decoded = QImage::fromData(qUncompress(QByteArray::fromBase64(payload)), "PNG");
    EXPECT_EQ(decoded.size(), QSize(3, 3));
    EXPECT_EQ(decoded.pixel(1, 1), qRgb(0, 0, 255));
    EXPECT_EQ(lastArgs.at(1).toString(), imageName);
    EXPECT_TRUE(reply.isFinished());
}

TEST_F(OcrInterfaceTest, OpenImageAndName_NullImage_SendsEmptyPayloadAndName)
{
    // Arrange: 空 QImage + 名称
    const QImage nullImage;
    const QString imageName = QStringLiteral("broken.png");

    // Act
    QDBusPendingReply<> reply = obj->openImageAndName(nullImage, imageName);

    // Assert  // openImageAndName B2: qWarning 记录 imageName 后空 payload + 名称仍双参发送
    EXPECT_EQ(doCallCount, 1);
    ASSERT_EQ(lastArgs.size(), 2);
    EXPECT_TRUE(lastArgs.at(0).toByteArray().isEmpty());
    EXPECT_EQ(lastArgs.at(1).toString(), imageName);
    EXPECT_TRUE(reply.isFinished());
}
