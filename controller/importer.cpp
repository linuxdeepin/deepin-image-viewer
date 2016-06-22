#include "importer.h"
#include "utils/imageutils.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include <libexif/exif-data.h>
#include <QDirIterator>
#include <QFileInfo>
#include <QFileDialog>
#include <QDebug>
#include <QDir>

Importer::Importer(QObject *parent)
    : QObject(parent),m_progress(1),m_imagesCount(0)
{
    connect(&m_futureWatcher, SIGNAL(finished()),
            this, SLOT(onFutureWatcherFinish()));
    connect(&m_futureWatcher, SIGNAL(resultReadyAt(int)),
            this, SLOT(onFutureResultReady(int)));
}

QString insertImage(const QString &path)
{
    const QStringList albums = Importer::instance()->getAlbums(path);
    QFileInfo fileInfo(path);
    DatabaseManager::ImageInfo imgInfo;
    imgInfo.name = fileInfo.fileName();
    imgInfo.path = fileInfo.absoluteFilePath();
    imgInfo.time = utils::image::getCreateDateTime(path);
    imgInfo.albums = albums;
    imgInfo.labels = QStringList();
    imgInfo.thumbnail = utils::image::getThumbnail(path);
    if (imgInfo.thumbnail.isNull()) {
        // Clear the invalid(eg.Empty image) image
        DatabaseManager::instance()->removeImage(imgInfo.name);
        return path;
    }

    DatabaseManager::instance()->insertImageInfo(imgInfo);
    if (! albums.isEmpty() && !QString(albums.first()).isEmpty()) {
        DatabaseManager::instance()->insertImageIntoAlbum(
          albums.first(),imgInfo.name, utils::base::timeToString(imgInfo.time));
    }

    return path;
}

void Importer::loadCacheImages()
{
    QStringList pathList = m_cacheImportList;
    QFuture<QString> future = QtConcurrent::mapped(pathList, insertImage);
    m_futureWatcher.setFuture(future);
}

QStringList Importer::getAlbums(const QString &path) const
{
    const QString album = m_albums.value(path);
    if (album.isEmpty()) {
        return QStringList();
    }
    else {
        QStringList l;
        l << album;
        return l;
    }
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

void Importer::showImportDialog()
{
    QString dir = QFileDialog::getExistingDirectory(
                nullptr, tr("Open Directory"),
                QDir::homePath(),
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    importFromPath(dir);
}

void Importer::importFromPath(const QString &path, const QString &album)
{
    QDir dir( path );
    if( !dir.exists() ) {
        return;
    }

    m_futureWatcher.setPaused(true);
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
            m_cacheImportList.append(filePath);
            m_albums.insert(filePath, album);
        }

        m_imagesCount ++;
    }

    m_futureWatcher.setPaused(false);
    if (!m_futureWatcher.isRunning()) {
        loadCacheImages();
    }

    if (m_cacheImportList.isEmpty())
        DatabaseManager::instance()->clearRecentImported();
}

void Importer::importSingleFile(const QString &filePath, const QString &album)
{
    //TODO unsupport tooltip
    if (QImage(filePath).isNull())
        return;
    m_albums.insert(filePath, album);
    m_cacheImportList << filePath;
    m_imagesCount ++;
    if (!m_futureWatcher.isRunning()) {
        loadCacheImages();
    }

    if (m_cacheImportList.isEmpty())
        DatabaseManager::instance()->clearRecentImported();
}

void Importer::onFutureWatcherFinish()
{
    // Imported finish
    if (m_cacheImportList.isEmpty()) {
        qDebug() << "Imported finish, no more cache!";
        m_imagesCount = 0;
        m_progress = 1;
        m_albums.clear();
        emit importProgressChanged(m_progress);
        emit SignalManager::instance()->showProcessTooltip(
                    tr("Imported successfully"), true);
    }
    else {
        loadCacheImages();
    }
}

void Importer::onFutureResultReady(int index)
{
    m_cacheImportList.removeAll(m_futureWatcher.resultAt(index));
    m_progress = 1 - (1.0 * m_cacheImportList.count() / m_imagesCount);
    emit importProgressChanged(m_progress);
}
