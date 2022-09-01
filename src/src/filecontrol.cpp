#include "filecontrol.h"

#include <QFileInfo>
#include <QDir>
#include <QMimeDatabase>
#include <QCollator>
#include <QUrl>
#include <QDebug>
#include <QDBusInterface>
#include <QThread>
#include <QProcess>
#include <QGuiApplication>
#include <QScreen>
#include <QDesktopServices>
#include <QClipboard>
#include <QDesktopWidget>
#include <QApplication>

#include <iostream>

#include "unionimage/unionimage_global.h"
#include "unionimage/unionimage.h"

#include "ocr/ocrinterface.h"

#include "printdialog/printhelper.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const QString SETTINGS_GROUP = "MAINWINDOW";
const QString SETTINGS_WINSIZE_W_KEY = "WindowWidth";
const QString SETTINGS_WINSIZE_H_KEY = "WindowHeight";
// 是否显示导航窗口
const QString SETTINGS_ENABLE_NAVIGATION = "EnableNavigation";
const int MAINWIDGET_MINIMUN_HEIGHT = 300;
const int MAINWIDGET_MINIMUN_WIDTH = 628;

bool compareByFileInfo(const QFileInfo &str1, const QFileInfo &str2)
{
    static QCollator sortCollator;
    sortCollator.setNumericMode(true);
    return sortCollator.compare(str1.baseName(), str2.baseName()) < 0;
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

FileControl::FileControl(QObject *parent) : QObject(parent)
{
    if (!m_ocrInterface) {
        m_ocrInterface = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus(), this);
    }

    m_config = LibConfigSetter::instance();

    // 实时保存旋转后图片太卡，因此采用10ms后延时保存的问题
    if (!m_tSaveImage) {
        m_tSaveImage = new QTimer(this);
        connect(m_tSaveImage, &QTimer::timeout, this, [ = ]() {
            //保存旋转的图片
            slotRotatePixCurrent();
            emit callSavePicDone();
        });
    }

    // 在1000ms以内只保存一次配置信息
    if (!m_tSaveSetting) {
        m_tSaveSetting = new QTimer(this);
        connect(m_tSaveSetting, &QTimer::timeout, this, [ = ]() {
            saveSetting();
        });
    }

    listsupportWallPaper << "bmp" << "cod" << "png" << "gif" << "ief" << "jpe" << "jpeg" << "jpg"
                         << "jfif" << "tif" << "tiff";
}

FileControl::~FileControl()
{
    saveSetting();
}

QString FileControl::getDirPath(const QString &path)
{
    QFileInfo firstFileInfo(path);

    return firstFileInfo.dir().path();
}

QStringList FileControl::getDirImagePath(const QString &path)
{
    if (path.isEmpty()) {
        return QStringList();
    }

    QStringList image_list;
    QString DirPath = QFileInfo(QUrl(path).toLocalFile()).dir().path();

    QDir _dirinit(DirPath);
    QFileInfoList m_AllPath = _dirinit.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);

    //修复Ｑt带后缀排序错误的问题
    std::sort(m_AllPath.begin(), m_AllPath.end(), compareByFileInfo);
    for (int i = 0; i < m_AllPath.size(); i++) {
        QString tmpPath = m_AllPath.at(i).filePath();
        if (tmpPath.isEmpty()) {
            continue;
        }
        //判断是否图片格式
        if (isImage(tmpPath)) {
            tmpPath = "file://" + tmpPath;
            image_list << tmpPath;
        }
    }
    return image_list;
}

QStringList FileControl::removeList(const QStringList &pathlist, int index)
{
    QStringList list = pathlist;
    list.removeAt(index);
    return list;
}

QStringList FileControl::renameOne(const QStringList &pathlist, const  QString &oldPath, const QString &newPath)
{

    QStringList list = pathlist;
    int index = pathlist.indexOf(oldPath);
    if (index >= 0 && index < pathlist.count()) {
        list[index] = newPath;
    }
    return list;
}

QString FileControl::getNamePath(const  QString &oldPath, const QString &newName)
{
    QFileInfo info(oldPath);
    QString path = info.path();
    QString suffix = info.suffix();
    QString newPath =  path + "/" + newName + "." + suffix;
    return newPath;
}

