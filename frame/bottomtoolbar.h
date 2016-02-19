#ifndef BOTTOMTOOLBAR_H
#define BOTTOMTOOLBAR_H

#include "blureframe.h"

class BottomToolbar : public BlureFrame
{
    Q_OBJECT
public:
    explicit BottomToolbar(QWidget *parent, QWidget *source);
};

#endif // BOTTOMTOOLBAR_H
