#include "bottomtoolbar.h"

BottomToolbar::BottomToolbar(QWidget *parent, QWidget *source)
    : BlureFrame(parent, source)
{
    setCoverBrush(QBrush(QColor(24, 24, 24, 230)));
}
