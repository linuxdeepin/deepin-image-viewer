#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "frame/mainwidget.h"
#include "controller/viewerthememanager.h"
#include "controller/dbmanager.h"
#include "controller/exporter.h"
#include "controller/importer.h"

#include <DMainWindow>
#include <QWidget>
#include <QDebug>

DWIDGET_USE_NAMESPACE

class Worker : public QObject {
    Q_OBJECT
public:
    Worker() {}
    ~Worker(){}
public slots:
    void initRec() {
        DBManager::instance();
        Exporter::instance();
        Importer::instance();
        qDebug() << "DBManager time";
    }


};
class MainWindow : public  DMainWindow
{
public:
    // If manager is false, the Manager panel(eg.TimelinePanel) will not be
    // initialize to save resource and avoid DB file lock.
    MainWindow(bool manager, QWidget *parent=0);

    void onThemeChanged(ViewerThemeManager::AppTheme theme);
protected:
    void resizeEvent(QResizeEvent *e) override;
//    void showEvent(QShowEvent *event);

private:
    void moveFirstWindow();
    void moveCenter();
    bool windowAtEdge();

    MainWidget *m_mainWidget;
};

#endif // MAINWINDOW_H
