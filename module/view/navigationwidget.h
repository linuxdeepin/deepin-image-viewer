#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include <QWidget>

class NavigationWidget : public QWidget
{
    Q_OBJECT
public:
    NavigationWidget(QWidget* parent = 0);
    void setImage(const QImage& img);
    void setRectInImage(const QRect& r);

Q_SIGNALS:
    void requestMove(int x, int y);
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
private:
    void tryMoveRect(const QPoint& p);
    qreal m_imageScale = 1.0;
    QImage m_img;
    QPixmap m_pix;
    QRect m_r;
};

#endif // NAVIGATIONWIDGET_H
