#include "navigationwidget.h"
#include <QPainter>
#include <dwindowclosebutton.h>
#include <QMouseEvent>
#include <QtDebug>

using namespace Dtk::Widget;
NavigationWidget::NavigationWidget(QWidget *parent)
    : QWidget(parent)
{
    resize(200, 150);
    DWindowCloseButton *b = new DWindowCloseButton(this);
    connect(b, &DWindowCloseButton::clicked, [this](){
        setAlwaysHidden(true);
    });
}

void NavigationWidget::setAlwaysHidden(bool value)
{
    m_hide = value;
    if (isAlwaysHidden())
        hide();
    else
        show();
}

void NavigationWidget::setImage(const QImage &img)
{
    m_img = img.scaled(rect().size(), Qt::KeepAspectRatio);
    m_pix = QPixmap::fromImage(m_img);
    m_imageScale = qreal(m_img.width())/qreal(img.width());
    m_r = QRect(0, 0, m_img.width(), m_img.height());
    update();
}

void NavigationWidget::setRectInImage(const QRect &r)
{
    if (m_img.isNull())
        return;
    m_r.setX((qreal)r.x() * m_imageScale);
    m_r.setY((qreal)r.y() * m_imageScale);
    m_r.setWidth((qreal)r.width() * m_imageScale);
    m_r.setHeight((qreal)r.height() * m_imageScale);
    update();
}

void NavigationWidget::mousePressEvent(QMouseEvent *e)
{
    tryMoveRect(e->pos());
}

void NavigationWidget::mouseMoveEvent(QMouseEvent *e)
{
    tryMoveRect(e->pos());
}

void NavigationWidget::tryMoveRect(const QPoint &p)
{
    const int x0 = (width()-m_img.width())/2;
    const int y0 = (height()-m_img.height())/2;
    const QRect r(x0, y0, m_img.width(), m_img.height());
    if (!r.contains(p))
        return;
    const int x = qMax(0, qMin(p.x() - x0 - m_r.width()/2, m_img.width()-m_r.width()));
    const int y = qMax(0, qMin(p.y() - y0 - m_r.height()/2, m_img.height()-m_r.height()));

    //qDebug("request move: %d %d %.1f %.1f", x, y, (qreal)x/m_imageScale, (qreal)y/m_imageScale);
    Q_EMIT requestMove((qreal)x/m_imageScale, (qreal)y/m_imageScale);
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
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setClipRegion(QRegion(0, 0, img.width(), img.height()) - m_r);
    p.fillRect(QRect(0, 0, m_img.width(), m_img.height()), QColor(0, 0, 0, 100));
    p.end();
    p.begin(this);
    p.fillRect(rect(), QColor(0, 0, 0, 100));
    p.drawImage((width()-m_img.width())/2, (height()-m_img.height())/2, img);
    p.end();
}
