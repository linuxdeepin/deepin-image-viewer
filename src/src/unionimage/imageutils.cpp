// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseutils.h"
#include "imageutils.h"
#include "unionimage.h"
#include <fstream>

#include <QBuffer>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QMimeDatabase>
#include <QMutexLocker>
#include <QPixmapCache>
#include <QProcess>
#include <QReadWriteLock>
#include <QUrl>
#include <QApplication>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

namespace Libutils {

namespace image {

const QImage scaleImage(const QString &path, const QSize &size)
{
    qCDebug(logImageViewer) << "Attempting to scale image:" << path << "to size:" << size;
    if (!imageSupportRead(path)) {
        qCWarning(logImageViewer) << "Image format not supported for scaling:" << path;
        return QImage();
    }
    QImageReader reader(path);
    reader.setAutoTransform(true);
    if (!reader.canRead()) {
        qCWarning(logImageViewer) << "Can\'t read image for scaling:" << path;
        return QImage();
    }

    QSize tSize = reader.size();
    if (!tSize.isValid()) {
        qCDebug(logImageViewer) << "Image size not valid from reader, trying to get from metadata:" << path;
        QStringList rl = getAllMetaData(path).value("Dimension").split("x");
        if (rl.length() == 2) {
            tSize = QSize(QString(rl.first()).toInt(),
                          QString(rl.last()).toInt());
            qCDebug(logImageViewer) << "Image size extracted from metadata:" << tSize;
        } else {
            qCWarning(logImageViewer) << "Failed to get image size from metadata for:" << path;
        }
    }
    tSize.scale(size, Qt::KeepAspectRatio);
    reader.setScaledSize(tSize);
    QImage tImg = reader.read();
    // Some format does not support scaling
    if (tImg.width() > size.width() || tImg.height() > size.height()) {
        if (tImg.isNull()) {
            qCWarning(logImageViewer) << "Failed to read scaled image after setting scaled size:" << path;
            return QImage();
        } else {
            qCDebug(logImageViewer) << "Image still too large after initial scaling, attempting save and rescale.";
            // Save as supported format and scale it again
            const QString tmp = QDir::tempPath() + "/scale_tmp_image.png";
            QFile::remove(tmp);
            if (tImg.save(tmp, "png", 50)) {
                qCDebug(logImageViewer) << "Saved temporary PNG for rescaling:" << tmp;
                return scaleImage(tmp, size);
            } else {
                qCWarning(logImageViewer) << "Failed to save temporary PNG for rescaling:" << tmp;
                return QImage();
            }
        }
    } else {
        qCDebug(logImageViewer) << "Successfully scaled image:" << path << "from" << tSize << "to" << size;
        return tImg;
    }
}

const QDateTime getCreateDateTime(const QString &path)
{
    qCDebug(logImageViewer) << "Getting creation date for:" << path;
    QDateTime dt;

    // fallback to metadata.
    if (!dt.isValid()) {
        QString s;
        s = getAllMetaData(path).value("DateTimeOriginal");
        if (s.isEmpty()) {
            qCDebug(logImageViewer) << "DateTimeOriginal metadata is empty, trying DateTimeDigitized.";
            s = getAllMetaData(path).value("DateTimeDigitized");
        }
        if (s.isEmpty()) {
            qCDebug(logImageViewer) << "DateTimeDigitized metadata is empty, falling back to current time string.";
            s = QDateTime::currentDateTime().toString();
        }
        dt = QDateTime::fromString(s, "yyyy.MM.dd HH:mm:ss");
        qCDebug(logImageViewer) << "Using metadata date:" << dt;
    }

    // fallback to file create time.
    if (!dt.isValid()) {
        qCDebug(logImageViewer) << "Metadata date is invalid, falling back to file birth time.";
        QFileInfo finfo(path);
        dt = finfo.birthTime();
        qCDebug(logImageViewer) << "Using file birth time:" << dt;
    }

    // fallback to today.
    if (!dt.isValid()) {
        qCDebug(logImageViewer) << "File birth time is invalid, falling back to current time.";
        dt = QDateTime::currentDateTime();
        qCDebug(logImageViewer) << "Using current time as fallback:" << dt;
    }

    return dt;
}

bool imageSupportRead(const QString &path)
{
    qCDebug(logImageViewer) << "Checking if image format is supported for reading:" << path;
    const QString suffix = QFileInfo(path).suffix();
    if (suffix == "icns") {
        qCDebug(logImageViewer) << "ICNS format supported:" << path;
        return true;
    }
    // take them here for good.
    QStringList errorList;
    errorList << "X3F";

    if (errorList.indexOf(suffix.toUpper()) != -1) {
        qCWarning(logImageViewer) << "Unsupported image format:" << suffix;
        return false;
    }
    qCDebug(logImageViewer) << "Image format supported:" << suffix;
    return true;
}

bool imageSupportSave(const QString &path)
{
    qCDebug(logImageViewer) << "Checking if image format is supported for saving:" << path;
    /*lmh0724使用USE_UNIONIMAGE*/
    return LibUnionImage_NameSpace::canSave(path);
}

bool rotate(const QString &path, int degree)
{
    /*lmh0724使用USE_UNIONIMAGE*/
    qCDebug(logImageViewer) << "Rotating image:" << path << "by" << degree << "degrees";
    QString erroMsg;
    bool result = LibUnionImage_NameSpace::rotateImageFile(degree, path, erroMsg);
    if (!result) {
        qCWarning(logImageViewer) << "Failed to rotate image:" << path << "error:" << erroMsg;
    }
    return result;
}

/*!
 * \brief cutSquareImage
 * Cut square image
 * \param pixmap
 * \return
 */
const QPixmap cutSquareImage(const QPixmap &pixmap)
{
    qCDebug(logImageViewer) << "Cutting square image from pixmap (overload 1).";
    return Libutils::image::cutSquareImage(pixmap, pixmap.size());
}

/*!
 * \brief cutSquareImage
 * Scale and cut a square image
 * \param pixmap
 * \param size
 * \return
 */
const QPixmap cutSquareImage(const QPixmap &pixmap, const QSize &size)
{
    qCDebug(logImageViewer) << "Cutting square image, original size:" << pixmap.size() << "target size:" << size;
    const qreal ratio = qApp->devicePixelRatio();
    QImage img = pixmap.toImage().scaled(size * ratio,
                                         Qt::KeepAspectRatioByExpanding,
                                         Qt::SmoothTransformation);
    const QSize s(size * ratio);
    const QRect r(0, 0, s.width(), s.height());

    img = img.copy(QRect(img.rect().center() - r.center(), s));
    img.setDevicePixelRatio(ratio);
    qCDebug(logImageViewer) << "Image scaled and copied, new size:" << img.size();

    return QPixmap::fromImage(img);
}

/*!
 * \brief getImagesInfo
        types<< ".BMP";
        types<< ".GIF";
        types<< ".JPG";
        types<< ".JPEG";
        types<< ".PNG";
        types<< ".PBM";
        types<< ".PGM";
        types<< ".PPM";
        types<< ".XBM";
        types<< ".XPM";
        types<< ".SVG";

        types<< ".DDS";
        types<< ".ICNS";
        types<< ".JP2";
        types<< ".MNG";
        types<< ".TGA";
        types<< ".TIFF";
        types<< ".WBMP";
        types<< ".WEBP";
 * \param dir
 * \param recursive
 * \return
 */
const QFileInfoList getImagesInfo(const QString &dir, bool recursive)
{
    qCDebug(logImageViewer) << "Getting image info for directory:" << dir << "recursive:" << recursive;
    QFileInfoList infos;

    if (!recursive) {
        qCDebug(logImageViewer) << "Getting image info for directory without recursion:" << dir;
        auto nsl = QDir(dir).entryInfoList(QDir::Files);
        for (QFileInfo info : nsl) {
            if (imageSupportRead(info.absoluteFilePath())) {
                infos << info;
                qCDebug(logImageViewer) << "Added supported image:" << info.absoluteFilePath();
            }
        }
        qCDebug(logImageViewer) << "Found" << infos.size() << "images in directory";
        return infos;
    }

    QDirIterator dirIterator(dir,
                             QDir::Files,
                             QDirIterator::Subdirectories);
    qCDebug(logImageViewer) << "Getting image info for directory with recursion:" << dir;
    while (dirIterator.hasNext()) {
        dirIterator.next();
        if (imageSupportRead(dirIterator.fileInfo().absoluteFilePath())) {
            infos << dirIterator.fileInfo();
            qCDebug(logImageViewer) << "Added supported image recursively:" << dirIterator.fileInfo().absoluteFilePath();
        }
    }

    qCDebug(logImageViewer) << "Found" << infos.size() << "images recursively";
    return infos;
}

int getOrientation(const QString &path)
{
    qCDebug(logImageViewer) << "Getting orientation for image:" << path;
    return LibUnionImage_NameSpace::getOrientation(path);
}

/*!
 * \brief getRotatedImage
 * Rotate image base on the exif orientation
 * \param path
 * \return
 */
const QImage getRotatedImage(const QString &path)
{
    qCDebug(logImageViewer) << "Getting rotated image:" << path;
    QImage tImg;
    QString format = LibUnionImage_NameSpace::detectImageFormat(path);
    if (format.isEmpty()) {
        qCDebug(logImageViewer) << "Detected format is empty, using default image reader.";
        QImageReader reader(path);
        reader.setAutoTransform(true);
        if (reader.canRead()) {
            tImg = reader.read();
            qCDebug(logImageViewer) << "Read image with auto transform";
        }
    } else {
        qCDebug(logImageViewer) << "Detected format:" << format << ", using format-specific image reader.";
        QImageReader readerF(path, format.toLatin1());
        readerF.setAutoTransform(true);
        if (readerF.canRead()) {
            tImg = readerF.read();
            qCDebug(logImageViewer) << "Read image with format:" << format;
        } else {
            qCWarning(logImageViewer) << "Can't read image:" << readerF.errorString() << "format:" << format;
            tImg = QImage(path);
        }
    }
    qCDebug(logImageViewer) << "Exiting getRotatedImage. Image null:" << tImg.isNull();
    return tImg;
}

QString size2HumanT(const qlonglong bytes)
{
    qCDebug(logImageViewer) << "Converting bytes to human readable size:" << bytes << "bytes";
    qlonglong kb = 1024;
    if (bytes < kb) {
        qCDebug(logImageViewer) << "Size is less than 1KB, returning in B.";
        return QString::number(bytes) + " B";
    } else if (bytes < kb * kb) {
        qCDebug(logImageViewer) << "Size is less than 1MB, returning in KB.";
        QString vs = QString::number(static_cast<double>(bytes) / kb, 'f', 1);
        if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
            qCDebug(logImageViewer) << "KB value is integer.";
            return QString::number(static_cast<int>(vs.toDouble())) + " KB";
        } else {
            qCDebug(logImageViewer) << "KB value is float.";
            return vs + " KB";
        }
    } else if (bytes < kb * kb * kb) {
        qCDebug(logImageViewer) << "Size is less than 1GB, returning in MB.";
        QString vs = QString::number(static_cast<double>(bytes) / kb / kb, 'f', 1);
        if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
            qCDebug(logImageViewer) << "MB value is integer.";
            return QString::number(static_cast<int>(vs.toDouble())) + " MB";
        } else {
            qCDebug(logImageViewer) << "MB value is float.";
            return vs + " MB";
        }
    } else {
        // 修改了当超过一个G的图片,应该用G返回,不应该返回一堆数字,bug68094
        qCDebug(logImageViewer) << "Size is greater than or equal to 1GB, returning in GB.";
        QString vs = QString::number(static_cast<double>(bytes) / kb / kb / kb, 'f', 1);
        if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
            qCDebug(logImageViewer) << "GB value is integer.";
            return QString::number(static_cast<int>(vs.toDouble())) + " GB";
        } else {
            qCDebug(logImageViewer) << "GB value is float.";
            return vs + " GB";
        }
    }
}

