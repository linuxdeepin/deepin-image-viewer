#include "toptimelinetips.h"
//#include <QGraphicsDropShadowEffect>

TopTimelineTips::TopTimelineTips(QWidget *parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

//    QGraphicsDropShadowEffect *dropShadow = new QGraphicsDropShadowEffect;
//    dropShadow->setBlurRadius(4);
//    dropShadow->setColor(QColor(0, 0, 0, 128));
//    dropShadow->setOffset(0, 2);
//    this->setGraphicsEffect(dropShadow);
}
