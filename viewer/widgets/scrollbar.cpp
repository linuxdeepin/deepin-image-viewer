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
#include "scrollbar.h"
#include <QDebug>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWheelEvent>

namespace {

const int ANIMATION_DUARTION = 1400;
const int DEFAULT_SPEED_TIME = 1;
const int MARK_DELAY = 1000;
const double MAX_SPEED_TIME = 14;

}  // namespace

ScrollBar::ScrollBar(QWidget *parent)
    : QSBToDScrollBar(parent)
    , m_speedTime(DEFAULT_SPEED_TIME)
    , m_directionFlag(1)
{
    m_timer = new QTimer;
    m_timer->setSingleShot(true);
    m_timer->setInterval(MARK_DELAY);

    m_animation = new QPropertyAnimation(this, "value");
    m_animation->setEasingCurve(QEasingCurve::OutQuart);
    m_animation->setDuration(ANIMATION_DUARTION);
    connect(m_animation, &QPropertyAnimation::finished, this, [=] {
        m_timer->start();
    });
    connect(m_animation, &QPropertyAnimation::valueChanged,
            this, [=] (const QVariant &v) {
       int iv = v.toInt();
       int startV = m_animation->startValue().toInt();
       int endV = m_animation->endValue().toInt();
       // Reset speedtime when animation almost done
       if ((startV < endV) && (iv > (endV - 40))) {
               m_speedTime = DEFAULT_SPEED_TIME;
       }
       else if (startV > endV && iv < (endV + 40) && iv > 0) {
            m_speedTime = DEFAULT_SPEED_TIME;
       }
    });
}

void ScrollBar::stopScroll()
{
    m_speedTime = DEFAULT_SPEED_TIME;
    m_animation->stop();
}

bool ScrollBar::isScrolling() const
{
    return (m_animation->state() == QPropertyAnimation::Running ||
            m_timer->isActive());
}

void ScrollBar::wheelEvent(QWheelEvent *e)
{
    // Active by touchpad
    //TODO: how to judge the wheelEvent is from touchpad?
    //this judge(e->pixelDelta.y() != m_oldScrollStep)
    //isn't precise!
    if (e->pixelDelta().y() == 0) {
        QWheelEvent ve(e->pos(), e->globalPos(), e->pixelDelta()
                       , e->angleDelta(), e->delta() * 16/*speed up*/
                       , Qt::Vertical, e->buttons(), e->modifiers());
        QSBToDScrollBar::wheelEvent(&ve);
        m_timer->start();
    }
    // Active by mouse
    else {
        // Direction inverted
        if (m_directionFlag * e->delta() < 0) {
            m_speedTime = DEFAULT_SPEED_TIME;
            m_directionFlag = e->delta();
        }

        int offset = - e->delta() * 2.5;
        if (m_animation->state() == QPropertyAnimation::Running) {
            m_speedTime += 0.4;
        }
        else {
            m_speedTime = DEFAULT_SPEED_TIME;
        }
        m_animation->stop();
        m_animation->setStartValue(value());
        m_animation->setEndValue(value() + offset * qMin(m_speedTime, MAX_SPEED_TIME));

        m_animation->start();
    }
    m_oldScrollStep = e->pixelDelta().y();

}
