// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filecontrol.h"
#include "types.h"
#include "unionimage/unionimage_global.h"
#include "unionimage/unionimage.h"
#include "printdialog/printhelper.h"
#include "ocr/ocrinterface.h"
#include "imagedata/imageinfo.h"

#include <DSysInfo>

#include <QFileInfo>
#include <QDir>
#include <QMimeDatabase>
#include <QCollator>
#include <QUrl>
#include <QDBusInterface>
#include <QThread>
#include <QProcess>
#include <QGuiApplication>
#include <QScreen>
#include <QDesktopServices>
#include <QClipboard>
#include <QApplication>
#include <QUrl>
#include <QDebug>
#include <QLoggingCategory>

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)
DCORE_USE_NAMESPACE

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

// 转换路径
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

FileControl::FileControl(QObject *parent)
    : QObject(parent)
{
    qCDebug(logImageViewer) << "FileControl constructor entered.";
    m_ocrInterface = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus(), this);
    m_shortcutViewProcess = new QProcess(this);
    m_config = LibConfigSetter::instance();
    imageFileWatcher = ImageFileWatcher::instance();
    qCDebug(logImageViewer) << "OCR interface, shortcut view process, config, and image file watcher initialized.";

    QObject::connect(imageFileWatcher, &ImageFileWatcher::imageFileChanged, this, &FileControl::imageFileChanged);
    qCDebug(logImageViewer) << "Connected imageFileWatcher::imageFileChanged signal.";

    // 在1000ms以内只保存一次配置信息
    if (!m_tSaveSetting) {
        m_tSaveSetting = new QTimer(this);
        connect(m_tSaveSetting, &QTimer::timeout, this, [=]() { saveSetting(); });
        qCDebug(logImageViewer) << "Save setting timer initialized and connected.";
    }

    listsupportWallPaper << "bmp"
                         << "cod"
                         << "png"
                         << "gif"
                         << "ief"
                         << "jpe"
                         << "jpeg"
                         << "jpg"
                         << "jfif"
                         << "tif"
                         << "tiff";
    qCDebug(logImageViewer) << "Supported wallpaper formats initialized.";
}

FileControl::~FileControl()
{
    qCDebug(logImageViewer) << "FileControl destructor entered.";
    saveSetting();
    qCDebug(logImageViewer) << "Settings saved on destruction.";
}

QString FileControl::standardPicturesPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

QStringList FileControl::getDirImagePath(const QString &path)
{
    qCDebug(logImageViewer) << "Getting image paths from directory:" << path;
    if (path.isEmpty()) {
        qCDebug(logImageViewer) << "Empty path provided";
        return QStringList();
    }

    QStringList image_list;
    QString DirPath = QFileInfo(QUrl(path).toLocalFile()).dir().path();
    qCDebug(logImageViewer) << "Directory path:" << DirPath;

    QDir _dirinit(DirPath);
    QFileInfoList m_AllPath = _dirinit.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
    qCDebug(logImageViewer) << "Found" << m_AllPath.size() << "entries in directory.";

    // 修复Ｑt带后缀排序错误的问题
    std::sort(m_AllPath.begin(), m_AllPath.end(), compareByFileInfo);
    qCDebug(logImageViewer) << "Sorted file info list.";
    for (int i = 0; i < m_AllPath.size(); i++) {
        QString tmpPath = m_AllPath.at(i).filePath();
        if (tmpPath.isEmpty()) {
            qCDebug(logImageViewer) << "Skipping empty file path at index" << i;
            continue;
        }
        // 判断是否图片格式
        if (isImage(tmpPath)) {
            image_list << QUrl::fromLocalFile(tmpPath).toString();
            qCDebug(logImageViewer) << "Added image to list:" << tmpPath;
        } else {
            qCDebug(logImageViewer) << "Skipping non-image file:" << tmpPath;
        }
    }
    qCDebug(logImageViewer) << "Found" << image_list.size() << "images in directory";
    return image_list;
}

/**
   @return 返回文件路径 \a path 所在的文件夹是否为当前监控的文件夹
 */
bool FileControl::isCurrentWatcherDir(const QUrl &path)
{
    qCDebug(logImageViewer) << "Checking if" << path.toLocalFile() << "is current watcher directory.";
    return imageFileWatcher->isCurrentDir(path.toLocalFile());
}

QString FileControl::getNamePath(const QString &oldPath, const QString &newName)
{
    qCDebug(logImageViewer) << "Getting new name path for old path:" << oldPath << "and new name:" << newName;
    QString old = oldPath;
    QString now = newName;

    if (old.startsWith("file://")) {
        old = QUrl(old).toLocalFile();
        qCDebug(logImageViewer) << "Converted old path to local file:" << old;
    }
    if (now.startsWith("file://")) {
        now = QUrl(now).toLocalFile();
        qCDebug(logImageViewer) << "Converted new name to local file:" << now;
    }

    QFileInfo info(old);
    QString path = info.path();
    QString suffix = info.suffix();
    QString newPath = path + "/" + newName + "." + suffix;
    qCDebug(logImageViewer) << "Constructed new path:" << newPath;
    return QUrl::fromLocalFile(newPath).toString();
}

