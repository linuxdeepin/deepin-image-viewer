// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 测试对象：src/src/declarative/mousetrackitem.cpp 的 MouseTrackItem
// （QQuickItem 派生，QML 鼠标轨迹项；offscreen 下构造，不调 show()，
//   鼠标事件用 QMouseEvent 程序化构造后直接派发给 protected 处理器）。
//
// 用例计数声明（min 按 level/factors 推导：low=1, mid=2, high=3）
// | method                | level | factors | min | actual |
// |-----------------------|-------|---------|-----|--------|
// | MouseTrackItem        | low   | -       | 1   | 2      |
// | setPressed            | low   | -       | 1   | 2      |
// | mousePressEvent       | low   | -       | 1   | 1      |
// | mouseReleaseEvent     | low   | -       | 1   | 1      |
// | mouseDoubleClickEvent | low   | -       | 1   | 1      |
// | pressed               | mid   | -       | 2   | 2      |
//
// 最小清单（test-types.md §8）：
// [x] 1  每个公开方法 ≥ 1 用例（6/6，含构造函数）
// [x] 2  输入维度等价类划分：press=true/false、状态变化/不变（信号发/不发）
// [x] 3  边界值显式覆盖：初始态、连续两次同值 setPressed（不触发分支）
// [x] 4  无 ≥3 组同质输入场景（布尔开关已由等价类用例覆盖），TEST_P 不适用
// [x] 5  分支清单已列出并映射到用例名（见下方 setPressed 段）
// [x] 6  if 分支两侧均有触发用例（setPressed 变化/不变）
// [x] 7  无显式 throw，异常路径不适用（Qt 返回值/信号式处理）
// [x] 8  负面场景：事件先 ignore() 后验证处理器重新 accept、重复同值按压
// [x] 9  负面用例验证状态保持：重复 setPressed(true) 状态不变且不重发信号
// [x] 10 依赖均为 Qt 内置类，仅直接构造，无需 stub/gMock
//
// 分支清单（来源：MouseTrackItem::setPressed，mousetrackitem.cpp:27-35）
// B1: isPressed != press → 更新成员并 Q_EMIT pressedChanged()
//     （true→false 与 false→true 两个方向均覆盖）
// B2: isPressed == press → 不更新不发声（保持原状态）
//
// 用例映射：
// - SetPressed_ValueChanged_EmitsPressedChangedOnce          → B1(false→true)+B2
// - SetPressed_FromTrueBackToFalse_EmitsAgainAndClearsFlag   → B1(true→false)
// - 其余事件/构造用例经由 setPressed 间接覆盖 B1/B2

#include <gtest/gtest.h>

#include <QMouseEvent>
#include <QQuickItem>
#include <QSignalSpy>

#include "mousetrackitem.h"
#include "stub_ext/stubext.h"

class MouseTrackItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        item = new MouseTrackItem();   // offscreen 下直接构造，不 show()
    }

    void TearDown() override
    {
        delete item;
        item = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    MouseTrackItem *item = nullptr;
};

// ═══════════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F 必须包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════════

// ── MouseTrackItem::MouseTrackItem ────────────────────────────────

TEST_F(MouseTrackItemTest, MouseTrackItem_ConstructWithParent_EnablesChildFilterAndLeftButton)
{
    // Arrange
    QQuickItem parentItem;

    // Act
    MouseTrackItem child(&parentItem);

    // Assert
    EXPECT_EQ(child.parent(), &parentItem);                    // 父子关系建立
    EXPECT_TRUE(child.filtersChildMouseEvents());              // ctor: setFiltersChildMouseEvents(true)
    EXPECT_EQ(child.acceptedMouseButtons(), Qt::LeftButton);   // ctor: 仅接受左键
    EXPECT_FALSE(child.pressed());                             // 初始未按压
}

TEST_F(MouseTrackItemTest, MouseTrackItem_DefaultConstruction_HasNoParentAndLeftButtonOnly)
{
    // Arrange
    const Qt::MouseButtons expectedButtons = Qt::LeftButton;

    // Act
    const Qt::MouseButtons buttons = item->acceptedMouseButtons();

    // Assert
    EXPECT_EQ(item->parent(), nullptr);              // 无父项
    EXPECT_EQ(buttons, expectedButtons);             // 默认即仅左键
    EXPECT_TRUE(item->filtersChildMouseEvents());    // 默认开启子项事件过滤
}

// ── MouseTrackItem::pressed ───────────────────────────────────────

