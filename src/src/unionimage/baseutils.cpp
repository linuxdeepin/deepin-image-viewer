// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseutils.h"
#include "imageutils.h"
#include <stdio.h>
#include <fcntl.h>
#include <fstream>
#include <linux/fs.h>

#include <QApplication>
#include <QClipboard>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFontMetrics>
#include <QFileInfo>
#include <QImage>
#include <QMimeData>
#include <QProcess>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QTextStream>
#include <QtMath>
#include <QImageReader>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

#include "unionimage.h"

namespace Libutils {

namespace base {

const QString DATETIME_FORMAT_NORMAL = "yyyy.MM.dd";
const QString DATETIME_FORMAT_EXIF = "yyyy:MM:dd HH:mm:ss";

QPixmap renderSVG(const QString &filePath, const QSize &size)
{
    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
        qCDebug(logImageViewer) << "Rendered SVG:" << filePath << "size:" << size << "ratio:" << ratio;
    } else {
        qCWarning(logImageViewer) << "Failed to read SVG file:" << filePath;
        pixmap.load(filePath);
    }

    return pixmap;
}

// QString sizeToHuman(const qlonglong bytes)
//{
//     qlonglong sb = 1024;
//     if (bytes < sb) {
//         return QString::number(bytes) + " B";
//     } else if (bytes < sb * sb) {
//         QString vs = QString::number((double)bytes / sb, 'f', 1);
//         if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
//             return QString::number((int)vs.toDouble()) + " KB";
//         } else {
//             return vs + " KB";
//         }
//     } else if (bytes < sb * sb * sb) {
//         QString vs = QString::number((double)bytes / sb / sb, 'f', 1);
//         if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
//             return QString::number((int)vs.toDouble()) + " MB";
//         } else {
//             return vs + " MB";
//         }
//     } else {
//         return QString::number(bytes);
//     }
// }

QString timeToString(const QDateTime &time, bool normalFormat)
{
    qCDebug(logImageViewer) << "Converting QDateTime to string. Normal format requested:" << normalFormat;
    if (normalFormat) {
        qCDebug(logImageViewer) << "Using normal date format for conversion.";
        return time.toString(DATETIME_FORMAT_NORMAL);
    } else {
        qCDebug(logImageViewer) << "Using EXIF date format for conversion.";
        return time.toString(DATETIME_FORMAT_EXIF);
    }
}

int stringWidth(const QFont &f, const QString &str)
{
    qCDebug(logImageViewer) << "Calculating string width for:" << str;
    QFontMetrics fm(f);
    return fm.boundingRect(str).width();
}

int stringHeight(const QFont &f, const QString &str)
{
    qCDebug(logImageViewer) << "Calculating string height for:" << str;
    QFontMetrics fm(f);
    return fm.boundingRect(str).height();
}

QDateTime stringToDateTime(const QString &time)
{
    qCDebug(logImageViewer) << "Converting string to QDateTime:" << time;
    QDateTime dt = QDateTime::fromString(time, DATETIME_FORMAT_EXIF);
    if (!dt.isValid()) {
        qCDebug(logImageViewer) << "EXIF format failed, trying normal format.";
        dt = QDateTime::fromString(time, DATETIME_FORMAT_NORMAL);
    }
    return dt;
}

void showInFileManager(const QString &path)
{
    if (path.isEmpty() || !QFile::exists(path)) {
        qCWarning(logImageViewer) << "Invalid path for file manager:" << path;
        return;
    }

#if 1
    QUrl url = QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath());
    qCDebug(logImageViewer) << "Opening file manager for:" << url.toString();
    QDesktopServices::openUrl(url);
#else
    QUrl url = QUrl::fromLocalFile(path);
#endif
    QDesktopServices::openUrl(url);
    //    QUrl url = QUrl::fromLocalFile(QFileInfo(path).dir().absolutePath());
    //    QUrlQuery query;
    //    query.addQueryItem("selectUrl", QUrl::fromLocalFile(path).toString());
    //    url.setQuery(query);
    //    qDebug() << "showInFileManager:" << url.toString();
    //    // Try dde-file-manager
    //    QProcess *fp = new QProcess();
    //    QObject::connect(fp, SIGNAL(finished(int)), fp, SLOT(deleteLater()));
    //    fp->start("dde-file-manager", QStringList(url.toString()));
    //    fp->waitForStarted(3000);
    //    if (fp->error() == QProcess::FailedToStart) {
    //        // Start dde-file-manager failed, try nautilus
    //        QDBusInterface iface("org.freedesktop.FileManager1",
    //                             "/org/freedesktop/FileManager1",
    //                             "org.freedesktop.FileManager1",
    //                             QDBusConnection::sessionBus());
    //        if (iface.isValid()) {
    //            // Convert filepath to URI first.
    //            const QStringList uris = { QUrl::fromLocalFile(path).toString() };
    //            qDebug() << "freedesktop.FileManager";
    //            // StartupId is empty here.
    //            QDBusPendingCall call = iface.asyncCall("ShowItems", uris, "");
    //            Q_UNUSED(call);
    //        }
    //        // Try to launch other file manager if nautilus is invalid
    //        else {
    //            qDebug() << "desktopService::openUrl";
    //            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).dir().absolutePath()));
    //        }
    //        fp->deleteLater();
    //    }
}

