#ifndef IMAGESLIDERFRAME_H
#define IMAGESLIDERFRAME_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <dimagebutton.h>

using namespace Dtk::Widget;

class ScaleSlider;
class Tooltip;

class ImageSliderFrame : public QWidget
{
    Q_OBJECT
public:
    explicit ImageSliderFrame(QWidget *parent = 0);
    void setValue(double perc);
    void setCurrentInfo(int count);
    ~ImageSliderFrame();

public slots:
    void setCurrentValue(int val);
    int currentValue();

signals:
    void valueChanged(double perc);

private:
    void initTooltip();
    void initTimer();
    void initStyleSheet();
private:
    int m_currentValue;
    ScaleSlider *m_slider;
    QLabel *m_tooltip;
    QLabel *m_valueLabel;
    QTimer *m_showTimer;
    QTimer *m_hideTimer;
    int m_setValueTimes;

};

#endif // IMAGESLIDERFRAME_H
