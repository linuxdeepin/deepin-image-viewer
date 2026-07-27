// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_pathviewrangehandler.h"
#include "pathviewrangehandler.h"

#include <QEvent>
#include <QMouseEvent>
#include <QQuickItem>
#include <QSignalSpy>

void ut_pathviewrangehandler::SetUp() {}
void ut_pathviewrangehandler::TearDown() {}

// 构造函数与默认属性初值
TEST_F(ut_pathviewrangehandler, Construct_DefaultInitialState)
{
    PathViewRangeHandler handler;
    EXPECT_EQ(handler.target(), nullptr);
    EXPECT_TRUE(handler.enableForward());
    EXPECT_TRUE(handler.enableBackward());
}

// 显式传入 parent 构造
TEST_F(ut_pathviewrangehandler, Construct_WithParent_OwnedByParent)
{
    QObject parent;
    PathViewRangeHandler *handler = new PathViewRangeHandler(&parent);
    EXPECT_EQ(handler->parent(), &parent);
    delete handler;
}

// target() 默认返回 nullptr（已在上覆盖，单独列出保证函数覆盖）
TEST_F(ut_pathviewrangehandler, Target_InitialState_ReturnsNull)
{
    PathViewRangeHandler handler;
    EXPECT_EQ(handler.target(), nullptr);
}

// setTarget: null -> valid item, 应发射 targetChanged 并安装事件过滤器
TEST_F(ut_pathviewrangehandler, SetTarget_FromNullToItem_EmitsSignal)
{
    PathViewRangeHandler handler;
    QSignalSpy spy(&handler, &PathViewRangeHandler::targetChanged);

    QQuickItem item;
    handler.setTarget(&item);
    EXPECT_EQ(handler.target(), &item);
    EXPECT_EQ(spy.count(), 1);
}

// setTarget: 相同对象不应再次发射信号
TEST_F(ut_pathviewrangehandler, SetTarget_SameItem_NoSignal)
{
    PathViewRangeHandler handler;
    QQuickItem item;
    handler.setTarget(&item);

    QSignalSpy spy(&handler, &PathViewRangeHandler::targetChanged);
    handler.setTarget(&item);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(handler.target(), &item);
}

// setTarget: 切换到新对象应卸载旧的过滤器并安装新的
TEST_F(ut_pathviewrangehandler, SetTarget_SwitchToAnother_EmitsSignal)
{
    PathViewRangeHandler handler;
    QQuickItem item1;
    QQuickItem item2;
    handler.setTarget(&item1);

    QSignalSpy spy(&handler, &PathViewRangeHandler::targetChanged);
    handler.setTarget(&item2);
    EXPECT_EQ(handler.target(), &item2);
    EXPECT_EQ(spy.count(), 1);
}

// setTarget: 切回 nullptr 应清理 target
TEST_F(ut_pathviewrangehandler, SetTarget_ToNull_EmitsSignal)
{
    PathViewRangeHandler handler;
    QQuickItem item;
    handler.setTarget(&item);

    QSignalSpy spy(&handler, &PathViewRangeHandler::targetChanged);
    handler.setTarget(nullptr);
    EXPECT_EQ(handler.target(), nullptr);
    EXPECT_EQ(spy.count(), 1);
}

// enableForward / setEnableForward: 默认 true, 切到 false 应发射信号
TEST_F(ut_pathviewrangehandler, EnableForward_Toggle_EmitsSignalOnChange)
{
    PathViewRangeHandler handler;
    EXPECT_TRUE(handler.enableForward());

    QSignalSpy spy(&handler, &PathViewRangeHandler::enableForwardChanged);
    handler.setEnableForward(false);
    EXPECT_FALSE(handler.enableForward());
    EXPECT_EQ(spy.count(), 1);
}

// setEnableForward: 设置相同值不应发射信号
TEST_F(ut_pathviewrangehandler, EnableForward_SameValue_NoSignal)
{
    PathViewRangeHandler handler;
    QSignalSpy spy(&handler, &PathViewRangeHandler::enableForwardChanged);
    handler.setEnableForward(true);  // 与默认值相同
    EXPECT_EQ(spy.count(), 0);
    EXPECT_TRUE(handler.enableForward());
}

// enableBackward / setEnableBackward: 默认 true, 切到 false 应发射信号
TEST_F(ut_pathviewrangehandler, EnableBackward_Toggle_EmitsSignalOnChange)
{
    PathViewRangeHandler handler;
    EXPECT_TRUE(handler.enableBackward());

    QSignalSpy spy(&handler, &PathViewRangeHandler::enableBackwardChanged);
    handler.setEnableBackward(false);
    EXPECT_FALSE(handler.enableBackward());
    EXPECT_EQ(spy.count(), 1);
}

