#ifndef BOTTOMTOOLBAR_H
#define BOTTOMTOOLBAR_H

#include "widgets/blureframe.h"
#include <QHBoxLayout>

class BottomToolbar : public BlurFrame
{
    Q_OBJECT
public:
    explicit BottomToolbar(QWidget *parent);
    void setContent(QWidget *content);

protected:
    void mouseMoveEvent(QMouseEvent *) override;

private:
    QHBoxLayout *m_mainLayout;
};

#endif // BOTTOMTOOLBAR_H
