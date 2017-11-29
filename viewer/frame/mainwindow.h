/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
