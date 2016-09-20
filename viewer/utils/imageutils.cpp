#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "utils/imageutils_libexif.h"
#include "utils/imageutils_freeimage.h"
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImage>
#include <QPixmapCache>
#include <QSvgRenderer>

namespace utils {

namespace image {

const QPixmap getThumbnail(const QString &path, bool exifOnly)
{
    Q_UNUSED(exifOnly)
    auto bitmap = freeimage::makeThumbnail(path, THUMBNAIL_MAX_SIZE);
    if (getOrientation(path).isEmpty() && bitmap != NULL) {
        return QPixmap::fromImage(freeimage::FIBitmapToQImage(bitmap));
    }
    else {
        return scaleImage(path);
    }
}

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
    return v;
}

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

}  // namespace image

}  //namespace utils
