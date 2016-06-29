#include "mainwindow.h"
#include "mainwidget.h"
#include <QDesktopWidget>


const int MAINWIDGET_MINIMUN_WIDTH = 450;
const int MAINWIDGET_MINIMUN_HEIGHT = 450;

MainWindow::MainWindow(bool manager, QWidget *parent):
    DWindowFrame(parent)
{
    QDesktopWidget dw;
    int ww = dw.geometry().width() * 0.67 < MAINWIDGET_MINIMUN_WIDTH ? MAINWIDGET_MINIMUN_WIDTH : dw.geometry().width() * 0.67;
    int wh = dw.geometry().height() * 0.62 < MAINWIDGET_MINIMUN_HEIGHT ? MAINWIDGET_MINIMUN_HEIGHT : dw.geometry().height() * 0.62;
    resize(ww, wh);
    setMinimumSize(MAINWIDGET_MINIMUN_WIDTH, MAINWIDGET_MINIMUN_HEIGHT);
    move((dw.geometry().width() - ww) / 2, (dw.geometry().height() - wh) / 4);

    mainWidget = new MainWidget(manager, this);
    addContenWidget(mainWidget);

}
