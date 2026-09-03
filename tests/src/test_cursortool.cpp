// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 测试对象：src/src/cursortool.cpp 的 CursorTool
// （绘制取色光标的工具类：QTimer 采样 QCursor::pos 发 cursorPosChanged，
//   跟随 DGuiApplicationHelper::applicationPaletteChanged 发 activeColorChanged。
//   ctor 内两个 lambda 均通过真实触发条件调用：lambda1 由 QTimer 真实超时驱动
//   （setCaptureCursor(true) + QTest::qWait 派发）；lambda2 由 setApplicationPalette
//   真实发射 applicationPaletteChanged（DTK 为队列发射，需事件派发后送达））。
//
// 历史缺陷已修复：palette lambda 的 connect 已传入 this 作为 context，
// 连接生命周期随 CursorTool 析构自动断开，不再产生悬垂 this 调用
// （回归用例 CursorTool_DestroyedInstance_IgnoresLaterPaletteChanges 覆盖；
//   原“palette 用例必须置于文件首位”的排序规避约束已不再必要）。
// 其余用例仍不触碰 DTK 全局 palette（activeColor 用 stub applicationPalette 重载）。
//
// 用例计数声明（min 按 level/factors 推导：low=1, mid=2, high=3）
// | method            | level | factors | min | actual |
// |-------------------|-------|---------|-----|--------|
// | CursorTool        | low   | -       | 1   | 4      |
// | activeColor       | low   | -       | 1   | 1      |
// | currentCursorPos  | low   | -       | 1   | 1      |
// | setCaptureCursor  | mid   | -       | 2   | 2      |
//
// 最小清单（test-types.md §8）：
// [x] 1  每个公开方法 ≥ 1 用例（4/4，含构造函数；ctor 两个 lambda 由 CursorTool_* 用例经真实触发覆盖）
// [x] 2  输入维度等价类划分：setCaptureCursor true/false；光标位置 变化/静止；父指针 null/非 null
// [x] 3  边界值显式覆盖：初始未激活定时器、静止位置（去重分支）、停止后重复采样为 0、重复 stop 幂等
// [x] 4  无 ≥3 组同质输入场景（布尔/位置输入断言逻辑逐例不同），TEST_P 不适用
// [x] 5  分支清单已列出并映射到用例名（见下方）
// [x] 6  if 分支两侧均有触发用例（lambda1 移动/静止、setCaptureCursor true/false）
// [x] 7  无显式 throw，异常路径不适用（Qt 信号槽式处理）
// [x] 8  负面场景：静止位置不发信号、停止采集后不再发信号、重复 stop 状态保持
// [x] 9  负面用例验证状态保持：m_lastPos 精确断言、停止后 isActive() 为 false、interval 不被破坏
// [x] 10 依赖均为 Qt/DTK 类：QCursor::pos / applicationPalette 静态重载用 static_cast 消歧 stub_ext；
//         palette lambda 用例走真实 DTK API（setApplicationPalette 真实发射）
//
// 分支清单（来源：get_code_snippet 取的 cursortool.cpp 真实源码）
// ctor 内 QTimer::timeout lambda（cursortool.cpp:23-31）：
// B1: pos != m_lastPos → 更新 m_lastPos 并 Q_EMIT cursorPosChanged(pos.x(), pos.y())
// B2: pos == m_lastPos → 不发信号（去重）
// setCaptureCursor（cursortool.cpp:56-66）：
// B3: b == true  → m_CaptureTimer->start()
// B4: b == false → m_CaptureTimer->stop()
// ctor 内 applicationPaletteChanged lambda（cursortool.cpp:33-37）：无分支，全 body 覆盖
//   （connect 以 this 为 context，实例析构后连接自动断开）
// activeColor / currentCursorPos：无分支
//
// 用例映射：
// - CursorTool_ApplicationPaletteChanged_EmitsActiveColorChangedWithNewHighlight → palette lambda（真实信号）
// - CursorTool_DestroyedInstance_IgnoresLaterPaletteChanges → palette lambda（context 连接回归：析构自动断开）
// - CursorTool_ConstructWithParent_CreatesIdleSampleTimerOwnedByTool             → ctor 方法体
// - CursorTool_SampleTimeoutWithMovingCursor_EmitsCursorPosChangedForMovesOnly   → B1+B2（真实 QTimer 超时）
// - ActiveColor_WithStubbedApplicationPalette_ReturnsExactHighlightColor         → activeColor 方法体
// - CurrentCursorPos_WithStubbedQCursorPos_ReturnsSamePoint                      → currentCursorPos 方法体
// - SetCaptureCursor_True_StartsSampleTimer                                      → B3
// - SetCaptureCursor_False_StopsActiveSampleTimer                                → B4

