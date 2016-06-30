#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "widgets/dwindowframe.h"
#include "frame/mainwidget.h"


class ConfigSetter;
class MainWindow : public DWindowFrame
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
