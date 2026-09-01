// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | Eventlogutils() ctor | low | - | 1 | 3 |
// | GetInstance | low | - | 1 | 2 |
// | writeLogs | mid | - | 2 | 4 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（inventory 3 个方法全覆盖；私有构造经 -fno-access-control 直接构造）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（resolve：成功/Initialize 缺失/WriteEventLog 缺失/全缺失；payload：空/单键/嵌套）
// 3. 每个等价类的边界值显式覆盖: [x]（空 QJsonObject payload、单键对象、含嵌套子对象）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（WriteLogs_FuncResolved 3 组 payload）
// 5. 分支清单 → 用例映射已列出: [x]（见下方分支清单块）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（ctor B1/B2/B3、GetInstance B1/B2、writeLogs B1/B2 全覆盖）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无显式 throw；符号缺失路径以提前 return + 桩计数/指针断言覆盖）
// 8. 负面场景有专门用例: [x]（InitializeUnresolved / WriteEventLogUnresolved / WriteFuncNull）
// 9. 负面用例验证强异常安全: [x]（WriteFuncNull 提前返回后入参 QJsonObject 未被改动断言）
// 10. stub_ext vs gMock 选择正确: [x]（QLibrary 为 Qt 类 → static_cast 消歧 + StubExt 桩 resolve，
//     隔离真实 libdeepin-event-log.so；无虚接口注入点，不适用 gMock）
//
// ─────────────────────────────────────────────────────────────
// 分支清单 → 用例映射（来源：get_code_snippet 真实源码）
// ─────────────────────────────────────────────────────────────

// 分支清单（来源：Eventlogutils::Eventlogutils 构造函数 eventlogutils.cpp:37-55）
// B1: !initFunc → qCWarning + return（不调用 Initialize）
// B2: !writeEventLogFunc → qCWarning + return（不调用 Initialize）
// B3: 两个函数指针就绪 → initFunc("deepin-image-viewer", true)
// 映射：Eventlogutils_Constructor_InitializeUnresolved_SkipsInitButKeepsWriteFunc → B1
//       Eventlogutils_Constructor_WriteEventLogUnresolved_SkipsInit → B2
//       Eventlogutils_Constructor_LibraryResolved_InitializesWithAppNameAndEnableSig → B3

// 分支清单（来源：Eventlogutils::GetInstance eventlogutils.cpp:15-23）
// B1: m_pInstance == nullptr → new Eventlogutils()
// B2: m_pInstance 非空 → 直接返回
// 映射：GetInstance_FirstCall_CreatesInitializedSingleton → B1
//       GetInstance_ExistingInstance_ReturnsSameWithoutReinit → B2

// 分支清单（来源：Eventlogutils::writeLogs eventlogutils.cpp:25-35）
// B1: !writeEventLogFunc → qCWarning + return（不写出）
// B2: 就绪 → writeEventLogFunc(QJsonDocument(data).toJson(Compact).toStdString())
// 映射：WriteLogs_WriteFuncNull_DropsLogsWithoutCrash → B1
//       WriteLogs_FuncResolved_WritesCompactJsonPayload（TEST_P）→ B2
//
// 疑似缺陷（只标红不修）：
// D1: 构造函数中 writeEventLogFunc 在 initFunc 判空之前已被赋值（eventlogutils.cpp:42-45）——
//     当 Initialize 解析失败走 B1 提前 return 时，writeEventLogFunc 仍为非空，后续 writeLogs
//     会在事件库未初始化的状态下调用 WriteEventLog，存在未定义行为风险。

#include <gtest/gtest.h>

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QString>

#include <string>

#include "stub_ext/stubext.h"

#include "eventlogutils.h"

