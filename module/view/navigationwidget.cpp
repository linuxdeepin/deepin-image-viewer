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

}  // namespace

using namespace Dtk::Widget;

const int IMAGE_MARGIN = 3;
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

    m_mainRect = QRect(rect().x() + IMAGE_MARGIN, rect().y() + IMAGE_MARGIN,
                       rect().width() - IMAGE_MARGIN*2, rect().height() - IMAGE_MARGIN*2);
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
    QRect tmpImageRect = QRect(m_mainRect.x()+1, m_mainRect.y()+1,
                               m_mainRect.width() - IMAGE_MARGIN*2, m_mainRect.height() - IMAGE_MARGIN*2);
    m_img = img.scaled(tmpImageRect.size(), Qt::KeepAspectRatio);

    m_pix = QPixmap::fromImage(m_img);

    if (img.width() > img.height()) {
        m_imageScale = qreal(m_img.width())/qreal(img.width());
    } else {
        m_imageScale = qreal(m_img.height())/qreal(img.height());
    }

    m_r = QRect(IMAGE_MARGIN, IMAGE_MARGIN, m_img.width(), m_img.height());
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
    const QRect r(x0, y0, m_img.width(), m_img.height());
    if (!r.contains(p))
        return;
    const int x = qMax(IMAGE_MARGIN, qMin(p.x() - x0 - m_r.width()/2, m_img.width()-m_r.width()));
    const int y = qMax(IMAGE_MARGIN, qMin(p.y() - y0 - m_r.height()/2, m_img.height()-m_r.height()));

    //qDebug("request move: %d %d %.1f %.1f", x, y, (qreal)x/m_imageScale, (qreal)y/m_imageScale);
    Q_EMIT requestMove((qreal)x/m_imageScale, (qreal)y/m_imageScale);
}

void NavigationWidget::addDropShadow() {
    QGraphicsDropShadowEffect *dropShadow = new QGraphicsDropShadowEffect;
    dropShadow->setBlurRadius(8);
    dropShadow->setColor(QColor(0, 0, 0, 128));
    dropShadow->setOffset(0, 4);
    this->setGraphicsEffect(dropShadow);
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
    p.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setClipRegion(QRegion(0, 0, img.width(), img.height()) - m_r);
    p.fillRect(QRect(0, 0, m_img.width(), m_img.height()), QColor(0, 0, 0, 150));
    p.end();
    p.begin(this);

    p.setPen(QColor(255, 255, 255, 76));
    p.setBrush(QBrush(QColor(0, 0, 0, 100)));
    p.drawRoundedRect(m_mainRect, 3, 3);

    p.drawImage(QRect((m_mainRect.width()-m_img.width())/2 + IMAGE_MARGIN,
                (m_mainRect.height()-m_img.height())/2 + IMAGE_MARGIN, img.width(), img.height()), img);
    p.end();
}
