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
#include "application.h"
#include "controller/configsetter.h"
#include "mainwindow.h"
#include "mainwidget.h"
#include <DTitlebar>
#include <QDesktopWidget>
#include <QFile>
#include <QStandardPaths>
//#include <QDebug>
#include <QDir>
#include <QScreen>
#include <QTimer>
#include <QShortcut>
#include <dgiovolumemanager.h>

namespace {

const int MAINWIDGET_MINIMUN_HEIGHT = 335;
const int MAINWIDGET_MINIMUN_WIDTH = 630;
const QString SETTINGS_GROUP = "MAINWINDOW";
const QString SETTINGS_WINSIZE_W_KEY = "WindowWidth";
const QString SETTINGS_WINSIZE_H_KEY = "WindowHeight";

}  //namespace

MainWindow::MainWindow(bool manager, QWidget *parent):
    DMainWindow(parent)
{
    // Maxmizing
    QShortcut *maxmizing = new QShortcut(QKeySequence("Ctrl+Alt+F"), this);
    maxmizing->setContext(Qt::WidgetWithChildrenShortcut);
    connect(maxmizing, &QShortcut::activated, this, [=] {
        if(isMaximized()){
            showNormal();
        }else {
            showMaximized();
        }
    });

    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    QDesktopWidget dw;
    const int defaultW = dw.geometry().width() * 0.80 < MAINWIDGET_MINIMUN_WIDTH
            ? MAINWIDGET_MINIMUN_WIDTH : dw.geometry().width() * 0.80;
    const int defaultH = dw.geometry().height() * 0.80 < MAINWIDGET_MINIMUN_HEIGHT
            ? MAINWIDGET_MINIMUN_HEIGHT : dw.geometry().height() * 0.80;
    const int ww = dApp->setter->value(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY,
                                       QVariant(defaultW)).toInt();
    const int wh = dApp->setter->value(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY,
                                       QVariant(defaultH)).toInt();

    setMinimumSize(MAINWIDGET_MINIMUN_WIDTH, MAINWIDGET_MINIMUN_HEIGHT);
    resize(ww, wh);

    dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, ww);
    dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, wh);
    m_mainWidget = new MainWidget(manager, this);
    QTimer::singleShot(200, [=]{
         setCentralWidget(m_mainWidget);
    });

    if (titlebar()) {
        titlebar()->setFixedHeight(50);
        titlebar()->setTitle("");
        titlebar()->setIcon( QIcon::fromTheme("deepin-image-viewer"));
        setTitlebarShadowEnabled(true);
        connect(dApp->signalM, &SignalManager::enterView,
                this, [=](bool a) {
            if(a){
                titlebar()->setFixedHeight(0);
                titlebar()->setTitle("");
                QIcon empty;
                titlebar()->setIcon(empty);
                setTitlebarShadowEnabled(false);
            }
            else{
                titlebar()->setFixedHeight(50);
                titlebar()->setTitle("");
                titlebar()->setIcon( QIcon::fromTheme("deepin-image-viewer"));
                setTitlebarShadowEnabled(true);
            }
        });
    }
    moveFirstWindow();

#ifndef LITE_DIV
    QThread* workerThread = new QThread;
    Worker* worker = new Worker();
    worker->moveToThread(workerThread);
    connect(workerThread, &QThread::finished, worker, &Worker::deleteLater);

    QTimer::singleShot(300, [=]{
        workerThread->start();
    });
#endif

//    QHBoxLayout *l = new QHBoxLayout(this);
//    l->setContentsMargins(0, 0, 0, 0);
//    l->addWidget(m_mainWidget);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &MainWindow::onThemeChanged);


    m_vfsManager = new DGioVolumeManager;
//    connect(m_vfsManager, &DGioVolumeManager::mountAdded, this, &AlbumView::onVfsMountChangedAdd);
//    connect(m_vfsManager, &DGioVolumeManager::mountRemoved, this, &AlbumView::onVfsMountChangedRemove);
    connect(m_vfsManager, &DGioVolumeManager::mountAdded, this, [=](){
            int a = 0;
            qDebug()<<"!!!!!!!!!!!!!!!!!!USB IN!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    });
    connect(m_vfsManager, &DGioVolumeManager::mountRemoved, this, [=](){
            int a = 0;
            qDebug()<<"!!!!!!!!!!!!!!!!!!USB OUT!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    });
}

void MainWindow::moveFirstWindow() {
    //TODO use QLocalServer more safe ?
    QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
    QFile processFile(QString("%1/%2").arg(cachePath).arg("process.pid"));

    if (processFile.exists()) {
        if (processFile.open(QIODevice::ReadWrite)) {
            int historyId = processFile.readAll().toInt();
            QDir hisProcessDir(QString("/proc/%1").arg(historyId));
            processFile.close();
            if (hisProcessDir.exists())
                return;

            if (processFile.open(QIODevice::ReadWrite|QIODevice::Truncate)) {
                QTextStream pidInfo(&processFile);
                pidInfo << dApp->applicationPid();
                processFile.close();
            }
            this->moveCenter();
        }
    } else {
        if (processFile.open(QIODevice::WriteOnly|QIODevice::Text)) {

            QTextStream pidInfo(&processFile);
            pidInfo << dApp->applicationPid();
            processFile.close();
            this->moveCenter();
        } else {
            qDebug() << "process File open failed!";
        }
    }

}

void MainWindow::moveCenter() {
    QPoint pos = QCursor::pos();
    QRect primaryGeometry;

    for (QScreen *screen : dApp->screens()) {
        if (screen->geometry().contains(pos)) {
            primaryGeometry = screen->geometry();
        }
    }

    if (primaryGeometry.isEmpty()) {
        primaryGeometry = dApp->primaryScreen()->geometry();
    }

    this->move(primaryGeometry.x() + (primaryGeometry.width() - this->width())/2,
               primaryGeometry.y() + (primaryGeometry.height() - this->height())/2);
}

void MainWindow::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        setBorderColor(QColor(0, 0, 0, 204));
    } else {
        setBorderColor(QColor(0, 0, 0, 38));
    }
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if (! isMaximized()
            && m_mainWidget->isVisible()
            && ! window()->isFullScreen()
            && ! window()->isMaximized() && !windowAtEdge()) {
        dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY,
                               QVariant(m_mainWidget->width()));
        dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY,
                               QVariant(m_mainWidget->height()));
    }

    emit dApp->signalM->updateTopToolbar();
    DMainWindow::resizeEvent(e);
}

//void MainWindow::showEvent(QShowEvent *event) {
//    Q_UNUSED(event);
//    qDebug() << "showEvent time";
//}

bool MainWindow::windowAtEdge() {
    //TODO: process the multi-screen
    QRect currentRect = window()->geometry();
    bool atSeperScreenPos = false;

    if (currentRect.x() == 0 || qAbs(currentRect.right() -
           dApp->primaryScreen()->geometry().width()) <= 5) {
        atSeperScreenPos = true;
    }

    return atSeperScreenPos;
}
