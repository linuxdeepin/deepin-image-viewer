#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QWidget>
#include "widgets/blureframe.h"

class AboutWindow : public BlurFrame
{
    Q_OBJECT
public:
    explicit AboutWindow(QWidget *parent);

private:
    void initStyleSheet();
};

#endif // ABOUTWINDOW_H
