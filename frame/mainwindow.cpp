#include "mainwindow.h"
#include "mainwidget.h"
#include <QDesktopWidget>


const int MAINWIDGET_MINIMUN_WIDTH = 700;
const int MAINWIDGET_MINIMUN_HEIGHT = 500;

MainWindow::MainWindow(QWidget *parent):
    DWindowFrame(parent)
{
    QDesktopWidget dw;
    int ww = dw.geometry().width() * 0.8 < 700 ? 700 : dw.geometry().width() * 0.8;
    int wh = dw.geometry().height() * 0.8 < 500 ? 500 : dw.geometry().height() * 0.8;
    resize(ww, wh);
    setMinimumSize(MAINWIDGET_MINIMUN_WIDTH, MAINWIDGET_MINIMUN_HEIGHT);
    move((dw.geometry().width() - ww) / 2, (dw.geometry().height() - wh) / 4);

    mainWidget = new MainWidget(this);
    addContenWidget(mainWidget);
}
