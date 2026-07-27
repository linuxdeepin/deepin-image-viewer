// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_eventlogutils.h"
#include "eventlogutils.h"

#include <QJsonObject>
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