bool FileControl::isImage(const QString &path)
{
    bool bRet = false;
    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForFile(path, QMimeDatabase::MatchContent);
    QMimeType mt1 = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension);
    if (mt.name().startsWith("image/") || mt.name().startsWith("video/x-mng") ||
            mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
        bRet = true;
    }
    return bRet;
}

void FileControl::setWallpaper(const QString &imgPath)
{
    slotRotatePixCurrent();
    QThread *th1 = QThread::create([ = ]() {
        if (!imgPath.isNull()) {
            QString path = imgPath;
            //202011/12 bug54279
            {
                //设置壁纸代码改变，采用DBus,原方法保留
                if (/*!qEnvironmentVariableIsEmpty("FLATPAK_APPID")*/1) {
                    // gdbus call -e -d com.deepin.daemon.Appearance -o /com/deepin/daemon/Appearance -m com.deepin.daemon.Appearance.Set background /home/test/test.png
                    qDebug() << "SettingWallpaper: " << "flatpak" << path;
                    QDBusInterface interface("com.deepin.daemon.Appearance",
                                                 "/com/deepin/daemon/Appearance",
                                                 "com.deepin.daemon.Appearance");

                    if (interface.isValid()) {
                        QString screenname;

                        //判断环境是否是wayland
                        auto e = QProcessEnvironment::systemEnvironment();
                        QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
                        QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

                        bool isWayland = false;
                        if (XDG_SESSION_TYPE != QLatin1String("wayland") && !WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
                            isWayland = false;
                        } else {
                            isWayland = true;
                        }
                        //wayland下设置壁纸使用，2020/09/21
                        if (isWayland) {
                            QDBusInterface interfaceWayland("com.deepin.daemon.Display", "/com/deepin/daemon/Display", "com.deepin.daemon.Display");
                            screenname = qvariant_cast< QString >(interfaceWayland.property("Primary"));
                        } else {
                            screenname = QGuiApplication::primaryScreen()->name();
                        }
                        QDBusMessage reply = interface.call(QStringLiteral("SetMonitorBackground"), screenname, path);
                        qDebug() << "SettingWallpaper: replay" << reply.errorMessage();
                    } else {
                        qWarning() << "SettingWallpaper failed" << interface.lastError();
                    }
                }
            }
        }
    });
    connect(th1, &QThread::finished, th1, &QObject::deleteLater);
    th1->start();
}

bool FileControl::deleteImagePath(const QString &path)
{
    QUrl displayUrl = QUrl(path);
    QDBusInterface interface(QStringLiteral("org.freedesktop.FileManager1"),
                                 QStringLiteral("/org/freedesktop/FileManager1"),
                                 QStringLiteral("org.freedesktop.FileManager1"));
    if (displayUrl.isValid()) {
        QStringList list;
        list << displayUrl.toString();
        interface.call("Trash", list).type() != QDBusMessage::ErrorMessage;
    }
    return true;
}

bool FileControl::displayinFileManager(const QString &path)
{
    bool bRet = false;
    QUrl displayUrl = QUrl(path);

    QDBusInterface interface(QStringLiteral("org.freedesktop.FileManager1"),
                                 QStringLiteral("/org/freedesktop/FileManager1"),
                                 QStringLiteral("org.freedesktop.FileManager1"));

    if (interface.isValid()) {
        QStringList list;
        list << displayUrl.toString();
        interface.call("ShowItems", list, "").type() != QDBusMessage::ErrorMessage;
        bRet = true;
    }
    return bRet;
}

void FileControl::copyImage(const QString &path)
{
    slotRotatePixCurrent();
    QString localPath = QUrl(path).toLocalFile();

    QClipboard *cb = qApp->clipboard();

    // Ownership of the new data is transferred to the clipboard.
    QMimeData *newMimeData = new QMimeData();

    // Copy old mimedata
//    const QMimeData* oldMimeData = cb->mimeData();
//    for ( const QString &f : oldMimeData->formats())
//        newMimeData->setData(f, oldMimeData->data(f));

    // Copy file (gnome)
    QByteArray gnomeFormat = QByteArray("copy\n");
    QString text;
    QList<QUrl> dataUrls;

    if (!localPath.isEmpty())
        text += localPath + '\n';
    dataUrls << QUrl::fromLocalFile(localPath);
    gnomeFormat.append(QUrl::fromLocalFile(localPath).toEncoded()).append("\n");


    newMimeData->setText(text.endsWith('\n') ? text.left(text.length() - 1) : text);
    newMimeData->setUrls(dataUrls);
    gnomeFormat.remove(gnomeFormat.length() - 1, 1);
    newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);

    // Copy Image Date
