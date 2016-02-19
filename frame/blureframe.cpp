#include "blureframe.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>
#include <QDebug>

BlureFrame::BlureFrame(QWidget *parent, QWidget *source)
    : QFrame(parent), m_sourceWidget(source)
{

}

void BlureFrame::setSourceWidget(QWidget *source)
{
    m_sourceWidget = source;
}

void BlureFrame::setCoverBrush(const QBrush &brush)
{
    m_coverBrush = brush;
    update();
}

void BlureFrame::setBlureRadius(int radius)
{
    m_blureRadius = radius;
    update();
}

void BlureFrame::paintEvent(QPaintEvent *)
{
    if (!parentWidget() || parentWidget() == m_sourceWidget)
        return;

    QGraphicsBlurEffect *effect = new QGraphicsBlurEffect(this);
    effect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
    effect->setBlurRadius(m_blureRadius);
    QPixmap bp;
    bp.convertFromImage(applyEffectToImage(m_sourceWidget->grab().toImage(), effect));
    bp = bp.copy(geometry());//Crop effective area

    QPainter p(this);
    p.drawPixmap(0, 0, width(), height(), bp);
    p.fillRect(0, 0, width(), height(), m_coverBrush);
    p.end();
}

QImage BlureFrame::applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent)
{
    if(src.isNull()) return QImage();   //No need to do anything else!
    if(!effect) return src;             //No need to do anything else!
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(src));
    item.setGraphicsEffect(effect);
    scene.addItem(&item);
    QImage res(src.size()+QSize(extent*2, extent*2), QImage::Format_ARGB32);
    res.fill(Qt::black);
    QPainter ptr(&res);
    scene.render(&ptr, QRectF(), QRectF( -extent, -extent, src.width()+extent*2, src.height()+extent*2 ) );

    return res;
}
