#ifndef SLIDERFRAME_H
#define SLIDERFRAME_H

#include <QWidget>
class ScaleSlider;

class SliderFrame : public QWidget
{
    Q_OBJECT
public:
    explicit SliderFrame(QWidget *parent = 0);
    void setValue(double perc);

signals:
    void valueChanged(double perc);

private:
    ScaleSlider *m_slider;
};

#endif // SLIDERFRAME_H
