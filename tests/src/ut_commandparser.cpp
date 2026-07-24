// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_commandparser.h"
#include "commandparser.h"

#include <QCoreApplication>
#include <QStringList>

void ut_commandparser::SetUp()
{
}

void ut_commandparser::TearDown()
{
}

// 测试单例实例获取
TEST_F(ut_commandparser, Instance)
{
    CommandParser *instance = CommandParser::instance();
    ASSERT_TRUE(instance != nullptr);

    // 再次获取应返回同一实例
    CommandParser *instance2 = CommandParser::instance();
    EXPECT_EQ(instance, instance2);
}

// 测试 process 方法解析参数
TEST_F(ut_commandparser, ProcessArguments)
{
    CommandParser *parser = CommandParser::instance();

    QStringList args;
    args << QCoreApplication::applicationFilePath() << "test_image.jpg";

    parser->process(args);

    // positionalArguments 应包含非选项参数
    QStringList positional = parser->positionalArguments();
    EXPECT_FALSE(positional.isEmpty());
}

// 测试 isSet 方法对未设置选项的返回值
TEST_F(ut_commandparser, IsSetOptionNotSet)
{
    CommandParser *parser = CommandParser::instance();

    // 未设置的选项应返回 false
    EXPECT_FALSE(parser->isSet("nonexistent_option"));
}

// 测试 value 方法获取选项值
TEST_F(ut_commandparser, ValueOfOption)
{
    CommandParser *parser = CommandParser::instance();

    // 不存在的选项值应为空
    QString val = parser->value("nonexistent_option");
    EXPECT_TRUE(val.isEmpty());
}
