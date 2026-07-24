// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_cursortool.h"
#include "cursortool.h"

#include <QCursor>
#include <QPoint>
#include <QSignalSpy>

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
