#include "imagesliderframe.h"
#include "widgets/scaleslider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QFile>

const int MAXIMUM = 1000;
const int MINIMUM = 50;
const int SLIDER_HEIGHT = 322;
const int SCALE_VALUE = 10;

ImageSliderFrame::ImageSliderFrame(QWidget *parent)
    : QWidget(parent), m_setValueTimes(0)
{
    this->setFixedSize(105, SLIDER_HEIGHT+20);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_slider = new ScaleSlider(ScaleSlider::Rect);

    m_slider->setOrientation(Qt::Vertical);
    m_slider->setMaximum(MAXIMUM);
    m_slider->setMinimum(MINIMUM);
    m_slider->setPenColor(QColor(0, 0, 0, 75));
    m_slider->setBrushColor(QColor(255, 255, 255, 75));
    m_slider->setFixedSize(22, SLIDER_HEIGHT);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(80, 0, 0, 0);
    layout->addWidget(m_slider);

    initTooltip();
    initTimer();
    initStyleSheet();
    setCurrentValue(MAXIMUM);
    connect(m_slider, &ScaleSlider::valueChanged, [this](int value) {
        double perc = (double)value / (MAXIMUM - MINIMUM);
        setCurrentValue(value);
        setCurrentInfo(value);
        int handlHeight = 16;
        m_tooltip->move(m_tooltip->x(),
                        (this->height() + 38 - SLIDER_HEIGHT) / 2
                        + handlHeight / 2
                        + (SLIDER_HEIGHT - handlHeight) * (1 - perc)
                        - m_tooltip->height() / 2);
        emit valueChanged(perc);

        m_tooltip->show();
        m_tooltip->raise();
    });

}

int ImageSliderFrame::currentValue() {
    return m_currentValue;
}

void ImageSliderFrame::setCurrentValue(int val) {
    m_currentValue = val;
    m_slider->setValue(val);
    m_valueLabel->setText(tr("%1%").arg(QString::number(val)));
}

void ImageSliderFrame::setValue(double perc)
{
    m_slider->setValue((1 - perc) * (MAXIMUM - MINIMUM));

    if (! m_showTimer->isActive()) {
        m_showTimer->start();
    }
    m_setValueTimes ++;
}

void ImageSliderFrame::setCurrentInfo(int count)
{
    m_valueLabel->setText(tr("%1%").arg(QString::number(count)));
}

ImageSliderFrame::~ImageSliderFrame() {

}
void ImageSliderFrame::initTooltip()
{
    m_tooltip = new QLabel(this);

    m_tooltip->setObjectName("Tooltip");
    m_tooltip->setFixedSize(74, 38);
    m_tooltip->move(5, 100);
    m_tooltip->hide();

    m_valueLabel = new QLabel();
    m_valueLabel->setObjectName("TooltipValueLabel");
    m_valueLabel->setFixedSize(50, 30);

    QVBoxLayout *lLayout = new QVBoxLayout();
    lLayout->setContentsMargins(1, 3, 0, 4);
    lLayout->addWidget(m_valueLabel);

    QHBoxLayout *mainLayout = new QHBoxLayout(m_tooltip);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(lLayout);

}

void ImageSliderFrame::initTimer()
{
    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    m_showTimer->setInterval(500);
    connect(m_showTimer, &QTimer::timeout, this, [this] {
       if (m_setValueTimes > 7) {
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

void ImageSliderFrame::initStyleSheet() {
    QFile f(":/qss/resources/qss/imageslider.qss");
    if (f.open(QIODevice::ReadOnly)) {
        setStyleSheet(f.readAll());
        f.close();
    }
    else {
        qDebug() << "Set style sheet for ImageSliderFrame failed";
    }
}
