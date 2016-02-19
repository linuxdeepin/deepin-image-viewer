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

private:
    void initWidgets();

private:
    QHBoxLayout *m_leftLayout;
};

#endif // TOPTOOLBAR_H
