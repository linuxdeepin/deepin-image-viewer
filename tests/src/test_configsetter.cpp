// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | LibConfigSetter(QObject *) ctor | low | - | 1 | 1 |
// | ~LibConfigSetter | low | - | 1 | 1 |
// | instance | mid | - | 2 | 2 |
// | setValue | low | - | 1 | 7 |
// | value | mid | - | 2 | 6 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（inventory 5 个方法全覆盖，含私有构造/析构，经 -fno-access-control 直接构造）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（group/key 存在、缺失、空、覆盖写、unicode；值类型 int/string/bool/double）
// 3. 每个等价类的边界值显式覆盖: [x]（空 group、空字符串值、缺失 key 默认值、bool 类型边界）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（Value_AfterSetValue 5 组参数）
// 5. 分支清单 → 用例映射已列出: [x]（见下方分支清单块）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（instance() 的 if 真假两路均覆盖；其余方法无分支）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无显式 throw；析构路径以 QObject::destroyed 信号断言覆盖）
// 8. 负面场景有专门用例: [x]（Value_MissingKey 默认值、空 group、空字符串值）
// 9. 负面用例验证强异常安全: [x]（读取缺失 key 后断言原始 QSettings 未被写入）
// 10. stub_ext vs gMock 选择正确: [x]（无需桩掉 QSettings——经 QTemporaryDir 替换 m_settings 后走真实
//     IniFormat 持久化语义，零真实配置污染；保留 StubExt 成员遵循模板约定）
//
// ─────────────────────────────────────────────────────────────
// 分支清单 → 用例映射（来源：get_code_snippet 真实源码）
// ─────────────────────────────────────────────────────────────

// 分支清单（来源：LibConfigSetter::instance configsetter.cpp:30-39）
// B1: !m_setter → m_setter = new LibConfigSetter()
// B2: m_setter 非空 → 直接返回已有指针
// 映射：Instance_FirstCallWhenNull_CreatesNewSingleton → B1
//       Instance_RepeatedCall_ReturnsSamePointer → B2
//
// 无分支方法（顺序执行，全路径覆盖）：
// LibConfigSetter(QObject*)（17-23）→ LibConfigSetter_Constructor_WithParent_CreatesOwnedSettingsObject
// setValue(group,key,value)（41-53）→ SetValue_NewGroupKey / SetValue_OverwriteExistingKey / Value_AfterSetValue(TEST_P)
// value(group,key,default)（55-71）→ Value_MissingKey / Value_AfterSetValue(TEST_P)
// ~LibConfigSetter()（25-27）→ LibConfigSetter_Destructor_DirectDelete_DestroysChildSettingsAndKeepsSingleton
//
// 疑似缺陷（只标红不修）：
// D1: ~LibConfigSetter 不重置静态指针 m_setter（configsetter.cpp:25-27）——若单例被外部
//     delete，后续 instance() 将返回悬垂指针。

#include <gtest/gtest.h>

#include <QMetaType>
#include <QObject>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QVariant>

#include "stub_ext/stubext.h"

#include "configsetter.h"

namespace {

// QSignalSpy 捕获的 QVariant 型信号参数可能是嵌套 QVariant，统一解包后比较
QVariant unwrapSignalVariant(const QVariant &arg)
{
    if (arg.metaType() == QMetaType::fromType<QVariant>())
        return arg.value<QVariant>();
    return arg;
}

struct ConfigRoundTripCase {
    QString group;
    QString key;
    QVariant stored;
    QVariant fallback;
};

}  // namespace

class LibConfigSetterTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        obj = LibConfigSetter::instance();
        ASSERT_NE(obj, nullptr);
        savedSetter = LibConfigSetter::m_setter;

        // 隔离真实配置：备份单例 QSettings 目标路径，替换为临时目录下的 ini。
        // setValue/value 走真实 QSettings(IniFormat) 语义，但只落临时文件，不污染用户配置。
        origConfigPath = obj->m_settings->fileName();
        delete obj->m_settings;
        obj->m_settings = new QSettings(tmpDir.filePath(QStringLiteral("ut_config.ini")),
                                        QSettings::IniFormat, obj);
    }

    void TearDown() override
    {
        // 用例内 instance() 可能新建了单例：销毁新建者并恢复原指针（防泄漏/悬垂）
        if (LibConfigSetter::m_setter != savedSetter && LibConfigSetter::m_setter != nullptr) {
            delete LibConfigSetter::m_setter;
            LibConfigSetter::m_setter = savedSetter;
        }
        // 还原单例内部 QSettings 指向原始路径（无未落盘写入，不改动真实文件）
        if (obj && obj->m_settings) {
            delete obj->m_settings;
            obj->m_settings = new QSettings(origConfigPath, QSettings::IniFormat, obj);
        }
        stub.clear();
    }

    stub_ext::StubExt stub;
    QTemporaryDir tmpDir;
    QString origConfigPath;
    LibConfigSetter *savedSetter = nullptr;
    LibConfigSetter *obj = nullptr;
};

