#include "blureframe.h"
#include "utils/baseutils.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>
#include <QPropertyAnimation>
#include <QMouseEvent>

const int ANIMATION_DURATION = 500;
const QEasingCurve ANIMATION_EASING_CURVE = QEasingCurve::InOutCubic;

BlureFrame::BlureFrame(QWidget *parent, QWidget *source)
    : QFrame(parent),
      m_sourceWidget(source),
      m_coverBrush(QBrush(QColor(0, 0, 0, 200))),
      m_blureRadius(50),
      m_borderRadius(0),
      m_borderWidth(0)
{
    m_geometryTimer = new QTimer(this);
    m_geometryTimer->setSingleShot(true);
    connect(m_geometryTimer, &QTimer::timeout, this, [=] {
        m_geometryChanging = false;
        this->update();
    });
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

void BlureFrame::mousePressEvent(QMouseEvent *e)
{
    m_pressPos = e->pos();
}

void BlureFrame::mouseMoveEvent(QMouseEvent *e)
{
    const QPoint t = pos() - m_pressPos + e->pos();
    move(t);
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

    p.drawPixmap(0, 0, width(), height(), getBlurePixmap());
    p.fillRect(0, 0, width(), height(), m_coverBrush);

    p.end();
}

QT_BEGIN_NAMESPACE
  extern Q_WIDGETS_EXPORT void qt_blurImage( QPainter *p,
                                             QImage &blurImage,
                                             qreal radius,
                                             bool quality,
                                             bool alphaOnly,
                                             int transposed = 0 );
QT_END_NAMESPACE
QPixmap BlureFrame::getBlurePixmap()
{
    if (m_geometryChanging ||
            ! m_blur ||
            ! parentWidget() ||
            parentWidget() == m_sourceWidget) {
        return QPixmap();
    }

    QImage si = m_sourceWidget->grab().toImage();
    QPixmap dp(si.size());
    dp.fill( Qt::transparent );
    QPainter painter( &dp );
    qt_blurImage( &painter, si, m_blureRadius, true, false );
    return dp.copy(geometry());

//    QGraphicsBlurEffect *effect = new QGraphicsBlurEffect(this);
//    effect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
//    effect->setBlurRadius(m_blureRadius);

//    QLabel* label = new QLabel();
//    label->setPixmap(m_sourceWidget->grab());
//    label->setGraphicsEffect(effect);
//    return label->grab().copy(geometry());

//    QPixmap bp(10, 10);
//    bp.convertFromImage(applyEffectToImage(m_sourceWidget->grab().toImage(), effect));
//    bp = bp.copy(geometry());//Crop effective area

//    return bp;
}

void BlureFrame::setBlurBackground(bool blur) {
    m_blur = blur;
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

void BlureFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    // FIXME temporary suspend generate the blure pixmap to save CPU usage
    m_geometryChanging = true;
    m_geometryTimer->start(3000);
}

void BlureFrame::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
        return;
    }
    QFrame::keyPressEvent(e);
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
