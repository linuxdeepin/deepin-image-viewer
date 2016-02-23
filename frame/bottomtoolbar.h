#ifndef BOTTOMTOOLBAR_H
#define BOTTOMTOOLBAR_H

#include "blureframe.h"
#include <QHBoxLayout>

class BottomToolbar : public BlureFrame
{
    Q_OBJECT
public:
    explicit BottomToolbar(QWidget *parent, QWidget *source);
    void setContent(QWidget *content);

private:
    QHBoxLayout *m_mainLayout;
};

#endif // BOTTOMTOOLBAR_H