//    QImage img(paths.first());
//    Q_ASSERT(!img.isNull());
//    newMimeData->setImageData(img);

    // Set the mimedata
//    cb->setMimeData(newMimeData);
    cb->setMimeData(newMimeData, QClipboard::Clipboard);
}

bool FileControl::isRotatable(const QString &path)
{
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();
    QFileInfo info(localPath);
    if (!info.isFile() || !info.exists() || !info.isWritable()) {
        bRet = false;
    } else {
        bRet = LibUnionImage_NameSpace::isImageSupportRotate(localPath);
    }
    return bRet;
}

bool FileControl::isCanWrite(const QString &path)
{
    QString localPath = QUrl(path).toLocalFile();
    QFileInfo info(localPath);
    bool bRet = info.isWritable() && QFileInfo(info.dir(), info.dir().path()).isWritable(); //是否可写
    return bRet;
}

bool FileControl::isCanDelete(const QString &path)
{
    bool bRet = false;
    bool isAlbum = false;
    QString localPath = QUrl(path).toLocalFile();
    QFileInfo info(localPath);
    bool isWritable = info.isWritable() && QFileInfo(info.dir(), info.dir().path()).isWritable(); //是否可写
    bool isReadable = info.isReadable() ; //是否可读
    imageViewerSpace::PathType pathType = LibUnionImage_NameSpace::getPathType(localPath);
    if ((imageViewerSpace::PathTypeAPPLE != pathType &&
            imageViewerSpace::PathTypeSAFEBOX != pathType &&
            imageViewerSpace::PathTypeRECYCLEBIN != pathType &&
            imageViewerSpace::PathTypeMTP != pathType &&
            imageViewerSpace::PathTypePTP != pathType &&
            isWritable && isReadable) || (isAlbum && isWritable)) {
        bRet = true;
    } else {
        bRet = false;
    }
    return bRet;
}


bool FileControl::isFile(const QString &path)
{
    QString localPath = QUrl(path).toLocalFile();
    return QFileInfo(localPath).isFile();
}

void FileControl::ocrImage(const QString &path)
{
    slotRotatePixCurrent();
    QString localPath = QUrl(path).toLocalFile();
    m_ocrInterface->openFile(localPath);
}

QString FileControl::parseCommandlineGetPath(const QString &path)
{
    Q_UNUSED(path)
    QString filepath = "";
    QStringList arguments = QCoreApplication::arguments();
    for (QString path : arguments) {
        path = UrlInfo(path).toLocalFile();
        if (QFileInfo(path).isFile()) {
            bool bRet = isImage(path);
            if (bRet) {
                filepath += "file://";
                filepath += path;
                return filepath;
            }
        }
    }

    return filepath;
}

bool FileControl::isDynamicImage(const QString &path)
{
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();

    imageViewerSpace::ImageType type = LibUnionImage_NameSpace::getImageType(localPath);
    if (imageViewerSpace::ImageTypeDynamic == type) {
        bRet = true;
    }
    return bRet;
}

bool FileControl::isNormalStaticImage(const QString &path)
{
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();

    imageViewerSpace::ImageType type = LibUnionImage_NameSpace::getImageType(localPath);
    if (imageViewerSpace::ImageTypeStatic == type || imageViewerSpace::ImageTypeMulti == type) {
        bRet = true;
    }
    return bRet;
}

bool FileControl::rotateFile(const QString &path, const int &rotateAngel)
{
    bool bRet = true;
    QString localPath = QUrl(path).toLocalFile();
    if (m_currentPath != localPath) {
        slotRotatePixCurrent();
        m_currentPath = localPath;
        m_rotateAngel = rotateAngel;
    } else {
        m_rotateAngel += rotateAngel;
    }

    m_rotateAngelOnShow += rotateAngel;

    // 减少频繁的触发旋转进行文件读取写入操作
    m_tSaveImage->setSingleShot(true);
    m_tSaveImage->start(200);

    return bRet;
}

