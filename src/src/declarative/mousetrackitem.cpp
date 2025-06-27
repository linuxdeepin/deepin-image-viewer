// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mousetrackitem.h"

#include <QMouseEvent>
#include <DLog>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

/**
   @brief 鼠标事件跟踪处理，用于处理 MouseArea 捕获 press 事件后无法向下传递的问题
 */
MouseTrackItem::MouseTrackItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    qCDebug(logImageViewer) << "MouseTrackItem constructor called.";
    setFiltersChildMouseEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    qCDebug(logImageViewer) << "MouseTrackItem initialized, filtersChildMouseEvents set to true, acceptedMouseButtons set to LeftButton.";
}

/**
   @brief 设置当前是否处于 \a press 状态
 */
void MouseTrackItem::setPressed(bool press)
{
    qCDebug(logImageViewer) << "MouseTrackItem::setPressed() called with press: " << press;
    if (isPressed != press) {
        isPressed = press;
        Q_EMIT pressedChanged();
        qCDebug(logImageViewer) << "isPressed changed to: " << isPressed << ", emitting pressedChanged.";
    }
}

/**
   @return 返回当前是否处于点击状态
 */
bool MouseTrackItem::pressed() const
{
    qCDebug(logImageViewer) << "MouseTrackItem::pressed() called.";
    return isPressed;
}

/**
   @brief 捕获鼠标点击事件 \a event
 */
void MouseTrackItem::mousePressEvent(QMouseEvent *event)
{
    qCDebug(logImageViewer) << "MouseTrackItem::mousePressEvent() called.";
    setPressed(true);
    event->accept();
    qCDebug(logImageViewer) << "Mouse press event accepted, pressed set to true.";
}

/**
   @brief 捕获鼠标释放事件 \a event
 */
void MouseTrackItem::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(logImageViewer) << "MouseTrackItem::mouseReleaseEvent() called.";
    setPressed(false);
    event->accept();
    qCDebug(logImageViewer) << "Mouse release event accepted, pressed set to false.";
}

/**
   @brief 捕获鼠标双击事件 \a event
 */
void MouseTrackItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    qCDebug(logImageViewer) << "MouseTrackItem::mouseDoubleClickEvent() called.";
    Q_EMIT doubleClicked();
    event->accept();
    qCDebug(logImageViewer) << "Mouse double click event accepted, emitting doubleClicked.";
}
