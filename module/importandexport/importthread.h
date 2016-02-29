#ifndef IMPORTTHREAD_H
#define IMPORTTHREAD_H

#include <QObject>
#include <QThread>
#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-tag.h>

class ImportThread : public QThread
{
    Q_OBJECT

public:
    ImportThread(const QString &path, const QString &album = "", QObject *parent = 0);

signals:
    void importFinish(QString path);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QPixmap getThumbnail(const QString &filePath);
    QPixmap scaleImage(const QString &filePath);
    QDateTime getCreateDateTime(const QString &filePath);
    QString readExifTag(ExifData *ed, ExifIfd eid, ExifTag tag);

private:
    QString m_filePath;
    QString m_album;
};

#endif // IMPORTTHREAD_H