void FileControl::slotRotatePixCurrent()
{
    // 由QML调用(切换图片)时，停止定时器，防止二次触发
    if (m_tSaveImage->isActive()) {
        m_tSaveImage->stop();
    }

    m_rotateAngel = m_rotateAngel % 360;
    if (0 != m_rotateAngel) {
        //20211019修改：特殊位置不执行写入操作
        imageViewerSpace::PathType pathType = LibUnionImage_NameSpace::getPathType(m_currentPath);

        if (pathType != imageViewerSpace::PathTypeMTP && pathType != imageViewerSpace::PathTypePTP && //安卓手机
                pathType != imageViewerSpace::PathTypeAPPLE && //苹果手机
                pathType != imageViewerSpace::PathTypeSAFEBOX && //保险箱
                pathType != imageViewerSpace::PathTypeRECYCLEBIN) { //回收站

            QString erroMsg;
            LibUnionImage_NameSpace::rotateImageFIle(m_rotateAngel, m_currentPath, erroMsg);
        }
    }
    m_rotateAngel = 0;
}

/**
 * @brief 缓存当前处理的图片的角度，若当前内部保存的旋转角度不为0，
 *      表示旋转图片信息暂未保存到文件，为保证图片角度和显示角度一致，
 *      保存当前的图片旋转信息，用于后续显示界面时展示。
 */
void FileControl::cacheCurrentImageAngle()
{
    if (m_tSaveImage->isActive()
            && m_rotateAngel % 360 != 0) {
        m_cacheImageAngle.insert(m_currentPath, m_rotateAngel % 360);
    }
    m_rotateAngelOnShow = 0;
}

/**
 * @brief 获取图片 \a path 缓存的图片角度信息，若无缓存信息，返回 \b 0 ，图片角度返回后被移除
 * @param path 图片路径
 * @return 缓存的角度信息
 *
 * @note 需要注意重命名、删除等文件信息变更。
 */
int FileControl::takeCachedImageAngle(const QString &path)
{
    QString localPath = QUrl(path).toLocalFile();
    if (m_cacheImageAngle.contains(localPath)) {

        emit callSavePicDone();
        return m_cacheImageAngle.take(localPath);
    }
    return 0;
}

bool FileControl::isReverseHeightWidth()
{
    // 判断当前旋转角度是否存在垂直方向的旋转
    return bool(m_rotateAngelOnShow % 180);
}

int FileControl::currentAngle()
{
    return m_rotateAngel;
}

QString FileControl::slotGetFileName(const QString &path)
{
    QString tmppath = path;
    QFileInfo info(tmppath);
    return info.completeBaseName();
}

QString FileControl::slotGetFileNameSuffix(const QString &path)
{
    QString tmppath = path;
    QFileInfo info(tmppath);
    return info.fileName();
}

QString FileControl::slotGetInfo(const QString &key, const QString &path)
{
    Q_UNUSED(path)
    QString returnString = m_currentAllInfo.value(key);
    if (returnString.isEmpty()) {
        returnString = "-";
    }

    return returnString;
}


bool FileControl::slotFileReName(const QString &name, const QString &filepath, bool isSuffix)
{
    QString localPath = QUrl(filepath).toLocalFile();
    QFile file(localPath);
    if (file.exists()) {
        QFileInfo info(localPath);
        QString path = info.path();
        QString suffix = info.suffix();
        QString _newName ;
        if (isSuffix) {
            _newName  = path + "/" + name;
        } else {
            _newName  = path + "/" + name + "." + suffix;
        }

        if (file.rename(_newName))
            return true;
        return false;
    }
    return false;
}

QString FileControl::slotFileSuffix(const QString &path, bool ret)
{
    QString returnSuffix = "";

    QString localPath = QUrl(path).toLocalFile();
    if (!path.isEmpty() && QFile::exists(localPath)) {
        QString tmppath = path;
        QFileInfo info(tmppath);
        if (ret) {
            returnSuffix = "." + info.completeSuffix();
        } else {
            returnSuffix = info.completeSuffix();
        }
    }

    return returnSuffix;
}

void FileControl::setCurrentImage(const QString &path)
{
    QString localPath = QUrl(path).toLocalFile();

    if (m_currentReader) {
        delete m_currentReader;
        m_currentReader = nullptr;
    }
    m_currentReader = new QImageReader(localPath);

    m_currentAllInfo = LibUnionImage_NameSpace::getAllMetaData(localPath);
}

