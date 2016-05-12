#include "baseutils.h"
#include <QDateTime>
#include <QFontMetrics>
#include <QDebug>

namespace utils {

namespace base {

const QString DATETIME_FORMAT = "yyyy.MM.dd";

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
    return time.toString(DATETIME_FORMAT);
}

QString formatExifTimeString(const QString &exifTimeStr)
{
    QDateTime dt = QDateTime::fromString(exifTimeStr, "yyyy:MM:dd HH:mm:ss");
    return dt.toString(DATETIME_FORMAT);
}

int stringWidth(const QFont &f, const QString &str)
{
    QFontMetrics fm(f);
    return fm.width(str);
}

QDateTime stringToDateTime(const QString &time)
{
    QDateTime dt = QDateTime::fromString(time, DATETIME_FORMAT);
    return dt;
}


}  // namespace base

}  // namespace utils
