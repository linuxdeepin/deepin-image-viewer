// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_configsetter.h"
#include "configsetter.h"

#include <QVariant>
#include <QSignalSpy>
#include <QTemporaryDir>

void ut_configsetter::SetUp()
{
}

void ut_configsetter::TearDown()
{
}

// 测试单例实例获取
TEST_F(ut_configsetter, Instance)
{
    LibConfigSetter *instance = LibConfigSetter::instance();
    ASSERT_TRUE(instance != nullptr);

    LibConfigSetter *instance2 = LibConfigSetter::instance();
    EXPECT_EQ(instance, instance2);
}

// 测试设置和读取配置值
TEST_F(ut_configsetter, SetValueAndGetValue)
{
    LibConfigSetter *setter = LibConfigSetter::instance();

    QString group = "test_group";
    QString key = "test_key";
    QVariant value = QString("test_value");

    setter->setValue(group, key, value);

    QVariant readValue = setter->value(group, key);
    EXPECT_EQ(readValue.toString(), value.toString());
}

// 测试读取不存在的键返回默认值
TEST_F(ut_configsetter, GetValueWithDefault)
{
    LibConfigSetter *setter = LibConfigSetter::instance();

    QVariant defaultValue = QString("default");
    QVariant readValue = setter->value("nonexistent_group", "nonexistent_key", defaultValue);
    EXPECT_EQ(readValue.toString(), defaultValue.toString());
}

// 测试 valueChanged 信号
TEST_F(ut_configsetter, ValueChangedSignal)
{
    LibConfigSetter *setter = LibConfigSetter::instance();

    QSignalSpy spy(setter, &LibConfigSetter::valueChanged);
    setter->setValue("signal_group", "signal_key", QString("signal_value"));
    EXPECT_EQ(spy.count(), 1);
}

// 测试私有构造函数与析构函数(-fno-access-control 允许访问私有成员)
// 仅构造与析构, 不写入数据以免污染共享配置文件
TEST_F(ut_configsetter, PrivateConstructorDestructor_RunsClean)
{
    LibConfigSetter *setter = new LibConfigSetter();
    ASSERT_TRUE(setter != nullptr);
    delete setter;
}

// 测试设置并读取整型值
TEST_F(ut_configsetter, SetValue_IntType_RoundTrips)
{
    LibConfigSetter *setter = LibConfigSetter::instance();
    setter->setValue("ut_int_group", "ut_int_key", 42);

    QVariant readValue = setter->value("ut_int_group", "ut_int_key");
    EXPECT_EQ(readValue.toInt(), 42);
}

// 测试读取不存在的键且未提供默认值时返回无效(空)结果
TEST_F(ut_configsetter, Value_NonexistentKeyNoDefault_ReturnsInvalid)
{
    LibConfigSetter *setter = LibConfigSetter::instance();
    QVariant v = setter->value("ut_nonexist_group", "ut_nonexist_key");
    EXPECT_FALSE(v.isValid());
}

// 测试 valueChanged 信号携带正确的 group/key/value 参数
TEST_F(ut_configsetter, ValueChangedSignal_CarriesCorrectParameters)
{
    LibConfigSetter *setter = LibConfigSetter::instance();

    QSignalSpy spy(setter, &LibConfigSetter::valueChanged);
    setter->setValue("ut_signal_group", "ut_signal_key", QString("ut_signal_value"));

    ASSERT_EQ(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), QStringLiteral("ut_signal_group"));
    EXPECT_EQ(args.at(1).toString(), QStringLiteral("ut_signal_key"));
    EXPECT_EQ(args.at(2).toString(), QStringLiteral("ut_signal_value"));
}
