/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LOCKWIDGET_H
#define LOCKWIDGET_H

#include <DLabel>

#include <QLabel>
#include <QMouseEvent>

#include "widgets/themewidget.h"

class QGestureEvent;
class QPinchGesture;
class QSwipeGesture;
class QPanGesture;

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;

class LockWidget : public ThemeWidget
{
    Q_OBJECT
public:
    LockWidget(const QString &darkFile, const QString &lightFile,
               QWidget *parent = nullptr);
    ~LockWidget();
signals:
    void nextRequested();
    void previousRequested();
    void showfullScreen();

    void sigMouseMove();
public slots:
    void setContentText(const QString &text);
    void handleGestureEvent(QGestureEvent *gesture);
protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *event) override;
private slots:
    void pinchTriggered(QPinchGesture *gesture);
private:
    void onThemeChanged(DGuiApplicationHelper::ColorType theme);
    QLbtoDLabel *m_thumbnailLabel;
    QPixmap m_logo;
    QString m_picString;
    bool m_theme;
    QLbtoDLabel *m_bgLabel;
    QLbtoDLabel *m_lockTips;
    int m_startx = 0;
    int m_maxTouchPoints = 0;

    //单击时间
    qint64 m_clickTime{0};
};
#endif // LOCKWIDGET_H