void copyImageToClipboard(const QStringList &paths)
{
    qCDebug(logImageViewer) << "Copying" << paths.size() << "images to clipboard";
    //  Get clipboard
    //    QClipboard *cb = QApplication::clipboard();
    QClipboard *cb = qApp->clipboard();

    // Ownership of the new data is transferred to the clipboard.
    QMimeData *newMimeData = new QMimeData();

    // Copy old mimedata
    //    const QMimeData* oldMimeData = cb->mimeData();
    //    for ( const QString &f : oldMimeData->formats())
    //        newMimeData->setData(f, oldMimeData->data(f));

    // Copy file (gnome)
    QByteArray gnomeFormat = QByteArray("copy\n");
    QString text;
    QList<QUrl> dataUrls;
    for (QString path : paths) {
        if (!path.isEmpty())
            text += path + '\n';
        dataUrls << QUrl::fromLocalFile(path);
        gnomeFormat.append(QUrl::fromLocalFile(path).toEncoded()).append("\n");
    }

    newMimeData->setText(text.endsWith('\n') ? text.left(text.length() - 1) : text);
    newMimeData->setUrls(dataUrls);
    gnomeFormat.remove(gnomeFormat.length() - 1, 1);
    newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);

    // Copy Image Date
    //    QImage img(paths.first());
    //    Q_ASSERT(!img.isNull());
    //    newMimeData->setImageData(img);

    // Set the mimedata
    //    cb->setMimeData(newMimeData);
    cb->setMimeData(newMimeData, QClipboard::Clipboard);
    qCDebug(logImageViewer) << "Clipboard updated with" << paths.size() << "images";
}

QString getFileContent(const QString &file)
{
    qCDebug(logImageViewer) << "Reading file content:" << file;
    QFile f(file);
    QString fileContent = "";
    if (f.open(QFile::ReadOnly)) {
        fileContent = QLatin1String(f.readAll());
        f.close();
        qCDebug(logImageViewer) << "Successfully read file content, size:" << fileContent.size();
    } else {
        qCWarning(logImageViewer) << "Failed to open file for reading:" << file;
    }
    return fileContent;
}

// bool writeTextFile(QString filePath, QString content)
//{
//     QFile file(filePath);
//     if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//         QTextStream in(&file);
//         in << content << endl;
//         file.close();
//         return true;
//     }

//    return false;
//}

QString getNotExistsTrashFileName(const QString &fileName)
{
    qCDebug(logImageViewer) << "Generating unique trash filename for:" << fileName;
    QByteArray name = fileName.toUtf8();

    int index = name.lastIndexOf('/');

    if (index >= 0) {
        qCDebug(logImageViewer) << "Found '/' in filename, extracting name part.";
        name = name.mid(index + 1);
    }

    index = name.lastIndexOf('.');
    QByteArray suffix;

    if (index >= 0) {
        qCDebug(logImageViewer) << "Found '.' in filename, extracting suffix.";
        suffix = name.mid(index);
    }

    if (suffix.size() > 200)
        suffix = suffix.left(200);
    qCDebug(logImageViewer) << "Adjusted suffix size to:" << suffix.size();

    name.chop(suffix.size());
    name = name.left(200 - suffix.size());
    qCDebug(logImageViewer) << "Adjusted name size to:" << name.size();

    QString trashpath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.local/share/Trash";
    qCDebug(logImageViewer) << "Trash path determined as:" << trashpath;

    while (true) {
        QFileInfo info(trashpath + name + suffix);
        // QFile::exists ==> If the file is a symlink that points to a non-existing file, false is returned.
        if (!info.isSymLink() && !info.exists()) {
            qCDebug(logImageViewer) << "Generated unique trash filename:" << QString::fromUtf8(name + suffix);
            break;
        }

        name = QCryptographicHash::hash(name, QCryptographicHash::Md5).toHex();
        qCDebug(logImageViewer) << "Generated new hash for trash filename";
    }

    return QString::fromUtf8(name + suffix);
}

