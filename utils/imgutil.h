#ifndef IMGUTIL_H
#define IMGUTIL_H

#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-tag.h>
#include <QPixmap>
#include <QDateTime>

namespace utils {

QPixmap getThumbnail(const QString &filePath);
QPixmap scaleImage(const QString &filePath);
QDateTime getCreateDateTime(const QString &filePath);

QString readExifTag(ExifData *ed, ExifIfd eid, ExifTag tag);
/*!
 * \brief saveImageWithExif
 * Read exif from sourcePath.
 */
void saveImageWithExif(const QImage& image,
                       const QString& path,
                       const QString& sourcePath = QString(),
                       const QTransform& mat = QTransform());

QImage saturation(int delta, QImage &origin);
QImage cool(int delta, QImage &origin);
QImage warm(int delta, QImage &origin);
QImage blure(const QImage &origin);
}  // namespace utils
#endif // IMGUTIL_H
