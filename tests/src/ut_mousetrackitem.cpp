// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_mousetrackitem.h"
#include "mousetrackitem.h"

#include <QMouseEvent>
#include <QSignalSpy>

void ut_mousetrackitem::SetUp() {}
void ut_mousetrackitem::TearDown() {}

// 构造函数: 默认状态为非按下
TEST_F(ut_mousetrackitem, Construct_DefaultNotPressed)
{
    MouseTrackItem item;
    EXPECT_FALSE(item.pressed());
}

// pressed() 默认返回 false (单独列出以保证函数覆盖)
TEST_F(ut_mousetrackitem, Pressed_InitialState_ReturnsFalse)
{
    MouseTrackItem item;
    EXPECT_FALSE(item.pressed());
}

// setPressed: 从 false -> true, 应发射 pressedChanged
TEST_F(ut_mousetrackitem, SetPressed_True_EmitsSignal)
{
    MouseTrackItem item;
    QSignalSpy spy(&item, &MouseTrackItem::pressedChanged);

    item.setPressed(true);
    EXPECT_TRUE(item.pressed());
    EXPECT_EQ(spy.count(), 1);
}

// setPressed: 设置相同值不应发射信号
TEST_F(ut_mousetrackitem, SetPressed_SameValue_NoSignal)
{
    MouseTrackItem item;
    QSignalSpy spy(&item, &MouseTrackItem::pressedChanged);

    item.setPressed(false);  // 与默认值相同
    EXPECT_FALSE(item.pressed());
    EXPECT_EQ(spy.count(), 0);
}

// setPressed: true -> false, 应发射 pressedChanged
TEST_F(ut_mousetrackitem, SetPressed_BackToFalse_EmitsSignal)
{
    MouseTrackItem item;
    item.setPressed(true);

    QSignalSpy spy(&item, &MouseTrackItem::pressedChanged);
    item.setPressed(false);
    EXPECT_FALSE(item.pressed());
    EXPECT_EQ(spy.count(), 1);
}

// mousePressEvent: 应将 pressed 置 true 并 accept 事件
TEST_F(ut_mousetrackitem, MousePressEvent_SetsPressedAndAccepts)
{
    MouseTrackItem item;
    QSignalSpy spy(&item, &MouseTrackItem::pressedChanged);

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 0),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    item.mousePressEvent(&event);

    EXPECT_TRUE(item.pressed());
    EXPECT_TRUE(event.isAccepted());
    EXPECT_EQ(spy.count(), 1);
}

// mouseReleaseEvent: 应将 pressed 置 false 并 accept 事件
TEST_F(ut_mousetrackitem, MouseReleaseEvent_ClearsPressedAndAccepts)
{
    MouseTrackItem item;
    item.setPressed(true);

    QSignalSpy spy(&item, &MouseTrackItem::pressedChanged);
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 0),
                      Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    item.mouseReleaseEvent(&event);

    EXPECT_FALSE(item.pressed());
    EXPECT_TRUE(event.isAccepted());
    EXPECT_EQ(spy.count(), 1);
}

// mouseDoubleClickEvent: 应发射 doubleClicked 并 accept 事件
TEST_F(ut_mousetrackitem, MouseDoubleClickEvent_EmitsDoubleClickedAndAccepts)
{
    MouseTrackItem item;
    QSignalSpy spy(&item, &MouseTrackItem::doubleClicked);

    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(0, 0), QPointF(0, 0),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    item.mouseDoubleClickEvent(&event);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(event.isAccepted());
}