/**
 * @brief 设置当前图片的帧号 \a index , 用于多页图切换不同图片。
 * @param index 图片帧号
 */
void FileControl::setCurrentFrameIndex(int index)
{
    if (m_currentReader) {
        m_currentReader->jumpToImage(index);
    }
}

int FileControl::getCurrentImageWidth()
{
    if (isReverseHeightWidth()) {
        int height = -1;
        if (m_currentReader) {
            height = m_currentReader->size().height();
            if (height <= 0)
                height = m_currentAllInfo.value("Height").toInt();
        }
        return height;
    }

    int width = -1;
    if (m_currentReader) {
        width = m_currentReader->size().width();
        if (width <= 0)
            width = m_currentAllInfo.value("Width").toInt();
    }

    return width;
}

int FileControl::getCurrentImageHeight()
{
    if (isReverseHeightWidth()) {
        int width = -1;
        if (m_currentReader) {
            width = m_currentReader->size().width();
            if (width <= 0)
                width = m_currentAllInfo.value("Width").toInt();
        }

        return width;
    }

    int height = -1;
    if (m_currentReader) {
        height = m_currentReader->size().height();
        if (height <= 0)
            height = m_currentAllInfo.value("Height").toInt();
    }
    return height;
}

double FileControl::getFitWindowScale(double WindowWidth, double WindowHeight)
{
    double scale = 0.0;
    double width = getCurrentImageWidth();
    double height = getCurrentImageHeight();
    double scaleWidth = width / WindowWidth;
    double scaleHeight = height / WindowHeight;

    if (scaleWidth > scaleHeight) {
        scale = scaleWidth;
    } else {
        scale = scaleHeight;
    }

    return scale;
}

bool FileControl::isShowToolTip(const QString &oldPath, const QString &name)
{
    bool bRet = false;
    QString path = QUrl(oldPath).toLocalFile();
    QFileInfo fileinfo(path);
    QString DirPath = fileinfo.path();
    QString filename = fileinfo.completeBaseName();
    if (filename == name)
        return false;

    QString format = fileinfo.suffix();

    QString fileabname = DirPath + "/" + name + "." + format;
    QFile file(fileabname);
    if (file.exists() && fileabname != path) {
        bRet = true;
    } else {
        bRet = false;
    }
    return bRet;
}

void FileControl::showPrintDialog(const QString &path)
{
    QString oldPath = QUrl(path).toLocalFile();
    PrintHelper::getIntance()->showPrintDialog(QStringList(oldPath));
}

QVariant FileControl::getConfigValue(const QString &group, const QString &key, const QVariant &defaultValue)
{
    return m_config->value(group, key, defaultValue);
}

void FileControl::setConfigValue(const QString &group, const QString &key, const QVariant &value)
{
    m_config->setValue(group, key, value);
}

int FileControl::getlastWidth()
{
    int reWidth = 0;
    int defaultW = 0;

    QDesktopWidget *dw = QApplication::desktop();
    if (double(dw->geometry().width()) * 0.60 < MAINWIDGET_MINIMUN_WIDTH) {
        defaultW = MAINWIDGET_MINIMUN_WIDTH;
    } else {
        defaultW = int(double(dw->geometry().width()) * 0.60);
    }

    const int ww = getConfigValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, QVariant(defaultW)).toInt();

    reWidth = ww >= MAINWIDGET_MINIMUN_WIDTH ? ww : MAINWIDGET_MINIMUN_WIDTH;
    m_windowWidth = reWidth;
    return reWidth;
}

int FileControl::getlastHeight()
{
    int reHeight = 0;
    int defaultH = 0;

    QDesktopWidget *dw = QApplication::desktop();
    if (double(dw->geometry().height()) * 0.60 < MAINWIDGET_MINIMUN_HEIGHT) {
        defaultH = MAINWIDGET_MINIMUN_HEIGHT;
    } else {
        defaultH = int(double(dw->geometry().height()) * 0.60);
    }
    const int wh = getConfigValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, QVariant(defaultH)).toInt();

    reHeight = wh >= MAINWIDGET_MINIMUN_HEIGHT ? wh : MAINWIDGET_MINIMUN_HEIGHT;
    m_windowHeight = reHeight;
    return reHeight;
}

