#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <dscrollbar.h>

class ScrollBar : public Dtk::Widget::DScrollBar
{
    Q_OBJECT
public:
    explicit ScrollBar(QWidget *parent = 0);
};

#endif // SCROLLBAR_H