#include <gtest/gtest.h>

#include <QCursor>
#include <QObject>
#include <QPalette>
#include <QPoint>
#include <QSignalSpy>
#include <QTimer>
#include <QtTest/QTest>

#include <DPalette>
#include <DGuiApplicationHelper>

#include "cursortool.h"
#include "stub_ext/stubext.h"

class CursorToolTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        tool = new CursorTool();
    }

    void TearDown() override
    {
        delete tool;
        tool = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    CursorTool *tool = nullptr;
};

// ═══════════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F 必须包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════════

// ── CursorTool::CursorTool 内 applicationPaletteChanged lambda（真实信号触发）──

TEST_F(CursorToolTest, CursorTool_ApplicationPaletteChanged_EmitsActiveColorChangedWithNewHighlight)
{
    // Arrange
    auto *helper = Dtk::Gui::DGuiApplicationHelper::instance();
    const Dtk::Gui::DPalette originalPalette = helper->applicationPalette();
    QSignalSpy spy(tool, &CursorTool::activeColorChanged);
    ASSERT_TRUE(spy.isValid());

    // Act
    Dtk::Gui::DPalette pal = originalPalette;
    pal.setColor(QPalette::Highlight, QColor(10, 200, 60));
    helper->setApplicationPalette(pal);          // 真实发射 applicationPaletteChanged（DTK 队列发射）
    QTest::qWait(300);                           // 事件派发后送达 ctor 连接的 lambda

    // Assert
    ASSERT_EQ(spy.count(), 1);                                               // palette lambda 被真实执行
    EXPECT_EQ(qvariant_cast<QColor>(spy.at(0).at(0)), QColor(10, 200, 60));  // 携带新高亮色
    EXPECT_EQ(tool->activeColor(), QColor(10, 200, 60));                     // 新色已生效（状态一致）

    // Cleanup：在 tool 析构前恢复全局 palette 并派发队列信号，避免污染单例状态
    helper->setApplicationPalette(originalPalette);
    QTest::qWait(200);
}

// ── ctor palette lambda 的 context 连接回归（实例析构后连接自动断开，不悬垂）──

