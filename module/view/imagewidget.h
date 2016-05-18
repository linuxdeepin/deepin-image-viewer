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
    QImage image() const {return m_image;}
    QString imageName() const;
    QString imagePath() const;
    void resetTransform();
    qreal scaleValue() const {return m_scale;}
    void setTransformOrigin(const QPoint& imageP, const QPoint& deviceP);
    QPoint mapToImage(const QPoint& p) const;
    QRect mapToImage(const QRect& r) const;
    QRect visibleImageRect() const;
    bool isWholeImageVisible() const;
    bool isMoving() const;

Q_SIGNALS:
    void scaleValueChanged(qreal);
    void transformChanged(const QTransform&);
    void doubleClicked();
    void rotated(int degree);
    void fliped(bool x, bool y);
public Q_SLOTS:
    void setImageMove(int x, int y);
    void setScaleValue(qreal value);
    void rotateClockWise() {rotate(m_rot+90);}
    void rotateCounterclockwise() {rotate(m_rot-90);}
    void rotate(int deg);
    void flipX();
    void flipY();
    void setFullScreen(QSize fullSize);
protected:
    void timerEvent(QTimerEvent* e) override;
    void paintEvent(QPaintEvent *) override;
    //void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *) override {
        Q_EMIT doubleClicked();
    }

private:
    void updateTransform();

    int m_flipX = 1;
    int m_flipY = 1;
    int m_rot = 0;
    int m_tid = 0;
    bool m_scaling = false;
    bool m_moving = false;
    qreal m_scale = 0; // when an image is loaded to fit widget, m_scale is not 1.0
    QPoint m_o_img, m_o_dev;
    QString m_path;
    QImage m_image;
    QPixmap m_pixmap;
    QTransform m_mat;

    QPoint m_pos, m_posG;
};
