#include "importer.h"
#include "importthread.h"
#include "controller/databasemanager.h"
#include <libexif/exif-data.h>
#include <QDirIterator>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

Importer::Importer(QObject *parent)
    : QObject(parent),m_readCount(0),m_progress(1)
{

}

Importer *Importer::m_importer = NULL;
Importer *Importer::instance()
{
    if (!m_importer) {
        m_importer = new Importer();
    }

    return m_importer;
}

double Importer::getProgress() const
{
    return m_progress;
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

        if (! DatabaseManager::instance()->imageExist(fileInfo.fileName())) {
            m_importList.append(filePath);
        }
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
    m_progress = QString::number((1 - (double)m_importList.count() / m_readCount), 'g', 2).toDouble();

    emit importProgressChanged(m_readCount - m_importList.count(), m_progress);
}
