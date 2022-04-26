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

#include <iostream>

#include "unionimage/unionimage_global.h"
#include "unionimage/unionimage.h"

#include "ocr/ocrinterface.h"

#include "printdialog/printhelper.h"

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

    //实时保存太卡，因此采用2s后延时保存的问题
    if (!m_tSaveImage) {
        m_tSaveImage = new QTimer(this);
        connect(m_tSaveImage, &QTimer::timeout, this, [ = ]() {
            slotRotatePixCurrent();
        });
    }
    m_config = LibConfigSetter::instance();
}

FileControl::~FileControl()
{
    //保存旋转的图片
    slotRotatePixCurrent();
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

    m_tSaveImage->setSingleShot(true);
    m_tSaveImage->start(1000);


    return bRet;
}

void FileControl::slotRotatePixCurrent()
{
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
    QString returnSuffix;

    QString tmppath = path;
    QFileInfo info(tmppath);
    if (ret) {

        returnSuffix = "." + info.completeSuffix();
    } else {
        returnSuffix =  info.completeSuffix();
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
//    QString error;
//    LibUnionImage_NameSpace::loadStaticImageFromFile(localPath, m_currentImage, error);
}

int FileControl::getCurrentImageWidth()
{
//    return m_currentImage.width();
    int width = -1;
    if (m_currentReader) {
        width = m_currentReader->size().width();
    }

    return width;
}

int FileControl::getCurrentImageHeight()
{
    int height = -1;
    if (m_currentReader) {
        height = m_currentReader->size().height();
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
    QString filename = fileinfo.fileName();
    QString format = fileinfo.suffix();

    QString fileabname = DirPath + "/" + name + "." + format;
    QFile file(fileabname);
    if (file.exists() && fileabname == path) {
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

void FileControl::getConfigValue(const QString &group, const QString &key, const QVariant &defaultValue)
{
    m_config->value(group, key, defaultValue);
}

void FileControl::setConfigValue(const QString &group, const QString &key, const QVariant &value)
{
    m_config->setValue(group, key, value);
}
