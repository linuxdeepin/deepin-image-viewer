/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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
#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <DScrollBar>

DWIDGET_USE_NAMESPACE
typedef DScrollBar QSBToDScrollBar;

class QTimer;
class QPropertyAnimation;
class ScrollBar : public QSBToDScrollBar
{
    Q_OBJECT
public:
    explicit ScrollBar(QWidget *parent = 0);
    void stopScroll();
    bool isScrolling() const;

protected:
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;

private:
    QTimer *m_timer;  // For mark is scrolling
    QPropertyAnimation *m_animation;
    double m_speedTime;
    int m_directionFlag;
    int m_oldScrollStep;
};

#endif // SCROLLBAR_H
