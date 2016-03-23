#include "imagewidget.h"
#include <QPainter>
#include <QtDebug>
#include <QMouseEvent>

ImageWidget::ImageWidget(QWidget *parent)
    : QWidget(parent)
{
}

void ImageWidget::setImage(const QString &path)
{
    setImage(QImage(path));
}

void ImageWidget::setImage(const QImage &image)
{
    m_image = image;
    m_pixmap = QPixmap::fromImage(m_image);
    //m_scale = 0.4;
    m_o_dev = rect().center();
    const QSize s = m_image.size().scaled(rect().size(), Qt::KeepAspectRatio);
    m_o_img = QPoint(m_image.width()/2, m_image.height()/2);
    m_scale = qreal(s.width())/qreal(m_image.size().width());
    m_flipX = m_flipY = 1;
    m_rot = 0;
    updateTransform();
    update(); //assume we are in main thread
}

void ImageWidget::setScaleValue(qreal value)
{
    m_scale_requested = value;
    m_scale = value;
    updateTransform();
}

void ImageWidget::rotate(int deg)
{
    m_rot = deg%360;
    updateTransform();
}

void ImageWidget::flipX()
{
    m_flipX = -m_flipX;
    updateTransform();
}

void ImageWidget::flipY()
{
    m_flipY = -m_flipY;
    updateTransform();
}

void ImageWidget::setTransformOrigin(const QPoint& imageP, const QPoint& deviceP)
{
    m_o_dev = deviceP;
    m_o_img = imageP;
    updateTransform();
}

QPoint ImageWidget::mapToImage(const QPoint &p)
{
    return m_mat.inverted().map(p);
}

void ImageWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setTransform(m_mat);
    p.drawPixmap(0, 0, m_pixmap);
}

void ImageWidget::mousePressEvent(QMouseEvent *event)
{
    QMouseEvent *me = static_cast<QMouseEvent*>(event);
    //Qt::MouseButton mbt = me->button();
    //if (mbt != Qt::LeftButton)
      //  return;
    m_pos = me->pos();
    m_posG = me->globalPos();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //QMouseEvent *me = static_cast<QMouseEvent*>(event);
    //Qt::MouseButton mbt = me->button();
    //if (mbt != Qt::LeftButton)
      //  return;
    m_pos = m_posG = QPoint();
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    QMouseEvent *me = static_cast<QMouseEvent*>(event);
    //Qt::MouseButton mbt = me->button();
    //if (mbt != Qt::LeftButton)
      //  return;
    QPoint dp = event->globalPos() - m_posG;
    setTransformOrigin(m_o_img, m_o_dev + dp);
    m_pos = me->pos();
    m_posG = me->globalPos();
}

void ImageWidget::wheelEvent(QWheelEvent *event)
{
    setTransformOrigin(mapToImage(event->pos()), event->pos());
    qreal deg = event->angleDelta().y()/8;
    QPoint dp = event->pixelDelta();
    qreal zoom = m_scale;
    if (dp.isNull())
        zoom += deg*3.14/180.0;
    else
        zoom += dp.y()/100.0;
    if (zoom < 0.5 || zoom > 10)
        return;
    setScaleValue(zoom);
    qDebug("zoom: %.3f", zoom);
}

void ImageWidget::updateTransform()
{
    //Q_EMIT scaleValueChanged(value); //TODO: emit only when scaled image is rendered
    const QSize s = m_image.size()*m_scale;//.scaled(rect().size(), Qt::KeepAspectRatio);
    m_scale = qreal(s.width())/qreal(m_image.size().width());
    //qDebug() << s << m_image.size();
    m_mat.reset();
    QPoint img_o = QTransform().rotate(m_rot).scale(m_scale*m_flipX, m_scale*m_flipY).map(m_o_img);
    m_mat.translate(m_o_dev.x() - (qreal)img_o.x(), m_o_dev.y() - (qreal)img_o.y());
    m_mat.rotate(m_rot);
    m_mat.scale(m_scale*m_flipX, m_scale*m_flipY);
    //qDebug() << m_o_dev << m_mat.inverted().map(m_o_dev);
    //qDebug() << m_mat;
    update();
}