bool FileControl::isImage(const QString &path)
{
    qCDebug(logImageViewer) << "Checking if path is an image:" << path;
    bool bRet = false;
    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForFile(path, QMimeDatabase::MatchContent);
    QMimeType mt1 = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension);
    qCDebug(logImageViewer) << "Mime type by content:" << mt.name() << ", by extension:" << mt1.name();
    if (mt.name().startsWith("image/") || mt.name().startsWith("video/x-mng") || mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
        bRet = true;
        qCDebug(logImageViewer) << "Path identified as image.";
    }
    qCDebug(logImageViewer) << "isImage returning:" << bRet;
    return bRet;
}

void FileControl::setWallpaper(const QString &imgPath)
{
    qCDebug(logImageViewer) << "Setting wallpaper:" << imgPath;
    QThread *th1 = QThread::create([=]() {
        if (!imgPath.isNull()) {
            QString path = imgPath;
            // 202011/12 bug54279
            {
                // 设置壁纸代码改变，采用DBus,原方法保留
                if (/*!qEnvironmentVariableIsEmpty("FLATPAK_APPID")*/ 1) {
                    // gdbus call -e -d com.deepin.daemon.Appearance -o /com/deepin/daemon/Appearance -m
                    // com.deepin.daemon.Appearance.Set background /home/test/test.png
                    qCDebug(logImageViewer) << "Setting wallpaper via DBus";
                    QDBusInterface interfaceV23(
                            "org.deepin.dde.Appearance1", "/org/deepin/dde/Appearance1", "org.deepin.dde.Appearance1");
                    QDBusInterface interfaceV20(
                            "com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", "com.deepin.daemon.Appearance");

                    if (interfaceV23.isValid() || interfaceV20.isValid()) {
                        QString screenname;

                        // 判断环境是否是wayland
                        auto e = QProcessEnvironment::systemEnvironment();
                        QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
                        QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

                        bool isWayland = false;
                        if (XDG_SESSION_TYPE != QLatin1String("wayland") && !WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
                            isWayland = false;
                            qCDebug(logImageViewer) << "Detected X11 session.";
                        } else {
                            isWayland = true;
                            qCDebug(logImageViewer) << "Detected Wayland session.";
                        }
                        qCDebug(logImageViewer) << "Display environment:" << (isWayland ? "Wayland" : "X11");

                        // wayland下设置壁纸使用，2020/09/21
                        if (isWayland) {
                            qCDebug(logImageViewer) << "Attempting to get primary screen for Wayland.";
                            QDBusInterface interfaceWaylandV23(
                                    "org.deepin.dde.Display1", "/org/deepin/dde/Display1", "org.deepin.dde.Display1");
                            if (interfaceWaylandV23.isValid()) {
                                screenname = qvariant_cast<QString>(interfaceWaylandV23.property("Primary"));
                                qCDebug(logImageViewer) << "Got primary screen from v23 Wayland interface:" << screenname;
                            } else {
                                qCDebug(logImageViewer) << "v23 Wayland interface not valid, trying v20.";
                                QDBusInterface interfaceWaylandV20(
                                        "com.deepin.daemon.Display", "/com/deepin/daemon/Display", "com.deepin.daemon.Display");
                                screenname = qvariant_cast<QString>(interfaceWaylandV20.property("Primary"));
                                qCDebug(logImageViewer) << "Got primary screen from v20 Wayland interface:" << screenname;
                            }
                        } else {
                            qCDebug(logImageViewer) << "Attempting to get primary screen for X11.";
                            screenname = QGuiApplication::primaryScreen()->name();
                            qCDebug(logImageViewer) << "Got primary screen from X11:" << screenname;
                        }

                        bool settingSucc = false;
                        if (interfaceV23.isValid()) {
                            qCDebug(logImageViewer) << "Attempting to set wallpaper via v23 interface: SetMonitorBackground(" << screenname << ", " << path << ")";
                            QDBusMessage reply = interfaceV23.call(QStringLiteral("SetMonitorBackground"), screenname, path);
                            settingSucc = reply.errorMessage().isEmpty();

                            qCDebug(logImageViewer) << "Attempting to set wallpaper via v23 interface";
                            if (!settingSucc) {
                                qCWarning(logImageViewer) << "Failed to set wallpaper via v23 interface:" << reply.errorMessage();
                            }
                        }

                        if (interfaceV20.isValid() && !settingSucc) {
                            QDBusMessage reply = interfaceV20.call(QStringLiteral("SetMonitorBackground"), screenname, path);
                            qCDebug(logImageViewer) << "Attempting to set wallpaper via v20 interface";
                            if (!reply.errorMessage().isEmpty()) {
                                qCWarning(logImageViewer) << "Failed to set wallpaper via v20 interface:" << reply.errorMessage();
                            }
                        }
                    } else {
                        qCWarning(logImageViewer) << "Failed to initialize wallpaper interfaces - v23:"
                                                  << interfaceV23.lastError().message() << "v20:" << interfaceV20.lastError().message();
                    }
                }
            }
        }
    });
    connect(th1, &QThread::finished, th1, &QObject::deleteLater);
    th1->start();
    qCDebug(logImageViewer) << "Wallpaper setting thread started.";
}

