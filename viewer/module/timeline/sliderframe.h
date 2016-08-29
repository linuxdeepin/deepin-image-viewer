#ifndef SLIDERFRAME_H
#define SLIDERFRAME_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

class ScaleSlider;
class Tooltip;

class SliderFrame : public QWidget
{
    Q_OBJECT
public:
    explicit SliderFrame(QWidget *parent = 0);
    void setValue(double perc);
    void setCurrentInfo(const QString &month, int count);
    bool containsMouse() const;


signals:
    void valueChanged(double perc);

private:
    void initTooltip();
    void initTimer();

private:
    ScaleSlider *m_slider;
    Tooltip *m_tooltip;
    QLabel *m_timelineLabel;
    QLabel *m_countLabel;
    QTimer *m_showTimer;
    QTimer *m_hideTimer;
    int m_setValueTimes;
};

#endif // SLIDERFRAME_H
