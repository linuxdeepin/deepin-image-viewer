#include "icontooltip.h"

IconTooltip::IconTooltip(QString iconName, QWidget *parent)
    : QLabel(parent) {
    setIconName(iconName);
}

void IconTooltip::setIconName(QString text) {
    this->clear();
    this->setText(text);
    QFont iconNameFont;

    QFontMetrics fontMetrics(iconNameFont);
    int iconNameWidth=fontMetrics.width(text);
    setFixedSize(iconNameWidth+10, 20);
    this->setStyleSheet("QLabel{ background-color:rgba(0, 0, 0, .7);color: white;\
                        border: 1px solid rgba(255, 255, 255, .2);border-radius: 4px;}");
    this->update();
}

//void IconTooltip::paintEvent(QPaintEvent *) {

//}

IconTooltip::~IconTooltip() {
}

