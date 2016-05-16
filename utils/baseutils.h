#ifndef BASEUTILS_H
#define BASEUTILS_H

#include <QObject>

namespace utils {

namespace base {

void        copyImageToClipboard(const QString &path);
void        showInFileManager(const QString &path);
int         stringWidth(const QFont &f, const QString &str);

QString     sizeToHuman(const qlonglong bytes);
QString     timeToString(const QDateTime &time);
QDateTime   stringToDateTime(const QString &time);
QString     formatExifTimeString(const QString &exifTimeStr);

}  // namespace base

}  // namespace utils

#endif // BASEUTILS_H
