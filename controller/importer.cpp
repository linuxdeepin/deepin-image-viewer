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
    : QObject(parent),
      m_dbManager(DatabaseManager::instance()),
      m_progress(1),
      m_imagesCount(0)
{
    connect(&m_futureWatcher, SIGNAL(finished()),
            this, SLOT(onFutureWatcherFinish()));
    connect(&m_futureWatcher, SIGNAL(resultReadyAt(int)),
            this, SLOT(onFutureResultReady(int)));
}

QString insertImage(QString path)
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

bool Importer::isRunning() const
{
    return m_progress != 1;
}

double Importer::getProgress() const
{
    return m_progress;
}

int Importer::finishedCount() const
{
    return m_imagesCount - m_cacheImportList.length();
}

void Importer::showImportDialog()
{
    QString dir = QFileDialog::getExistingDirectory(
                nullptr, tr("Open Directory"),
                QDir::homePath(),
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    importFromPath(dir);
}

void Importer::stopImport()
{
    m_futureWatcher.cancel();
    m_futureWatcher.waitForFinished();
    m_cacheImportList.clear();
    m_albums.clear();
    m_progress = 1.0;
    m_imagesCount = 0;
    emit importProgressChanged(m_progress);
}

void Importer::importFromPath(const QString &path, const QString &album)
{
    const QFileInfoList infos = utils::image::getImagesInfo(path);
    if( !QDir(path).exists() || infos.isEmpty() ) {
        return;
    }

    if (m_cacheImportList.isEmpty())
        emit importStart();

    m_futureWatcher.setPaused(true);
    for (QFileInfo info : infos) {
        m_cacheImportList.append(info.absoluteFilePath());
        m_albums.insert(info.absoluteFilePath(), album);
        m_imagesCount ++;
    }

    m_futureWatcher.setPaused(false);
    if (!m_futureWatcher.isRunning()) {
        loadCacheImages();
    }

    if (m_cacheImportList.isEmpty())
        m_dbManager->clearRecentImported();
}

void Importer::importSingleFile(const QString &filePath, const QString &album)
{
    if (QImage(filePath).isNull())
        return;
    m_albums.insert(filePath, album);
    m_cacheImportList << filePath;
    m_imagesCount ++;
    if (!m_futureWatcher.isRunning()) {
        loadCacheImages();
    }

    if (m_cacheImportList.isEmpty())
        m_dbManager->clearRecentImported();
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