bool trashFile(const QString &file)
{
#ifdef QT_GUI_LIB
    qCDebug(logImageViewer) << "Moving file to trash:" << file;
    QString trashPath;
    QString trashInfoPath;
    QString trashFilesPath;

    QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    // There maby others location for trash like $HOME/.trash or
    // $XDG_DATA_HOME/Trash, but our stupid FileManager coder said we should
    // assume that the trash lcation is $HOME/.local/share/Trash,so...
    trashPath = home + "/.local/share/Trash";
    trashInfoPath = trashPath + "/info";
    trashFilesPath = trashPath + "/files";
    if (!QDir(trashFilesPath).exists()) {
        qCDebug(logImageViewer) << "Creating trash files directory";
        QDir().mkpath(trashFilesPath);
    }
    if (!QDir(trashInfoPath).exists()) {
        qCDebug(logImageViewer) << "Creating trash info directory";
        QDir().mkpath(trashInfoPath);
    }

    QFileInfo originalInfo(file);
    if (!originalInfo.exists()) {
        qCWarning(logImageViewer) << "File doesn't exists, can't move to trash";
        return false;
    }
    // Info for restore
    QString infoStr;
    infoStr += "[Trash Info]\nPath=";
    infoStr += QString(originalInfo.absoluteFilePath().toUtf8().toPercentEncoding("/"));
    infoStr += "\nDeletionDate=";
    infoStr += QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss.zzzZ");
    infoStr += "\n";
    qCDebug(logImageViewer) << "Trash info string generated.";

    QString trashname = getNotExistsTrashFileName(originalInfo.fileName());
    QString infopath = trashInfoPath + "/" + trashname + ".trashinfo";
    QString filepath = trashFilesPath + "/" + trashname;
    int nr = 1;
    qCDebug(logImageViewer) << "Initial trash name:" << trashname;
    while (QFileInfo(infopath).exists() || QFileInfo(filepath).exists()) {
        qCDebug(logImageViewer) << "Trash file or info path already exists, generating new name.";
        nr++;
        trashname = originalInfo.baseName() + "." + QString::number(nr);
        if (!originalInfo.completeSuffix().isEmpty()) {
            trashname += QString(".") + originalInfo.completeSuffix();
            qCDebug(logImageViewer) << "Adding suffix to new trash name.";
        }
        infopath = trashInfoPath + "/" + trashname + ".trashinfo";
        filepath = trashFilesPath + "/" + trashname;
        qCDebug(logImageViewer) << "New trash name generated:" << trashname;
    }
    QFile infoFile(infopath);
    if (infoFile.open(QIODevice::WriteOnly)) {
        qCDebug(logImageViewer) << "Successfully opened trash info file for writing:" << infopath;
        infoFile.write(infoStr.toUtf8());
        infoFile.close();
        qCDebug(logImageViewer) << "Trash info file written and closed.";

        if (!QDir().rename(originalInfo.absoluteFilePath(), filepath)) {
            qCWarning(logImageViewer) << "move to trash failed!";
            return false;
        }
        qCDebug(logImageViewer) << "File successfully moved to trash.";
    } else {
        qCDebug(logImageViewer) << "Move to trash failed! Could not write *.trashinfo!";
        return false;
    }
    // Remove thumbnail
    Libutils::image::removeThumbnail(file);
    qCDebug(logImageViewer) << "Thumbnail removed for file:" << file;
    return true;
#else
    Q_UNUSED(file);
    qCWarning(logImageViewer) << "Trash in server-mode not supported";
    return false;
#endif
}

// bool trashFiles(const QStringList &files)
//{
//     bool v = true;
//     for (QString file : files) {
//         if (! trashFile(file))
//             v = false;
//     }

//    return v;
//}