bool FileControl::deleteImagePath(const QString &path)
{
    qCDebug(logImageViewer) << "Attempting to delete image:" << path;
    QUrl displayUrl = QUrl(path);

    if (displayUrl.isValid()) {
        qCDebug(logImageViewer) << "URL is valid:" << displayUrl.toString();
        QStringList list;
        list << displayUrl.toString();
        QDBusInterface interface(QStringLiteral("org.freedesktop.FileManager1"),
                                 QStringLiteral("/org/freedesktop/FileManager1"),
                                 QStringLiteral("org.freedesktop.FileManager1"));
        qCDebug(logImageViewer) << "DBus interface for FileManager1 created.";
        // 默认超时时间大约25s, 修改为最大限制
        interface.setTimeout(INT_MAX);
        qCDebug(logImageViewer) << "DBus interface timeout set to INT_MAX.";
        auto pendingCall = interface.asyncCall("Trash", list);
        qCDebug(logImageViewer) << "Async call to Trash initiated.";
        while (!pendingCall.isFinished()) {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            qCDebug(logImageViewer) << "Waiting for DBus Trash call to finish...";
        }

        if (pendingCall.isError()) {
            auto error = pendingCall.error();
            qCWarning(logImageViewer) << "Failed to delete image via DBus:" << error.name() << error.message();
            return false;
        }

        // 删除信息未通过 DBus 返回，直接判断文件是否已被删除
        if (QFile::exists(displayUrl.toLocalFile())) {
            qCWarning(logImageViewer) << "Delete operation failed - file still exists:" << displayUrl.toLocalFile();
            return false;
        }

        qCDebug(logImageViewer) << "Successfully deleted image:" << path;
        return true;
    }
    qCWarning(logImageViewer) << "Invalid URL for deletion:" << path;
    return false;
}

bool FileControl::displayinFileManager(const QString &path)
{
    qCDebug(logImageViewer) << "Attempting to display in file manager:" << path;
    bool bRet = false;
    QUrl displayUrl = QUrl(path);

    QDBusInterface interface(QStringLiteral("org.freedesktop.FileManager1"),
                             QStringLiteral("/org/freedesktop/FileManager1"),
                             QStringLiteral("org.freedesktop.FileManager1"));
    qCDebug(logImageViewer) << "DBus interface for FileManager1 created.";

    if (interface.isValid()) {
        qCDebug(logImageViewer) << "DBus interface is valid.";
        QStringList list;
        list << displayUrl.toString();
        bRet = interface.call("ShowItems", list, "").type() != QDBusMessage::ErrorMessage;
    }
    return bRet;
}

void FileControl::copyImage(const QString &path)
{
    qCDebug(logImageViewer) << "Copying image to clipboard:" << path;
    QString localPath = QUrl(path).toLocalFile();

    QClipboard *cb = qApp->clipboard();

    // Ownership of the new data is transferred to the clipboard.
    QMimeData *newMimeData = new QMimeData();

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

    cb->setMimeData(newMimeData, QClipboard::Clipboard);
    qCDebug(logImageViewer) << "Image copied to clipboard successfully";
}

void FileControl::copyText(const QString &str)
{
    qCDebug(logImageViewer) << "Copying text to clipboard:" << str;
    qApp->clipboard()->setText(str);
    qCDebug(logImageViewer) << "Text copied to clipboard.";
}

bool FileControl::isRotatable(const QString &path)
{
    qCDebug(logImageViewer) << "Checking if image is rotatable:" << path;
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();
    QFileInfo info(localPath);
    if (!info.isFile() || !info.exists() || !info.isWritable()) {
        qCDebug(logImageViewer) << "Image is not rotatable: Not a file, does not exist, or not writable.";
        bRet = false;
    } else {
        bRet = LibUnionImage_NameSpace::isImageSupportRotate(localPath);
        qCDebug(logImageViewer) << "Image support rotate check result: " << bRet;
    }
    return bRet;
}

bool FileControl::isCanWrite(const QString &path)
{
    qCDebug(logImageViewer) << "Checking if path is writable:" << path;
    QString localPath = QUrl(path).toLocalFile();
    QFileInfo info(localPath);
    bool bRet = info.isWritable() && QFileInfo(info.dir(), info.dir().path()).isWritable();   // 是否可写
    qCDebug(logImageViewer) << "Path writable check result: " << bRet;
    return bRet;
}

bool FileControl::isCanDelete(const QString &path)
{
    qCDebug(logImageViewer) << "Checking if path is deletable:" << path;
    bool bRet = false;
    bool isAlbum = false;
    QString localPath = QUrl(path).toLocalFile();
    QFileInfo info(localPath);
    bool isWritable = info.isWritable() && QFileInfo(info.dir(), info.dir().path()).isWritable();   // 是否可写
    bool isReadable = info.isReadable();   // 是否可读
    imageViewerSpace::PathType pathType = LibUnionImage_NameSpace::getPathType(localPath);
    qCDebug(logImageViewer) << "Path type: " << pathType << ", isWritable: " << isWritable << ", isReadable: " << isReadable;
    if ((imageViewerSpace::PathTypeAPPLE != pathType && imageViewerSpace::PathTypeSAFEBOX != pathType && imageViewerSpace::PathTypeRECYCLEBIN != pathType && imageViewerSpace::PathTypeMTP != pathType && imageViewerSpace::PathTypePTP != pathType && isWritable && isReadable) || (isAlbum && isWritable)) {
        qCDebug(logImageViewer) << "Path is deletable based on conditions.";
        bRet = true;
    } else {
        qCDebug(logImageViewer) << "Path is not deletable based on conditions.";
        bRet = false;
    }
    return bRet;
}

