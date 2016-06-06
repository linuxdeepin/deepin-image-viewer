#ifndef SCALESLIDER_H
#define SCALESLIDER_H

#include <QSlider>

class ScaleSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor)
public:
    enum SliderShape {
        Line,
        Rect,
    };
    explicit ScaleSlider(SliderShape defaultShape = SliderShape::Line, QWidget *parent = 0);

    QColor penColor() const;
    QColor brushColor() const;
    void setPenColor(const QColor &penColor);
    void setBrushColor(const QColor &brushColor);
signals:
    void mousePress();
    void mouseRelease();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    SliderShape m_defaultShape = SliderShape::Line;

    void paintEvent(QPaintEvent *e) override;
    QColor m_penColor;
    QColor m_brushColor;
};

#endif // SCALESLIDER_H
