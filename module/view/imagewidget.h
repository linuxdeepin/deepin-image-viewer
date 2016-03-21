#pragma once

#include <QWidget>
#include <QImage>

class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    ImageWidget(QWidget *parent = 0);

    void setImage(const QString& path);
    void setImage(const QImage& image);
    qreal scaleValue() const {return m_scale;}
    void setTransformOrigin(const QPoint& imageP, const QPoint& deviceP);
    QPoint mapToImage(const QPoint& p);
Q_SIGNALS:
    void scaleValueChanged(qreal);
public Q_SLOTS:
    void setScaleValue(qreal value);

protected:
    void paintEvent(QPaintEvent *);
    //void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
private:
    void updateTransform();

    qreal m_scale = 0; // when an image is loaded to fit widget, m_scale is not 1.0
    qreal m_scale_requested = 0;
    QPoint m_o_img, m_o_dev;
    QImage m_image;
    QPixmap m_pixmap;
    QTransform m_mat;

    QPoint m_pos, m_posG;
};
