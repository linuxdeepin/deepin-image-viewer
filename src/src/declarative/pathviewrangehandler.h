// SPDX-FileCopyrightText: 2024-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PATHVIEWRANGEHANDLER_H
#define PATHVIEWRANGEHANDLER_H

#include <QObject>
#include <QPointF>
#include <QQuickItem>
#include <QQuickWindow>
#include <QEvent>
#include <QMouseEvent>

class PathViewRangeHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged FINAL)
    Q_PROPERTY(bool enableForward READ enableForward WRITE setEnableForward NOTIFY enableForwardChanged FINAL)
    Q_PROPERTY(bool enableBackward READ enableBackward WRITE setEnableBackward NOTIFY enableBackwardChanged FINAL)

public:
    explicit PathViewRangeHandler(QObject *parent = nullptr);
    ~PathViewRangeHandler();

    QQuickItem *target() const;
    void setTarget(QQuickItem *view);
    Q_SIGNAL void targetChanged();

    bool enableForward() const;
    void setEnableForward(bool b);
    Q_SIGNAL void enableForwardChanged();

    bool enableBackward() const;
    void setEnableBackward(bool b);
    Q_SIGNAL void enableBackwardChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QQuickItem *targetView { nullptr };
    bool enableForwardFlag { true };   // 允许向前
    bool enableBackwardFlag { true };  // 允许向后
    QPointF basePoint;
    bool hasBasePoint { false };
};

// A delegate's MouseArea may lose its grab to PathView and never receive the
// matching release/cancel. Clear the shared drag flag at the window boundary
// before Qt Quick dispatches the release to whichever item owns the grab.
class WindowPointerReleaseFilter : public QObject
{
public:
    using QObject::QObject;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (auto *window = qobject_cast<QQuickWindow *>(watched)) {
            if (event->type() == QEvent::MouseButtonRelease) {
                auto *mouse = static_cast<QMouseEvent *>(event);
                if (mouse->button() == Qt::LeftButton)
                    window->setProperty("imagePointerDown", false);
            } else if (event->type() == QEvent::MouseMove) {
                // A fast flick can lose the release inside the compositor. If
                // the window sees movement without the left button held, the
                // drag must be over: clear the shared flag so a plain hover can
                // never keep translating the picture.
                auto *mouse = static_cast<QMouseEvent *>(event);
                if (!(mouse->buttons() & Qt::LeftButton))
                    window->setProperty("imagePointerDown", false);
            } else if (event->type() == QEvent::WindowDeactivate) {
                window->setProperty("imagePointerDown", false);
            }
        }
        return false;
    }
};

#endif  // PATHVIEWRANGEHANDLER_H
