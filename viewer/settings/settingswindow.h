#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class SettingsWindow : public  DMainWindow
{
public:
    // If manager is false, the Manager panel(eg.TimelinePanel) will not be
    // initialize to save resource and avoid DB file lock.
    SettingsWindow(QWidget *parent=0);
};

#endif // SETTINGSWINDOW_H