void FileControl::ocrImage(const QString &path, int index)
{
    qCDebug(logImageViewer) << "Starting OCR for image:" << path << "index:" << index;
    QString localPath = QUrl(path).toLocalFile();
    // 此处借用已取得的缓存信息，一般状态下，调用OCR前已完成图像的加载
    ImageInfo info(path);

    if (Types::MultiImage != info.type()) {   // 非多页图使用路径直接进行识别
        qCDebug(logImageViewer) << "Processing single page image for OCR";
        m_ocrInterface->openFile(localPath);
        qCDebug(logImageViewer) << "Called OCR interface openFile for single image.";
    } else {   // 多页图需要确定识别哪一页
        qCDebug(logImageViewer) << "Processing multi-page image for OCR, page:" << index;
        QImageReader imageReader(localPath);
        qCDebug(logImageViewer) << "ImageReader created for: " << localPath;
        imageReader.jumpToImage(index);
        qCDebug(logImageViewer) << "Jumped to image index: " << index;
        auto image = imageReader.read();
        qCDebug(logImageViewer) << "Image read from reader.";
        auto tempDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        qCDebug(logImageViewer) << "Cache location: " << tempDir;
        QDir dir(tempDir);
        if (!dir.exists()) {
            qCDebug(logImageViewer) << "Cache directory does not exist, creating it.";
            dir.mkpath(".");
        }
        auto tempFileName = tempDir + QDir::separator() + "rec.png";

        image.save(tempFileName);
        qCDebug(logImageViewer) << "Saved temporary image for OCR:" << tempFileName;
        m_ocrInterface->openFile(tempFileName);
        qCDebug(logImageViewer) << "Called OCR interface openFile for temporary image.";
    }
}

QString FileControl::parseCommandlineGetPath()
{
    qCDebug(logImageViewer) << "Parsing commandline to get path.";
    QString filepath = "";
    QStringList arguments = QCoreApplication::arguments();
    for (QString path : arguments) {
        path = UrlInfo(path).toLocalFile();
        if (QFileInfo(path).isFile()) {
            bool bRet = isImage(path);
            if (bRet) {
                qCDebug(logImageViewer) << "Found image file in commandline arguments:" << path;
                return QUrl::fromLocalFile(path).toString();
            }
        }
    }

    qCDebug(logImageViewer) << "No image file found in commandline arguments.";
    return filepath;
}

QString FileControl::slotGetFileName(const QString &path)
{
    qCDebug(logImageViewer) << "Getting file name for path: " << path;
    QString tmppath = path;

    if (path.startsWith("file://")) {
        qCDebug(logImageViewer) << "Path starts with file://, converting to local file.";
        tmppath = QUrl(tmppath).toLocalFile();
    }

    QFileInfo info(tmppath);
    qCDebug(logImageViewer) << "Returning complete base name: " << info.completeBaseName();
    return info.completeBaseName();
}

QString FileControl::slotGetFileNameSuffix(const QString &path)
{
    qCDebug(logImageViewer) << "Getting file name with suffix for path: " << path;
    QString tmppath = path;

    if (path.startsWith("file://")) {
        qCDebug(logImageViewer) << "Path starts with file://, converting to local file.";
        tmppath = QUrl(tmppath).toLocalFile();
    }

    QFileInfo info(tmppath);
    qCDebug(logImageViewer) << "Returning file name with suffix: " << info.fileName();
    return info.fileName();
}

QString FileControl::slotGetInfo(const QString &key, const QString &path)
{
    qCDebug(logImageViewer) << "Getting info for key: " << key << ", path: " << path;
    QString localPath = QUrl(path).toLocalFile();
    if (localPath != m_currentPath) {
        qCDebug(logImageViewer) << "Local path changed, updating current path and metadata.";
        m_currentPath = localPath;
        m_currentAllInfo = LibUnionImage_NameSpace::getAllMetaData(localPath);
    }

    QString returnString = m_currentAllInfo.value(key);
    if (returnString.isEmpty()) {
        qCDebug(logImageViewer) << "Retrieved info is empty for key: " << key << ", setting to '-'.";
        returnString = "-";
    }

    qCDebug(logImageViewer) << "Returning info: " << returnString;
    return returnString;
}

bool FileControl::slotFileReName(const QString &name, const QString &filepath, bool isSuffix)
{
    qCDebug(logImageViewer) << "Attempting to rename file. New name: " << name << ", filepath: " << filepath << ", isSuffix: " << isSuffix;
    QString localPath = QUrl(filepath).toLocalFile();
    QFile file(localPath);
    if (file.exists()) {
        qCDebug(logImageViewer) << "File exists, proceeding with rename.";
        QFileInfo info(localPath);
        QString path = info.path();
        QString suffix = info.suffix();
        QString _newName;
        if (isSuffix) {
            _newName = path + "/" + name;
            qCDebug(logImageViewer) << "Rename with suffix, new name: " << _newName;
        } else {
            _newName = path + "/" + name + "." + suffix;
            qCDebug(logImageViewer) << "Rename without suffix, new name: " << _newName;
        }

        if (file.rename(_newName)) {
            qCDebug(logImageViewer) << "File renamed successfully to: " << _newName;
            imageFileWatcher->fileRename(localPath, _newName);
            qCDebug(logImageViewer) << "Image file watcher notified about rename.";

            Q_EMIT imageRenamed(QUrl::fromLocalFile(localPath), QUrl::fromLocalFile(_newName));
            qCDebug(logImageViewer) << "Emitted imageRenamed signal.";
            return true;
        }

        qCWarning(logImageViewer) << "Failed to rename file from " << localPath << " to " << _newName;
        return false;
    }
    qCWarning(logImageViewer) << "File does not exist: " << localPath;
    return false;
}

