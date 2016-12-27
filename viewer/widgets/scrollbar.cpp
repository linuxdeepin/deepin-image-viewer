#include "scrollbar.h"
#include <QDebug>
#include <QWheelEvent>
#include <QPropertyAnimation>

namespace {

const int DEFAULT_SPEED_TIME = 1;
const double MAX_SPEED_TIME = 14;
const int ANIMATION_DUARTION = 1400;

}  // namespace

ScrollBar::ScrollBar(QWidget *parent)
    : QScrollBar(parent)
    , m_speedTime(DEFAULT_SPEED_TIME)
{
    m_animation = new QPropertyAnimation(this, "value");
    m_animation->setEasingCurve(QEasingCurve::OutQuint);
    m_animation->setDuration(ANIMATION_DUARTION);
    connect(m_animation, &QPropertyAnimation::finished, this, [=] {
        m_animation->setEasingCurve(QEasingCurve::OutQuint);
        m_animation->setDuration(ANIMATION_DUARTION);
    });
}

void ScrollBar::stopScroll()
{
    m_speedTime = DEFAULT_SPEED_TIME;
    m_animation->stop();
}

void ScrollBar::wheelEvent(QWheelEvent *e)
{
    // Active by touchpad
    if (e->pixelDelta().y() != 0) {
        QWheelEvent ve(e->pos(), e->globalPos(), e->pixelDelta()
                       , e->angleDelta(), e->delta() * 16/*speed up*/
                       , Qt::Vertical, e->buttons(), e->modifiers());
        QScrollBar::wheelEvent(&ve);
    }
    // Active by mouse
    else {
        int offset = - e->delta() * 3;
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
}
