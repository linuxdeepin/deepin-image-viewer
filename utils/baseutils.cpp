#include "baseutils.h"
#include <QDateTime>
#include <QDesktopServices>
#include <QFontMetrics>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>

namespace utils {

namespace base {

const QString DATETIME_FORMAT_NORMAL = "yyyy.MM.dd";
const QString DATETIME_FORMAT_EXIF = "yyyy:MM:dd HH:mm:ss";

QString sizeToHuman(const qlonglong bytes)
{
    qlonglong sb = 1024;
    if (bytes < sb) {
        return QString::number(bytes) + " B";
    }
    else if (bytes < sb * sb) {
        return QString::number((double)bytes / sb, 'f', 1) + " KB";
    }
    else if (bytes < sb * sb * sb) {
        return QString::number((double)bytes / sb / sb, 'f', 1) + " MB";
    }
    else {
        return QString::number(bytes);
    }
}

QString timeToString(const QDateTime &time)
{
    return time.toString(DATETIME_FORMAT_NORMAL);
}

QString formatExifTimeString(const QString &exifTimeStr)
{
    QDateTime dt = QDateTime::fromString(exifTimeStr, DATETIME_FORMAT_EXIF);
    return dt.toString(DATETIME_FORMAT_NORMAL);
}

int stringWidth(const QFont &f, const QString &str)
{
    QFontMetrics fm(f);
    return fm.width(str);
}

QDateTime stringToDateTime(const QString &time)
{
    QDateTime dt = QDateTime::fromString(time, DATETIME_FORMAT_NORMAL);
    if (! dt.isValid()) {
        dt = QDateTime::fromString(time, DATETIME_FORMAT_EXIF);
    }
    return dt;
}

void showInFileManager(const QString &path)
{
    if (! path.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).path()));
}


}  // namespace base

}  // namespace utils
