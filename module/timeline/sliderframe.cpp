#include "sliderframe.h"
#include "scaleslider.h"
#include <QVBoxLayout>
#include <qdebug.h>

const int MAXIMUM = 25;
const int MINIMUM = 0;
SliderFrame::SliderFrame(QWidget *parent)
    : QWidget(parent)
{
    m_slider = new ScaleSlider();
    m_slider->setOrientation(Qt::Vertical);
    m_slider->setMaximum(25);
    m_slider->setMinimum(0);
    m_slider->setTickInterval(1);
    m_slider->setTickPosition(QSlider::TicksRight);
    m_slider->setFixedSize(16, 313);
    connect(m_slider, &ScaleSlider::valueChanged, [this](int value) {
        emit valueChanged((double)value / (MAXIMUM - MINIMUM));
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(3, 0, 0, 0);
    layout->addWidget(m_slider, 0, Qt::AlignLeft | Qt::AlignVCenter);
}

void SliderFrame::setValue(double perc)
{
    m_slider->setValue((1 - perc) * (MAXIMUM - MINIMUM));
}
