#include "application.h"
#include "controller/configsetter.h"
#include "mainwindow.h"
#include "mainwidget.h"
#include <DTitlebar>
#include <QDesktopWidget>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include <QScreen>

namespace {

const int MAINWIDGET_MINIMUN_HEIGHT = 480;
const int MAINWIDGET_MINIMUN_WIDTH = 550;
const QString SETTINGS_GROUP = "MAINWINDOW";
const QString SETTINGS_WINSIZE_W_KEY = "WindowWidth";
const QString SETTINGS_WINSIZE_H_KEY = "WindowHeight";

}  //namespace

MainWindow::MainWindow(bool manager, QWidget *parent):
    DMainWindow(parent)
{
    QDesktopWidget dw;
    const int defaultW = dw.geometry().width() * 0.65 < MAINWIDGET_MINIMUN_WIDTH
            ? MAINWIDGET_MINIMUN_WIDTH : dw.geometry().width() * 0.65;
    const int defaultH = dw.geometry().height() * 0.7 < MAINWIDGET_MINIMUN_HEIGHT
            ? MAINWIDGET_MINIMUN_HEIGHT : dw.geometry().height() * 0.7;
    const int ww = dApp->setter->value(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY,
                                       QVariant(defaultW)).toInt();
    const int wh = dApp->setter->value(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY,
                                       QVariant(defaultH)).toInt();
    resize(ww, wh);
    setMinimumSize(MAINWIDGET_MINIMUN_WIDTH, MAINWIDGET_MINIMUN_HEIGHT);

    m_mainWidget = new MainWidget(manager, this);
    setCentralWidget(m_mainWidget);
    if (titleBar()) titleBar()->setFixedHeight(0);
    moveFirstWindow();

//    QHBoxLayout *l = new QHBoxLayout(this);
//    l->setContentsMargins(0, 0, 0, 0);
//    l->addWidget(m_mainWidget);

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

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if (! isMaximized()
            && m_mainWidget->isVisible()
            && ! window()->isFullScreen()
            && ! window()->isMaximized()) {
        dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY,
                               QVariant(m_mainWidget->width()));
        dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY,
                               QVariant(m_mainWidget->height()));
    }

    DMainWindow::resizeEvent(e);
}
