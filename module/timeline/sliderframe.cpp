#include "sliderframe.h"
#include "scaleslider.h"
#include "widgets/tooltip.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>

const int MAXIMUM = 25;
const int MINIMUM = 0;
const int SLIDER_HEIGHT = 313;

SliderFrame::SliderFrame(QWidget *parent)
    : QWidget(parent), m_setValueTimes(0)
{
    m_slider = new ScaleSlider();
    m_slider->setOrientation(Qt::Vertical);
    m_slider->setMaximum(25);
    m_slider->setMinimum(0);
    m_slider->setTickInterval(1);
    m_slider->setTickPosition(QSlider::TicksRight);
    m_slider->setFixedSize(16, SLIDER_HEIGHT);
    connect(m_slider, &ScaleSlider::valueChanged, [this](int value) {
        double perc = (double)value / (MAXIMUM - MINIMUM);
        int handlHeight = 16;
        m_tooltip->move(m_tooltip->x(),
                        (this->height() - SLIDER_HEIGHT) / 2
                        + handlHeight / 2
                        + (SLIDER_HEIGHT - handlHeight) * (1 - perc)
                        - m_tooltip->height() / 2);
        emit valueChanged(perc);
    });
    connect(m_slider, &ScaleSlider::mousePress, this, [=] {m_pressed = true;});
    connect(m_slider, &ScaleSlider::mouseRelease, this, [=] {m_pressed = false;});

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(3, 0, 0, 0);
    layout->addWidget(m_slider, 0, Qt::AlignLeft | Qt::AlignVCenter);

    initTooltip();
    initTimer();

    m_slider->setValue(MAXIMUM);
}

void SliderFrame::setValue(double perc)
{
    m_slider->setValue((1 - perc) * (MAXIMUM - MINIMUM));
    if (! m_showTimer->isActive()) {
        m_showTimer->start();
    }
    m_setValueTimes ++;
}

void SliderFrame::setCurrentInfo(const QString &month, int count)
{
    m_timelineLabel->setText(month);
    m_countLabel->setText(tr("%1 Images").arg(QString::number(count)));
}

void SliderFrame::initTooltip()
{
    m_tooltip = new Tooltip(this);
    m_tooltip->setFixedSize(112, 39);
    m_tooltip->move(21, 100);
    m_tooltip->show();

    m_timelineLabel = new QLabel();
    m_timelineLabel->setObjectName("TooltipTimelineLabel");
    m_timelineLabel->setFixedSize(64, 19);

    m_countLabel = new QLabel();
    m_countLabel->setObjectName("TooltipCountLabel");
    m_countLabel->setFixedSize(37, 9);

    QVBoxLayout *lLayout = new QVBoxLayout();
    lLayout->setContentsMargins(22, 2, 0, 7);
    lLayout->addWidget(m_timelineLabel);
    lLayout->addWidget(m_countLabel);

    QLabel *handleLabel = new QLabel();
    handleLabel->setPixmap(QPixmap(":/images/resources/images/drag_handle.png"));

    QHBoxLayout *mainLayout = new QHBoxLayout(m_tooltip);
    mainLayout->setContentsMargins(0, 0, 10, 3);
    mainLayout->addLayout(lLayout);
    mainLayout->addWidget(handleLabel, 0, Qt::AlignVCenter | Qt::AlignRight);
}

void SliderFrame::initTimer()
{
    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    m_showTimer->setInterval(500);
    connect(m_showTimer, &QTimer::timeout, this, [this] {
        // the limit value should change with scroller animation duration
        if (m_setValueTimes > 20) {
            this->show();
            m_hideTimer->start();
        }
        m_setValueTimes = 0;
    });

    m_hideTimer = new QTimer(this);
    m_hideTimer->setInterval(2000);
    connect(m_hideTimer, &QTimer::timeout, this, [this] {
        // Check Slider
        QRect sliderRect = m_slider->geometry();
        sliderRect.moveTopLeft(m_slider->parentWidget()->mapToGlobal(sliderRect.topLeft()));

        // Check Tooltip
        QRect tooltipRect = m_tooltip->geometry();
        tooltipRect.moveTopLeft(this->mapToGlobal(tooltipRect.topLeft()));

        if (! sliderRect.contains(QCursor::pos()) && ! tooltipRect.contains(QCursor::pos())) {
            hide();
            m_hideTimer->stop();
        }
    });
}

bool SliderFrame::pressed() const
{
    return m_pressed;
}
