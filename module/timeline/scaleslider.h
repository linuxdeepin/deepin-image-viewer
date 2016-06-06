#ifndef SCALESLIDER_H
#define SCALESLIDER_H

#include <QSlider>

class ScaleSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor)
public:
    explicit ScaleSlider(QWidget *parent = 0);

    QColor penColor() const;
    void setPenColor(const QColor &penColor);

signals:
    void mousePress();
    void mouseRelease();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    QColor m_penColor;
};

#endif // SCALESLIDER_H
