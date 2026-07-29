// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_commandparser.h"
#include "commandparser.h"

#include <QCoreApplication>
#include <QStringList>
#include <QCommandLineOption>

#include "stub.h"
#include "printhelper.h"

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

// ---- 桩函数 ----
// 桩: 替换 QCoreApplication::arguments, 避免测试进程真实的 --gtest_filter
// 等参数被 QCommandLineParser 视为未知选项而触发 exit
static QStringList ut_commandparser_stub_appArguments()
{
    return QStringList() << QCoreApplication::applicationFilePath()
                         << QStringLiteral("stub_image.png");
}

// 桩: 替换 PrintHelper::showPrintDialog, 避免弹出真实打印对话框(首参为 this)
static void ut_commandparser_stub_showPrintDialog(PrintHelper *,
                                                  const QStringList &,
                                                  QWidget *)
{
}

// 测试无参 process() 重载(内部调用 qApp->arguments())
TEST_F(ut_commandparser, ProcessNoArgs_UsesAppArguments_ParsesPositional)
{
    Stub stub;
    stub.set(&QCoreApplication::arguments, ut_commandparser_stub_appArguments);

    CommandParser *parser = CommandParser::instance();
    parser->process();

    QStringList positional = parser->positionalArguments();
    ASSERT_FALSE(positional.isEmpty());
    EXPECT_EQ(positional.first(), QStringLiteral("stub_image.png"));
}

// 测试 quickPrint 在无位置参数时提前返回(不弹打印对话框)
TEST_F(ut_commandparser, QuickPrint_NoPositional_ReturnsEarly)
{
    CommandParser *parser = CommandParser::instance();

    QStringList args;
    args << QCoreApplication::applicationFilePath();
    parser->process(args);

    // 无位置参数, 提前返回, 不应崩溃
    parser->quickPrint();
    SUCCEED();
}

// 测试 quickPrint 在有位置参数时调用打印对话框(打桩避免 GUI 阻塞)
TEST_F(ut_commandparser, QuickPrint_WithPositional_CallsPrintDialog)
{
    CommandParser *parser = CommandParser::instance();

    QStringList args;
    args << QCoreApplication::applicationFilePath() << QStringLiteral("image.jpg");
    parser->process(args);

    Stub stub;
    stub.set(ADDR(PrintHelper, showPrintDialog),
             ut_commandparser_stub_showPrintDialog);

    // 打桩后调用不应弹窗也不应崩溃
    parser->quickPrint();
    SUCCEED();
}

// 测试 isSet 对已设置选项(--print)返回 true
TEST_F(ut_commandparser, IsSet_SetOption_ReturnsTrue)
{
    CommandParser *parser = CommandParser::instance();

    QStringList args;
    args << QCoreApplication::applicationFilePath() << QStringLiteral("--print");
    parser->process(args);

    EXPECT_TRUE(parser->isSet(QStringLiteral("print")));
}

// 测试 value 对已设置的标志选项返回空字符串
TEST_F(ut_commandparser, Value_SetFlagOption_ReturnsEmpty)
{
    CommandParser *parser = CommandParser::instance();

    QStringList args;
    args << QCoreApplication::applicationFilePath() << QStringLiteral("--print");
    parser->process(args);

    QString val = parser->value(QStringLiteral("print"));
    EXPECT_TRUE(val.isEmpty());
}

// 测试私有方法 addOption(-fno-access-control 启用) 添加自定义选项后可识别
TEST_F(ut_commandparser, AddOption_CustomOption_BecomesRecognized)
{
    CommandParser *parser = CommandParser::instance();

    QCommandLineOption customOpt(QStringLiteral("ut_custom_option"));
    parser->addOption(customOpt);

    QStringList args;
    args << QCoreApplication::applicationFilePath()
         << QStringLiteral("--ut_custom_option");
    parser->process(args);

    EXPECT_TRUE(parser->isSet(QStringLiteral("ut_custom_option")));
}
