#include "mainwindow.h"
#include "mainwidget.h"
#include "controller/configsetter.h"
#include <QDesktopWidget>

namespace {

const int MAINWIDGET_MINIMUN_HEIGHT = 480;
const int MAINWIDGET_MINIMUN_WIDTH = 480;
const QString SETTINGS_GROUP = "MAINWINDOW";
const QString SETTINGS_WINSIZE_W_KEY = "WindowWidth";
const QString SETTINGS_WINSIZE_H_KEY = "WindowHeight";

}  //namespace

MainWindow::MainWindow(bool manager, QWidget *parent):
    WindowFrame(parent),
    m_setter(ConfigSetter::instance())
{
    QDesktopWidget dw;
    const int defaultW = dw.geometry().width() * 0.65 < MAINWIDGET_MINIMUN_WIDTH
            ? MAINWIDGET_MINIMUN_WIDTH : dw.geometry().width() * 0.65;
    const int defaultH = dw.geometry().height() * 0.7 < MAINWIDGET_MINIMUN_HEIGHT
            ? MAINWIDGET_MINIMUN_HEIGHT : dw.geometry().height() * 0.7;
    const int ww = m_setter->value(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY,
                                   QVariant(defaultW)).toInt();
    const int wh = m_setter->value(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY,
                                   QVariant(defaultH)).toInt();
    resize(ww, wh);
    setMinimumSize(MAINWIDGET_MINIMUN_WIDTH, MAINWIDGET_MINIMUN_HEIGHT);

    m_mainWidget = new MainWidget(manager, this);
    addContenWidget(m_mainWidget);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if (! isMaximized() && m_mainWidget->isVisible()) {
        m_setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY,
                           QVariant(m_mainWidget->width()));
        m_setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY,
                           QVariant(m_mainWidget->height()));
    }
    WindowFrame::resizeEvent(e);
}
