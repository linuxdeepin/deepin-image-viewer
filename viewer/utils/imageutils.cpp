#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "utils/imageutils_freeimage.h"
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QBuffer>
#include <QImage>

namespace utils {

namespace image {

const QPixmap getThumbnail(const QString &path, bool exifOnly)
{
    Q_UNUSED(exifOnly)
    return QPixmap::fromImage(
                freeimage::FIBitmapToQImage(
                    freeimage::makeThumbnail(path, THUMBNAIL_MAX_SIZE)));
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
    const QString dts = freeimage::getAllMetaData(path)["DateTimeOriginal"];
    if (! dts.isEmpty()) {
        QDateTime dt = utils::base::stringToDateTime(dts);
        if (dt.isValid()) {
            return dt;
        }
    }

    QFileInfo info(path);
    return info.created();
}

bool isImageSupported(const QString &path)
{
    return freeimage::isSupportsReading(path);
}

bool rotate(const QString &path, int degree)
{
//    const QTransform t = QTransform().rotate(degree);
//    saveImageWithExif(QImage(path).transformed(t), path, path, t);
    if (degree % 90 != 0)
        return false;
    FIBITMAP *dib = freeimage::readFileToFIBITMAP(path);
    FIBITMAP *rotated = FreeImage_Rotate(dib, degree);
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

const QFileInfoList getImagesInfo(const QString &dir, bool recursive)
{
    QFileInfoList infos;

//    QStringList types;
//    types<< ".BMP";
//    types<< ".GIF";
//    types<< ".JPG";
//    types<< ".JPEG";
//    types<< ".PNG";
//    types<< ".PBM";
//    types<< ".PGM";
//    types<< ".PPM";
//    types<< ".XBM";
//    types<< ".XPM";
//    types<< ".SVG";

//    types<< ".DDS";
//    types<< ".ICNS";
//    types<< ".JP2";
//    types<< ".MNG";
//    types<< ".TGA";
//    types<< ".TIFF";
//    types<< ".WBMP";
//    types<< ".WEBP";

    if (! recursive) {
        infos = QDir(dir).entryInfoList(QDir::Files | QDir::NoSymLinks);
        for (QFileInfo info : infos) {
            if (! freeimage::isSupportsReading(info.absoluteFilePath())) {
                infos.removeAll(info);
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
        if (freeimage::isSupportsReading(
                    dirIterator.fileInfo().absoluteFilePath())) {
            infos << dirIterator.fileInfo();
        }
    }

    return infos;
}

const QString getOrientation(const QString &path)
{
    return freeimage::getAllMetaData(path)["Orientation"];
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
    if (o.isEmpty() || o == "top, left side")
        return img;
    QTransform t;
    if (o == "bottom, right side") {
        t.rotate(-180);
    }
    else if (o == "left, bottom side") {
        t.rotate(-90);
    }
    else if (o == "right, top side") {
        t.rotate(90);
    }
    img = img.transformed(t, Qt::SmoothTransformation);

    return img;
}

const QMap<QString, QString> getAllMetaData(const QString &path)
{
    return freeimage::getAllMetaData(path);
}

}  // namespace image

}  //namespace utils
