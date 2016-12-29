#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "frame/mainwidget.h"
#include "controller/viewerthememanager.h"
#include <DMainWindow>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class MainWindow : public  DMainWindow
{
public:
    // If manager is false, the Manager panel(eg.TimelinePanel) will not be
    // initialize to save resource and avoid DB file lock.
    MainWindow(bool manager, QWidget *parent=0);

    void onThemeChanged(ViewerThemeManager::AppTheme theme);
protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    void moveFirstWindow();
    void moveCenter();

    MainWidget *m_mainWidget;
};

#endif // MAINWINDOW_H