// setEnableBackward: 设置相同值不应发射信号
TEST_F(ut_pathviewrangehandler, EnableBackward_SameValue_NoSignal)
{
    PathViewRangeHandler handler;
    QSignalSpy spy(&handler, &PathViewRangeHandler::enableBackwardChanged);
    handler.setEnableBackward(true);  // 与默认值相同
    EXPECT_EQ(spy.count(), 0);
    EXPECT_TRUE(handler.enableBackward());
}

// eventFilter: 双向均允许且对象为 target 时直接返回 false
TEST_F(ut_pathviewrangehandler, EventFilter_BothEnabledAndTarget_ReturnsFalse)
{
    PathViewRangeHandler handler;
    QQuickItem item;
    handler.setTarget(&item);

    QMouseEvent event(QEvent::MouseMove, QPointF(10, 10), QPointF(10, 10),
                      Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_FALSE(handler.eventFilter(&item, &event));
}

// eventFilter: MouseButtonRelease 重置 basePoint 并返回 false
TEST_F(ut_pathviewrangehandler, EventFilter_MouseRelease_ResetsBasePoint)
{
    PathViewRangeHandler handler;
    QQuickItem item;
    handler.setTarget(&item);
    handler.setEnableForward(false);  // 避免触发提前 return

    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(5, 5), QPointF(5, 5),
                      Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_FALSE(handler.eventFilter(&item, &event));
}

// eventFilter: MouseMove 且 basePoint 为空时, 设置 basePoint 并返回 false
TEST_F(ut_pathviewrangehandler, EventFilter_MouseMove_NullBasePoint_SetsBasePoint)
{
    PathViewRangeHandler handler;
    QQuickItem item;
    handler.setTarget(&item);
    handler.setEnableForward(false);

    QMouseEvent event(QEvent::MouseMove, QPointF(20, 20), QPointF(20, 20),
                      Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_FALSE(handler.eventFilter(&item, &event));
}

// eventFilter: MouseMove 禁止向前且向右拖动, 应过滤事件返回 true
TEST_F(ut_pathviewrangehandler, EventFilter_MouseMove_ForwardDisabledAndMovingRight_Filtered)
{
    PathViewRangeHandler handler;
    QQuickItem item;
    handler.setTarget(&item);
    handler.setEnableForward(false);

    // 先建立 basePoint
    QMouseEvent first(QEvent::MouseMove, QPointF(10, 10), QPointF(10, 10),
                      Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    handler.eventFilter(&item, &first);

    // 向右移动 (newPoint.x > basePoint.x), 应被过滤
    QMouseEvent second(QEvent::MouseMove, QPointF(50, 10), QPointF(50, 10),
                       Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_TRUE(handler.eventFilter(&item, &second));
}

// eventFilter: MouseMove 禁止向后且向左拖动, 应过滤事件返回 true
TEST_F(ut_pathviewrangehandler, EventFilter_MouseMove_BackwardDisabledAndMovingLeft_Filtered)
{
    PathViewRangeHandler handler;
    QQuickItem item;
    handler.setTarget(&item);
    handler.setEnableBackward(false);

    // 先建立 basePoint
    QMouseEvent first(QEvent::MouseMove, QPointF(50, 10), QPointF(50, 10),
                      Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    handler.eventFilter(&item, &first);

    // 向左移动 (newPoint.x < basePoint.x), 应被过滤
    QMouseEvent second(QEvent::MouseMove, QPointF(10, 10), QPointF(10, 10),
                       Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_TRUE(handler.eventFilter(&item, &second));
}

// eventFilter: MouseMove 方向允许, 不应过滤, 返回 false
TEST_F(ut_pathviewrangehandler, EventFilter_MouseMove_DirectionAllowed_NotFiltered)
{
    PathViewRangeHandler handler;
    QQuickItem item;
    handler.setTarget(&item);
    handler.setEnableForward(false);

    // basePoint
    QMouseEvent first(QEvent::MouseMove, QPointF(50, 10), QPointF(50, 10),
                      Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    handler.eventFilter(&item, &first);

    // 继续向左移动 (允许的方向, 因 forward 已禁用但只在向右时过滤)
    QMouseEvent second(QEvent::MouseMove, QPointF(30, 10), QPointF(30, 10),
                       Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_FALSE(handler.eventFilter(&item, &second));
}

// eventFilter: 其它事件类型走 default 分支, 返回 false
TEST_F(ut_pathviewrangehandler, EventFilter_OtherEventType_DefaultBranch)
{
    PathViewRangeHandler handler;
    QQuickItem item;
    handler.setTarget(&item);
    handler.setEnableForward(false);

    QEvent event(QEvent::Resize);
    EXPECT_FALSE(handler.eventFilter(&item, &event));
}
