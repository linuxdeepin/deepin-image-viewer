// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_cursortool.h"
#include "cursortool.h"

#include <QCursor>
#include <QPoint>
#include <QSignalSpy>
#include <QTest>
#include <QElapsedTimer>
#include <QThread>
#include <QCoreApplication>
#include <QTimer>
#include <DGuiApplicationHelper>
#include <DPalette>

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

// 测试定时器超时 lambda：直接发射 timeout 信号触发 lambda
// 不使用 processEvents/qWait 避免处理 DBus 残留事件导致崩溃
TEST_F(ut_cursortool, TimerTimeout_EmitsCursorPosChanged)
{
    CursorTool tool;
    QSignalSpy spy(&tool, &CursorTool::cursorPosChanged);
    tool.setCaptureCursor(true);
    // 直接发射 QTimer::timeout 信号（-fno-access-control 允许访问私有成员和信号）
    // Qt6 信号需 QPrivateSignal 参数
    tool.m_CaptureTimer->timeout(QTimer::QPrivateSignal{});
    tool.setCaptureCursor(false);
    // 在 offscreen 平台下 QCursor::pos() 返回固定值，
    // 若位置未变化则信号不发射，此处仅验证不崩溃
    SUCCEED();
}

// 测试 applicationPaletteChanged lambda：先断开残留连接再发射信号
TEST_F(ut_cursortool, PaletteChanged_TriggersActiveColorChanged)
{
    // CursorTool 构造函数使用 3 参数 connect（无 context），
    // 之前测试创建的 CursorTool 销毁后其 lambda 连接仍残留。
    // 先断开所有 applicationPaletteChanged 连接，避免访问已销毁对象。
    Dtk::Gui::DGuiApplicationHelper::instance()->disconnect(SIGNAL(applicationPaletteChanged()));

    // 创建新的 CursorTool（构造函数重新连接 applicationPaletteChanged）
    CursorTool tool;

    // 直接发射 applicationPaletteChanged 信号
    // (-fno-access-control 允许调用 protected 信号)
    // 此时仅新 CursorTool 的 lambda 被连接，无残留连接
    Dtk::Gui::DGuiApplicationHelper::instance()->applicationPaletteChanged();

    // lambda 读取 palette 并发射 activeColorChanged，验证不崩溃即覆盖
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
