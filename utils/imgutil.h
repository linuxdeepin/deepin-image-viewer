#ifndef IMGUTIL_H
#define IMGUTIL_H

#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-tag.h>
#include <QPixmap>
#include <QDateTime>

namespace utils {

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

static ExifItem ExifDataBasics[] = {
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_NAME, "Name"},
    {EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL, "Date photoed"},
    {EXIF_IFD_0, EXIF_TAG_DATE_TIME, "Date modified"},
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_WIDTH, "Width (pixel)"},
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_HEIGHT, "Height (pixel)"},
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_SIZE, "File size"},
    {EXIF_IFD_COUNT, 0, 0}
};

static ExifItem ExifDataDetails[] = {
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_NAME, "Name"},
    {EXIF_IFD_0, EXIF_TAG_MAKE, QT_TR_NOOP("Manufacture")},
    {EXIF_IFD_0, EXIF_TAG_MODEL, QT_TR_NOOP("Model")},
    {EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL, "Date photoed"},
    {EXIF_IFD_0, EXIF_TAG_DATE_TIME, "Date modified"},
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_WIDTH, "Width (pixel)"},
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_HEIGHT, "Height (pixel)"},
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_SIZE, "File size"},
    {EXIF_IFD_EXIF, EXIF_TAG_COLOR_SPACE, "Colorspace"},
    {EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_MODE, QT_TR_NOOP("Exposure mode")},
    {EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_PROGRAM, "Exposure program"},
    {EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_TIME, "Exposure time"},
    {EXIF_IFD_EXIF, EXIF_TAG_FLASH, "Flash"},
    {EXIF_IFD_EXIF, EXIF_TAG_APERTURE_VALUE, "Aperture"},
    {EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH, "Focal length"},
    {EXIF_IFD_EXIF, EXIF_TAG_ISO_SPEED_RATINGS, "ISO"},
    {EXIF_IFD_EXIF, EXIF_TAG_MAX_APERTURE_VALUE, "Max aperture"},
    {EXIF_IFD_EXIF, EXIF_TAG_METERING_MODE, "Metering mode"},
    {EXIF_IFD_EXIF, EXIF_TAG_WHITE_BALANCE, "White balance"},
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_FLASH_COMPENSATION, "Flash compensation"},
    {EXIF_IFD_COUNT, EXIF_TAG_EXTEND_LENS_MODEL, "Lens model"},
    {EXIF_IFD_COUNT, 0, 0}
};

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

}  // namespace utils
#endif // IMGUTIL_H