QString FileControl::slotFileSuffix(const QString &path, bool ret)
{
    qCDebug(logImageViewer) << "FileControl::slotFileSuffix() called for path: " << path << ", ret: " << ret;
    QString returnSuffix = "";

    QString localPath = QUrl(path).toLocalFile();
    if (!path.isEmpty() && QFile::exists(localPath)) {
        qCDebug(logImageViewer) << "Path is not empty and file exists: " << localPath;
        QString tmppath = path;
        QFileInfo info(tmppath);
        if (ret) {
            qCDebug(logImageViewer) << "Returning complete suffix with dot.";
            returnSuffix = "." + info.completeSuffix();
        } else {
            qCDebug(logImageViewer) << "Returning complete suffix without dot.";
            returnSuffix = info.completeSuffix();
        }
    } else {
        qCDebug(logImageViewer) << "Path is empty or file does not exist: " << localPath;
    }

    qCDebug(logImageViewer) << "Returning suffix: " << returnSuffix;
    return returnSuffix;
}

bool FileControl::isShowToolTip(const QString &oldPath, const QString &name)
{
    qCDebug(logImageViewer) << "FileControl::isShowToolTip() called for oldPath: " << oldPath << ", name: " << name;
    bool bRet = false;
    QString path = QUrl(oldPath).toLocalFile();
    QFileInfo fileinfo(path);
    QString DirPath = fileinfo.path();
    QString filename = fileinfo.completeBaseName();
    if (filename == name) {
        qCDebug(logImageViewer) << "Filename is the same, no tooltip needed.";
        return false;
    }

    QString format = fileinfo.suffix();

    QString fileabname = DirPath + "/" + name + "." + format;
    QFile file(fileabname);
    if (file.exists() && fileabname != path) {
        qCDebug(logImageViewer) << "File exists and is different from old path, tooltip will be shown.";
        bRet = true;
    } else {
        qCDebug(logImageViewer) << "File does not exist or is the same as old path, no tooltip needed.";
        bRet = false;
    }
    qCDebug(logImageViewer) << "Returning isShowToolTip: " << bRet;
    return bRet;
}

void FileControl::showPrintDialog(const QString &path)
{
    qCDebug(logImageViewer) << "FileControl::showPrintDialog() called for path: " << path;
    QString oldPath = QUrl(path).toLocalFile();
    PrintHelper::getIntance()->showPrintDialog(QStringList(oldPath));
    qCDebug(logImageViewer) << "Print dialog shown for file: " << oldPath;
}

QVariant FileControl::getConfigValue(const QString &group, const QString &key, const QVariant &defaultValue)
{
    qCDebug(logImageViewer) << "FileControl::getConfigValue() called for group: " << group << ", key: " << key << ", defaultValue: " << defaultValue;
    QVariant value = m_config->value(group, key, defaultValue);
    qCDebug(logImageViewer) << "Config value retrieved: " << value;
    return value;
}

void FileControl::setConfigValue(const QString &group, const QString &key, const QVariant &value)
{
    qCDebug(logImageViewer) << "FileControl::setConfigValue() called for group: " << group << ", key: " << key << ", value: " << value;
    m_config->setValue(group, key, value);
    qCDebug(logImageViewer) << "Config value set.";
}

int FileControl::getlastWidth()
{
    qCDebug(logImageViewer) << "FileControl::getlastWidth() called.";
    int reWidth = 0;
    int defaultW = 0;

    // 多屏下仅采用单个屏幕处理， 使用主屏的参考宽度计算
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qCWarning(logImageViewer) << "Primary screen not found, returning minimum width.";
        return MAINWIDGET_MINIMUN_WIDTH;
    }

    if (QGuiApplication::screens().size() > 1 && screen) {
        qCDebug(logImageViewer) << "Multiple screens detected, calculating default width based on primary screen size.";
        defaultW = int(double(screen->size().width()) * 0.60);
    } else {
        qCDebug(logImageViewer) << "Single screen or no valid screen, calculating default width based on primary screen geometry.";
        defaultW = int(double(screen->geometry().width()) * 0.60);
    }
    qCDebug(logImageViewer) << "Default width calculated: " << defaultW;

    const int ww = getConfigValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, QVariant(defaultW)).toInt();
    qCDebug(logImageViewer) << "Retrieved window width from config: " << ww;

    reWidth = ww >= MAINWIDGET_MINIMUN_WIDTH ? ww : MAINWIDGET_MINIMUN_WIDTH;
    m_windowWidth = reWidth;
    qCDebug(logImageViewer) << "Final window width: " << reWidth;
    return reWidth;
}

