/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
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
#include "controller/dbusclient.h"
#include "mainwidget.h"
//#include <QDebug>
#include <dgiovolumemanager.h>
#include <DDialog>
#include <DFontSizeManager>
#include <DStackedWidget>
#include <QDir>
#include <QScreen>
#include <QShortcut>
#include <QTimer>
#include <QJsonParseError>
#include <QJsonArray>
#include <QShortcut>

#include "utils/baseutils.h"
#include "../service/dbusimageview_adaptor.h"
#include "utils/shortcut.h"
#include "accessibility/ac-desktop-define.h"

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
                         : dw.geometry().width() * 3 / 5;
    const int defaultH = dw.geometry().height() * 0.60 < MAINWIDGET_MINIMUN_HEIGHT
                         ? MAINWIDGET_MINIMUN_HEIGHT
                         : dw.geometry().height() * 3 / 5;
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
    m_pCenterWidget->setCurrentIndex(0);
#ifdef OPENACCESSIBLE
    //setObjectName(MAIN_WIDOW);
    //setAccessibleName(MAIN_WIDOW);
    m_pCenterWidget->setObjectName(CENTER_WIDGET);
    m_pCenterWidget->setAccessibleName(CENTER_WIDGET);
#endif
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
    QList<DTitlebar *>titlebar = findChildren <DTitlebar *>();
    for (DTitlebar *bar : titlebar) {
        if (bar) {
            bar->setFocusPolicy(Qt::ClickFocus);
        }
    }
    connect(dApp, &Application::TabkeyPress, this, [ = ](QObject * obj) {
        initAllViewTabKeyOrder(obj);
    }, Qt::DirectConnection);

    setCentralWidget(m_pCenterWidget);
    moveFirstWindow();

    /*lmh0806儒码优化*/
    QTimer::singleShot(dApp->m_timer, [ = ] {
        m_slidePanel =  new SlideShowPanel();
        m_pCenterWidget->addWidget(m_slidePanel);
        initConnection();
        initshortcut();
        initdbus();
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
        m_diskManager = new DDiskManager(this);
        m_diskManager->setWatchChanges(true);
        //m_vfsManager出现bug 当先插入u盘再打开程序时不能检测到u盘拔出时检测不到 正确做法disk和gio都要链接信号和槽，不能直接链接gio
        connect(m_diskManager, &DDiskManager::mountAdded, this, [ = ]()
        {
            qDebug() << "!!!!!!!!!!!!!!!!!!USB IN!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
            emit dApp->signalM->usbOutIn(true);
        });
        connect(m_diskManager, &DDiskManager::diskDeviceRemoved, this, [ = ]()
        {
            qDebug() << "!!!!!!!!!!!!!!!!!!USB OUT!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
            emit dApp->signalM->usbOutIn(false);
            if (m_picInUSB) {
                m_picInUSB = false;
            }
        });
        connect(dApp->signalM, &SignalManager::picInUSB, this, [ = ](bool immediately)
        {
            if (immediately) {
                m_picInUSB = true;
            }
        });
        new ImageViewAdaptor(this);
    });
}

void MainWindow::initshortcut()
{
    QShortcut *esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    esc->setContext(Qt::WindowShortcut);
    //解决在打开图片后,Ctrl+O快捷键无效，将快捷键实现放到此处，方便鱼其他地方调用
    QShortcut *ctrlq = new QShortcut(QKeySequence("Ctrl+O"), this);
    ctrlq->setContext(Qt::WindowShortcut);
    connect(ctrlq, &QShortcut::activated, this, [ = ] {
        //修复bug62208，当幻灯片存在的情况下，不打开文管
        SlideShowPanel *panel =  findChild<SlideShowPanel *>(SLIDE_SHOW_WIDGET);
        if (panel && !panel->isVisible())
        {
            emit dApp->signalM->sigOpenFileDialog();
            //修复bug63328,打开文件需要改变状态,需要重新控制状态
            if (window()->isFullScreen()) {
                titlebar()->setVisible(false);
                emit dApp->signalM->enterView(false);
                emit dApp->signalM->sigShowFullScreen();
            }
        }
    });
    connect(esc, &QShortcut::activated, this, [ = ] {
        if (IMAGEVIEW == m_pCenterWidget->currentIndex())
            emit sigExitFull();
        else
        {
            if (window()->isFullScreen()) {
                emit dApp->signalM->sigESCKeyActivated();
                emit dApp->signalM->sigESCKeyStopSlide();
            } else if (0 == m_pCenterWidget->currentIndex()) {
                this->close();
            }
            emit dApp->signalM->hideExtensionPanel();
        }
    });
}

void MainWindow::initdbus()
{
//    m_dbus = new Dbusclient();
}


