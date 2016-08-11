#include "importer.h"
#include "application.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include <libexif/exif-data.h>
#include "utils/imageutils.h"
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QTimer>

Importer::Importer(QObject *parent)
    : QObject(parent),
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
    imgInfo.time = fileInfo.created();//utils::image::getCreateDateTime(path);
    imgInfo.albums = albums;
    imgInfo.labels = QStringList();
//    DatabaseManager::instance()->insertImageInfo(imgInfo);

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

/*!
 * \brief Importer::nap
 * Nap for unblock main UI
 */
void Importer::nap()
{
    if (m_progress != 1) {
        m_futureWatcher.setPaused(true);
        TIMER_SINGLESHOT(500,
        {m_futureWatcher.setPaused(false);},this);
    }
}

void Importer::showImportDialog(const QString &album)
{
    QString dir = QFileDialog::getExistingDirectory(
                nullptr, tr("Open Directory"),
                QDir::homePath(),
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    importDir(dir, album);
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

void Importer::importDir(const QString &path, const QString &album)
{
    const QFileInfoList infos = utils::image::getImagesInfo(path);
    if( !QDir(path).exists() || infos.isEmpty() ) {
        return;
    }

    // To avoid the repeat names
    QStringList imgNames;
    QList<DatabaseManager::ImageInfo> imgInfos;
    for (QFileInfo info : infos) {
        if (imgNames.indexOf(info.fileName()) != -1)
            continue;
        imgNames << info.fileName();
        DatabaseManager::ImageInfo imgInfo;
        imgInfo.name = info.fileName();
        imgInfo.path = info.absoluteFilePath();
        imgInfo.time = info.created();//utils::image::getCreateDateTime(path);
        imgInfo.albums = QStringList(album);
        imgInfo.labels = QStringList();

        imgInfos << imgInfo;
    }

    dApp->databaseM->insertImageInfos(imgInfos);

    emit importProgressChanged(1);
}

void Importer::importFiles(const QStringList &files, const QString &album)
{
    if (files.isEmpty())
        return;
    QList<DatabaseManager::ImageInfo> imgInfos;
    for (QString file : files) {
        QFileInfo info(file);
        DatabaseManager::ImageInfo imgInfo;
        imgInfo.name = info.fileName();
        imgInfo.path = info.absoluteFilePath();
        imgInfo.time = info.created();//utils::image::getCreateDateTime(file);
        imgInfo.albums = QStringList(album);
        imgInfo.labels = QStringList();

        imgInfos << imgInfo;
    }
    dApp->databaseM->insertImageInfos(imgInfos);

    emit importProgressChanged(1);
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
//        emit dApp->signalM->showProcessTooltip(
//                    tr("Imported successfully"), true);
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