TEST_F(CursorToolTest, CursorTool_DestroyedInstance_IgnoresLaterPaletteChanges)
{
    // Arrange：短生命周期实例的作用域短于 DGuiApplicationHelper 单例；
    // 存活的 SetUp tool 挂 spy 观测 palette 变化的实际到达情况
    auto *helper = Dtk::Gui::DGuiApplicationHelper::instance();
    const Dtk::Gui::DPalette originalPalette = helper->applicationPalette();
    QSignalSpy spy(tool, &CursorTool::activeColorChanged);
    ASSERT_TRUE(spy.isValid());

    {
        CursorTool shortLived;
        QSignalSpy shortSpy(&shortLived, &CursorTool::activeColorChanged);
        ASSERT_TRUE(shortSpy.isValid());

        // Act（第一段）：短生命周期实例存活期间变更 palette，两实例连接均应送达
        Dtk::Gui::DPalette pal = originalPalette;
        pal.setColor(QPalette::Highlight, QColor(11, 22, 33));
        helper->setApplicationPalette(pal);
        QTest::qWait(300);

        // Assert（第一段）：两实例各收到一次（连接正常建立）
        ASSERT_EQ(shortSpy.count(), 1);
        ASSERT_EQ(spy.count(), 1);
    }   // shortLived 析构：connect 以其为 context，连接应随之自动断开

    // Act（第二段）：析构后再变更 palette（修复前此处以悬垂 this 调用 lambda）
    Dtk::Gui::DPalette pal2 = originalPalette;
    pal2.setColor(QPalette::Highlight, QColor(44, 55, 66));
    helper->setApplicationPalette(pal2);
    QTest::qWait(300);

    // Assert（第二段）：存活实例恰好再收一次（共 2），已析构实例不再被激活
    //（无崩溃即回归通过，ASAN 下悬垂调用表现为 heap-use-after-free）
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(tool->activeColor(), QColor(44, 55, 66));

    // Cleanup：恢复全局 palette 并派发，避免污染单例状态
    helper->setApplicationPalette(originalPalette);
    QTest::qWait(200);
}

// ── CursorTool::CursorTool ────────────────────────────────────────

TEST_F(CursorToolTest, CursorTool_ConstructWithParent_CreatesIdleSampleTimerOwnedByTool)
{
    // Arrange
    QObject parent;

    // Act
    CursorTool *childTool = new CursorTool(&parent);

    // Assert
    EXPECT_NE(childTool->m_CaptureTimer, nullptr);                       // 采样定时器已创建
    EXPECT_EQ(childTool->m_CaptureTimer->parent(), childTool);           // 归属 tool（防泄漏）
    EXPECT_FALSE(childTool->m_CaptureTimer->isActive());                 // 构造后未开始采集
    EXPECT_GT(childTool->m_CaptureTimer->interval(), 0);                 // 采样间隔已配置（sc_ESampleInterval）
}

// ── CursorTool::CursorTool 内 QTimer::timeout lambda（真实超时触发）──

TEST_F(CursorToolTest, CursorTool_SampleTimeoutWithMovingCursor_EmitsCursorPosChangedForMovesOnly)
{
    // Arrange
    QPoint stubPos(10, 20);
    stub.set_lamda(static_cast<QPoint (*)()>(&QCursor::pos),
                   [&stubPos]() -> QPoint { return stubPos; });
    QSignalSpy spy(tool, &CursorTool::cursorPosChanged);
    ASSERT_TRUE(spy.isValid());
    const int interval = tool->m_CaptureTimer->interval();

    // Act
    tool->setCaptureCursor(true);                 // B3: 启动采样
    QTest::qWait(interval + 200);                 // 等待至少一次真实 timeout（静止在 (10,20)）
    const int afterFirst = spy.count();
    stubPos = QPoint(30, 40);                     // 光标移动
    QTest::qWait(interval + 200);
    const int afterSecond = spy.count();
    QTest::qWait(interval + 200);                 // 保持静止
    const int afterStationary = spy.count();
    tool->setCaptureCursor(false);                // B4: 停止采样
    QTest::qWait(interval + 200);
    const int afterStop = spy.count();

    // Assert
    ASSERT_EQ(afterFirst, 1);                                             // B1: 首次采样发信号
    EXPECT_EQ(spy.at(0).at(0).toInt(), 10);
    EXPECT_EQ(spy.at(0).at(1).toInt(), 20);
    ASSERT_EQ(afterSecond, 2);                                            // B1: 位置变化再发信号
    EXPECT_EQ(spy.at(1).at(0).toInt(), 30);
    EXPECT_EQ(spy.at(1).at(1).toInt(), 40);
    EXPECT_EQ(afterStationary, 2);                                        // B2: 位置不变不发信号
    EXPECT_EQ(afterStop, 2);                                              // 停止后不再采样
    EXPECT_EQ(tool->m_lastPos, QPoint(30, 40));                           // 状态保持为最后位置
}

