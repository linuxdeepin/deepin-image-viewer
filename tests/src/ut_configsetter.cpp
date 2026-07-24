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
