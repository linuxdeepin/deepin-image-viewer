// SPDX-FileCopyrightText: 2020 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef CMAKE_BUILD
#include "config.h"
#endif
//#define protected public
//#include <DApplication>
//#undef protected

#include "mainwindow/mainwindow.h"
#include "application.h"
#include "eventlogutils.h"

#include <libimageviewer/imageengine.h>

#include <DWidgetUtil>
#include <DMainWindow>
#include <DLog>
#include <DApplicationSettings>
#include <DVtableHook>

#include <QTranslator>
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QtDBus/QDBusConnection>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// 最小宽高
#define MAINWIDGET_MINIMUN_HEIGHT 300
#define MAINWIDGET_MINIMUN_WIDTH 628

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
//解析
static QString getImagenameFromPath(const QString &path_name)
{
    QStringList list = path_name.split(".");
    if (list.isEmpty())
        return " ";
    else
        return list.last();
}
bool checkOnly()
{
    // single
    QString userName = QDir::homePath().section("/", -1, -1);
    std::string path = ("/home/" + userName + "/.cache/deepin/deepin-image-viewer/").toStdString();
    QDir tdir(path.c_str());
    if (!tdir.exists()) {
        bool ret = tdir.mkpath(path.c_str());
        qDebug() << ret;
    }

    path += "single";
    int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    int flock = lockf(fd, F_TLOCK, 0);

    if (fd == -1) {
        perror("open lockfile/n");
        return false;
    }
    if (flock == -1) {
        perror("lock file error/n");
        return false;
    }
    return true;
}

//判断是否是wayland
bool CheckWayland()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive))
        return true;
    else {
        return false;
    }
}
//转换路径
QUrl UrlInfo(QString path)
{
    QUrl url;
    // Just check if the path is an existing file.
    if (QFile::exists(path)) {
        url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(path));
        return url;
    }

    const auto match = QRegularExpression(QStringLiteral(":(\\d+)(?::(\\d+))?:?$")).match(path);

    if (match.isValid()) {
        // cut away line/column specification from the path.
        path.chop(match.capturedLength());
    }

    // make relative paths absolute using the current working directory
    // prefer local file, if in doubt!
    url = QUrl::fromUserInput(path, QDir::currentPath(), QUrl::AssumeLocalFile);

    // in some cases, this will fail, e.g.
    // assume a local file and just convert it to an url.
    if (!url.isValid()) {
        // create absolute file path, we will e.g. pass this over dbus to other processes
        url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(path));
    }
    return url;
}

/**
   @brief 绑定 imageeidtor 中的授权信息抛出接口，使用 QMetaObject 信息以动态获取接口，以确保前后均兼容
 */
void connectAuthNotify()
{
    QByteArray normalSignal = QMetaObject::normalizedSignature("sigAuthoriseNotify(const QJsonObject &)");
    int index = ImageEngine::instance()->metaObject()->indexOfSignal(normalSignal.constData());
    if (-1 != index) {
        bool ret = QObject::connect(ImageEngine::instance(),
                                    SIGNAL(sigAuthoriseNotify(QJsonObject)),
                                    Eventlogutils::GetInstance(),
                                    SLOT(forwardLogData(QJsonObject)));
        if (!ret) {
            qWarning() << qPrintable("sigAuthoriseNotify() exists, but connect failed!");
        }
    } else {
        qInfo() << "Not detect sigAuthoriseNotify()";
    }
}

/**
   @brief 埋点记录启动数据，不同埋点版本接口不同，兼容处理
 */
void reportStartupEventLog(const QString &imageformat, bool bRet)
{
    QJsonObject objStartEvent{
        {"tid", Eventlogutils::StartUp},
        {"vsersion", VERSION},
        {"imageformat", imageformat},
        {"opensuccess", bRet},
        {"mode", 1},
    };

    if (Eventlogutils::GetInstance()->sendLogsEnabled()) {
        // 接口更新(1.1->1.2)调整为sendLog，2为上报日志
        QJsonObject policyObj;
        policyObj.insert("reportMode", 2);
        QJsonObject sendData;
        sendData.insert("policy", policyObj);
        sendData.insert("info", objStartEvent);
        Eventlogutils::GetInstance()->sendLogs(sendData);
    } else {
        Eventlogutils::GetInstance()->writeLogs(objStartEvent);
    }
}

