#ifndef IMAGEUTILS_LIBEXIF_H
#define IMAGEUTILS_LIBEXIF_H

#include "baseutils.h"
#include <exif-data.h>
#include <QDateTime>
#include <QFileInfo>
#include <QString>

#endif // IMAGEUTILS_LIBEXIF_H

namespace utils {

namespace image {

namespace libexif {

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


QDateTime getCreateDateTime(const QString &path)
{
    ExifData *ed = exif_data_new_from_file(path.toUtf8().data());
    if (ed) {
        QDateTime dt = utils::base::stringToDateTime(
                    readExifTag(ed, EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL));
        //Free the EXIF data
        exif_data_unref(ed);
        if (dt.isValid())
            return dt;
    }
    QFileInfo info(path);
    return info.created();
}

}  // namespace libexif

}  // namespace image

}  // namespace utils