void MainWindow::initConnection()
{
    if (!dApp->isPanelDev()) {
        QShortcut *scViewShortcut = new QShortcut(QKeySequence("Ctrl+Shift+/"), this);
        scViewShortcut->setObjectName(SC_VIEW_SHORTCUT);
        // connect(scE, SIGNAL(activated()), dApp, SLOT(quit()));
        connect(scViewShortcut, &QShortcut::activated, this, [ = ] {
            qDebug() << "receive Ctrl+Shift+/";
            QRect rect = window()->geometry();
            QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
            Shortcut sc;
            QStringList shortcutString;
            QString param1 = "-j=" + sc.toStr();
            QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
            shortcutString << "-b" << param1 << param2;
            qDebug() << shortcutString;
            QProcess::startDetached("deepin-shortcut-viewer", shortcutString);
        });
    }
    connect(m_slidePanel, SIGNAL(sigloadSlideshowpath(bool)), m_mainWidget, SIGNAL(mainwgtloadslideshowpath(bool)));
//    connect(m_mainWidget, SIGNAL(sigmaindgtslideshowpath(bool, DBImgInfoList)), m_slidePanel, SLOT(Receiveslideshowpathlst(bool, DBImgInfoList)));
    connect(this, SIGNAL(sigExitFull()), m_mainWidget, SIGNAL(sigExitFullScreen()));
    //幻灯片显示
    connect(dApp->signalM, &SignalManager::showSlidePanel, this, [ = ](int index) {
        Q_UNUSED(index);
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
        // delete m_slidePanel;
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
            // int historyId = processFile.readAll().toInt();
            // processFile.close();
            // QDir hisProcessDir(QString("/proc/%1").arg(historyId));

            // if (hisProcessDir.exists())
            //    return;
            //修复程序退出太慢，关闭程序后台进程退出完成前再次点击窗口未居中
            m_sharememory.setKey("deepin-image-viewer-siglewindow");
            //用于上一个进程异常退出共享内存没有释放，再此处释放
            if (m_sharememory.attach())
                m_sharememory.detach();
            if (!m_sharememory.create(4)) {
                //创建失败则关联程序
                if (!m_sharememory.isAttached()) //检测程序当前是否关联共享内存
                    m_sharememory.attach();
                return;
            }
            if (processFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
                QTextStream pidInfo(&processFile);
                pidInfo << dApp->m_app->applicationPid();
                processFile.close();
            }
            this->moveCenter();
        }
    } else {
        if (processFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream pidInfo(&processFile);
            pidInfo << dApp->m_app->applicationPid();
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

    for (QScreen *screen : dApp->m_app->screens()) {
        if (screen->geometry().contains(pos)) {
            primaryGeometry = screen->geometry();
        }
    }

    if (primaryGeometry.isEmpty()) {
        primaryGeometry = dApp->m_app->primaryScreen()->geometry();
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

void MainWindow::initAllViewTabKeyOrder(QObject *obj)
{
    Q_UNUSED(obj);
    DStackedWidget *stackwidget = findChild<DStackedWidget *>(VIEW_PANEL_STACK);
    if (stackwidget) {
        switch (stackwidget->currentIndex()) {
        case 1: {
            initEmptyTabOrder();
            break;
        }
        default: {
            initNormalPicTabOrder();
            break;
        }
        }
    }
}

void MainWindow::initEmptyTabOrder()
{
    titlebar()->show();
    m_emptyTabOrder.clear();
    QWidget *openBtn = findChild<QWidget *>(OPEN_IMAGE);
    QWidget *optionButton =  titlebar()->findChild<QWidget *>("DTitlebarDWindowOptionButton");
    QWidget *minButton =  titlebar()->findChild<QWidget *>("DTitlebarDWindowMinButton");
    QWidget *maxButton =  titlebar()->findChild<QWidget *>("DTitlebarDWindowMaxButton");
    QWidget *closeButton =  titlebar()->findChild<QWidget *>("DTitlebarDWindowCloseButton");
    if (openBtn && optionButton && minButton && maxButton && closeButton) {
        m_emptyTabOrder.insert(0, openBtn);
        m_emptyTabOrder.insert(1, optionButton);
        m_emptyTabOrder.insert(2, minButton);
        m_emptyTabOrder.insert(3, maxButton);
        m_emptyTabOrder.insert(4, closeButton);
    }
    for (int idx = 0; idx < m_emptyTabOrder.count() - 1; idx++) {
        this->setTabOrder(m_emptyTabOrder.at(idx), m_emptyTabOrder.at(idx + 1));
    }
}

void MainWindow::initNormalPicTabOrder()
{
    titlebar()->hide();
    QWidget *view = findChild<QWidget *>(IMAGE_VIEW);
    if (view) {
        view->setFocusPolicy(Qt::ClickFocus);
    }
    m_NormalPicTabOrder.clear();
    QWidget *preBtn = findChild<QWidget *>(PRE_BUTTON);
    QWidget *nextBtn = findChild<QWidget *>(NEXT_BUTTON);
    QWidget *adaptBtn = findChild<QWidget *>(ADAPT_BUTTON);
    QWidget *adaptscreenBtn = findChild<QWidget *>(ADAPT_SCREEN_BUTTON);
    QWidget *clockrightBtn = findChild<QWidget *>(COUNTER_CLOCKWISE_ROTATION);
    QWidget *clockleftBtn = findChild<QWidget *>(CLOCKWISE_ROTATION);
    QWidget *trashBtn = findChild<QWidget *>(TRASH_BUTTON);
    QWidget *optionButton =  titlebar()->findChild<QWidget *>("DTitlebarDWindowOptionButton");
    QWidget *minButton =  titlebar()->findChild<QWidget *>("DTitlebarDWindowMinButton");
    QWidget *maxButton =  titlebar()->findChild<QWidget *>("DTitlebarDWindowMaxButton");
    QWidget *closeButton =  titlebar()->findChild<QWidget *>("DTitlebarDWindowCloseButton");
    QList<TopToolbar *>titlebar1 = findChildren <TopToolbar *>(TOP_TOOL_BAR);

    for (TopToolbar *bar : titlebar1) {
        if (bar) {
            optionButton =  bar->findChild<QWidget *>("DTitlebarDWindowOptionButton");
            minButton =  bar->findChild<QWidget *>("DTitlebarDWindowMinButton");
            maxButton =  bar->findChild<QWidget *>("DTitlebarDWindowMaxButton");
            closeButton =  bar->findChild<QWidget *>("DTitlebarDWindowCloseButton");

        }
    }

    m_NormalPicTabOrder.insert(0, preBtn);
    m_NormalPicTabOrder.insert(1, nextBtn);
    m_NormalPicTabOrder.insert(2, adaptBtn);
    m_NormalPicTabOrder.insert(3, adaptscreenBtn);
    m_NormalPicTabOrder.insert(4, clockrightBtn);
    m_NormalPicTabOrder.insert(5, clockleftBtn);
    m_NormalPicTabOrder.insert(6, trashBtn);
    QWidget    *upBtn = findChild<QWidget *>(MOREPIC_UP_BUTTON);
    QWidget    *downBtn = findChild<QWidget *>(MOREPIC_DOWN_BUTTON);
    if (upBtn && downBtn) {
        m_NormalPicTabOrder.push_back(upBtn);
        m_NormalPicTabOrder.push_back(downBtn);
    }
    if (!window()->isFullScreen()) {
        m_NormalPicTabOrder.push_back(optionButton);
        m_NormalPicTabOrder.push_back(minButton);
        m_NormalPicTabOrder.push_back(maxButton);
        m_NormalPicTabOrder.push_back(closeButton);
    } else {
        optionButton->setFocusPolicy(Qt::ClickFocus);
        minButton->setFocusPolicy(Qt::ClickFocus);
        maxButton->setFocusPolicy(Qt::ClickFocus);
        closeButton->setFocusPolicy(Qt::ClickFocus);

        QList<QWidget *>fullBtnlist = findChildren<QWidget *>("DTitlebarDWindowQuitFullscreenButton");
        for (QWidget *fullBtn : fullBtnlist) {
            if (fullBtn) {
                fullBtn->setFocusPolicy(Qt::ClickFocus);
            }
        }

    }

    for (int idx = 0; idx < m_NormalPicTabOrder.count() - 1; idx++) {
        this->setTabOrder(m_NormalPicTabOrder.at(idx), m_NormalPicTabOrder.at(idx + 1));
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    if (m_sharememory.isAttached()) //检测程序当前是否关联共享内存
        m_sharememory.detach();
    emit dApp->signalM->hideExtensionPanel();
    emit dApp->endApplication();
}

bool MainWindow::windowAtEdge()
{
    // TODO: process the multi-screen
    QRect currentRect = window()->geometry();
    bool atSeperScreenPos = false;

    if (currentRect.x() == 0 ||
            qAbs(currentRect.right() - dApp->m_app->primaryScreen()->geometry().width()) <= 5) {
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
/*lmh0810 never used*/
//int MainWindow::showDialog()
//{
//    qDebug() << "!!!!!!!!!!!!!!!!!!showDialog!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
//    DDialog *dialog = new DDialog;

//    QPixmap pixmap = utils::base::renderSVG(":/assets/common/warning.svg", QSize(32, 32));
//    QIcon icon(pixmap);
//    dialog->setIcon(icon);

//    //    dialog->setMessage(tr("The removable device has been plugged out, are you sure to delete
//    //    the thumbnails of the removable device?"));
//    dialog->setMessage(tr("Image file not found"));

//    dialog->addButton(tr("Cancel"));
//    dialog->addButton(tr("Delete"), true, DDialog::ButtonRecommend);
//    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
//    effect->setOffset(0, 4);
//    effect->setColor(QColor(0, 145, 255, 76));
//    effect->setBlurRadius(4);
//    dialog->getButton(1)->setGraphicsEffect(effect);

//    int mode = dialog->exec();

//    return mode;
//}

void MainWindow::OpenImage(const QString &path)
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

        //20210111lmh平板可以多次改变，正常模式不需要
        if (dApp->isPanelDev()) {
            m_flag = false;
        } else {
            m_flag = true;
        }
    }
    //新的图片打开需要激活窗口20210113
    activateWindow();

//    qint64 temptime = m_currenttime.secsTo(stime);
//    if (temptime < 0) return;
//    if (temptime < 2) {


//    }
}