///*!
// * \brief wrapStr
// * Split info string by Space
// * \param str
// * \param font
// * \param maxWidth
// * \return
// */
// QString wrapStr(const QString &str, const QFont &font, int maxWidth)
//{
//    QFontMetrics fm(font);
//    QString ns;
//    QString ss;
//    for (int i = 0; i < str.length(); i ++) {
//        if (/*str.at(i).isSpace()||*/ fm.boundingRect(ss).width() > maxWidth) {
//            ss = QString();
//            ns += "\n";
//        }
//        ns += str.at(i);
//        ss += str.at(i);
//    }
//    return ns;
////    return str;
//}

QString SpliteText(const QString &text, const QFont &font, int nLabelSize, bool bReturn)
{
    qCDebug(logImageViewer) << "Splitting text, length:" << text.length() << "label size:" << nLabelSize;
    QFontMetrics fm(font);
    int nTextSize = fm.horizontalAdvance(text);
    qCDebug(logImageViewer) << "Calculated text size:" << nTextSize;
    if (nTextSize > nLabelSize) {
        qCDebug(logImageViewer) << "Text size exceeds label size, splitting text.";
        int nPos = 0;
        long nOffset = 0;
        for (int i = 0; i < text.size(); i++) {
            nOffset += fm.horizontalAdvance(text.at(i));
            if (nOffset >= nLabelSize) {
                nPos = i;
                qCDebug(logImageViewer) << "Split position found at index:" << nPos;
                break;
            }
        }

        nPos = (nPos - 1 < 0) ? 0 : nPos - 1;
        qCDebug(logImageViewer) << "Adjusted split position:" << nPos;

        QString qstrLeftData = text.left(nPos);
        QString qstrMidData = text.mid(nPos);
        qCDebug(logImageViewer) << "Left data:" << qstrLeftData.length() << "chars, Mid data:" << qstrMidData.length() << "chars.";
        if (bReturn) {
            qCDebug(logImageViewer) << "Replacing spaces with newlines in left and mid data.";
            qstrLeftData.replace(" ", "\n");
            qstrMidData.replace(" ", "\n");
            if (qstrLeftData != "")
                return qstrLeftData + SpliteText(qstrMidData, font, nLabelSize);
        } else {
            if (qstrLeftData != "")
                return qstrLeftData + "\n" + SpliteText(qstrMidData, font, nLabelSize);
        }
        qCDebug(logImageViewer) << "Returning split text.";
    }
    qCDebug(logImageViewer) << "Text size is within limits, returning original text.";
    return text;
}

// QString symFilePath(const QString &path)
//{
//     QFileInfo fileInfo(path);
//     if (fileInfo.isSymLink()) {
//         return fileInfo.symLinkTarget();
//     } else {
//         return path;
//     }
// }

QString hash(const QString &str)
{
    qCDebug(logImageViewer) << "Calculating MD5 hash for string.";
    return QString(QCryptographicHash::hash(str.toUtf8(),
                                            QCryptographicHash::Md5)
                           .toHex());
}

bool onMountDevice(const QString &path)
{
    qCDebug(logImageViewer) << "Checking if path is on a mounted device:" << path;
    return (path.startsWith("/media/") || path.startsWith("/run/media/"));
}

bool mountDeviceExist(const QString &path)
{
    qCDebug(logImageViewer) << "Checking mount device existence for path:" << path;
    QString mountPoint;
    if (path.startsWith("/media/")) {
        qCDebug(logImageViewer) << "Path starts with /media/, extracting mount point.";
        const int sp = path.indexOf("/", 7) + 1;
        const int ep = path.indexOf("/", sp) + 1;
        mountPoint = path.mid(0, ep);

    } else if (path.startsWith("/run/media/")) {
        qCDebug(logImageViewer) << "Path starts with /run/media/, extracting mount point.";
        const int sp = path.indexOf("/", 11) + 1;
        const int ep = path.indexOf("/", sp) + 1;
        mountPoint = path.mid(0, ep);
    }
    qCDebug(logImageViewer) << "Determined mount point:" << mountPoint;
    return QFileInfo(mountPoint).exists();
}
// bool        isCommandExist(const QString &command)
//{
//     QProcess *proc = new QProcess;
//     QString cm = QString("which %1\n").arg(command);
//     proc->start(cm);
//     proc->waitForFinished(1000);

//    if (proc->exitCode() == 0) {
//        return true;
//    } else {
//        return false;
//    }

//}
}   // namespace base

}   // namespace utils