TEST_F(MouseTrackItemTest, Pressed_InitialState_ReturnsFalse)
{
    // Arrange
    const bool expectedInitial = false;

    // Act
    const bool initial = item->pressed();

    // Assert
    EXPECT_EQ(initial, expectedInitial);    // branch: 初始 isPressed == false
    EXPECT_EQ(item->pressed(), initial);    // 重复读取结果稳定
}

TEST_F(MouseTrackItemTest, Pressed_AfterStateToggle_TracksCurrentFlag)
{
    // Arrange（-fno-access-control：直接置成员构造既有状态）
    item->isPressed = true;

    // Act
    const bool afterSetTrue = item->pressed();
    item->isPressed = false;
    const bool afterSetFalse = item->pressed();

    // Assert
    EXPECT_EQ(afterSetTrue, true);      // getter 透传成员状态（true 侧）
    EXPECT_EQ(afterSetFalse, false);    // getter 透传成员状态（false 侧）
}

// ── MouseTrackItem::setPressed ────────────────────────────────────

TEST_F(MouseTrackItemTest, SetPressed_ValueChanged_EmitsPressedChangedOnce)
{
    // Arrange
    QSignalSpy spy(item, &MouseTrackItem::pressedChanged);

    // Act
    item->setPressed(true);    // B1: false→true，状态变化
    item->setPressed(true);    // B2: true→true，状态不变

    // Assert
    EXPECT_EQ(spy.count(), 1);        // branch B1+B2: 仅变化时发声一次
    EXPECT_TRUE(item->pressed());     // 状态已更新
}

TEST_F(MouseTrackItemTest, SetPressed_FromTrueBackToFalse_EmitsAgainAndClearsFlag)
{
    // Arrange
    item->setPressed(true);
    QSignalSpy spy(item, &MouseTrackItem::pressedChanged);

    // Act
    item->setPressed(false);    // B1: true→false 方向

    // Assert
    EXPECT_EQ(spy.count(), 1);         // branch B1(true→false): 再发声一次
    EXPECT_FALSE(item->pressed());     // 标志已清除
}

// ── MouseTrackItem::mousePressEvent ───────────────────────────────

TEST_F(MouseTrackItemTest, MousePressEvent_LeftButtonPress_SetsPressedTrueAndAcceptsEvent)
{
    // Arrange
    QSignalSpy spy(item, &MouseTrackItem::pressedChanged);
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(3, 4), QPointF(3, 4),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    press.ignore();    // 先置为忽略，验证处理器内部 accept()

    // Act
    item->mousePressEvent(&press);

    // Assert
    EXPECT_TRUE(item->pressed());        // 经 setPressed(true) 置位（B1）
    EXPECT_TRUE(press.isAccepted());     // 事件被接受
    EXPECT_EQ(spy.count(), 1);           // 副作用：pressedChanged 恰好一次
}

// ── MouseTrackItem::mouseReleaseEvent ─────────────────────────────

TEST_F(MouseTrackItemTest, MouseReleaseEvent_LeftButtonRelease_ClearsPressedAndAcceptsEvent)
{
    // Arrange
    item->setPressed(true);
    QSignalSpy spy(item, &MouseTrackItem::pressedChanged);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(3, 4), QPointF(3, 4),
                        Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    release.ignore();

    // Act
    item->mouseReleaseEvent(&release);

    // Assert
    EXPECT_FALSE(item->pressed());       // 经 setPressed(false) 复位（B1）
    EXPECT_TRUE(release.isAccepted());   // 事件被接受
    EXPECT_EQ(spy.count(), 1);           // 副作用：复位发声一次
}

// ── MouseTrackItem::mouseDoubleClickEvent ─────────────────────────

TEST_F(MouseTrackItemTest, MouseDoubleClickEvent_DoubleClick_EmitsDoubleClickedAndAcceptsEvent)
{
    // Arrange
    QSignalSpy spy(item, &MouseTrackItem::doubleClicked);
    QMouseEvent dblClick(QEvent::MouseButtonDblClick, QPointF(5, 6), QPointF(5, 6),
                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    dblClick.ignore();

    // Act
    item->mouseDoubleClickEvent(&dblClick);

    // Assert
    EXPECT_EQ(spy.count(), 1);               // 副作用：doubleClicked 恰好一次
    EXPECT_TRUE(dblClick.isAccepted());      // 事件被接受
    EXPECT_FALSE(item->pressed());           // 状态保持：双击不改变按压标志
}
