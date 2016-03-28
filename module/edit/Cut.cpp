#include "Cut.h"
#include <QPainter>
#include <QPen>
#include <QMouseEvent>
#include <QThread>
#include <QtDebug>
#include <QCoreApplication>
#include <QtMath>

void CutWidget::setAspectRatio(qreal value)
{
    if (m_ar == value)
        return;
    m_ar = value;
}

void CutWidget::setImage(const QImage &img)
{
    m_rotating = false;
    m_rot = 0;
    m_pixmap = QPixmap::fromImage(img);
    m_rect = QRect(rect().center() - QPoint(100, 50), rect().center() + QPoint(100, 50));
    m_clip = QRegion(rect()) - m_rect;
    updateTransform();
}

QRect CutWidget::cutRect() const
{
    return m_mat.inverted().mapRect(m_rect);
}

void CutWidget::mousePressEvent(QMouseEvent *event)
{
    m_pos0 = event->pos();
    m_pos1 = event->pos();
    m_pos = event->pos();
    m_posG = event->globalPos();
    if (m_rect.contains(event->pos())) {

    } else {
        m_rotating = true;
    }

    updateTransform();
}

void CutWidget::mouseMoveEvent(QMouseEvent *event)
{
    // TODO: aspect ratio
    const int dx = event->globalPos().x() - m_posG.x();
    const int dy = event->globalPos().y() - m_posG.y();
    m_pos1 = event->pos();
    const QRect r = m_rect.adjusted(dx, dy, dx, dy);
    if (r.contains(m_pos) && !m_rotating) {
        const QPoint p = m_pos - m_rect.topLeft();
        if (p.x() < m_rect.width()/3) {
            if (p.y() < m_rect.height()/3) {
                m_rect.adjust(dx, dy, 0, 0);
            } else if (p.y() < m_rect.height()*2/3) {
                m_rect.adjust(dx, 0, 0, 0);
            } else {
                m_rect.adjust(dx, 0, 0, dy);
            }
        } else if (p.x() < m_rect.width()*2/3) {
            if (p.y() < m_rect.height()/3) {
                m_rect.adjust(0, dy, 0, 0);
            } else if (p.y() < m_rect.height()*2/3) {
                m_rect.adjust(dx, dy, dx, dy);
            } else {
                m_rect.adjust(0, 0, 0, dy);
            }
        } else {
            if (p.y() < m_rect.height()/3) {
                m_rect.adjust(0, dy, dx, 0);
            } else if (p.y() < m_rect.height()*2/3) {
                m_rect.adjust(0, 0, dx, 0);
            } else {
                m_rect.adjust(0, 0, dx, dy);
            }
        }
    } else {
        if (m_rotating) {
            const QVector2D v(QPoint(0, 1));
            const QVector2D v0(m_pos0 - rect().center());
            const QVector2D v1(m_pos1 - rect().center());
            const qreal cos0 = QVector2D::dotProduct(v0, v)/v.length()/v0.length();
            const qreal cos1 = QVector2D::dotProduct(v1, v)/v.length()/v1.length();
            const qreal dAngle = 180.0*(qAcos(cos1) - qAcos(cos0))/M_PI;
            if (m_pos0.x() < rect().center().x())
                m_rot = m_rot0 + dAngle;
            else
                m_rot = m_rot0 - dAngle;
            m_rot %= 360;
        } else {
            //m_rect = QRect(m_pos0, m_pos1).normalized();
        }
    }
    if (m_ar > 0) {
        const int h = (qreal)m_rect.width()/m_ar;
        m_rect.setHeight(h);
    }
    m_clip = QRegion(rect()) - m_rect;
    m_pos = event->pos();
    m_posG = event->globalPos();
    updateTransform();
}

void CutWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_rotating = false;
    m_rot0 = m_rot;
}

void CutWidget::paintEvent(QPaintEvent *)
{
    if (m_pixmap.isNull())
        return;
    QPainter p(this);
    p.save();
    p.setTransform(m_mat);
    p.drawPixmap(QPoint(), m_pixmap);
    p.restore();
    p.save();
    p.setOpacity(0.5);
    p.setClipRegion(m_clip);
    p.fillRect(rect(), Qt::black);
    p.restore();
    QPen pen;
    pen.setStyle(Qt::DashLine);
    pen.setColor(Qt::white);
    p.setPen(pen);
    p.drawRect(m_rect);

    p.translate(m_rect.topLeft());
    pen.setStyle(Qt::SolidLine);
    p.setPen(pen);
    QVector<QLine> lines;

    lines = QVector<QLine>()
          << QLine(m_rect.width()/3, 0, m_rect.width()/3, m_rect.height())
          << QLine(m_rect.width()*2/3, 0, m_rect.width()*2/3, m_rect.height())
          << QLine(0, m_rect.height()/3, m_rect.width(), m_rect.height()/3)
          << QLine(0, m_rect.height()*2/3, m_rect.width(), m_rect.height()*2/3);
    p.drawLines(lines);
    pen.setWidth(pen.width()*2);
    p.setPen(pen);
    static const int k = 5;
    lines = QVector<QLine>()
            << QLine(0, 0, k, 0)
            << QLine(0, 0, 0, k)
            << QLine(0, m_rect.height()-k, 0, m_rect.height())
            << QLine(0, m_rect.height(), k, m_rect.height())
            << QLine(m_rect.width() - k, m_rect.height(), m_rect.width(), m_rect.height())
            << QLine(m_rect.width(), m_rect.height() - k, m_rect.width(), m_rect.height())
            << QLine(m_rect.width(), 0, m_rect.width(), k)
            << QLine(m_rect.width() - k, 0, m_rect.width(), 0)
               ;
    p.drawLines(lines);
}

void CutWidget::resizeEvent(QResizeEvent *)
{
    m_clip = QRegion(rect()) - m_rect; //?
    updateTransform();
}

void CutWidget::updateTransform()
{
    if (m_pixmap.isNull())
        return;
    QSize ss = m_pixmap.size().scaled(width(), height(), Qt::KeepAspectRatio);
    const qreal s = (qreal)ss.width()/(qreal)m_pixmap.width();
    m_mat.reset();
    QPoint img_o = QTransform().rotate(m_rot).scale(s, s).map(QPoint(m_pixmap.width()/2, m_pixmap.height()/2));
    m_mat.translate(rect().center().x() - (qreal)img_o.x(), rect().center().y() - (qreal)img_o.y());
    m_mat.rotate(m_rot);
    m_mat.scale(s, s);
    update();
}