namespace {

// —— 桩函数：签名与 eventlogutils.cpp 中 reinterpret_cast 的目标函数指针类型一致 ——
int initCalls = 0;
int writeCalls = 0;
std::string lastAppName;
bool lastEnableSig = false;
std::string lastPayload;

// resolve 桩模式：0=全部解析成功 1=Initialize 缺失 2=WriteEventLog 缺失 3=全部缺失
int resolveMode = 0;

bool fakeInitialize(const std::string &packagename, bool enable_sig)
{
    ++initCalls;
    lastAppName = packagename;
    lastEnableSig = enable_sig;
    return true;
}

void fakeWriteEventLog(const std::string &eventdata)
{
    ++writeCalls;
    lastPayload = eventdata;
}

QFunctionPointer resolveEventlogSymbol(const char *symbol)
{
    if (resolveMode == 3)
        return nullptr;
    if (qstrcmp(symbol, "Initialize") == 0)
        return resolveMode == 1 ? nullptr : reinterpret_cast<QFunctionPointer>(&fakeInitialize);
    if (qstrcmp(symbol, "WriteEventLog") == 0)
        return resolveMode == 2 ? nullptr : reinterpret_cast<QFunctionPointer>(&fakeWriteEventLog);
    return nullptr;
}

void resetEventlogCounters()
{
    initCalls = 0;
    writeCalls = 0;
    lastAppName.clear();
    lastEnableSig = false;
    lastPayload.clear();
}

QJsonObject makeFlatObject()
{
    QJsonObject o;
    o.insert(QStringLiteral("toolName"), QJsonValue(QStringLiteral("deepin-image-viewer")));
    return o;
}

QJsonObject makeNestedObject()
{
    QJsonObject inner;
    inner.insert(QStringLiteral("page"), QJsonValue(QStringLiteral("main")));
    QJsonObject o;
    o.insert(QStringLiteral("tid"), QJsonValue(1000000001));
    o.insert(QStringLiteral("extra"), QJsonValue(inner));
    return o;
}

}  // namespace

class EventlogutilsTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        resetEventlogCounters();
        resolveMode = 0;
        savedInstance = Eventlogutils::m_pInstance;

        // 隔离真实 libdeepin-event-log.so：QLibrary::resolve 按符号名返回桩函数指针，
        // 避免测试机上真实事件库被 Initialize/WriteEventLog 触达
        stub.set_lamda(static_cast<QFunctionPointer (QLibrary::*)(const char *)>(&QLibrary::resolve),
                       [](QLibrary *, const char *symbol) -> QFunctionPointer {
                           return resolveEventlogSymbol(symbol);
                       });
    }

    void TearDown() override
    {
        // 用例内 GetInstance 可能新建单例：销毁新建者并恢复原指针（防跨文件污染）
        if (Eventlogutils::m_pInstance != savedInstance && Eventlogutils::m_pInstance != nullptr) {
            delete Eventlogutils::m_pInstance;
            Eventlogutils::m_pInstance = savedInstance;
        }
        resolveMode = 0;
        resetEventlogCounters();
        stub.clear();
    }

    stub_ext::StubExt stub;
    Eventlogutils *savedInstance = nullptr;
};

// ═══════════════════════════════════════════════════════════════
// ⚠️ 以下每个 TEST_F 必须包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════

TEST_F(EventlogutilsTest, Eventlogutils_Constructor_LibraryResolved_InitializesWithAppNameAndEnableSig)
{
    // Arrange
    resolveMode = 0;  // 显式确认：两个符号均解析成功（SetUp 桩已生效）

    // Act（私有构造，经 -fno-access-control 直接构造）
    Eventlogutils utils;

    // Assert
    EXPECT_EQ(initCalls, 1);                                     // B3: Initialize 恰被调用一次
    EXPECT_EQ(lastAppName, std::string("deepin-image-viewer"));  // 包名正确
    EXPECT_TRUE(lastEnableSig);                                  // enable_sig 实参为 true
    EXPECT_NE(utils.initFunc, nullptr);                          // 两个函数指针均已就绪
    EXPECT_NE(utils.writeEventLogFunc, nullptr);
}

TEST_F(EventlogutilsTest, Eventlogutils_Constructor_InitializeUnresolved_SkipsInitButKeepsWriteFunc)
{
    // Arrange
    resolveMode = 1;  // Initialize 解析失败

    // Act
    Eventlogutils utils;

    // Assert
    EXPECT_EQ(initCalls, 0);                     // B1: 提前 return，Initialize 未被调用
    EXPECT_EQ(utils.initFunc, nullptr);          // initFunc 保持为空
    EXPECT_NE(utils.writeEventLogFunc, nullptr); // 缺陷 D1：WriteEventLog 指针仍被赋值
}

