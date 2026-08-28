// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_eventlogutils.h"
#include "eventlogutils.h"

#include <QJsonObject>
#include <QLibrary>
#include <QJsonDocument>
#include <string>

void ut_eventlogutils::SetUp() {}
void ut_eventlogutils::TearDown() {}

// 测试 GetInstance 返回非空单例
TEST_F(ut_eventlogutils, GetInstance_ReturnsNonNull)
{
    Eventlogutils *inst = Eventlogutils::GetInstance();
    EXPECT_NE(inst, nullptr);
}

// 测试 GetInstance 多次调用返回同一实例
TEST_F(ut_eventlogutils, GetInstance_ReturnsSameSingleton)
{
    Eventlogutils *inst1 = Eventlogutils::GetInstance();
    Eventlogutils *inst2 = Eventlogutils::GetInstance();
    EXPECT_EQ(inst1, inst2);
}

// 测试 writeLogs 在 writeEventLogFunc 为空(库未加载)时不崩溃
TEST_F(ut_eventlogutils, WriteLogs_WithNullFuncPtr_NoCrash)
{
    Eventlogutils *inst = Eventlogutils::GetInstance();
    // 测试环境下 libdeepin-event-log.so 通常不存在，函数指针为 null
    QJsonObject data;
    data["tid"] = QString::number(Eventlogutils::StartUp);
    inst->writeLogs(data);
    SUCCEED();
}

// 测试 writeLogs 在设置 writeEventLogFunc 后调用该函数
TEST_F(ut_eventlogutils, WriteLogs_WithFuncPtrSet_FuncCalled)
{
    Eventlogutils *inst = Eventlogutils::GetInstance();

    static bool stubCalled = false;
    static std::string capturedData;
    stubCalled = false;
    capturedData.clear();

    // 桩函数（非捕获 lambda 可转换为函数指针）
    auto stubFunc = +[](const std::string &data) {
        stubCalled = true;
        capturedData = data;
    };

    // 保存原值，借助 -fno-access-control 访问私有成员
    auto orig = inst->writeEventLogFunc;
    inst->writeEventLogFunc = stubFunc;

    QJsonObject data;
    data["tid"] = QString::number(Eventlogutils::Quit);
    data["msg"] = QStringLiteral("ut_test_message");
    inst->writeLogs(data);

    EXPECT_TRUE(stubCalled);
    EXPECT_FALSE(capturedData.empty());
    // 验证传递的是 JSON 紧凑格式
    EXPECT_NE(capturedData.find("ut_test_message"), std::string::npos);

    // 恢复原值
    inst->writeEventLogFunc = orig;
}

// 测试 writeLogs 传递空 JSON 对象不崩溃
TEST_F(ut_eventlogutils, WriteLogs_EmptyJson_NoCrash)
{
    Eventlogutils *inst = Eventlogutils::GetInstance();
    QJsonObject empty;
    inst->writeLogs(empty);
    SUCCEED();
}

// 测试 writeLogs 在 writeEventLogFunc 被手动设为 null 时走 L29-30 分支
TEST_F(ut_eventlogutils, WriteLogs_ManuallyNullFuncPtr_HitsNullBranch)
{
    Eventlogutils *inst = Eventlogutils::GetInstance();

    // 保存原值，设为 nullptr 以触发 L29-30 的 qCWarning + return
    auto orig = inst->writeEventLogFunc;
    inst->writeEventLogFunc = nullptr;

    QJsonObject data;
    data["tid"] = QString::number(Eventlogutils::StartUp);
    inst->writeLogs(data);  // 不应崩溃，走 null 分支

    // 恢复原值
    inst->writeEventLogFunc = orig;
    SUCCEED();
}

// L45-50: 覆盖构造函数中 initFunc / writeEventLogFunc 为 null 的分支
// 通过桩 QLibrary::resolve 返回 nullptr，强制构造函数走 null 检查分支
#include "stub.h"  // moved

// First call (Initialize) returns non-null, second call (WriteEventLog) returns null
static int resolve_call_count = 0;
static QFunctionPointer stub_QLibrary_resolve_alt(const char *symbol)
{
    resolve_call_count++;
    if (resolve_call_count == 1) {
        // Return a dummy non-null function pointer for "Initialize"
        return reinterpret_cast<QFunctionPointer>(0x1);
    }
    // Return null for "WriteEventLog" to hit L49-50
    return nullptr;
}

// All calls return null — hits L45-46 (initFunc null)
static QFunctionPointer stub_QLibrary_resolve_null(const char *)
{
    return nullptr;
}

TEST_F(ut_eventlogutils, Constructor_ResolveFails_HitsNullBranches)
{
    // Test 1: initFunc null (L45-46)
    {
        Stub stub;
        stub.set(static_cast<QFunctionPointer(QLibrary::*)(const char*)>(&QLibrary::resolve), stub_QLibrary_resolve_null);

        Eventlogutils *saved = Eventlogutils::m_pInstance;
        Eventlogutils::m_pInstance = nullptr;

        Eventlogutils *inst = Eventlogutils::GetInstance();
        EXPECT_NE(inst, nullptr);

        delete inst;
        Eventlogutils::m_pInstance = saved;
    }

    // Test 2: initFunc non-null but writeEventLogFunc null (L49-50)
    {
        resolve_call_count = 0;
        Stub stub;
        stub.set(static_cast<QFunctionPointer(QLibrary::*)(const char*)>(&QLibrary::resolve), stub_QLibrary_resolve_alt);

        Eventlogutils *saved = Eventlogutils::m_pInstance;
        Eventlogutils::m_pInstance = nullptr;

        Eventlogutils *inst = Eventlogutils::GetInstance();
        EXPECT_NE(inst, nullptr);

        delete inst;
        Eventlogutils::m_pInstance = saved;
    }
}
