#include "slider.h"
#include "utils/baseutils.h"

Slider::Slider(QWidget *parent)
    : QSlider(parent)
{
    setStyleSheet(utils::base::getFileContent(":/qss/resources/qss/Slider.qss"));
}

Slider::Slider(Qt::Orientation orientation, QWidget *parent)
    :QSlider(orientation, parent)
{
    setStyleSheet(utils::base::getFileContent(":/qss/resources/qss/Slider.qss"));
}