const QMap<QString, QString> getAllMetaData(const QString &path)
{
    qCDebug(logImageViewer) << "Getting metadata for:" << path;
    QMap<QString, QString> admMap;
    // 移除秒　　2020/6/5 DJH
    // 需要转义才能读出：或者/　　2020/8/21 DJH
    QFileInfo info(path);
    if (admMap.contains("DateTime")) {
        qCDebug(logImageViewer) << "DateTime already in map, converting format.";
        QDateTime time = QDateTime::fromString(admMap["DateTime"], "yyyy:MM:dd hh:mm:ss");
        admMap["DateTimeOriginal"] = time.toString("yyyy/MM/dd hh:mm");
    } else {
        qCDebug(logImageViewer) << "DateTime not in map, using last modified time for DateTimeOriginal.";
        admMap.insert("DateTimeOriginal", info.lastModified().toString("yyyy/MM/dd HH:mm"));
    }
    admMap.insert("DateTimeDigitized", info.lastModified().toString("yyyy/MM/dd HH:mm"));
    qCDebug(logImageViewer) << "DateTimeDigitized set to last modified time.";
    //    // The value of width and height might incorrect
    QImageReader reader(path);
    int w = reader.size().width();
    int h = reader.size().height();
    admMap.insert("Dimension", QString::number(w) + "x" + QString::number(h));
    admMap.insert("FileName", info.fileName());
    // 应该使用qfileinfo的格式
    admMap.insert("FileFormat", info.suffix());
    admMap.insert("FileSize", size2HumanT(info.size()));

    qCDebug(logImageViewer) << "Metadata retrieved - dimensions:" << w << "x" << h
                            << "format:" << info.suffix() << "size:" << size2HumanT(info.size());
    return admMap;
}

