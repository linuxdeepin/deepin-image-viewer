#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include <QWidget>

class NavigationWidget : public QWidget
{
public:
    NavigationWidget(QWidget* parent = 0);
    void setImage(const QImage& img);
    void setRectInImage(const QRect& r);
    QSize sizeHint() const {
        return QSize(200, 200);
    }
protected:
    void paintEvent(QPaintEvent *);
private:
    qreal m_imageScale = 1.0;
    QImage m_img;
    QPixmap m_pix;
    QRect m_r;
};

#endif // NAVIGATIONWIDGET_H
