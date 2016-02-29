#include "importer.h"
#include "importthread.h"
#include "controller/databasemanager.h"
#include <libexif/exif-data.h>
#include <QDirIterator>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

Importer::Importer(QObject *parent) : QObject(parent)
{

}

void Importer::importFromPath(const QString &path)
{
    QDir dir( path );
    if( !dir.exists() ) {
        return;
    }

    QStringList filters;
    filters << QString("*.jpeg")
            <<QString("*.jpg")
           <<QString("*.png")
          <<QString("*.tiff")
         <<QString("*.gif")
       <<QString("*.bmp");

    QDirIterator dirIterator(path,
                             filters,
                             QDir::Files | QDir::NoSymLinks,
                             QDirIterator::Subdirectories);
    while(dirIterator.hasNext())
    {
        dirIterator.next();
        QFileInfo fileInfo = dirIterator.fileInfo();
        QString filePath = fileInfo.absoluteFilePath();

        DatabaseManager dm(filePath);
        if (! dm.imageExist(fileInfo.fileName())) {
            m_importList.append(filePath);
        }
        QSqlDatabase::removeDatabase(filePath);
    }

    //for calculate import progress
    m_readCount = m_importList.count();
    for (QString path : m_importList) {
        ImportThread *t = new ImportThread(path, "", this);
        connect(t, &ImportThread::finished, t, &ImportThread::deleteLater);
        connect(t, &ImportThread::importFinish, this, &Importer::importThreadFinish);
        t->start();
    }
}

void Importer::importSingleFile(const QString &filePath)
{
    m_importList << filePath;
    m_readCount = m_importList.count();

    ImportThread *t = new ImportThread(filePath, "", this);
    connect(t, &ImportThread::finished, t, &ImportThread::deleteLater);
    connect(t, &ImportThread::importFinish, this, &Importer::importThreadFinish);
    t->start();
}

void Importer::importThreadFinish(const QString &filePath)
{
    m_importList.removeAll(filePath);
    qDebug() << "Import Progress: " << (1 - m_importList.count() / (double)m_readCount);
}