const QPixmap cachePixmap(const QString &path)
{
    qCDebug(logImageViewer) << "Caching pixmap:" << path;
    QPixmap pp;
    if (!QPixmapCache::find(path, &pp)) {
        qCDebug(logImageViewer) << "Pixmap not found in cache, loading from file.";
        pp = QPixmap(path);
        QPixmapCache::insert(path, pp);
        qCDebug(logImageViewer) << "Added pixmap to cache, size:" << pp.size();
    } else {
        qCDebug(logImageViewer) << "Retrieved pixmap from cache, size:" << pp.size();
    }
    return pp;
}

const QString toMd5(const QByteArray &data)
{
    qCDebug(logImageViewer) << "Calculating MD5 hash for QByteArray, data size:" << data.size();
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

/*!
 * \brief thumbnailAttribute
 * Read the attributes of file for generage thumbnail
 * \param url
 * \return
 */
QMap<QString, QString> thumbnailAttribute(const QUrl &url)
{
    qCDebug(logImageViewer) << "Getting thumbnail attributes for URL:" << url.toString();
    QMap<QString, QString> set;

    if (url.isLocalFile()) {
        qCDebug(logImageViewer) << "URL is a local file, extracting attributes.";
        const QString path = url.path();
        QFileInfo info(path);
        set.insert("Thumb::Mimetype", QMimeDatabase().mimeTypeForFile(path).name());
        set.insert("Thumb::Size", QString::number(info.size()));
        set.insert("Thumb::URI", url.toString());
        set.insert("Thumb::MTime", QString::number(info.lastModified().toSecsSinceEpoch()));
        set.insert("Software", "Deepin Image Viewer");

        QImageReader reader(path);
        if (reader.canRead()) {
            qCDebug(logImageViewer) << "Image reader can read file, extracting dimensions.";
            set.insert("Thumb::Image::Width", QString::number(reader.size().width()));
            set.insert("Thumb::Image::Height", QString::number(reader.size().height()));
        }
    } else {
        qCDebug(logImageViewer) << "URL is not a local file, skipping attribute extraction.";
        // TODO for other's scheme
    }
    qCDebug(logImageViewer) << "Thumbnail attributes collected.";
    return set;
}

const QString thumbnailCachePath()
{
    qCDebug(logImageViewer) << "Determining thumbnail cache path.";
    QString cacheP;

    QStringList systemEnvs = QProcess::systemEnvironment();
    for (QString it : systemEnvs) {
        QStringList el = it.split("=");
        if (el.length() == 2 && el.first() == "XDG_CACHE_HOME") {
            cacheP = el.last();
            qCDebug(logImageViewer) << "XDG_CACHE_HOME environment variable found:" << cacheP;
            break;
        }
    }
    cacheP = cacheP.isEmpty() ? (QDir::homePath() + "/.cache") : cacheP;
    qCDebug(logImageViewer) << "Base cache path:" << cacheP;

    // Check specific size dir
    const QString thumbCacheP = cacheP + "/thumbnails";
    qCDebug(logImageViewer) << "Thumbnail cache base path:" << thumbCacheP;
    QDir().mkpath(thumbCacheP + "/normal");
    QDir().mkpath(thumbCacheP + "/large");
    QDir().mkpath(thumbCacheP + "/fail");
    qCDebug(logImageViewer) << "Created thumbnail subdirectories: normal, large, fail.";

    return thumbCacheP;
}

QMutex mutex;
const QPixmap getThumbnail(const QString &path, bool cacheOnly)
{
    qCDebug(logImageViewer) << "Attempting to get thumbnail for:" << path << ", cacheOnly:" << cacheOnly;
    QMutexLocker locker(&mutex);
    // 优先读取自身缓存的图片
    //    if (dApp->m_imagemap.value(path).isNull()) {
    //        return dApp->m_imagemap.value(path);
    //    }
    const QString cacheP = thumbnailCachePath();
    const QUrl url = QUrl::fromLocalFile(path);
    const QString md5s = toMd5(url.toString(QUrl::FullyEncoded).toLocal8Bit());
    const QString encodePath = cacheP + "/large/" + md5s + ".png";
    const QString failEncodePath = cacheP + "/fail/" + md5s + ".png";
    if (QFileInfo(encodePath).exists()) {
        qCDebug(logImageViewer) << "Large thumbnail found in cache:" << encodePath;
        return QPixmap(encodePath);
    }
    /*lmh0724使用USE_UNIONIMAGE*/
    else if (QFileInfo(failEncodePath).exists()) {
        qCWarning(logImageViewer) << "Fail-thumbnail exists, won't regenerate for:" << path;
        qDebug() << "Fail-thumbnail exist, won't regenerate: ";
        return QPixmap();
    } else {
        qCDebug(logImageViewer) << "Thumbnail not found, attempting to generate (if not cacheOnly).";
        // Try to generate thumbnail and load it later
        if (!cacheOnly && generateThumbnail(path)) {
            qCDebug(logImageViewer) << "Thumbnail generated successfully, loading from:" << encodePath;
            return QPixmap(encodePath);
        } else {
            qCDebug(logImageViewer) << "Thumbnail not generated (either cacheOnly or generation failed).";
            return QPixmap();
        }
    }
}

/*!
 * \brief generateThumbnail
 * Generate and save thumbnail for specific size
 * \return
 */
bool generateThumbnail(const QString &path)
{
    qCDebug(logImageViewer) << "Generating thumbnail for:" << path;
    const QUrl url = QUrl::fromLocalFile(path);
    const QString md5 = toMd5(url.toString(QUrl::FullyEncoded).toLocal8Bit());
    const auto attributes = thumbnailAttribute(url);
    const QString cacheP = thumbnailCachePath();

    // Large thumbnail
    QImage lImg = scaleImage(path,
                             QSize(THUMBNAIL_MAX_SIZE, THUMBNAIL_MAX_SIZE));
    qCDebug(logImageViewer) << "Large thumbnail image scaled. Is null:" << lImg.isNull();

    // Normal thumbnail
    QImage nImg = lImg.scaled(
            QSize(THUMBNAIL_NORMAL_SIZE, THUMBNAIL_NORMAL_SIZE), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    qCDebug(logImageViewer) << "Normal thumbnail image scaled. Is null:" << nImg.isNull();

    // Create filed thumbnail
    if (lImg.isNull() || nImg.isNull()) {
        qCWarning(logImageViewer) << "Failed to generate large or normal thumbnail, creating failure marker.";
        const QString failedP = cacheP + "/fail/" + md5 + ".png";
        QImage img(1, 1, QImage::Format_ARGB32_Premultiplied);
        const auto keys = attributes.keys();
        for (QString key : keys) {
            img.setText(key, attributes[key]);
        }

        qCWarning(logImageViewer) << "Failed to generate thumbnail, saving failure marker:" << failedP;
        img.save(failedP, "png");
        return false;
    } else {
        qCDebug(logImageViewer) << "Thumbnails generated successfully, saving attributes.";
        for (QString key : attributes.keys()) {
            lImg.setText(key, attributes[key]);
            nImg.setText(key, attributes[key]);
        }
        const QString largeP = cacheP + "/large/" + md5 + ".png";
        const QString normalP = cacheP + "/normal/" + md5 + ".png";
        if (lImg.save(largeP, "png", 50) && nImg.save(normalP, "png", 50)) {
            qCDebug(logImageViewer) << "Successfully generated thumbnails - large:" << largeP << "normal:" << normalP;
            return true;
        } else {
            qCWarning(logImageViewer) << "Failed to save thumbnails";
            return false;
        }
    }
}

const QString thumbnailPath(const QString &path, ThumbnailType type)
{
    qCDebug(logImageViewer) << "Getting thumbnail path for:" << path << "type:" << type;
    const QString cacheP = thumbnailCachePath();
    const QUrl url = QUrl::fromLocalFile(path);
    const QString md5s = toMd5(url.toString(QUrl::FullyEncoded).toLocal8Bit());
    QString tp;
    switch (type) {
    case ThumbNormal:
        tp = cacheP + "/normal/" + md5s + ".png";
        qCDebug(logImageViewer) << "Normal thumbnail path:" << tp;
        break;
    case ThumbLarge:
        tp = cacheP + "/large/" + md5s + ".png";
        qCDebug(logImageViewer) << "Large thumbnail path:" << tp;
        break;
    case ThumbFail:
        tp = cacheP + "/fail/" + md5s + ".png";
        qCDebug(logImageViewer) << "Fail thumbnail path:" << tp;
        break;
    default:
        break;
    }
    qCDebug(logImageViewer) << "Thumbnail path:" << tp;
    return tp;
}

void removeThumbnail(const QString &path)
{
    qCDebug(logImageViewer) << "Removing thumbnails for:" << path;
    QFile(thumbnailPath(path, ThumbLarge)).remove();
    QFile(thumbnailPath(path, ThumbNormal)).remove();
    QFile(thumbnailPath(path, ThumbFail)).remove();
    qCDebug(logImageViewer) << "Thumbnails removed successfully";
}

bool thumbnailExist(const QString &path, ThumbnailType type)
{
    qCDebug(logImageViewer) << "Checking if thumbnail exists for:" << path << "type:" << type;
    if (QFileInfo(thumbnailPath(path, type)).exists()
        //            || QFileInfo(thumbnailPath(path, ThumbNormal)).exists()
        //            || QFileInfo(thumbnailPath(path, ThumbFail)).exists()
    ) {
        qCDebug(logImageViewer) << "Thumbnail exists for:" << path << "type:" << type;
        return true;
    } else {
        qCDebug(logImageViewer) << "Thumbnail does not exist for:" << path << "type:" << type;
        return false;
    }
}
/*
static QStringList fromByteArrayList(const QByteArrayList &list)
{
    QStringList sList;

    for (const QByteArray &i : list)
        sList << "*." + QString::fromLatin1(i);

    // extern image format
    sList << "*.cr2"
          << "*.dng"
          << "*.nef"
          << "*.mef"
          << "*.raf"
          << "*.raw"
          << "*.orf"
          << "*.mrw"
          << "*.jpe"
          << "*.xbm";

    return sList;
}
*/
QStringList supportedImageFormats()
{
    qCDebug(logImageViewer) << "Getting supported image formats";
    /*lmh0724使用USE_UNIONIMAGE*/
    QStringList list;
    for (auto str : LibUnionImage_NameSpace::unionImageSupportFormat()) {
        str = "*." + str;
        list += str;
    }
    qCDebug(logImageViewer) << "Supported image formats:" << list;
    return list;
}

bool imageSupportWallPaper(const QString &path)
{
    qCDebug(logImageViewer) << "Checking wallpaper support for:" << path;
    bool iRet = false;
    QStringList listsupportWallPaper;
    listsupportWallPaper << "bmp"
                         //                         << "cod"
                         << "png"
                         << "gif"
                         << "ief"
                         << "jpe"
                         << "jpeg"
                         << "jpg"
                         << "jfif"
                         //                         << "svg"
                         << "tif"
                         << "tiff"
            //                         << "ras"
            //                         << "cmx"
            //                         << "ico"
            //                         << "pnm"
            //                         << "pbm"
            //                         << "pgm"
            //                         << "ppm"
            //                         << "rgb"
            //                         << "xbm"
            //                         << "xpm"
            //                         << "xwd"
            ;
    //
    QImageReader reader(path);
    if (reader.imageCount() > 0) {
        // 2020/11/12 bug54279
        if (listsupportWallPaper.contains(reader.format().toLower()) && listsupportWallPaper.contains(QFileInfo(path).suffix().toLower())) {
            iRet = true;
            qCDebug(logImageViewer) << "Image format supported for wallpaper:" << reader.format();
        } else {
            qCDebug(logImageViewer) << "Image format not supported for wallpaper:" << reader.format();
        }
    }
    qCDebug(logImageViewer) << "Wallpaper support check result:" << iRet;
    return iRet;
}

// bool suffixisImage(const QString &path)
//{
// #ifdef USE_UNIONIMAGE
//     return UnionImage_NameSpace::suffixisImage(path);
// #else
//     bool iRet = false;
//     QFileInfo info(path);
//     QMimeDatabase db;
//     QMimeType mt = db.mimeTypeForFile(path, QMimeDatabase::MatchContent);
//     QMimeType mt1 = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension);
//     QString str = info.suffix();
//     // if (!m_nosupportformat.contains(str, Qt::CaseSensitive)) {
//     if (mt.name().startsWith("image/") || mt.name().startsWith("video/x-mng") ||
//             mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
//         iRet = true;
//     }
//     return iRet;
// #endif
// }

QString makeVaultLocalPath(const QString &path, const QString &base)
{
    qCDebug(logImageViewer) << "Making vault local path for:" << path << "base:" << base;
    QString basePath = base;
    if (basePath.isEmpty()) {
        basePath = VAULT_DECRYPT_DIR_NAME;
    }
    QString result = VAULT_BASE_PATH + QDir::separator() + basePath + (path.startsWith('/') ? "" : "/") + path;
    qCDebug(logImageViewer) << "Vault local path:" << result;
    return result;
}

bool isVaultFile(const QString &path)
{
    qCDebug(logImageViewer) << "Checking if file is in vault:" << path;
    bool bVaultFile = false;
    QString rootPath = makeVaultLocalPath("", "");
    if (rootPath.back() == QChar('/')) {
        qCDebug(logImageViewer) << "Removing trailing slash from root path";
        rootPath.chop(1);
    }

    if (path.contains(rootPath) && path.left(6) != "search") {
        qCDebug(logImageViewer) << "File is in vault:" << path;
        bVaultFile = true;
    }

    qCDebug(logImageViewer) << "Vault file check result:" << bVaultFile;
    return bVaultFile;
}
bool isCanRemove(const QString &path)
{
    qCDebug(logImageViewer) << "Checking if file can be removed:" << path;
    bool bRet = true;
    QString trashPath = QDir::homePath() + "/.local/share/Trash";
    // 新增保险箱的判断,回收站判断
    if (isVaultFile(path) || path.contains(trashPath)) {
        qCDebug(logImageViewer) << "File is in vault or trash path, cannot be removed";
        bRet = false;
    }
    qCDebug(logImageViewer) << "File can be removed check result:" << bRet;
    return bRet;
}
}   // namespace image

}   // namespace utils
