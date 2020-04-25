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
#include "mainwindow.h"
#include <DTitlebar>
#include <QDesktopWidget>
#include <QFile>
#include <QStandardPaths>
#include "application.h"
#include "controller/configsetter.h"
#include "mainwidget.h"
//#include <QDebug>
#include <dgiovolumemanager.h>
#include <DDialog>
#include <DFontSizeManager>
#include <QDir>
#include <QScreen>
#include <QShortcut>
#include <QTimer>
#include <QJsonParseError>
#include <QJsonArray>
#include "utils/baseutils.h"
#include "../service/dbusimageview_adaptor.h"

#define IMAGEVIEW 0
#define SLIDESHOW 1

namespace {

const int MAINWIDGET_MINIMUN_HEIGHT = 335;
const int MAINWIDGET_MINIMUN_WIDTH = 630;
const QString SETTINGS_GROUP = "MAINWINDOW";
const QString SETTINGS_WINSIZE_W_KEY = "WindowWidth";
const QString SETTINGS_WINSIZE_H_KEY = "WindowHeight";

}  // namespace

MainWindow::MainWindow(bool manager, QWidget *parent)
    : DMainWindow(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    QDesktopWidget dw;
    const int defaultW = dw.geometry().width() * 0.60 < MAINWIDGET_MINIMUN_WIDTH
                         ? MAINWIDGET_MINIMUN_WIDTH
                         : dw.geometry().width() * 0.60;
    const int defaultH = dw.geometry().height() * 0.60 < MAINWIDGET_MINIMUN_HEIGHT
                         ? MAINWIDGET_MINIMUN_HEIGHT
                         : dw.geometry().height() * 0.60;
    const int ww =
        dApp->setter->value(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, QVariant(defaultW)).toInt();
    const int wh =
        dApp->setter->value(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, QVariant(defaultH)).toInt();

    setMinimumSize(MAINWIDGET_MINIMUN_WIDTH, MAINWIDGET_MINIMUN_HEIGHT);
    resize(ww, wh);

    dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, ww);
    dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, wh);
    m_pCenterWidget = new QSWToDStackedWidget(this);
    m_mainWidget = new MainWidget(manager, this);
    m_pCenterWidget->addWidget(m_mainWidget);
    m_slidePanel =  new SlideShowPanel();
    m_pCenterWidget->addWidget(m_slidePanel);
    m_pCenterWidget->setCurrentIndex(0);
    //    QTimer::singleShot(200, [=]{
    setCentralWidget(m_pCenterWidget);
    //    });
    initConnection();
    initshortcut();
    if (titlebar()) {
        titlebar()->setFixedHeight(50);
        titlebar()->setTitle("");
        titlebar()->setIcon(QIcon::fromTheme("deepin-image-viewer"));
        setTitlebarShadowEnabled(true);
        connect(dApp->signalM, &SignalManager::enterView, this, [ = ](bool a) {
            if (a) {
                titlebar()->setFixedHeight(0);
                titlebar()->setTitle("");
                QIcon empty;
                titlebar()->setIcon(empty);
                setTitlebarShadowEnabled(false);
            } else {
                titlebar()->setFixedHeight(50);
                titlebar()->setTitle("");
                titlebar()->setIcon(QIcon::fromTheme("deepin-image-viewer"));
                setTitlebarShadowEnabled(true);
            }
        });
    }

#ifndef LITE_DIV
    QThread *workerThread = new QThread;
    Worker *worker = new Worker();
    worker->moveToThread(workerThread);
    connect(workerThread, &QThread::finished, worker, &Worker::deleteLater);

    QTimer::singleShot(300, [ = ] { workerThread->start(); });
#endif

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &MainWindow::onThemeChanged);

    m_vfsManager = new DGioVolumeManager;
    connect(m_vfsManager, &DGioVolumeManager::mountAdded, this, [ = ]() {
        qDebug() << "!!!!!!!!!!!!!!!!!!USB IN!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
        emit dApp->signalM->usbOutIn(true);
    });
    connect(m_vfsManager, &DGioVolumeManager::mountRemoved, this, [ = ]() {
        qDebug() << "!!!!!!!!!!!!!!!!!!USB OUT!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
        emit dApp->signalM->usbOutIn(false);
        if (m_picInUSB) {
            m_picInUSB = false;
        }
    });
    connect(dApp->signalM, &SignalManager::picInUSB, this, [ = ](bool immediately) {
        if (immediately) {
            m_picInUSB = true;
        }
    });
    new ImageViewAdaptor(this);
    m_currenttime = QDateTime::currentDateTime();
    moveFirstWindow();
}

void MainWindow::initshortcut()
{
    QShortcut *esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    esc->setContext(Qt::WindowShortcut);
    connect(esc, &QShortcut::activated, this, [ = ] {
        if(IMAGEVIEW == m_pCenterWidget->currentIndex())
            emit sigExitFull();
        else
        {
            if (window()->isFullScreen())
            {
                emit dApp->signalM->sigESCKeyActivated();
                emit dApp->signalM->sigESCKeyStopSlide();
            } else if (0 == m_pCenterWidget->currentIndex())
            {
                this->close();
            }
            emit dApp->signalM->hideExtensionPanel();
        }
    });
}