// ═══════════════════════════════════════════════════════════════
// ⚠️ 以下每个 TEST_F 必须包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════

TEST_F(LibConfigSetterTest, LibConfigSetter_Constructor_WithParent_CreatesOwnedSettingsObject)
{
    // Arrange
    QObject parentOwner;

    // Act（私有构造，经 -fno-access-control 直接构造）
    LibConfigSetter *cs = new LibConfigSetter(&parentOwner);

    // Assert
    EXPECT_EQ(cs->parent(), &parentOwner);               // parent 正确挂接
    ASSERT_NE(cs->m_settings, nullptr);                  // 构造即创建 QSettings
    EXPECT_EQ(cs->m_settings->parent(), cs);             // QSettings 以 this 为父（归属正确）
    EXPECT_FALSE(cs->m_settings->fileName().isEmpty());  // 指向 CONFIG_PATH 对应文件名
    EXPECT_NO_THROW(delete cs);                          // 附带：直接销毁无异常
}

TEST_F(LibConfigSetterTest, LibConfigSetter_Destructor_DirectDelete_DestroysChildSettingsAndKeepsSingleton)
{
    // Arrange（直接构造独立对象，不触碰单例 obj）
    LibConfigSetter *cs = new LibConfigSetter();
    QSignalSpy settingsDestroyedSpy(cs->m_settings, &QObject::destroyed);

    // Act
    delete cs;

    // Assert
    EXPECT_EQ(settingsDestroyedSpy.count(), 1);          // 子 QSettings 随父析构（QObject 父子机制）
    EXPECT_EQ(LibConfigSetter::m_setter, savedSetter);   // 析构未重置单例指针（缺陷 D1 行为记录）
}

TEST_F(LibConfigSetterTest, Instance_FirstCallWhenNull_CreatesNewSingleton)
{
    // Arrange
    LibConfigSetter::m_setter = nullptr;

    // Act
    LibConfigSetter *fresh = LibConfigSetter::instance();

    // Assert
    EXPECT_NE(fresh, nullptr);                       // B1: 新建成功
    EXPECT_NE(fresh, savedSetter);                   // 确为新建而非复用旧指针
    EXPECT_EQ(LibConfigSetter::m_setter, fresh);     // 静态指针已更新
    // 新建单例由 TearDown 统一识别（m_setter != savedSetter）销毁并恢复
}

TEST_F(LibConfigSetterTest, Instance_RepeatedCall_ReturnsSamePointer)
{
    // Arrange
    LibConfigSetter::m_setter = nullptr;
    LibConfigSetter *first = LibConfigSetter::instance();

    // Act
    LibConfigSetter *second = LibConfigSetter::instance();

    // Assert
    EXPECT_EQ(second, first);                     // B2: 复用同一实例
    EXPECT_EQ(LibConfigSetter::m_setter, first);  // 静态指针未再变化
}

TEST_F(LibConfigSetterTest, SetValue_NewGroupKey_PersistsNestedValueAndEmitsChangedSignal)
{
    // Arrange
    QSignalSpy spy(obj, &LibConfigSetter::valueChanged);

    // Act
    obj->setValue(QStringLiteral("general"), QStringLiteral("window_width"), QVariant(1024));

    // Assert
    EXPECT_EQ(spy.count(), 1);                                                     // 写入发一次信号
    EXPECT_EQ(spy.at(0).at(0).toString(), QStringLiteral("general"));              // 信号携带 group
    EXPECT_EQ(spy.at(0).at(1).toString(), QStringLiteral("window_width"));        // 信号携带 key
    EXPECT_EQ(unwrapSignalVariant(spy.at(0).at(2)), QVariant(1024));              // 信号携带 value
    EXPECT_EQ(obj->m_settings->value(QStringLiteral("general/window_width")).toInt(),
              1024);  // beginGroup 嵌套生效：裸读 group/key 路径可取到
    EXPECT_EQ(obj->value("general", "window_width", QVariant(-1)).toInt(), 1024); // 经 value() 读回一致
}

