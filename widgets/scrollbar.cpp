#include "scrollbar.h"
#include <QPropertyAnimation>

ScrollBar::ScrollBar(QWidget *parent)
    : QScrollBar(parent)
{
    connect(this, &ScrollBar::valueChanged, this, &ScrollBar::onValueChange);
    animation = new QPropertyAnimation(this, "value");
    animation->setDuration(600);
    animation->setEasingCurve(QEasingCurve::OutCubic);
}

void ScrollBar::wheelEvent(QWheelEvent *event)
{
    if (animation->state() == QPropertyAnimation::Running) {
        setValue(animation->endValue().toInt());
        animation->stop();
    }
    m_oldValue = this->value();
    QScrollBar::wheelEvent(event);
}

void ScrollBar::onValueChange(int value)
{
    if (m_oldValue != 0) {
        int cv = animation->currentValue().toInt();
        animation->setStartValue(cv != 0 ? cv : m_oldValue);
        animation->setEndValue(value);
        m_oldValue = 0;
        animation->start();
    }
}


