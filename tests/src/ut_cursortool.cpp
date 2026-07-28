// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_cursortool.h"
#include "cursortool.h"

#include <QCursor>
#include <QPoint>
#include <QSignalSpy>
#include <QTest>

#include "stub.h"

void ut_cursortool::SetUp()
{
}

void ut_cursortool::TearDown()
{
}

// 测试构造与析构
TEST_F(ut_cursortool, Construct)
{
    CursorTool *tool = new CursorTool();
    ASSERT_TRUE(tool != nullptr);
    delete tool;
}

// 测试 currentCursorPos 返回当前光标位置
TEST_F(ut_cursortool, CurrentCursorPos)
{
    CursorTool tool;
    QPoint pos = tool.currentCursorPos();
    // 应与 QCursor::pos() 返回一致
    EXPECT_EQ(pos, QCursor::pos());
}

// 测试 setCaptureCursor 启动/停止采样
TEST_F(ut_cursortool, SetCaptureCursor)
{
    CursorTool tool;
    // 启动采样不应崩溃
    tool.setCaptureCursor(true);
    tool.setCaptureCursor(false);
}

// 测试 activeColor 返回有效颜色
TEST_F(ut_cursortool, ActiveColor)
{
    CursorTool tool;
    QColor color = tool.activeColor();
    EXPECT_TRUE(color.isValid());
}

// ---- 桩函数 ----
// 桩: 替换 QCursor::pos, 提供确定性的光标位置以触发 cursorPosChanged 信号
static QPoint ut_cursortool_stub_cursorPos()
{
    return QPoint(100, 200);
}

// 测试采样定时器触发后发出 cursorPosChanged 信号(光标位置变化时)
TEST_F(ut_cursortool, SetCaptureCursor_PositionChanged_EmitsSignal)
{
    Stub stub;
    stub.set(static_cast<QPoint (*)()>(&QCursor::pos),
             ut_cursortool_stub_cursorPos);

    CursorTool tool;
    QSignalSpy spy(&tool, &CursorTool::cursorPosChanged);

    tool.setCaptureCursor(true);
    // 采样间隔 50ms, 等待足够时间触发首次位置变化
    QTest::qWait(200);
    tool.setCaptureCursor(false);

    // m_lastPos 初始为 (0,0), 桩返回 (100,200), 首次触发应发信号
    EXPECT_GE(spy.count(), 1);
    if (spy.count() > 0) {
        QList<QVariant> args = spy.takeFirst();
        EXPECT_EQ(args.at(0).toInt(), 100);
        EXPECT_EQ(args.at(1).toInt(), 200);
    }
}

// 测试重复启停采样定时器不崩溃(覆盖 if/else 两个分支多次进入)
TEST_F(ut_cursortool, SetCaptureCursor_RepeatedToggle_NoCrash)
{
    CursorTool tool;
    // 停止状态下再次停止(走 stop 分支)
    tool.setCaptureCursor(false);
    // 连续启动(走 start 分支)
    tool.setCaptureCursor(true);
    tool.setCaptureCursor(true);
    // 再次停止
    tool.setCaptureCursor(false);
    tool.setCaptureCursor(false);
    SUCCEED();
}

// 测试构造函数传入父对象时建立正确的父子关系
TEST_F(ut_cursortool, Constructor_WithParent_HoldsParent)
{
    QObject parent;
    CursorTool *tool = new CursorTool(&parent);
    ASSERT_TRUE(tool != nullptr);
    EXPECT_EQ(tool->parent(), &parent);
    delete tool;
}
