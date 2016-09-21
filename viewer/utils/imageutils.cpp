#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "utils/imageutils_libexif.h"
#include "utils/imageutils_freeimage.h"
#include <QBuffer>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QMimeDatabase>
#include <QPixmapCache>
#include <QProcess>
#include <QSvgRenderer>
#include <QUrl>

namespace utils {

namespace image {

const QPixmap scaleImage(const QString &path, const QSize &size)
{
    QImage img = getRotatedImage(path);
    if (img.isNull())
        return QPixmap();
    QSize targetSize;
    if (img.width() > img.height()) {
        targetSize = QSize(size.width(),
                           (double)size.width() / img.width() *
                           img.height());
    }
    else {
        targetSize = QSize((double)size.height() / img.height() *
                           img.width(), size.height());
    }

    // pre-scale improve performance
    const QImage tmpImg = img.scaled(targetSize * 2,
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
    return QPixmap::fromImage(tmpImg.scaled(targetSize,
                                            Qt::IgnoreAspectRatio,
                                            Qt::SmoothTransformation));
}

const QDateTime getCreateDateTime(const QString &path)
{
    return libexif::getCreateDateTime(path);
}

bool isImageSupported(const QString &path)
{
    if (freeimage::isSupportsReading(path))
        return true;
    else
        return QSvgRenderer().load(path);
}

bool rotate(const QString &path, int degree)
{
    if (degree % 90 != 0)
        return false;
    FIBITMAP *dib = freeimage::readFileToFIBITMAP(path);
    FIBITMAP *rotated = FreeImage_Rotate(dib, -degree);
    bool v = freeimage::writeFIBITMAPToFile(rotated, path);
    FreeImage_Unload(dib);
    FreeImage_Unload(rotated);

    // The thumbnail should regenerate by caller
    removeThumbnail(path);

    return v;
}

/*!
 * \brief cutSquareImage
 * Cut square image
 * \param pixmap
 * \return
 */
const QPixmap cutSquareImage(const QPixmap &pixmap)
{
    const int minL = qMin(pixmap.width(), pixmap.height());
    QImage img = pixmap.toImage();
    img = img.copy((img.width() - minL) / 2,
                   (img.height() - minL) / 2,
                   minL, minL);
    return QPixmap::fromImage(img);
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
    QImage img = pixmap.toImage().scaled(size,
                                         Qt::KeepAspectRatioByExpanding,
                                         Qt::SmoothTransformation);

    img = img.copy((img.width() - size.width()) / 2,
                   (img.height() - size.height()) / 2,
                   size.width(), size.height());
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

    QFileInfoList infos;

    if (! recursive) {
        auto nsl = QDir(dir).entryInfoList(QDir::Files | QDir::NoSymLinks);
        for (QFileInfo info : nsl) {
            if (isImageSupported(info.absoluteFilePath())) {
                infos << info;
            }
        }
        return infos;
    }

    QDirIterator dirIterator(dir,
                             QDir::Files | QDir::NoSymLinks,
                             QDirIterator::Subdirectories);
    while(dirIterator.hasNext())
    {
        dirIterator.next();
        if (isImageSupported(dirIterator.fileInfo().absoluteFilePath())) {
            infos << dirIterator.fileInfo();
        }
    }

    return infos;
}

const QString getOrientation(const QString &path)
{
    return libexif::orientation(path);
}

/*!
 * \brief getRotatedImage
 * Rotate image base on the exif orientation
 * \param path
 * \return
 */
const QImage getRotatedImage(const QString &path)
{
    // FIXME it should read the orientation enum value
    QImage img(path);

    const QString o = getOrientation(path);
    if (o.isEmpty() || o == "Top-left")
        return img;
    QTransform t;
    if (o == "Bottom-right") {
        t.rotate(-180);
    }
    else if (o == "Left-bottom") {
        t.rotate(-90);
    }
    else if (o == "Right-top") {
        t.rotate(90);
    }
    img = img.transformed(t, Qt::SmoothTransformation);

    return img;
}

const QMap<QString, QString> getAllMetaData(const QString &path)
{
    return freeimage::getAllMetaData(path);
}

const QPixmap cachePixmap(const QString &path)
{
    QPixmap pp;
    if (! QPixmapCache::find(path, &pp)) {
        pp = QPixmap(path);
        QPixmapCache::insert(path, pp);
    }
    return pp;
}

const QString toMd5(const QString data)
{
    QByteArray bdata;
    bdata.append(data);
    QString md5 = QCryptographicHash::hash(bdata, QCryptographicHash::Md5)
            .toHex().data();
    return md5;
}

/*!
 * \brief thumbnailAttribute
 * Read the attributes of file for generage thumbnail
 * \param url
 * \return
 */
QMap<QString,QString> thumbnailAttribute(const QUrl&  url)
{
    QMap<QString,QString> set;

    if(url.isLocalFile()) {
        const QString path = url.path();
        QFileInfo info(path);
        set.insert("Thumb::Mimetype", QMimeDatabase().mimeTypeForFile(path).name());
        set.insert("Thumb::Size", QString::number(info.size()));
        set.insert("Thumb::URI", url.toString());
        set.insert("Thumb::MTime", QString::number(info.lastModified().toMSecsSinceEpoch()/1000));
        set.insert("Software", "Deepin Image Viewer");

        QImageReader reader(path);
        if(reader.canRead()){
            set.insert("Thumb::Image::Width", QString::number(reader.size().width()));
            set.insert("Thumb::Image::Height", QString::number(reader.size().height()));
        }
        return set;
    }
    else{
        //TODO for other's scheme
    }

    return set;
}

const QString thumbnailCachePath()
{
    QString cacheP;

    QStringList systemEnvs = QProcess::systemEnvironment();
    for (QString it : systemEnvs) {
        QStringList el = it.split("=");
        if(el.length() == 2 && el.first() == "XDG_CACHE_HOME") {
            cacheP = el.last();
            break;
        }
    }
    cacheP = cacheP.isEmpty() ? (QDir::homePath() + "/.cache") : cacheP;

    // Check specific size dir
    const QString thumbCacheP = cacheP + "/thumbnails";
    QDir().mkpath(thumbCacheP + "/normal");
    QDir().mkpath(thumbCacheP + "/large");
    QDir().mkpath(thumbCacheP + "/fail");

    return thumbCacheP;
}

const QPixmap getThumbnail(const QString &path, bool cacheOnly)
{
    const QString cacheP = thumbnailCachePath();
    const QString md5s = toMd5("file://" + path);
    const QString encodePath = cacheP + "/large/" + md5s + ".png";
    const QString failEncodePath = cacheP + "/fail/" + md5s + ".png";
    if (QFileInfo(encodePath).exists()) {
        return QPixmap(encodePath);
    }
    else if (QFileInfo(failEncodePath).exists()) {
        return QPixmap();
    }
    else {
        // Try to generate thumbnail and load it later
        if (! cacheOnly && generateThumbnail(path)) {
            return QPixmap(encodePath);
        }
        else {
            return QPixmap();
        }
    }
//    if (getOrientation(path).isEmpty() && ! highQuality) {
//        auto bitmap = freeimage::makeThumbnail(path, THUMBNAIL_MAX_SIZE);
//        if (bitmap != NULL) {
//            return QPixmap::fromImage(freeimage::FIBitmapToQImage(bitmap));
//        }
//    }

//    return scaleImage(path);
}

/*!
 * \brief generateThumbnail
 * Generate and save thumbnail for specific size
 * \return
 */
bool generateThumbnail(const QString &path)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    if (! reader.canRead()) return false;

    const QUrl url("file://" + path);
    const QString md5 = toMd5(url.toString());
    const auto attributes = thumbnailAttribute(url);
    const QString cacheP = thumbnailCachePath();

    // Large thumbnail
    QSize lSize = reader.size();
    lSize.scale(QSize(qMin(THUMBNAIL_MAX_SIZE, lSize.width()),
                      qMin(THUMBNAIL_MAX_SIZE, lSize.height())),
                Qt::KeepAspectRatio);
    reader.setScaledSize(lSize);
    QImage lImg = reader.read();

    // Normal thumbnail
    QImage nImg = lImg.scaled(
                QSize(THUMBNAIL_NORMAL_SIZE, THUMBNAIL_NORMAL_SIZE)
                , Qt::KeepAspectRatio
                , Qt::SmoothTransformation);

    // Create filed thumbnail
    if(lImg.isNull() || nImg.isNull()) {
        const QString failedP = cacheP + "/fail/" + md5 + ".png";
        QImage img(1,1,QImage::Format_ARGB32_Premultiplied);
        const auto keys = attributes.keys();
        for (QString key : keys) {
            img.setText(key, attributes[key]);
        }

        qDebug()<<"Save failed thumbnail:" << img.save(failedP,  "png")
               << failedP << url;
        return false;
    }
    else {
        for (QString key : attributes.keys()) {
            lImg.setText(key, attributes[key]);
            nImg.setText(key, attributes[key]);
        }
        const QString largeP = cacheP + "/large/" + md5 + ".png";
        const QString normalP = cacheP + "/normal/" + md5 + ".png";
        if (lImg.save(largeP, "png", 50) && nImg.save(normalP, "png", 50)) {
            return true;
        }
        else {
            return false;
        }
    }
}

const QString thumbnailPath(const QString &path, ThumbnailType type)
{
    const QString cacheP = thumbnailCachePath();
    const QString md5s = toMd5("file://" + path);
    QString tp;
    switch (type) {
    case ThumbNormal:
        tp = cacheP + "/normal/" + md5s + ".png";
        break;
    case ThumbLarge:
        tp = cacheP + "/large/" + md5s + ".png";
        break;
    case ThumbFail:
        tp = cacheP + "/fail/" + md5s + ".png";
        break;
    default:
        break;
    }
    return tp;
}

void removeThumbnail(const QString &path)
{
    QFile(thumbnailPath(path, ThumbLarge)).remove();
    QFile(thumbnailPath(path, ThumbNormal)).remove();
    QFile(thumbnailPath(path, ThumbFail)).remove();
}

bool thumbnailExist(const QString &path)
{
    if (QFileInfo(thumbnailPath(path, ThumbLarge)).exists() ||
//            QFileInfo(thumbnailPath(path, ThumbNormal)).exists() ||
            QFileInfo(thumbnailPath(path, ThumbFail)).exists()) {
        return true;
    }
    else {
        return false;
    }
}

}  // namespace image

}  //namespace utils
