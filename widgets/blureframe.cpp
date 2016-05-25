#include "blureframe.h"
#include <QDebug>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>
#include <QPropertyAnimation>

const int ANIMATION_DURATION = 500;
const QEasingCurve ANIMATION_EASING_CURVE = QEasingCurve::InOutCubic;

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

void BlureFrame::setPos(const QPoint &pos)
{
    QFrame::move(pos);
}

void BlureFrame::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    // Draw outside border
    path.addRoundedRect(this->rect(), m_borderRadius, m_borderRadius);
    QPen pen(m_borderColor, m_borderWidth);
    p.setPen(pen);
    p.drawPath(path);

    // Draw content
    QRect insideRect;
    insideRect.setRect(this->rect().x() + m_borderWidth,
                       this->rect().y() + m_borderWidth,
                       this->rect().width() - m_borderWidth * 2,
                       this->rect().height() - m_borderWidth * 2);
    QPainterPath ip;
    ip.addRoundedRect(insideRect, m_borderRadius, m_borderRadius);
    p.setClipPath(ip);

    p.drawPixmap(0, 0, width(), height(), getResultPixmap());
    p.fillRect(0, 0, width(), height(), m_coverBrush);


    p.end();
}

QPixmap BlureFrame::getResultPixmap()
{
    if (!parentWidget() || parentWidget() == m_sourceWidget)
        return QPixmap();

    QGraphicsBlurEffect *effect = new QGraphicsBlurEffect(this);
    effect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
    effect->setBlurRadius(m_blureRadius);
    QPixmap bp;
    bp.convertFromImage(applyEffectToImage(m_sourceWidget->grab().toImage(), effect));
    bp = bp.copy(geometry());//Crop effective area

    return bp;
}

void BlureFrame::moveWithAnimation(int x, int y)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    animation->setDuration(ANIMATION_DURATION);
    animation->setEasingCurve(ANIMATION_EASING_CURVE);
    animation->setStartValue(pos());
    animation->setEndValue(QPoint(x, y));
    animation->start();
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
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

QColor BlureFrame::getBorderColor() const
{
    return m_borderColor;
}

void BlureFrame::setBorderColor(const QColor &borderColor)
{
    m_borderColor = borderColor;
}

int BlureFrame::getBorderWidth() const
{
    return m_borderWidth;
}

void BlureFrame::setBorderWidth(int borderWidth)
{
    m_borderWidth = borderWidth;
}

int BlureFrame::getBorderRadius() const
{
    return m_borderRadius;
}

void BlureFrame::setBorderRadius(int borderRadius)
{
    m_borderRadius = borderRadius;
}