// ── CursorTool::activeColor ───────────────────────────────────────

TEST_F(CursorToolTest, ActiveColor_WithStubbedApplicationPalette_ReturnsExactHighlightColor)
{
    // Arrange（applicationPalette 有两个重载，static_cast 消歧；stub 避免触碰 DTK 全局 palette）
    stub.set_lamda(
        static_cast<Dtk::Gui::DPalette (Dtk::Gui::DGuiApplicationHelper::*)() const>(
            &Dtk::Gui::DGuiApplicationHelper::applicationPalette),
        [](Dtk::Gui::DGuiApplicationHelper *) -> Dtk::Gui::DPalette {
            Dtk::Gui::DPalette pal;
            pal.setColor(QPalette::Highlight, QColor(200, 100, 50));
            return pal;
        });

    // Act
    const QColor actual = tool->activeColor();

    // Assert
    EXPECT_EQ(actual, QColor(200, 100, 50));         // 精确颜色值（palette highlight）
    EXPECT_EQ(actual.name(), QString("#c86432"));    // 200,100,50 → #c86432
}

// ── CursorTool::currentCursorPos ──────────────────────────────────

TEST_F(CursorToolTest, CurrentCursorPos_WithStubbedQCursorPos_ReturnsSamePoint)
{
    // Arrange
    QPoint stubPos(123, 456);
    stub.set_lamda(static_cast<QPoint (*)()>(&QCursor::pos),
                   [&stubPos]() -> QPoint { return stubPos; });

    // Act
    const QPoint first = tool->currentCursorPos();
    stubPos = QPoint(7, 8);                      // 光标位置变化
    const QPoint second = tool->currentCursorPos();

    // Assert
    EXPECT_EQ(first, QPoint(123, 456));          // 与 QCursor::pos() 返回一致
    EXPECT_EQ(second, QPoint(7, 8));             // 位置变化后同步更新
}

// ── CursorTool::setCaptureCursor ──────────────────────────────────

TEST_F(CursorToolTest, SetCaptureCursor_True_StartsSampleTimer)
{
    // Arrange
    QPoint stubPos(10, 20);                      // 隔离真实光标访问，防止采样期读到平台光标
    stub.set_lamda(static_cast<QPoint (*)()>(&QCursor::pos),
                   [&stubPos]() -> QPoint { return stubPos; });
    EXPECT_FALSE(tool->m_CaptureTimer->isActive());  // 前置：初始未激活
    const int intervalBeforeAct = tool->m_CaptureTimer->interval();

    // Act
    tool->setCaptureCursor(true);                // B3

    // Assert
    EXPECT_TRUE(tool->m_CaptureTimer->isActive());   // 定时器已启动
    EXPECT_EQ(tool->m_CaptureTimer->interval(), intervalBeforeAct);  // 启动不改变采样间隔
}

TEST_F(CursorToolTest, SetCaptureCursor_False_StopsActiveSampleTimer)
{
    // Arrange
    QPoint stubPos(10, 20);
    stub.set_lamda(static_cast<QPoint (*)()>(&QCursor::pos),
                   [&stubPos]() -> QPoint { return stubPos; });
    tool->setCaptureCursor(true);
    ASSERT_TRUE(tool->m_CaptureTimer->isActive());
    const int intervalBeforeAct = tool->m_CaptureTimer->interval();

    // Act
    tool->setCaptureCursor(false);               // B4
    const bool stoppedFirst = tool->m_CaptureTimer->isActive();
    tool->setCaptureCursor(false);               // B4: 重复 stop（幂等/状态保持）

    // Assert
    EXPECT_FALSE(stoppedFirst);                          // 已停止
    EXPECT_FALSE(tool->m_CaptureTimer->isActive());      // 重复 stop 状态保持，不崩溃不异常
    EXPECT_EQ(tool->m_CaptureTimer->interval(), intervalBeforeAct);  // 配置未被破坏（强状态保持）
}
