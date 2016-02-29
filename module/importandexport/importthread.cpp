#include "importthread.h"
#include "controller/databasemanager.h"
#include <QFileInfo>
#include <QPixmap>
#include <QImage>
#include <QTimer>
#include <QDebug>

const int THUMBNAIL_MAX_SIZE = 160;

ImportThread::ImportThread(const QString &path, const QString &album, QObject *parent)
    :QThread(parent), m_filePath(path), m_album(album)
{

}

void ImportThread::run()
{
    QFileInfo fileInfo(m_filePath);
    DatabaseManager::ImageInfo imgInfo;
    imgInfo.name = fileInfo.fileName();
    imgInfo.path = fileInfo.absoluteFilePath();
    imgInfo.time = getCreateDateTime(m_filePath);
    imgInfo.albums = QStringList(m_album);
    imgInfo.labels = QStringList();
    imgInfo.thumbnail = getThumbnail(m_filePath);

    //Note: Use single connection in whole application will cause crash,
    //so create diffrent connection for eatch thread by filename.
    DatabaseManager dm(fileInfo.fileName());
    dm.insertImageInfo(imgInfo);

    emit importFinish(m_filePath);
    QSqlDatabase::removeDatabase(fileInfo.fileName());
}

QPixmap ImportThread::getThumbnail(const QString &filePath)
{
    QPixmap pixmap;
    ExifData *ed = exif_data_new_from_file(filePath.toUtf8().data());
    if (ed) {
        //Make sure the image had a thumbnail before trying to write it
        if (ed->data && ed->size) {
            pixmap.loadFromData(ed->data, ed->size);
        } else {
            qDebug() << QString("NO Exif thumbnail in file %1!").arg(filePath);
            pixmap = scaleImage(filePath);
        }
        //Free the EXIF data
        exif_data_unref(ed);
    }
    else {
        pixmap = scaleImage(filePath);
    }

    return pixmap;
}

QPixmap ImportThread::scaleImage(const QString &filePath)
{
    QImage img(filePath);
    QSize targetSize;
    if (img.width() > img.height()) {
        targetSize = QSize(THUMBNAIL_MAX_SIZE, (double)THUMBNAIL_MAX_SIZE / img.width() * img.height());
    }
    else {
        targetSize = QSize((double)THUMBNAIL_MAX_SIZE / img.height() * img.width(), THUMBNAIL_MAX_SIZE);
    }

    return QPixmap::fromImage(img.scaled(targetSize * 2)
                              .scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

QDateTime ImportThread::getCreateDateTime(const QString &filePath)
{
    ExifData *ed = exif_data_new_from_file(filePath.toUtf8().data());
    if (ed) {
        QDateTime dt;
        dt = QDateTime::fromString(readExifTag(ed, EXIF_IFD_0, EXIF_TAG_DATE_TIME), "yyyy:MM:dd HH:mm:ss");
        //Free the EXIF data
        exif_data_unref(ed);
        return dt;
    }
    else {
        QFileInfo info(filePath);
        return info.created();
    }
}

QString ImportThread::readExifTag(ExifData *ed, ExifIfd eid, ExifTag tag)
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
