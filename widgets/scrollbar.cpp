#include "scrollbar.h"
#include <QPropertyAnimation>

ScrollBar::ScrollBar(QWidget *parent)
    : QScrollBar(parent)
{
    connect(this, &ScrollBar::valueChanged, this, &ScrollBar::onValueChange);
    m_animation = new QPropertyAnimation(this, "value");
    m_animation->setDuration(600);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void ScrollBar::wheelEvent(QWheelEvent *event)
{
    // Fix me: the maximum() will change during wheel, i don't know why
    if (m_animation->state() == QPropertyAnimation::Running) {
        setValue(m_animation->endValue().toInt());
        m_animation->stop();
        m_setFromOutside = true;
    }
    else {
        m_setFromOutside = false;
    }
    m_oldValue = this->value();
    QScrollBar::wheelEvent(event);
}

void ScrollBar::onValueChange(int value)
{
    if (m_oldValue != 0) {
        int cv = m_animation->currentValue().toInt();
        // If value set from outside before, it should not use the cv for start value
        m_animation->setStartValue(m_setFromOutside ? cv : m_oldValue);
        m_animation->setEndValue(value);
        m_oldValue = 0;
        m_animation->start();
    }
}


