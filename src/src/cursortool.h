/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     RenBin <renbin@uniontech.com>
 *
 * Maintainer: TanLang <tanlang@uniontech.com>
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

#ifndef CURSORTOOL_H
#define CURSORTOOL_H

#include <QObject>
#include <QPoint>

class QTimer;

/**
 * @brief 用于主动捕获光标坐标变更时间
 *
 * @note 由于QML中MouseArea无法穿透Hover事件，且上层Item过多，
 *      使用定时查询获取光标位置。
 */
class CursorTool : public QObject
{
    Q_OBJECT
public:
    enum Interval {
        ESampleInterval = 50,   // 采样间隔 50ms
    };

    explicit CursorTool(QObject *parent = nullptr);

    // 设置启动/停止采样
    Q_INVOKABLE void setCaptureCursor(bool b);
    // 当前光标位置信号
    Q_SIGNAL void cursorPos(int x, int y);

    // 获取光标初始活动色
    Q_INVOKABLE QColor activeColor();
    // 光标框选活动色改变
    Q_SIGNAL void activeColorChanged(const QColor &);

private:
    QTimer      *m_CaptureTimer;        // 采样捕获定时器
    QPoint      m_lastPos;              // 记录最后的位置
};

#endif // CURSORTOOL_H
