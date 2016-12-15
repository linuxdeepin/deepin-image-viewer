#include "slider.h"
#include "utils/baseutils.h"
#include <QMouseEvent>

Slider::Slider(QWidget *parent)
    : QSlider(parent)
{
//    setStyleSheet(utils::base::getFileContent(":/qss/resources/qss/Slider.qss"));
}

Slider::Slider(Qt::Orientation orientation, QWidget *parent)
    :QSlider(orientation, parent)
{
//    setStyleSheet(utils::base::getFileContent(":/qss/resources/qss/Slider.qss"));
}

void Slider::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        double dv = 0;
        if (orientation() == Qt::Vertical)
            dv = ((maximum()-minimum()) * (height()- e->y()) * 1.0) / height();
        else
            dv = ((maximum()-minimum()) * e->x() * 1.0) / width();
        setValue(minimum() + qRound(dv)) ;

        e->accept();
    }
    QSlider::mousePressEvent(e);
}
