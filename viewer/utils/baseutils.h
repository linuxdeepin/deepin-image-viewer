#ifndef BASEUTILS_H
#define BASEUTILS_H

#include <QObject>
#include <QTimer>
#include <QColor>

#if QT_VERSION >= 0x050500
#define TIMER_SINGLESHOT(Time, Code, captured...){ \
    QTimer::singleShot(Time, [captured] {Code});\
}
#else
#define TIMER_SINGLESHOT(Time, Code, captured...){ \
    QTimer *timer = new QTimer;\
        timer->setSingleShot(true);\
        QObject::connect(timer, &QTimer::timeout, [timer, captured] {\
        timer->deleteLater();\
        Code\
    });\
    timer->start(Time);\
}

#endif

namespace utils {
namespace common {
const QColor DARK_BACKGROUND_COLOR = QColor("#202020");
const QColor LIGHT_BACKGROUND_COLOR = QColor("#FFFFFF");
const QColor LIGHT_CHECKER_COLOR = QColor("#FFFFFF");
const QColor DARK_CHECKER_COLOR = QColor("#CCCCCC");
const QColor BORDER_COLOR_SELECTED = QColor("#01bdff");
const QColor SELECTED_RECT_COLOR = QColor(44, 167, 248, 26);
}
namespace timeline {
const QString DARK_DEFAULT_THUMBNAIL = ":/resources/dark/images/default_thumbnail.png";
const QString LIGHT_DEFAULT_THUMBNAIL = ":/resources/light/images/default_thumbnail.png";
const QColor DARK_BORDER_COLOR = QColor(255, 255, 255, 35);
const QColor LIGHT_BORDER_COLOR = QColor(0, 0, 0, 35);

const QColor DARK_DATECOLOR = QColor("#FFFFFF");
const QColor LIGHT_DATECOLOR = QColor(48, 48, 48);

const QColor DARK_SEPERATOR_COLOR = QColor(255, 255, 255, 20);
const QColor LIGHT_SEPERATOR_COLOR = QColor(0, 0, 0, 20);
}
namespace base {
void        copyOneImageToClipboard(const QString& path);
void        copyImageToClipboard(const QStringList &paths);
void        showInFileManager(const QString &path);
int         stringWidth(const QFont &f, const QString &str);
int         stringHeight(const QFont &f, const QString &str);

QString     hash(const QString &str);
QString     wrapStr(const QString &str, const QFont &font, int maxWidth);
QString     sizeToHuman(const qlonglong bytes);
QString     timeToString(const QDateTime &time, bool normalFormat = false);
QDateTime   stringToDateTime(const QString &time);
QString     getFileContent(const QString &file);
QString     symFilePath(const QString &path);
bool        writeTextFile(QString filePath, QString content);

bool        trashFile(const QString &file);
bool        trashFiles(const QStringList &files);

bool        onMountDevice(const QString &path);
bool        mountDeviceExist(const QString &path);

}  // namespace base

}  // namespace utils

#endif // BASEUTILS_H