TEST_F(EventlogutilsTest, Eventlogutils_Constructor_WriteEventLogUnresolved_SkipsInit)
{
    // Arrange
    resolveMode = 2;  // WriteEventLog 解析失败

    // Act
    Eventlogutils utils;

    // Assert
    EXPECT_EQ(initCalls, 0);                      // B2: 提前 return，Initialize 未被调用
    EXPECT_EQ(utils.writeEventLogFunc, nullptr);  // writeEventLogFunc 保持为空
    EXPECT_NE(utils.initFunc, nullptr);           // Initialize 已解析但因 B2 未被调用
}

TEST_F(EventlogutilsTest, GetInstance_FirstCall_CreatesInitializedSingleton)
{
    // Arrange
    Eventlogutils::m_pInstance = nullptr;

    // Act
    Eventlogutils *first = Eventlogutils::GetInstance();

    // Assert
    EXPECT_NE(first, nullptr);  // B1: 新建实例
    EXPECT_EQ(initCalls, 1);    // 构造过程完成事件库初始化
    // 新建单例由 TearDown 识别（m_pInstance != savedInstance）销毁并恢复
}

TEST_F(EventlogutilsTest, GetInstance_ExistingInstance_ReturnsSameWithoutReinit)
{
    // Arrange
    Eventlogutils::m_pInstance = nullptr;
    Eventlogutils *first = Eventlogutils::GetInstance();
    const int callsAfterFirst = initCalls;

    // Act
    Eventlogutils *second = Eventlogutils::GetInstance();

    // Assert
    EXPECT_EQ(second, first);               // B2: 复用已有实例
    EXPECT_EQ(initCalls, callsAfterFirst);  // 未重新构造 → Initialize 未被再次调用
}

TEST_F(EventlogutilsTest, WriteLogs_WriteFuncNull_DropsLogsWithoutCrash)
{
    // Arrange
    resolveMode = 3;  // 两个符号均解析失败 → writeEventLogFunc 为空
    Eventlogutils utils;
    ASSERT_EQ(utils.writeEventLogFunc, nullptr);
    QJsonObject data;
    data.insert(QStringLiteral("tid"), QJsonValue(1000000003));

    // Act
    utils.writeLogs(data);

    // Assert
    EXPECT_EQ(writeCalls, 0);  // B1: 直接 return，不写出日志
    EXPECT_EQ(data.value(QStringLiteral("tid")).toInt(),
              1000000003);  // 入参未被改动（强异常安全）
}

// 参数化子 Fixture：主 Fixture 保持 ::testing::Test，子 Fixture 双继承 WithParamInterface
struct EventlogutilsWriteLogsTest : public EventlogutilsTest,
                                    public ::testing::WithParamInterface<QJsonObject> {};

// 同质多组 payload：空对象 / 单键 / 嵌套对象，均以 Compact JSON 精确写出
TEST_P(EventlogutilsWriteLogsTest, WriteLogs_FuncResolved_WritesCompactJsonPayload)
{
    QJsonObject data = GetParam();  // writeLogs 形参为非 const

    // Arrange（resolveMode=0 → 函数指针就绪）
    Eventlogutils utils;
    ASSERT_NE(utils.writeEventLogFunc, nullptr);

    // Act
    utils.writeLogs(data);

    // Assert
    EXPECT_EQ(writeCalls, 1);  // B2: 恰好写出一次
    EXPECT_EQ(QByteArray::fromStdString(lastPayload),
              QJsonDocument(data).toJson(QJsonDocument::Compact));  // 载荷为 Compact JSON 精确值
}

INSTANTIATE_TEST_SUITE_P(PayloadVariants, EventlogutilsWriteLogsTest,
                         ::testing::Values(QJsonObject(), makeFlatObject(), makeNestedObject()));