int main(int argc, char *argv[])
{
    // for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    //判断是否是wayland
    if (CheckWayland()) {
        //默认走xdgv6,该库没有维护了，因此需要添加该代码
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    }

    Application a(argc, argv);
    a.setAttribute(Qt::AA_ForceRasterWidgets);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setOrganizationName("deepin");
    a.setApplicationName("deepin-image-viewer");
    a.loadTranslator();
    a.setApplicationDisplayName(QObject::tr("Image Viewer"));
    a.setProductIcon(QIcon::fromTheme("deepin-image-viewer"));
    a.setApplicationDescription(
        QObject::tr("Image Viewer is an image viewing tool with fashion interface and smooth performance."));
    a.loadTranslator();
    // save theme
    DApplicationSettings saveTheme;
    Q_UNUSED(saveTheme);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    a.setApplicationVersion("1.0.0");
#ifdef CMAKE_BUILD
    //增加版本号
    a.setApplicationVersion(VERSION);
#endif

    qInfo()
        << QString("%1 start, PID: %2, Version: %3").arg(a.applicationName()).arg(a.applicationPid()).arg(a.applicationVersion());
    qDebug() << "LogFile:" << DLogManager::getlogFilePath();

    // 在主窗口显示前调用识别是否启用打印窗口
#ifdef IMAGEVIEWER_CLASS_QUICKPRINT
    a.parseCommandLine();
    if (a.isCommandSet(Application::Print)) {
        // 打印后直接退出
        return a.triggerPrintAction();
    }
#endif

    // 构造前关联通知信号，部分信息在构造时通知
    connectAuthNotify();

    // 主窗体应该new出来,不应该是static变量 修改为从单例获取
    DMainWindow *mainwindow = new DMainWindow();
    MainWindow *w = new MainWindow(mainwindow);
    mainwindow->setCentralWidget(w);
    w->setDMainWindow(mainwindow);

    //初始化大小为上次关闭大小
    if (mainwindow) {
        int defaultW = 0;
        int defaultH = 0;
        QDesktopWidget *dw = QApplication::desktop();
        // 多屏下仅采用单个屏幕处理， 使用主屏的参考宽度计算
        QScreen *screen = QGuiApplication::primaryScreen();
        bool useScreen = bool(QGuiApplication::screens().size() > 1 && screen);

        if (double(dw->geometry().width()) * 0.60 < MAINWIDGET_MINIMUN_WIDTH) {
            defaultW = MAINWIDGET_MINIMUN_WIDTH;
        } else {
            if (useScreen) {
                defaultW = int(double(screen->size().width()) * 0.60);
            } else {
                defaultW = int(double(dw->geometry().width()) * 0.60);
            }
        }

        if (double(dw->geometry().height()) * 0.60 < MAINWIDGET_MINIMUN_HEIGHT) {
            defaultH = MAINWIDGET_MINIMUN_HEIGHT;
        } else {
            if (useScreen) {
                defaultH = int(double(screen->size().height()) * 0.60);
            } else {
                defaultH = int(double(dw->geometry().height()) * 0.60);
            }
        }

        int ww = w->value(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, QVariant(defaultW)).toInt();
        int wh = w->value(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, QVariant(defaultH)).toInt();
        mainwindow->resize(ww, wh);
        mainwindow->setMinimumSize(MAINWIDGET_MINIMUN_WIDTH, MAINWIDGET_MINIMUN_HEIGHT);
    }
    QString filepath = "";
    QString imageformat = "";
    bool bRet = false;
    QStringList arguments = QCoreApplication::arguments();
    for (QString path : arguments) {
        path = UrlInfo(path).toLocalFile();
        if (QFileInfo(path).isFile()) {
            bRet = w->slotDrogImg(QStringList(path));
            if (bRet) {
                imageformat = getImagenameFromPath(path);
                break;
            }
        }
    }

    // 埋点记录启动数据
    reportStartupEventLog(imageformat, bRet);

    mainwindow->show();
    if (bRet) {
        // 正常打开窗口时，隐藏默认的标题栏。
        mainwindow->titlebar()->setVisible(false);
    }

    //修复窗口会一直在中间变小的问题
    if (checkOnly()) {
        Dtk::Widget::moveToCenter(mainwindow);
        QPoint pt = mainwindow->geometry().topLeft();
        if(pt.x() < 0 || pt.y() < 0) {
            mainwindow->move(0, 0);
        }
    }

    QObject::connect(dApp, &Application::sigQuit, w, &MainWindow::quitApp, Qt::DirectConnection);

    // 临时修改，注册DBus服务以正常启动进程，后续添加相关的DBus接口
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("com.deepin.ImageViewer");

    return a.exec();
}