TEST_F(LibConfigSetterTest, SetValue_OverwriteExistingKey_ReplacesValueAndEmitsAgain)
{
    // Arrange
    obj->setValue(QStringLiteral("general"), QStringLiteral("theme"),
                  QVariant(QStringLiteral("dark")));
    QSignalSpy spy(obj, &LibConfigSetter::valueChanged);

    // Act
    obj->setValue(QStringLiteral("general"), QStringLiteral("theme"),
                  QVariant(QStringLiteral("light")));

    // Assert
    EXPECT_EQ(spy.count(), 1);  // 覆盖写同样发一次信号
    EXPECT_EQ(obj->value("general", "theme", QVariant(QString())).toString(),
              QStringLiteral("light"));  // 新值生效
    EXPECT_EQ(obj->m_settings->value(QStringLiteral("general/theme")).toString(),
              QStringLiteral("light"));  // 旧值被替换
}

TEST_F(LibConfigSetterTest, Value_MissingKey_ReturnsDefaultValueAndKeepsSettingsClean)
{
    // Arrange
    const QString missingGroup = QStringLiteral("no_such_group");
    const QString missingKey = QStringLiteral("no_such_key");
    EXPECT_FALSE(obj->m_settings->contains(missingGroup + QLatin1Char('/') + missingKey));  // 前置：尚未写入

    // Act
    const QVariant ret = obj->value(missingGroup, missingKey, QVariant(QStringLiteral("fallback")));

    // Assert
    EXPECT_EQ(ret.toString(), QStringLiteral("fallback"));  // 缺失 key 返回默认值
    EXPECT_EQ(obj->value("no_such_group", "other_key", QVariant(7)).toInt(),
              7);  // 不同类型默认值同样透传
    EXPECT_FALSE(obj->m_settings->contains(
        QStringLiteral("no_such_group/no_such_key")));  // 读取不产生写入（强异常安全）
}

// 参数化子 Fixture：主 Fixture 保持 ::testing::Test，子 Fixture 双继承 WithParamInterface
struct LibConfigSetterRoundTripTest : public LibConfigSetterTest,
                                      public ::testing::WithParamInterface<ConfigRoundTripCase> {};

// 同质多组输入：不同 group/key/值类型（含空 group、空字符串值、unicode）写入→读回一致
TEST_P(LibConfigSetterRoundTripTest, Value_AfterSetValue_ReturnsStoredValueNotFallback)
{
    const ConfigRoundTripCase &c = GetParam();

    // Arrange
    obj->setValue(c.group, c.key, c.stored);

    // Act
    const QVariant got = obj->value(c.group, c.key, c.fallback);

    // Assert
    EXPECT_EQ(got, c.stored);  // 读回写入值
    EXPECT_NE(got, c.fallback);  // 未落到默认值
    EXPECT_EQ(obj->value(c.group, c.key + QStringLiteral("_missing"), c.fallback),
              c.fallback);  // 相邻缺失 key 仍返回默认值
}

INSTANTIATE_TEST_SUITE_P(
    ConfigTypes, LibConfigSetterRoundTripTest,
    ::testing::Values(
        ConfigRoundTripCase{QStringLiteral("general"), QStringLiteral("width"), QVariant(1024),
                            QVariant(-1)},
        ConfigRoundTripCase{QStringLiteral("general"), QStringLiteral("theme"),
                            QVariant(QStringLiteral("light")), QVariant(QStringLiteral("dark"))},
        ConfigRoundTripCase{QStringLiteral("view"), QStringLiteral("fullscreen"), QVariant(true),
                            QVariant(false)},
        ConfigRoundTripCase{QString(), QStringLiteral("root_key"), QVariant(QString()),
                            QVariant(QStringLiteral("none"))},
        ConfigRoundTripCase{QStringLiteral("图片-分组"), QStringLiteral("旋转角度"), QVariant(3.5),
                            QVariant(0.0)}));
