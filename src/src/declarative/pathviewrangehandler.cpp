// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pathviewrangehandler.h"

#include <QEvent>
#include <QMouseEvent>
#include <DLog>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

/**
   @class PathViewRangeHandler
   @brief 用于 PathView 范围控制的辅助类，限制 PathView 在识别到图片列表头尾时
    不会拖拽超过边界。
 */
PathViewRangeHandler::PathViewRangeHandler(QObject *parent)
    : QObject(parent)
{
    qCDebug(logImageViewer) << "PathViewRangeHandler constructor called.";
}

QQuickItem *PathViewRangeHandler::target() const
{
    qCDebug(logImageViewer) << "PathViewRangeHandler::target() called.";
    return targetView;
}

void PathViewRangeHandler::setTarget(QQuickItem *view)
{
    qCDebug(logImageViewer) << "PathViewRangeHandler::setTarget() called with view: " << view;
    if (view != targetView) {
        qCDebug(logImageViewer) << "Target view changed.";
        if (targetView) {
            qCDebug(logImageViewer) << "Removing event filter from old target view.";
            targetView->removeEventFilter(this);
        }

        targetView = view;

        if (targetView) {
            qCDebug(logImageViewer) << "Installing event filter on new target view.";
            targetView->installEventFilter(this);
        }
        Q_EMIT targetChanged();
        qCDebug(logImageViewer) << "Emitted targetChanged signal.";
    }
}

bool PathViewRangeHandler::enableForward() const
{
    qCDebug(logImageViewer) << "PathViewRangeHandler::enableForward() called, returning: " << enableForwardFlag;
    return enableForwardFlag;
}

void PathViewRangeHandler::setEnableForward(bool b)
{
    qCDebug(logImageViewer) << "PathViewRangeHandler::setEnableForward() called with value: " << b;
    if (b != enableForwardFlag) {
        enableForwardFlag = b;
        Q_EMIT enableForwardChanged();
        qCDebug(logImageViewer) << "enableForwardFlag changed to: " << enableForwardFlag << ", emitting enableForwardChanged.";
    }
}

bool PathViewRangeHandler::enableBackward() const
{
    qCDebug(logImageViewer) << "PathViewRangeHandler::enableBackward() called, returning: " << enableBackwardFlag;
    return enableBackwardFlag;
}

void PathViewRangeHandler::setEnableBackward(bool b)
{
    qCDebug(logImageViewer) << "PathViewRangeHandler::setEnableBackward() called with value: " << b;
    if (b != enableBackwardFlag) {
        enableBackwardFlag = b;
        Q_EMIT enableBackwardChanged();
        qCDebug(logImageViewer) << "enableBackwardFlag changed to: " << enableBackwardFlag << ", emitting enableBackwardChanged.";
    }
}

bool PathViewRangeHandler::eventFilter(QObject *obj, QEvent *event)
{
    qCDebug(logImageViewer) << "PathViewRangeHandler::eventFilter() called for object: " << obj << ", event type: " << event->type();
    if (enableForwardFlag && enableBackwardFlag && obj == targetView) {
        qCDebug(logImageViewer) << "Forward and backward enabled, and object is target view, returning false.";
        return false;
    }

    switch (event->type()) {
        case QEvent::MouseButtonRelease: {
            qCDebug(logImageViewer) << "Event type: MouseButtonRelease.";
            basePoint = QPointF();
            qCDebug(logImageViewer) << "basePoint reset.";
            break;
        }
        case QEvent::MouseMove: {
            qCDebug(logImageViewer) << "Event type: MouseMove.";
            auto mouseEvent = dynamic_cast<QMouseEvent *>(event);

            if (basePoint.isNull()) {
                qCDebug(logImageViewer) << "basePoint is null, setting to current mouse position.";
                // currentIndex 会在拖动时动态变更，因此在切换到受限图片时更新触发限制状态
                basePoint = mouseEvent->position();
            } else {
                qCDebug(logImageViewer) << "basePoint is not null, checking for filtering.";
                bool filter = false;
                auto newPoint = mouseEvent->position();
                if (!enableForwardFlag && newPoint.x() > (basePoint.x())) {
                    qCDebug(logImageViewer) << "Filtering: enableForwardFlag is false and mouse moved forward.";
                    filter = true;
                }
                if (!enableBackwardFlag && newPoint.x() < (basePoint.x())) {
                    qCDebug(logImageViewer) << "Filtering: enableBackwardFlag is false and mouse moved backward.";
                    filter = true;
                }

                if (filter) {
                    qCDebug(logImageViewer) << "Event filtered, ignoring and returning true.";
                    event->ignore();
                    return true;
                }
            }

            break;
        }
        default:
            qCDebug(logImageViewer) << "Event type: " << event->type() << ", default handling.";
            break;
    }

    return false;
}
