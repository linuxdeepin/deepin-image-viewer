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

#include "cursortool.h"

#include <QTimer>
#include <QCursor>
#include <DGuiApplicationHelper>

CursorTool::CursorTool(QObject *parent)
    : QObject(parent)
{
    m_CaptureTimer = new QTimer(this);
    m_CaptureTimer->setInterval(ESampleInterval);

    connect(m_CaptureTimer, &QTimer::timeout, this, [this]() {
        QPoint pos = QCursor::pos();
        if (pos != m_lastPos) {
            m_lastPos = pos;
            // 发送当前光标的全局位置
            Q_EMIT this->cursorPos(pos.x(), pos.y());
        }
    });

    connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::applicationPaletteChanged, [this](){
        auto newColor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
        emit activeColorChanged(newColor);
    });
}

/**
 * @param b 设置是否启动定时查询光标位置
 *  默认间隔20ms
 */
void CursorTool::setCaptureCursor(bool b)
{
    if (b) {
        m_CaptureTimer->start();
    } else {
        m_CaptureTimer->stop();
    }
}

QColor CursorTool::activeColor()
{
    return Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
}
