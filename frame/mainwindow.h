#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "widgets/dwindowframe.h"
#include "frame/mainwidget.h"


class MainWindow : public DWindowFrame
{
public:
    MainWindow(QWidget *parent=0);

    MainWidget *mainWidget;
};

#endif // MAINWINDOW_H
