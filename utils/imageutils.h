#ifndef IMGUTIL_H
#define IMGUTIL_H

#include "baseutils.h"
#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-tag.h>
#include <QPixmap>
#include <QDateTime>

namespace utils {

namespace image {

enum ExifExtendTag {
    EXIF_TAG_EXTEND_NAME = EXIF_TAG_PADDING << 1,
    EXIF_TAG_EXTEND_WIDTH  = EXIF_TAG_PADDING << 2,
    EXIF_TAG_EXTEND_HEIGHT  = EXIF_TAG_PADDING << 3,
    EXIF_TAG_EXTEND_SIZE  = EXIF_TAG_PADDING << 4,
    EXIF_TAG_EXTEND_FLASH_COMPENSATION  = EXIF_TAG_PADDING << 5,  // Contain by MakerNote
    EXIF_TAG_EXTEND_LENS_MODEL  = EXIF_TAG_PADDING << 6 // Contain by MakerNote
};

struct ExifItem {
    ExifIfd ifd;
    int tag;
    const char* name;
};

ExifItem *getExifItemList(bool isDetails);
const QStringList supportImageTypes();
bool imageIsSupport(const QString &filepath);
QPixmap getThumbnail(const QString &filePath);
QPixmap scaleImage(const QString &filePath);
QDateTime getCreateDateTime(const QString &filePath);

QString readExifTag(ExifData *ed, ExifIfd eid, ExifTag tag);
QMap<QString, QString> GetExifFromPath(const QString& path, bool isDetails);

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

}  // namespace image

}  // namespace utils

#endif // IMGUTIL_H