int FileControl::getlastHeight()
{
    qCDebug(logImageViewer) << "FileControl::getlastHeight() called.";
    int reHeight = 0;
    int defaultH = 0;

    // 多屏下仅采用单个屏幕处理， 使用主屏的参考高度计算
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qCWarning(logImageViewer) << "Primary screen not found, returning minimum height.";
        return MAINWIDGET_MINIMUN_HEIGHT;
    }

    if (QGuiApplication::screens().size() > 1 && screen) {
        qCDebug(logImageViewer) << "Multiple screens detected, calculating default height based on primary screen size.";
        defaultH = int(double(screen->size().height()) * 0.60);
    } else {
        qCDebug(logImageViewer) << "Single screen or no valid screen, calculating default height based on primary screen geometry.";
        defaultH = int(double(screen->geometry().height()) * 0.60);
    }
    qCDebug(logImageViewer) << "Default height calculated: " << defaultH;

    const int wh = getConfigValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, QVariant(defaultH)).toInt();
    qCDebug(logImageViewer) << "Retrieved window height from config: " << wh;

    reHeight = wh >= MAINWIDGET_MINIMUN_HEIGHT ? wh : MAINWIDGET_MINIMUN_HEIGHT;
    m_windowHeight = reHeight;
    qCDebug(logImageViewer) << "Final window height: " << reHeight;
    return reHeight;
}

void FileControl::setSettingWidth(int width)
{
    qCDebug(logImageViewer) << "FileControl::setSettingWidth() called with width: " << width;
    m_windowWidth = width;
    m_tSaveSetting->setSingleShot(true);
    m_tSaveSetting->start(1000);
    qCDebug(logImageViewer) << "Setting width and starting save setting timer.";
}

void FileControl::setSettingHeight(int height)
{
    qCDebug(logImageViewer) << "FileControl::setSettingHeight() called with height: " << height;
    m_windowHeight = height;
    m_tSaveSetting->setSingleShot(true);
    m_tSaveSetting->start(1000);
    qCDebug(logImageViewer) << "Setting height and starting save setting timer.";
}

void FileControl::setEnableNavigation(bool b)
{
    qCDebug(logImageViewer) << "FileControl::setEnableNavigation() called with value: " << b;
    setConfigValue(SETTINGS_GROUP, SETTINGS_ENABLE_NAVIGATION, b);
    qCDebug(logImageViewer) << "Navigation enabled setting saved.";
}

bool FileControl::isEnableNavigation()
{
    qCDebug(logImageViewer) << "FileControl::isEnableNavigation() called.";
    bool enabled = getConfigValue(SETTINGS_GROUP, SETTINGS_ENABLE_NAVIGATION, true).toBool();
    qCDebug(logImageViewer) << "Returning enable navigation status: " << enabled;
    return enabled;
}

void FileControl::saveSetting()
{
    qCDebug(logImageViewer) << "FileControl::saveSetting() called.";
    if (m_lastSaveWidth != m_windowWidth) {
        qCDebug(logImageViewer) << "Saving window width: " << m_windowWidth;
        setConfigValue(SETTINGS_GROUP, SETTINGS_WINSIZE_W_KEY, m_windowWidth);
        m_lastSaveWidth = m_windowWidth;
    }
    if (m_lastSaveHeight != m_windowHeight) {
        qCDebug(logImageViewer) << "Saving window height: " << m_windowHeight;
        setConfigValue(SETTINGS_GROUP, SETTINGS_WINSIZE_H_KEY, m_windowHeight);
        m_lastSaveHeight = m_windowHeight;
    }
    qCDebug(logImageViewer) << "Settings saved.";
}

bool FileControl::isSupportSetWallpaper(const QString &path)
{
    qCDebug(logImageViewer) << "FileControl::isSupportSetWallpaper() called for path: " << path;
    QString path1 = QUrl(path).toLocalFile();
    QFileInfo fileinfo(path1);
    QString format = fileinfo.suffix().toLower();
    // 设置为壁纸需要判断是否有读取权限
    if (listsupportWallPaper.contains(format) && fileinfo.isReadable()) {
        qCDebug(logImageViewer) << "Image format " << format << " is supported and readable for wallpaper. Returning true.";
        return true;
    }
    qCDebug(logImageViewer) << "Image format " << format << " not supported or not readable for wallpaper. Returning false.";
    return false;
}

bool FileControl::isCheckOnly()
{
    qCDebug(logImageViewer) << "FileControl::isCheckOnly() called.";
    // single
    QString userName = QDir::homePath().section("/", -1, -1);
    std::string path = ("/home/" + userName + "/.cache/deepin/deepin-image-viewer/").toStdString();
    QDir tdir(path.c_str());
    qCDebug(logImageViewer) << "Cache directory path: " << QString::fromStdString(path);
    if (!tdir.exists()) {
        qCDebug(logImageViewer) << "Cache directory does not exist, attempting to create.";
        bool ret = tdir.mkpath(path.c_str());
        qCDebug(logImageViewer) << "Cache directory creation result: " << ret;
    }

    path += "single";
    qCDebug(logImageViewer) << "Attempting to open lockfile: " << QString::fromStdString(path);
    int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    int flock = lockf(fd, F_TLOCK, 0);

    if (fd == -1) {
        perror("open lockfile/n");
        qCWarning(logImageViewer) << "Failed to open lockfile.";
        return false;
    }
    if (flock == -1) {
        perror("lock file error/n");
        qCWarning(logImageViewer) << "Failed to lock file.";
        return false;
    }
    qCDebug(logImageViewer) << "Lock file opened and locked successfully.";
    return true;
}

