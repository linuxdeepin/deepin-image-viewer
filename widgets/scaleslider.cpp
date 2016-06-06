#include "scaleslider.h"
#include <QStylePainter>
#include <QStyleOptionSlider>
#include <QDebug>

ScaleSlider::ScaleSlider(SliderShape defaultShape, QWidget *parent) :
    QSlider(parent)
{
    m_defaultShape = defaultShape;

}

void ScaleSlider::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QStylePainter p(this);
    p.setPen(m_penColor);
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    QRect handle = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    // draw tick marks
    // do this manually because they are very badly behaved with style sheets
    int interval = tickInterval();
    if (interval == 0) {
        interval = pageStep();
    }

    if (tickPosition() != NoTicks) {
        for (int i = minimum(); i <= maximum(); i += interval) {
            if (orientation() == Qt::Horizontal) {
                int x = qRound((double)(((double)(i - minimum()) / (maximum() - minimum())) *
                                       (double)(width() - handle.width()) + (handle.width() / 2.0))) - 1;
                int h = i % 2 == 0 ? 8 : 4;
                if (tickPosition() == TicksBothSides || tickPosition() == TicksAbove) {
                    int y = this->rect().height() / 2 - h;
                    p.drawLine(x, y, x, y + h);
                }
                if (tickPosition() == TicksBothSides || tickPosition() == TicksBelow) {
                    int y = this->rect().height() / 2;
                    p.drawLine(x, y, x, y + h);

                }
            }
            else {
                int y = qRound((double)(((double)(i - minimum()) / (maximum() - minimum())) *
                                       (double)(height() - handle.height()) + (handle.height() / 2.0))) - 1;
                int w = i % 2 == 0 ? 8 : 4;
                if (tickPosition() == TicksBothSides || tickPosition() == TicksLeft) {
                    int x = this->rect().width() / 2 - w;
                    p.drawLine(x, y, x + w, y);
                }
                if (tickPosition() == TicksBothSides || tickPosition() == TicksRight) {
                    int x = this->rect().width() / 2;
                    p.drawLine(x, y, x + w, y);

                }
            }
        }
    }

    // Draw Groove
    if (m_defaultShape == SliderShape::Line) {
        if (orientation() == Qt::Horizontal) {
            p.drawLine(handle.width() / 2, height() / 2, width() - handle.width() / 2, height() / 2);
        }
        else {
            p.drawLine(width() / 2, handle.height() / 2, width() / 2, height() - handle.height() / 2);
        }
    } else {
        p.setBrush(m_brushColor);
        if (orientation() == Qt::Horizontal) {
             p.drawRoundedRect(QRect(this->rect().x(), this->rect().y() + this->rect().height()/2 - 3, this->rect().width(), 2*2), 1, 1);
        }
        else {
            p.drawRoundedRect(QRect(this->rect().x() + this->rect().width()/2 - 3, this->rect().y() + 3, 2*2, this->rect().height() - 10), 1, 1);

        }
    }

    // draw the slider handle
    opt.subControls = QStyle::SC_SliderHandle;
    p.drawComplexControl(QStyle::CC_Slider, opt);
}

QColor ScaleSlider::penColor() const
{
    return m_penColor;
}

void ScaleSlider::setPenColor(const QColor &penColor)
{
    m_penColor = penColor;
}

QColor ScaleSlider::brushColor() const {
    return m_brushColor;
}

void ScaleSlider::setBrushColor(const QColor &brushColor) {
    m_brushColor = brushColor;
}

void ScaleSlider::mousePressEvent(QMouseEvent *e)
{
    emit mousePress();
    QSlider::mousePressEvent(e);
}

void ScaleSlider::mouseReleaseEvent(QMouseEvent *e)
{
    emit mouseRelease();
    QSlider::mouseReleaseEvent(e);
}
