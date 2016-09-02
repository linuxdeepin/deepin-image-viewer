#include "toptimelinetips.h"

TopTimelineTips::TopTimelineTips(QWidget *parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setFixedHeight(24);
}

void TopTimelineTips::setLeftMargin(int v)
{
    setContentsMargins(v + 17, 0, 0, 0);
}
