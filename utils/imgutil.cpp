#include "utils/imgutil.h"
#include <QDebug>
#include <QFileInfo>

namespace {

const int THUMBNAIL_MAX_SIZE = 384;

}  // namespace
namespace utils {

QPixmap getThumbnail(const QString &filePath)
{
    QPixmap pixmap;
    ExifData *ed = exif_data_new_from_file(filePath.toUtf8().data());
    if (ed) {
        // Make sure the image had a thumbnail before trying to write it
        if (ed->data && ed->size) {
            pixmap.loadFromData(ed->data, ed->size);
            // Free the EXIF data
            exif_data_unref(ed);
        } else {
            qDebug() << QString("NO Exif thumbnail in file %1!").arg(filePath);
            pixmap = scaleImage(filePath);
        }
    }
    else {
        pixmap = scaleImage(filePath);
    }

    return pixmap;
}

QPixmap scaleImage(const QString &filePath)
{
    QImage img(filePath);
    QSize targetSize;
    if (img.width() > img.height()) {
        targetSize = QSize(THUMBNAIL_MAX_SIZE,
                           (double)THUMBNAIL_MAX_SIZE / img.width() *
                           img.height());
    }
    else {
        targetSize = QSize((double)THUMBNAIL_MAX_SIZE / img.height() *
                           img.width(), THUMBNAIL_MAX_SIZE);
    }

    return QPixmap::fromImage(img.scaled(targetSize * 2)
                              .scaled(targetSize, Qt::IgnoreAspectRatio,
                                      Qt::SmoothTransformation));
}

QDateTime getCreateDateTime(const QString &filePath)
{
    ExifData *ed = exif_data_new_from_file(filePath.toUtf8().data());
    if (ed) {
        QDateTime dt;
        dt = QDateTime::fromString(readExifTag(ed,
                                               EXIF_IFD_0, EXIF_TAG_DATE_TIME),
                                   "yyyy:MM:dd HH:mm:ss");
        //Free the EXIF data
        exif_data_unref(ed);
        return dt;
    }
    else {
        QFileInfo info(filePath);
        return info.created();
    }
}

QString readExifTag(ExifData *ed, ExifIfd eid, ExifTag tag)
{
    ExifEntry *entry = exif_content_get_entry(ed->ifd[eid], tag);

    if (entry){
        char buf[1024];
        exif_entry_get_value(entry, buf, sizeof(buf));

        if (*buf) {
            return QString(buf).trimmed();
        }
    }

    return QString();
}

}
