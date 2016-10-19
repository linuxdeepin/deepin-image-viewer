#include "application.h"
#include "controller/configsetter.h"
#include "navigationwidget.h"
#include <QPainter>
#include <dwindowclosebutton.h>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QtDebug>

namespace {

const QString SETTINGS_GROUP = "VIEWPANEL";
const QString SETTINGS_ALWAYSHIDDEN_KEY = "NavigationAlwaysHidden";
const int IMAGE_MARGIN = 6;
const int IMAGE_MARGIN_BOTTOM = 9;

}  // namespace

using namespace Dtk::Widget;

NavigationWidget::NavigationWidget(QWidget *parent)
    : QWidget(parent)
{
    resize(150, 112);

    DWindowCloseButton *b = new DWindowCloseButton(this);
    b->move(QPoint(this->x() + this->width() - 28,
                   rect().topRight().y() + 2));
    connect(b, &DWindowCloseButton::clicked, [this](){
        setAlwaysHidden(true);
    });

    m_mainRect = QRect(rect().x() + IMAGE_MARGIN,
                       rect().y(),
                       rect().width() - IMAGE_MARGIN*2,
                       rect().height() - IMAGE_MARGIN_BOTTOM);
}

void NavigationWidget::setAlwaysHidden(bool value)
{
    dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_ALWAYSHIDDEN_KEY,
                           QVariant(value));
    if (isAlwaysHidden())
        hide();
    else
        show();
}

bool NavigationWidget::isAlwaysHidden() const
{
    return dApp->setter->value(SETTINGS_GROUP, SETTINGS_ALWAYSHIDDEN_KEY,
                              QVariant(false)).toBool();
}

void NavigationWidget::setImage(const QImage &img)
{
    QRect tmpImageRect = QRect(m_mainRect.x(), m_mainRect.y(),
                               m_mainRect.width(), m_mainRect.height());


    m_originRect = img.rect();
    m_img = img.scaled(tmpImageRect.size(), Qt::KeepAspectRatio);
    m_pix = QPixmap::fromImage(m_img);

    if (img.width() > img.height()) {
        m_imageScale = qreal(m_img.width())/qreal(img.width());
    } else {
        m_imageScale = qreal(m_img.height())/qreal(img.height());
    }

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
    if (e->button() == Qt::LeftButton)
        tryMoveRect(e->pos());
}

void NavigationWidget::mouseMoveEvent(QMouseEvent *e)
{
    tryMoveRect(e->pos());
}

void NavigationWidget::tryMoveRect(const QPoint &p)
{
    const int x0 = (m_mainRect.width()-m_img.width())/2;
    const int y0 = (m_mainRect.height()-m_img.height())/2;
    const QRect imageRect(x0, y0, m_img.width(), m_img.height());
    if (! imageRect.contains(p))
        return;
    const qreal x = 1.0 * (p.x() - x0) / m_img.width() * m_originRect.width();
    const qreal y = 1.0 * (p.y() - y0) / m_img.height() * m_originRect.height();

    Q_EMIT requestMove(x, y);
}

void NavigationWidget::paintEvent(QPaintEvent *)
{
    QImage img(m_img);
    if (m_img.isNull()) {
        QPainter p(this);
        p.fillRect(m_r, QColor(0, 0, 0, 100));
        return;
    }

    QPainter p(&img);
    p.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setClipRegion(QRegion(0, 0, img.width(), img.height()) - m_r);
    p.fillRect(QRect(0, 0, m_img.width(), m_img.height()), QColor(0, 0, 0, 150));
    p.setPen(QColor(255, 255, 255, 80));
    p.drawRect(m_r);
    p.end();
    p.begin(this);

    QImage background(":/images/resources/images/naviwindow_background.png");

    p.drawImage(this->rect(), background);
    QRect imageDrawRect = QRect((m_mainRect.width() - img.width())/2 + IMAGE_MARGIN,
                                (m_mainRect.height() - img.height())/2 + IMAGE_MARGIN, img.width(),
                                img.height() - IMAGE_MARGIN_BOTTOM);

    p.drawImage(imageDrawRect, img);
    p.setPen(QColor(255, 255, 255, 50));
    p.drawRect(imageDrawRect);
    p.end();
}
