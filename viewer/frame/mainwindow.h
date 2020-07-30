/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "controller/dbmanager.h"
#include "controller/importer.h"
#include "controller/viewerthememanager.h"
#include "frame/mainwidget.h"
#include "module/slideshow/slideshowpanel.h"

#include <dgiomount.h>
#include <DMainWindow>
#include <QDebug>
#include <QWidget>
#include <DStackedWidget>
#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <QSharedMemory>

DWIDGET_USE_NAMESPACE
typedef QStackedWidget QSWToDStackedWidget;

class Dbusclient;

#undef signals
extern "C" {
#include <gio/gio.h>
}
#define signals public
#ifndef LITE_DIV
class Worker : public QObject
{
    Q_OBJECT
public:
    Worker() {}
    ~Worker() {}
public slots:
    void initRec()
    {
        DBManager::instance();
        Exporter::instance();
        Importer::instance();
        qDebug() << "DBManager time";
    }
};
#endif
class DGioVolumeManager;
class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    // If manager is false, the Manager panel(eg.TimelinePanel) will not be
    // initialize to save resource and avoid DB file lock.
    MainWindow(bool manager, QWidget *parent = 0);
    void initConnection();
    void initshortcut();
    /**
     * @brief initdbus
     * create dbusclient
     */
    void initdbus();
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
    int showDialog();
public slots:
    void OpenImage(QString path);
protected:
    void resizeEvent(QResizeEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
    //    void showEvent(QShowEvent *event);
signals:
    void sigExitFull();
private:
    void moveFirstWindow();
    void moveCenter();
    bool windowAtEdge();
    void paraOpenImageInfo(QString source, QString &path, QStringList &pathlist, QDateTime &stime);
private:
    MainWidget *m_mainWidget=nullptr;
    QSWToDStackedWidget *m_pCenterWidget;
    DGioVolumeManager *m_vfsManager;
    DDiskManager *m_diskManager;
    SlideShowPanel *m_slidePanel;
    bool m_picInUSB = false;
    QDateTime          m_currenttime;
    bool               m_flag = false;
    Dbusclient *m_dbus;
    QSharedMemory m_sharememory;
};

#endif  // MAINWINDOW_H
