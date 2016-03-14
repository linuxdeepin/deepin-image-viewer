#include "imagewidget.h"
#include <QPainter>
#include <QtDebug>

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
    updateTransform();
    update(); //assume we are in main thread
}

void ImageWidget::setScaleValue(qreal value)
{
    m_scale_requested = value;
    m_scale = value;
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

void ImageWidget::updateTransform()
{
    //Q_EMIT scaleValueChanged(value); //TODO: emit only when scaled image is rendered
    const QSize s = m_image.size()*m_scale;//.scaled(rect().size(), Qt::KeepAspectRatio);
    m_scale = qreal(s.width())/qreal(m_image.size().width());
    qDebug() << s << m_image.size();
    m_mat.reset();
    // qpainter transform origin
    m_mat.scale(m_scale, m_scale);
    m_mat.translate((qreal)(m_o_dev.x()/m_scale - m_o_img.x()), (qreal)(m_o_dev.y()/m_scale - m_o_img.y()));
    qDebug() << m_o_dev << m_mat.inverted().map(m_o_dev);
}