bool FileControl::isCanSupportOcr(const QString &path)
{
    qCDebug(logImageViewer) << "FileControl::isCanSupportOcr() called for path: " << path;
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();
    QFileInfo info(localPath);
    imageViewerSpace::ImageType type = LibUnionImage_NameSpace::getImageType(localPath);
    qCDebug(logImageViewer) << "Image type: " << type << ", isReadable: " << info.isReadable();
    if (imageViewerSpace::ImageTypeDynamic != type && info.isReadable()) {
        qCDebug(logImageViewer) << "Image supports OCR. Returning true.";
        bRet = true;
    }
    qCDebug(logImageViewer) << "Image does not support OCR. Returning false.";
    return bRet;
}

bool FileControl::isCanRename(const QString &path)
{
    qCDebug(logImageViewer) << "FileControl::isCanRename() called for path: " << path;
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();
    imageViewerSpace::PathType pathType = LibUnionImage_NameSpace::getPathType(localPath);   // 路径类型
    QFileInfo info(localPath);
    bool isWritable = info.isWritable() && QFileInfo(info.dir(), info.dir().path()).isWritable();   // 是否可写
    qCDebug(logImageViewer) << "Path type: " << pathType << ", isWritable: " << isWritable << ", isReadable: " << info.isReadable();
    if (info.isReadable() && isWritable && imageViewerSpace::PathTypeMTP != pathType && imageViewerSpace::PathTypePTP != pathType && imageViewerSpace::PathTypeAPPLE != pathType) {
        qCDebug(logImageViewer) << "Path is renameable. Returning true.";
        bRet = true;
    }
    qCDebug(logImageViewer) << "Path is not renameable. Returning false.";
    return bRet;
}

bool FileControl::isCanReadable(const QString &path)
{
    qCDebug(logImageViewer) << "FileControl::isCanReadable() called for path: " << path;
    bool bRet = false;
    QString localPath = QUrl(path).toLocalFile();
    QFileInfo info(localPath);
    if (info.isReadable()) {
        qCDebug(logImageViewer) << "Path is readable. Returning true.";
        bRet = true;
    }
    qCDebug(logImageViewer) << "Path is not readable. Returning false.";
    return bRet;
}

/**
 * @brief 根据传入的文件路径列表 \a filePaths 重设缓存的文件信息，
 *      若在图片打开过程中文件被修改，将发送信号至界面或其它处理。
 */
void FileControl::resetImageFiles(const QStringList &filePaths)
{
    qCDebug(logImageViewer) << "FileControl::resetImageFiles() called, count:" << filePaths.size();
    // 变更监控的文件
    imageFileWatcher->resetImageFiles(filePaths);
    qCDebug(logImageViewer) << "Image file watcher reset complete.";
    // 清理缩略图缓存记录
    ImageInfo::clearCache();
    qCDebug(logImageViewer) << "ImageInfo cache cleared.";
    qCDebug(logImageViewer) << "Image files reset complete.";
}

/**
 * @return 返回公司Logo图标地址
 */
QUrl FileControl::getCompanyLogo()
{
    qCDebug(logImageViewer) << "FileControl::getCompanyLogo() called.";
    QString logoPath = DSysInfo::distributionOrgLogo(DSysInfo::Distribution, DSysInfo::Light, ":/assets/images/deepin-logo.svg");
    qCDebug(logImageViewer) << "Company logo path: " << logoPath;
    return QUrl::fromLocalFile(logoPath);
}

void FileControl::terminateShortcutPanelProcess()
{
    qCDebug(logImageViewer) << "FileControl::terminateShortcutPanelProcess() called.";
    m_shortcutViewProcess->terminate();
    qCDebug(logImageViewer) << "Shortcut panel process terminated.";
    m_shortcutViewProcess->waitForFinished(2000);
    qCDebug(logImageViewer) << "Shortcut panel process waited for finish.";
}

void FileControl::showShortcutPanel(int windowCenterX, int windowCenterY)
{
    qCDebug(logImageViewer) << "FileControl::showShortcutPanel() called at position:" << windowCenterX << "," << windowCenterY;
    QPoint pos(windowCenterX, windowCenterY);
    QStringList shortcutString;
    auto json = createShortcutString();
    qCDebug(logImageViewer) << "Shortcut string created.";

    QString param1 = "-j=" + json;
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;
    qCDebug(logImageViewer) << "Shortcut parameters: " << shortcutString;

    terminateShortcutPanelProcess();
    m_shortcutViewProcess->start("deepin-shortcut-viewer", shortcutString);
    qCDebug(logImageViewer) << "Shortcut panel process started.";
}

