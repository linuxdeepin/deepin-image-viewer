#ifndef TOPTOOLBAR_H
#define TOPTOOLBAR_H

#include "blureframe.h"
#include "dwindowmaxbutton.h"
#include "dwindowminbutton.h"
#include "dwindowclosebutton.h"
#include "dwindowoptionbutton.h"
#include "dwindowrestorebutton.h"
#include <QHBoxLayout>

using namespace Dtk::Widget;

class TopToolbar : public BlureFrame
{
    Q_OBJECT
public:
    TopToolbar(QWidget *parent, QWidget *source);
    void setLeftContent(QWidget *content);
    void setMiddleContent(QWidget *content);

protected:
    void resizeEvent(QResizeEvent *e);

private:
    void initWidgets();

private:
    QHBoxLayout *m_leftLayout;
    QHBoxLayout *m_middleLayout;
    QWidget *m_leftContent;
    QWidget *m_middleContent;
    QWidget *m_rightContent;
};

#endif // TOPTOOLBAR_H