void FileControl::setSettingWidth(int width)
{
    m_windowWidth = width;
//    setConfigValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, m_windowWidth);
    m_tSaveSetting->setSingleShot(true);
    m_tSaveSetting->start(1000);
}

void FileControl::setSettingHeight(int height)
{
    m_windowHeight = height;
    m_tSaveSetting->setSingleShot(true);
    m_tSaveSetting->start(1000);
//    setConfigValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, m_windowHeight);

}

void FileControl::setEnableNavigation(bool b)
{
    setConfigValue(SETTINGS_GROUP, SETTINGS_ENABLE_NAVIGATION, b);
}

bool FileControl::isEnableNavigation()
{
    return getConfigValue(SETTINGS_GROUP, SETTINGS_ENABLE_NAVIGATION, true).toBool();
}

void FileControl::saveSetting()
{
    if (m_lastSaveWidth != m_windowWidth) {
        setConfigValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, m_windowWidth);
        m_lastSaveWidth = m_windowWidth;
    }
    if (m_lastSaveHeight != m_windowHeight) {
        setConfigValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, m_windowHeight);
        m_lastSaveHeight = m_windowHeight;
    }
}

bool FileControl::isSupportSetWallpaper(const QString &path)
{
    QString path1 = QUrl(path).toLocalFile();
    QFileInfo fileinfo(path1);
    QString format = fileinfo.suffix().toLower();
    // 设置为壁纸需要判断是否有读取权限
    if (listsupportWallPaper.contains(format)
            && fileinfo.isReadable()) {
        return true;
    }
    return false;
}

bool FileControl::isCheckOnly()
{
    //single
    QString userName = QDir::homePath().section("/", -1, -1);
    std::string path = ("/home/" + userName + "/.cache/deepin/deepin-image-viewer/").toStdString();
    QDir tdir(path.c_str());
    if (!tdir.exists()) {
        bool ret =  tdir.mkpath(path.c_str());
        qDebug() << ret ;
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

bool FileControl::isCanSupportOcr(const QString &path)
{
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();
    QFileInfo info(localPath);
    imageViewerSpace::ImageType type = LibUnionImage_NameSpace::getImageType(localPath);
    if (imageViewerSpace::ImageTypeDynamic != type && info.isReadable()) {
        bRet = true;
    }
    return bRet;
}

bool FileControl::isCanRename(const QString &path)
{
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();
    imageViewerSpace::PathType pathType = LibUnionImage_NameSpace::getPathType(localPath);//路径类型
    QFileInfo info(localPath);
    bool isWritable = info.isWritable() && QFileInfo(info.dir(), info.dir().path()).isWritable(); //是否可写
    if (info.isReadable() && isWritable &&
            imageViewerSpace::PathTypeMTP != pathType &&
            imageViewerSpace::PathTypePTP != pathType &&
            imageViewerSpace::PathTypeAPPLE != pathType) {
        bRet = true;
    }
    return bRet;
}

bool FileControl::isCanReadable(const QString &path)
{
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();
    QFileInfo info(localPath);
    if (info.isReadable()) {
        bRet = true;
    }
    return bRet;
}

bool FileControl::isSvgImage(const QString &path)
{
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();
    imageViewerSpace::ImageType imgType = LibUnionImage_NameSpace::getImageType(localPath);
    if (imgType == imageViewerSpace::ImageTypeSvg) {
        bRet = true;
    }
    return bRet;
}


/**
 * @return 根据传入的文件路径 \a path 读取判断文件是否为多页图，
 *      会在排除 *.gif 等动态图后判断, 例如 .tif 等文件格式。
 */
bool FileControl::isMultiImage(const QString &path)
{
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();
    imageViewerSpace::ImageType type = LibUnionImage_NameSpace::getImageType(localPath);
    if (imageViewerSpace::ImageTypeMulti == type) {
        bRet = true;
    }

    return bRet;
}

/**
 * @return 返回当前图片 \a path 的总页数，对动态图片或*.tif 等多页图有效。
 */
int FileControl::getImageCount(const QString &path)
{
    QString localPath = QUrl(path).toLocalFile();
    if (!localPath.isEmpty()) {
        QImageReader imgreader(localPath);
        return imgreader.imageCount();
    }
    return 0;
}