QString FileControl::createShortcutString()
{
    qCDebug(logImageViewer) << "FileControl::createShortcutString() called.";
    if (!m_shortcutString.isEmpty()) {
        qCDebug(logImageViewer) << "Returning cached shortcut string.";
        return m_shortcutString;
    }

    QJsonObject shortcut1;
    shortcut1.insert("name", tr("Fullscreen"));
    shortcut1.insert("value", "F11");
    qCDebug(logImageViewer) << "Added Fullscreen shortcut.";

    QJsonObject shortcut2;
    shortcut2.insert("name", tr("Exit fullscreen"));
    shortcut2.insert("value", "Esc");
    qCDebug(logImageViewer) << "Added Exit fullscreen shortcut.";

    QJsonObject shortcut3;
    shortcut3.insert("name", tr("Extract text"));
    shortcut3.insert("value", "Alt + O");
    qCDebug(logImageViewer) << "Added Extract text shortcut.";

    QJsonObject shortcut4;
    shortcut4.insert("name", tr("Slide show"));
    shortcut4.insert("value", "F5");
    qCDebug(logImageViewer) << "Added Slide show shortcut.";

    QJsonObject shortcut5;
    shortcut5.insert("name", tr("Rename"));
    shortcut5.insert("value", "F2");
    qCDebug(logImageViewer) << "Added Rename shortcut.";

    QJsonObject shortcut6;
    shortcut6.insert("name", tr("Copy"));
    shortcut6.insert("value", "Ctrl + C");
    qCDebug(logImageViewer) << "Added Copy shortcut.";

    QJsonObject shortcut7;
    shortcut7.insert("name", tr("Delete"));
    shortcut7.insert("value", "Delete");
    qCDebug(logImageViewer) << "Added Delete shortcut.";

    QJsonObject shortcut8;
    shortcut8.insert("name", tr("Rotate clockwise"));
    shortcut8.insert("value", "Ctrl + R");
    qCDebug(logImageViewer) << "Added Rotate clockwise shortcut.";

    QJsonObject shortcut9;
    shortcut9.insert("name", tr("Rotate counterclockwise"));
    shortcut9.insert("value", "Ctrl + Shift + R");
    qCDebug(logImageViewer) << "Added Rotate counterclockwise shortcut.";

    QJsonObject shortcut10;
    shortcut10.insert("name", tr("Set as wallpaper"));
    shortcut10.insert("value", "Ctrl + F9");
    qCDebug(logImageViewer) << "Added Set as wallpaper shortcut.";

    QJsonObject shortcut11;
    shortcut11.insert("name", tr("Display in file manager"));
    shortcut11.insert("value", "Alt + D");

    QJsonObject shortcut12;
    shortcut12.insert("name", tr("Image info"));
    shortcut12.insert("value", "Ctrl + I");

    QJsonObject shortcut13;
    shortcut13.insert("name", tr("Previous"));
    shortcut13.insert("value", "Left");

    QJsonObject shortcut14;
    shortcut14.insert("name", tr("Next"));
    shortcut14.insert("value", "Right");

    QJsonObject shortcut15;
    shortcut15.insert("name", tr("Zoom in"));
    shortcut15.insert("value", "Ctrl + '+'");

    QJsonObject shortcut16;
    shortcut16.insert("name", tr("Zoom out"));
    shortcut16.insert("value", "Ctrl + '-'");

    QJsonObject shortcut17;
    shortcut17.insert("name", tr("Open"));
    shortcut17.insert("value", "Ctrl + O");

    QJsonObject shortcut18;
    shortcut18.insert("name", tr("Print"));
    shortcut18.insert("value", "Ctrl + P");

    QJsonArray shortcutArray1;
    shortcutArray1.append(shortcut1);
    shortcutArray1.append(shortcut2);
    shortcutArray1.append(shortcut3);
    shortcutArray1.append(shortcut4);
    shortcutArray1.append(shortcut5);
    shortcutArray1.append(shortcut6);
    shortcutArray1.append(shortcut7);
    shortcutArray1.append(shortcut8);
    shortcutArray1.append(shortcut9);
    shortcutArray1.append(shortcut10);
    shortcutArray1.append(shortcut11);
    shortcutArray1.append(shortcut12);
    shortcutArray1.append(shortcut13);
    shortcutArray1.append(shortcut14);
    shortcutArray1.append(shortcut15);
    shortcutArray1.append(shortcut16);
    shortcutArray1.append(shortcut17);
    shortcutArray1.append(shortcut18);

    QJsonObject shortcut_group1;
    shortcut_group1.insert("groupName", tr("Image Viewing"));
    shortcut_group1.insert("groupItems", shortcutArray1);

    QJsonObject shortcut19;
    shortcut19.insert("name", tr("Help"));
    shortcut19.insert("value", "F1");

    QJsonObject shortcut20;
    shortcut20.insert("name", tr("Display shortcuts"));
    shortcut20.insert("value", "Ctrl + Shift + ?");

    QJsonArray shortcutArray2;
    shortcutArray2.append(shortcut19);
    shortcutArray2.append(shortcut20);

    QJsonObject shortcut_group2;
    shortcut_group2.insert("groupName", tr("Settings"));
    shortcut_group2.insert("groupItems", shortcutArray2);

    QJsonObject shortcut21;
    shortcut21.insert("name", tr("Copy"));
    shortcut21.insert("value", "Ctrl + C");

    QJsonObject shortcut22;
    shortcut22.insert("name", tr("Select all"));
    shortcut22.insert("value", "Ctrl + A");

    QJsonArray shortcutArray3;
    shortcutArray3.append(shortcut21);
    shortcutArray3.append(shortcut22);

    QJsonObject shortcut_group3;
    shortcut_group3.insert("groupName", tr("Live Text"));
    shortcut_group3.insert("groupItems", shortcutArray3);

    QJsonArray shortcutArrayall;
    shortcutArrayall.append(shortcut_group1);
    shortcutArrayall.append(shortcut_group3);
    shortcutArrayall.append(shortcut_group2);

    QJsonObject main_shortcut;
    main_shortcut.insert("shortcut", shortcutArrayall);

    m_shortcutString = QJsonDocument(main_shortcut).toJson();

    qCDebug(logImageViewer) << "Shortcut string created: " << m_shortcutString;
    return m_shortcutString;
}