void MainWindow::initConnection()
{

    connect(this,SIGNAL(sigExitFull()), m_mainWidget,SIGNAL(sigExitFullScreen()));
    //幻灯片显示
    connect(dApp->signalM, &SignalManager::showSlidePanel, this, [ = ](int index) {
//        if (VIEW_IMAGE != index)
//        {
//            m_backIndex = index;
//        }
        // m_backIndex_fromSlide = index;
        titlebar()->setVisible(false);
        setTitlebarShadowEnabled(false);
        m_pCenterWidget->setCurrentIndex(SLIDESHOW);
    });
    //隐藏幻灯片显示
    connect(dApp->signalM, &SignalManager::hideSlidePanel, this, [ = ]() {
        emit dApp->signalM->hideExtensionPanel();
        //if (0 != m_backIndex_fromSlide) {
        titlebar()->setVisible(true);
        setTitlebarShadowEnabled(true);
        m_pCenterWidget->setCurrentIndex(IMAGEVIEW);
        //   emit dApp->signalM->hideBottomToolbar(false);
        //  emit dApp->signalM->hideExtensionPanel(false);
        //  emit dApp->signalM->hideTopToolbar(false);

    });
}

void MainWindow::moveFirstWindow()
{
    // TODO use QLocalServer more safe ?
    QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
    QFile processFile(QString("%1/%2").arg(cachePath).arg("process.pid"));

    if (processFile.exists()) {
        if (processFile.open(QIODevice::ReadWrite)) {
            int historyId = processFile.readAll().toInt();
            QDir hisProcessDir(QString("/proc/%1").arg(historyId));
            processFile.close();
            if (hisProcessDir.exists())
                return;

            if (processFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
                QTextStream pidInfo(&processFile);
                pidInfo << dApp->applicationPid();
                processFile.close();
            }
            this->moveCenter();
        }
    } else {
        if (processFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream pidInfo(&processFile);
            pidInfo << dApp->applicationPid();
            processFile.close();
            this->moveCenter();
        } else {
            qDebug() << "process File open failed!";
        }
    }
}

void MainWindow::moveCenter()
{
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

    this->move(primaryGeometry.x() + (primaryGeometry.width() - this->width()) / 2,
               primaryGeometry.y() + (primaryGeometry.height() - this->height()) / 2);
}

void MainWindow::onThemeChanged(ViewerThemeManager::AppTheme theme)
{
    if (theme == ViewerThemeManager::Dark) {
        setBorderColor(QColor(0, 0, 0, 204));
    } else {
        setBorderColor(QColor(0, 0, 0, 38));
    }
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if (!isMaximized() && m_mainWidget->isVisible() && !window()->isFullScreen() &&
            !window()->isMaximized() && !windowAtEdge()) {
        dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY,
                               QVariant(m_mainWidget->width()));
        dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY,
                               QVariant(m_mainWidget->height()));
    }

    emit dApp->signalM->updateTopToolbar();
    DMainWindow::resizeEvent(e);
}

bool MainWindow::windowAtEdge()
{
    // TODO: process the multi-screen
    QRect currentRect = window()->geometry();
    bool atSeperScreenPos = false;

    if (currentRect.x() == 0 ||
            qAbs(currentRect.right() - dApp->primaryScreen()->geometry().width()) <= 5) {
        atSeperScreenPos = true;
    }

    return atSeperScreenPos;
}

void MainWindow::paraOpenImageInfo(QString source, QString &path, QStringList &pathlist, QDateTime &stime)
{
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(source.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return;
    }
    QJsonObject rootObj = jsonDoc.object();

    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    if (rootObj.contains("OpenTime")) {
        stime = QDateTime::fromString(rootObj.value("OpenTime").toString(), "yyyy-MM-ddThh:mm:ss");
    }
    if (rootObj.contains("ImagePath")) {
        path = rootObj.value("ImagePath").toString();
    }
    if (rootObj.contains("ImagePathList")) {
        QJsonArray subArray = rootObj.value("ImagePathList").toArray();
        for (int i = 0; i < subArray.size(); i++) {
            QString subObj = subArray.at(i).toString();
            pathlist.append(subObj);
        }
    }
}

int MainWindow::showDialog()
{
    qDebug() << "!!!!!!!!!!!!!!!!!!showDialog!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    DDialog *dialog = new DDialog;

    QPixmap pixmap = utils::base::renderSVG(":/resources/common/warning.svg", QSize(32, 32));
    dialog->setIconPixmap(pixmap);

    //    dialog->setMessage(tr("The removable device has been plugged out, are you sure to delete
    //    the thumbnails of the removable device?"));
    dialog->setMessage(tr("Image file not found"));

    dialog->addButton(tr("Cancel"));
    dialog->addButton(tr("Delete"), true, DDialog::ButtonRecommend);
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
    effect->setOffset(0, 4);
    effect->setColor(QColor(0, 145, 255, 76));
    effect->setBlurRadius(4);
    dialog->getButton(1)->setGraphicsEffect(effect);

    int mode = dialog->exec();

    return mode;
}

void MainWindow::OpenImage(QString path)
{
    QString spath;
    QStringList pathlist;
    QDateTime stime;
    paraOpenImageInfo(path, spath, pathlist, stime);

    if (!m_flag) {
        SignalManager::ViewInfo info;
        info.album = "";
#ifndef LITE_DIV
        info.inDatabase = false;
#endif
        info.lastPanel = nullptr;
        info.path = spath;
        info.paths = pathlist;

        emit dApp->signalM->viewImage(info);
        m_flag = true;
    }

//    qint64 temptime = m_currenttime.secsTo(stime);
//    if (temptime < 0) return;
//    if (temptime < 2) {


//    }
}
