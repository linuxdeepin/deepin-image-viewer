#include "navigationwidget.h"
#include <QPainter>
#include <QtDebug>

NavigationWidget::NavigationWidget(QWidget *parent)
    : QWidget(parent)
{

}

void NavigationWidget::setImage(const QImage &img)
{
    m_img = img.scaled(rect().size(), Qt::KeepAspectRatio);
    m_pix = QPixmap::fromImage(m_img);
    m_imageScale = qreal(m_img.width())/qreal(img.width());
    update();
}

void NavigationWidget::setRectInImage(const QRect &r)
{
    m_r.setX((qreal)r.x() * m_imageScale);
    m_r.setY((qreal)r.y() * m_imageScale);
    m_r.setWidth((qreal)r.width() * m_imageScale);
    m_r.setHeight((qreal)r.height() * m_imageScale);
    update();
}

void NavigationWidget::paintEvent(QPaintEvent *)
{
    if (m_img.isNull()) {
        QPainter p(this);
        p.fillRect(m_r, QColor(255, 0, 0, 100));
        return;
    }
    QImage img(m_img);
    QPainter p(&img);
    p.setClipRegion(QRegion(0, 0, img.width(), img.height()) - m_r);
    p.fillRect(QRect(0, 0, img.width(), img.height()), QColor(0, 0, 0, 100));
    p.end();
    p.begin(this);
    p.drawImage((width()-m_img.width())/2, (height()-m_img.height())/2, img);
    p.end();
}
