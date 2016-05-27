#include "baseutils.h"
#include <fcntl.h>
#include <fstream>
#include <linux/fs.h>
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFontMetrics>
#include <QFileInfo>
#include <QImage>
#include <QMimeData>
#include <QUrl>
#include <QDebug>
#include <QTextStream>

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

void copyImageToClipboard(const QString &path)
{
    //  Get clipboard
    QClipboard *cb = QApplication::clipboard();

    // Ownership of the new data is transferred to the clipboard.
    QMimeData* newMimeData = new QMimeData();

    // Copy old mimedata
    const QMimeData* oldMimeData = cb->mimeData();
    for ( const QString &f : oldMimeData->formats())
        newMimeData->setData(f, oldMimeData->data(f));

    // Copy file (gnome)
    QByteArray gnomeFormat = QByteArray("copy\n").append(
                QUrl::fromLocalFile(path).toEncoded());
    newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);

    // Set the mimedata
    cb->setMimeData(newMimeData);
}

QString getFileContent(const QString &file) {
    QFile f(file);
    QString fileContent = "";
    if (f.open(QFile::ReadOnly))
    {
        fileContent = QLatin1String(f.readAll());
        f.close();
    }
    return fileContent;
}

bool writeTextFile(QString filePath, QString content) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadWrite|QIODevice::Text)) {
        QTextStream in(&file);
        in << content << endl;
        file.close();
        return true;
    }

    return false;
}

bool trashFile(const QString &file)
{
#ifdef QT_GUI_LIB
    bool TrashInitialized = false;
    QString TrashPath;
    QString TrashPathInfo;
    QString TrashPathFiles;

    if( !TrashInitialized ){
        QStringList paths;
        const char* xdg_data_home = getenv( "XDG_DATA_HOME" );
        if( xdg_data_home ){
            qDebug() << "XDG_DATA_HOME not yet tested";
            QString xdgTrash( xdg_data_home );
            paths.append( xdgTrash + "/Trash" );
        }
        QString home = QStandardPaths::writableLocation( QStandardPaths::HomeLocation );
        paths.append( home + "/.local/share/Trash" );
        paths.append( home + "/.trash" );
        for ( QString path : paths ){
            if( TrashPath.isEmpty() ){
                QDir dir( path );
                if( dir.exists() ){
                    TrashPath = path;
                }
            }
        }
        if( TrashPath.isEmpty() ) {
            qWarning() << "Cant detect trash folder";
            return false;
        }
        TrashPathInfo = TrashPath + "/info";
        TrashPathFiles = TrashPath + "/files";
        if( !QDir( TrashPathInfo ).exists() || !QDir( TrashPathFiles ).exists() ) {
            qWarning() << "Trash doesnt looks like FreeDesktop.org Trash specification";
            return false;
        }
        TrashInitialized = true;
    }
    QFileInfo original( file );
    if( !original.exists() ) {
        qWarning() << "File doesnt exists, cant move to trash";
        return false;
    }
//    QString info;
//    info += "[Trash Info]\nPath=";
//    info += original.absoluteFilePath();
//    info += "\nDeletionDate=";
//    info += QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss.zzzZ");
//    info += "\n";
    QString trashname = original.fileName();
    QString infopath = TrashPathInfo + "/" + trashname + ".trashinfo";
    QString filepath = TrashPathFiles + "/" + trashname;
    int nr = 1;
    while( QFileInfo( infopath ).exists() || QFileInfo( filepath ).exists() ){
        nr++;
        trashname = original.baseName() + "." + QString::number( nr );
        if( !original.completeSuffix().isEmpty() ){
            trashname += QString( "." ) + original.completeSuffix();
        }
        infopath = TrashPathInfo + "/" + trashname + ".trashinfo";
        filepath = TrashPathFiles + "/" + trashname;
    }
    QDir dir;
    if( !dir.rename( original.absoluteFilePath(), filepath ) ){
        qWarning() << "move to trash failed!";
        return false;
    }
//    File infofile;
//    infofile.createUtf8( infopath, info );
    return true;
#else
    Q_UNUSED( file );
    qWarning() << "Trash in server-mode not supported";
    return false;
#endif
}

}  // namespace base

}  // namespace utils
