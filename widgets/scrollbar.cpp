#include "scrollbar.h"
#include <QPropertyAnimation>

ScrollBar::ScrollBar(QWidget *parent)
    : QScrollBar(parent)
{
    connect(this, &ScrollBar::valueChanged, this, &ScrollBar::onValueChange);
    animation = new QPropertyAnimation(this, "value");
    animation->setDuration(500);
    animation->setEasingCurve(QEasingCurve::OutQuad);
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
        animation->setStartValue(m_oldValue);
        animation->setEndValue(value);
        m_oldValue = 0;
        animation->start();
    }
}


