#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "frame/mainwidget.h"
#include "widgets/windowframe.h"
#include <QWidget>


class ConfigSetter;
class MainWindow : public WindowFrame
{
public:
    // If manager is false, the Manager panel(eg.TimelinePanel) will not be
    // initialize to save resource and avoid DB file lock.
    MainWindow(bool manager, QWidget *parent=0);

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    MainWidget *m_mainWidget;
    ConfigSetter *m_setter;
};

#endif // MAINWINDOW_H
