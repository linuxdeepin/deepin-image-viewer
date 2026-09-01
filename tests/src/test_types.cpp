// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 测试对象：src/src/types.cpp 的 Types（QObject 派生的枚举载体类，
// types.h 声明 ItemRole/ImageType/StackPage 三组枚举，构造/析构仅记日志）。
//
// 用例计数声明（min 按 level/factors 推导：low=1, mid=2, high=3）
// | method  | level | factors | min | actual |
// |---------|-------|---------|-----|--------|
// | Types   | low   | -       | 1   | 3      |
// | ~Types  | low   | -       | 1   | 1      |
//
// 最小清单（test-types.md §8）：
// [x] 1  每个公开方法 ≥ 1 用例（2/2，含构造/析构；另附枚举契约用例）
// [x] 2  输入维度等价类划分：带 parent / 不带 parent / 析构后状态
// [x] 3  边界值显式覆盖：无父（nullptr）、空子列表、枚举首/末成员
// [x] 4  枚举值属同质多组精确断言，但为编译期常量比较，用单用例集中断言
// [x] 5  方法无分支（构造/析构仅日志），分支清单不适用
// [x] 6  无 if/switch/throw 分支
// [x] 7  无显式 throw，异常路径不适用
// [x] 8  负面场景：默认构造（无 parent）→ parent() 为空
// [x] 9  负面用例验证状态保持：析构后父对象子列表恢复为空
// [x] 10 纯 QObject 构造/析构，无外部依赖，无需 stub/gMock

#include <gtest/gtest.h>

#include <QObject>
#include <QStringList>

#include "stub_ext/stubext.h"
#include "types.h"

class TypesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        obj = nullptr;    // 构造方式（带/不带 parent）由各用例自行决定
    }

    void TearDown() override
    {
        delete obj;       // 由用例内手动 delete 的用例此指针已置空
        obj = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    QObject host;         // 充当 parent 的宿主对象
    Types *obj = nullptr;
};

// ═══════════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F 必须包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════════

// ── Types::Types ──────────────────────────────────────────────────

TEST_F(TypesTest, Types_ConstructWithParent_RegistersChildUnderHost)
{
    // Arrange
    const int childrenBefore = host.children().size();

    // Act
    obj = new Types(&host);

    // Assert
    EXPECT_EQ(obj->parent(), &host);                       // ctor: parent 透传 QObject 基类
    EXPECT_EQ(host.children().size(), childrenBefore + 1); // 副作用：注册进父对象子列表
    EXPECT_TRUE(host.children().contains(static_cast<QObject *>(obj)));
}

TEST_F(TypesTest, Types_ConstructWithoutParent_HasNullParentAndNoChildren)
{
    // Arrange
    const int expectedChildCount = 0;

    // Act
    obj = new Types();

    // Assert
    EXPECT_EQ(obj->parent(), nullptr);                 // 负面：无 parent 时为空指针
    EXPECT_EQ(obj->children().size(), expectedChildCount);  // 状态：自身无子对象
}

// ── Types::~Types ─────────────────────────────────────────────────

TEST_F(TypesTest, Types_Destructor_DetachesFromParentChildren)
{
    // Arrange
    Types *ephemeral = new Types(&host);
    const void *raw = ephemeral;

    // Act
    delete ephemeral;

    // Assert
    EXPECT_FALSE(host.children().contains(raw));   // 副作用：从父对象子列表摘除
    EXPECT_EQ(host.children().size(), 0);          // 状态：宿主恢复无子状态
}

// ── types.h 枚举契约（补强：MCP snippet types.h:21-47 精确值） ────

TEST_F(TypesTest, Types_EnumContracts_MatchDeclaredConstantValues)
{
    // Arrange（枚举值为编译期常量，取成员数作交叉校验基准）
    const int imageTypeMemberCount = 7;

    // Act（读取各枚举成员的底层值）
    const int imageTypeSpan = static_cast<int>(Types::NonexistImage)
                            - static_cast<int>(Types::NullImage) + 1;

    // Assert
    EXPECT_EQ(imageTypeSpan, imageTypeMemberCount);                        // ImageType 0..6 连续
    EXPECT_EQ(static_cast<int>(Types::ImageUrlRole), Qt::UserRole + 1);    // ItemRole 基点
    EXPECT_EQ(static_cast<int>(Types::FrameIndexRole), Qt::UserRole + 2);
    EXPECT_EQ(static_cast<int>(Types::ImageAngleRole), Qt::UserRole + 3);
    EXPECT_EQ(static_cast<int>(Types::OpenImagePage), 0);                  // StackPage 0..2 连续
    EXPECT_EQ(static_cast<int>(Types::SliderShowPage), 2);
}
