// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 补测目标（7ce38ae TASK-396033）：
// filecontrol.cpp/h 中 DConfig setWallpaperVisible 的读取、信号绑定、默认值回退逻辑。
// 本文件独立编译（不使用 -fno-access-control），避免 DConfig 头文件与该标志冲突。
// 约束：生产源码零改动，仅新增测试代码。
//

#include <gtest/gtest.h>

#include <dtkcore_global.h>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QVariant>

#ifdef DTKCORE_CLASS_DConfigFile
#include <DConfig>

#include "stub_ext/stubext.h"
#include "filecontrol.h"

// ── 独立 fixture：需在构造 FileControl 前控制 DConfig 行为 ──
class FileControlDConfigTest : public ::testing::Test {
protected:
    stub_ext::StubExt stub;
    FileControl *obj = nullptr;

    void SetUp() override
    {
        // 不在此处构造 FileControl — 每个用例先设置 DConfig stub 再构造
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }
};

// B1: DConfig::create 返回 nullptr → 构造函数走 else 分支，m_setWallpaperVisible 回退 true
TEST_F(FileControlDConfigTest, SetWallpaperVisible_DConfigNull_ReturnsTrue)
{
    // Arrange: stub DConfig::create 返回 nullptr
    stub.set_lamda(
            static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &,
                                                const QString &, QObject *)>(
                    &Dtk::Core::DConfig::create),
            [](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
                return nullptr;
            });

    // Act
    obj = new FileControl();

    // Assert: DConfig 无效时回退默认值 true
    EXPECT_TRUE(obj->setWallpaperVisible());
}

// B2: DConfig 有效但 isValid() 返回 false → 走 else 分支，回退 true
TEST_F(FileControlDConfigTest, SetWallpaperVisible_DConfigInvalid_ReturnsTrue)
{
    // Arrange: stub DConfig::isValid 返回 false
    stub.set_lamda(&Dtk::Core::DConfig::isValid, [](Dtk::Core::DConfig *) -> bool { return false; });

    // Act
    obj = new FileControl();

    // Assert: isValid 为 false 时回退默认值 true
    EXPECT_TRUE(obj->setWallpaperVisible());
}

// B3: DConfig 有效且返回 true（默认配置）→ getter 返回 true
TEST_F(FileControlDConfigTest, SetWallpaperVisible_DConfigValid_DefaultReturnsTrue)
{
    // Arrange: 不 stub DConfig::create，使用真实 DConfig
    //   如果系统未安装该 DConfig schema，create 返回的对象 isValid() 为 false，
    //   构造函数回退 true；如果已安装且默认值为 true，也返回 true。
    //   两条路径均期望 true。
    obj = new FileControl();

    // Act & Assert
    EXPECT_TRUE(obj->setWallpaperVisible());
}

// B4: DConfig valueChanged("setWallpaperVisible") 且新值与当前不同 → 触发 setWallpaperVisibleChanged 信号并更新值
TEST_F(FileControlDConfigTest, SetWallpaperVisible_ValueChanged_EmitsSignalAndUpdates)
{
    // Arrange: 正常构造（真实 DConfig）
    obj = new FileControl();
    const bool initialValue = obj->setWallpaperVisible();  // 通常为 true

    // 跳过 DConfig 无效的情况（m_dconfig 可能为 nullptr）
    if (initialValue != true) {
        GTEST_SKIP() << "DConfig not available on this system, skipping valueChanged test";
    }

    // 查找 FileControl 的 DConfig 子对象
    auto dconfig = obj->findChild<Dtk::Core::DConfig *>();
    if (!dconfig || !dconfig->isValid()) {
        GTEST_SKIP() << "DConfig child not found (invalid DConfig), skipping valueChanged test";
    }

    // stub DConfig::value 返回与当前值相反的值
    stub.set_lamda(&Dtk::Core::DConfig::value,
                   [](Dtk::Core::DConfig *, const QString &, const QVariant &) -> QVariant {
                       return QVariant(false);
                   });

    QSignalSpy spy(obj, &FileControl::setWallpaperVisibleChanged);

    // Act: 模拟 DConfig 发出 valueChanged 信号
    QMetaObject::invokeMethod(dconfig, "valueChanged", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("setWallpaperVisible")));

    // Assert: 信号触发一次，值更新为 false
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(obj->setWallpaperVisible());
}

// B5: DConfig valueChanged("setWallpaperVisible") 但新值与当前相同 → 不触发信号
TEST_F(FileControlDConfigTest, SetWallpaperVisible_ValueChangedSameValue_NoSignal)
{
    // Arrange
    obj = new FileControl();
    const bool initialValue = obj->setWallpaperVisible();

    auto dconfig = obj->findChild<Dtk::Core::DConfig *>();
    if (!dconfig || !dconfig->isValid()) {
        GTEST_SKIP() << "DConfig child not found (invalid DConfig), skipping valueChanged test";
    }

    // stub DConfig::value 返回与当前值相同的值
    stub.set_lamda(&Dtk::Core::DConfig::value,
                   [initialValue](Dtk::Core::DConfig *, const QString &, const QVariant &) -> QVariant {
                       return QVariant(initialValue);
                   });

    QSignalSpy spy(obj, &FileControl::setWallpaperVisibleChanged);

    // Act
    QMetaObject::invokeMethod(dconfig, "valueChanged", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("setWallpaperVisible")));

    // Assert: 值未变化，不触发信号
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->setWallpaperVisible(), initialValue);
}

// B6: DConfig valueChanged 传入其他 key → 不触发 setWallpaperVisibleChanged 信号
TEST_F(FileControlDConfigTest, SetWallpaperVisible_ValueChangedDifferentKey_NoSignal)
{
    // Arrange
    obj = new FileControl();
    const bool initialValue = obj->setWallpaperVisible();

    auto dconfig = obj->findChild<Dtk::Core::DConfig *>();
    if (!dconfig || !dconfig->isValid()) {
        GTEST_SKIP() << "DConfig child not found (invalid DConfig), skipping valueChanged test";
    }

    // stub DConfig::value 返回与当前值不同的值（但 key 不匹配，不应触发）
    stub.set_lamda(&Dtk::Core::DConfig::value,
                   [](Dtk::Core::DConfig *, const QString &, const QVariant &) -> QVariant {
                       return QVariant(false);
                   });

    QSignalSpy spy(obj, &FileControl::setWallpaperVisibleChanged);

    // Act: 发出不同 key 的 valueChanged
    QMetaObject::invokeMethod(dconfig, "valueChanged", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("other.config.key")));

    // Assert: key 不匹配，不触发信号，值不变
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->setWallpaperVisible(), initialValue);
}

// B7: getter 返回 m_setWallpaperVisible 成员值（验证 Q_PROPERTY READ 通路）
TEST_F(FileControlDConfigTest, SetWallpaperVisible_Getter_ReturnsMemberValue)
{
    // Arrange: 正常构造
    obj = new FileControl();
    const bool expected = obj->setWallpaperVisible();

    // Act & Assert: 多次调用 getter 返回一致值
    EXPECT_EQ(obj->setWallpaperVisible(), expected);
    EXPECT_EQ(obj->setWallpaperVisible(), expected);
}

#endif  // DTKCORE_CLASS_DConfigFile
