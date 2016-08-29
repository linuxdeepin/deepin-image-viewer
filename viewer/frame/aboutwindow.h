#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QWidget>
#include "widgets/blureframe.h"

class AboutWindow : public BlureFrame
{
    Q_OBJECT
public:
    explicit AboutWindow(QWidget *parent, QWidget *source);

private:
    void initStyleSheet();
};

#endif // ABOUTWINDOW_H
