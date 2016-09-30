#ifndef BASEUTILS_H
#define BASEUTILS_H

#include <QObject>
#include <QTimer>

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

namespace base {

void        copyImageToClipboard(const QStringList &paths);
void        showInFileManager(const QString &path);
int         stringWidth(const QFont &f, const QString &str);
int         stringHeight(const QFont &f, const QString &str);

QString     wrapStr(const QString &str, const QFont &font, int maxWidth);
QString     sizeToHuman(const qlonglong bytes);
QString     timeToString(const QDateTime &time, bool normalFormat = false);
QDateTime   stringToDateTime(const QString &time);
QString     getFileContent(const QString &file);
QString     symFilePath(const QString &path);
bool        writeTextFile(QString filePath, QString content);

bool        trashFile(const QString &file);
bool        trashFiles(const QStringList &files);

}  // namespace base

}  // namespace utils

#endif // BASEUTILS_H
