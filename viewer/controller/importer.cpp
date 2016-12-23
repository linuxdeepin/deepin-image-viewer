#include "importer.h"
#include "application.h"
#include "utils/imageutils.h"
#include <QDirIterator>
#include <QFileDialog>
#include <QTimer>
#include <QDebug>

namespace {

QStringList collectSubDirs(const QString &path)
{
    QStringList dirs;
    QDirIterator dirIterator(path,
                             QDir::Dirs | QDir::NoDotDot,
                             QDirIterator::Subdirectories);
    while(dirIterator.hasNext()) {
        dirIterator.next();
        dirs.append(dirIterator.filePath());
    }
    return dirs;
}

void insertIntoAlbum(QMap<QString, QFileInfoList> vs)
{
    QStringList albums = vs.keys();
    albums.removeAll(" ");
    for (QString album : albums) {
        auto infos = vs[album];
        QStringList paths;
        for (auto info : infos) {
            paths << info.absoluteFilePath();
        }
        dApp->dbM->insertIntoAlbum(album, paths);
    }
}

}  // namespace

Importer::Importer(QObject *parent)
    : QObject(parent)
{

}

bool Importer::isRunning() const
{
    return ! m_threads.isEmpty();
}

void Importer::appendDir(const QString &path, const QString &album)
{
    if (m_dirs.contains(path))
        return;
    else
        m_dirs << path;

    emit progressChanged();
    emit currentImport(path);

    DirCollectThread *dt = new DirCollectThread(path, album);
    connect(dt, &DirCollectThread::resultReady,
            dApp->dbM, &DBManager::insertImgInfos, Qt::DirectConnection);
    connect(dt, &DirCollectThread::insertAlbumRequest,
            dApp->dbM, &DBManager::insertIntoAlbum, Qt::DirectConnection);
    connect(dt, &DirCollectThread::currentImport,
            this, &Importer::currentImport);
    connect(dt, &DirCollectThread::finished,
            this, [=] {
        m_threads.removeAll(dt);
        if (m_threads.isEmpty()) {
            emit imported(true);
            m_dirs.clear();
        }

        dt->deleteLater();
    });
    dt->start();
    m_threads.append(dt);
}

void Importer::appendFiles(const QStringList &paths, const QString &album)
{
    emit progressChanged();

    FilesCollectThread *ft = new FilesCollectThread(paths, album);
    connect(ft, &FilesCollectThread::resultReady,
            dApp->dbM, &DBManager::insertImgInfos);
    connect(ft, &FilesCollectThread::insertAlbumRequest,
            dApp->dbM, &DBManager::insertIntoAlbum);
    connect(ft, &FilesCollectThread::currentImport,
            this, &Importer::currentImport);
    connect(ft, &FilesCollectThread::finished,
            this, [=] {
        m_threads.removeAll(ft);
        if (m_threads.isEmpty())
            emit imported(true);

        ft->deleteLater();
    });
    ft->start();
    m_threads.append(ft);
}

void Importer::stop()
{
    for (auto t : m_threads) {
        t->quit();
        t->wait();
    }

    emit imported(true);
}

void Importer::showImportDialog(const QString &album)
{
    QString dir = QFileDialog::getExistingDirectory(
                nullptr, tr("Open Directory"), QDir::homePath(),
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) return;
    appendDir(dir, album);
}

DirCollectThread::DirCollectThread(const QString &root, const QString &album)
    :QThread(NULL)
    , m_album(album)
    , m_root(root)
{

}

void DirCollectThread::run()
{
    QStringList subDirs = collectSubDirs(m_root);
    DBImgInfoList dbInfos;
    QStringList paths;

    int generateCount = 0;
    int cacheCount = 0;
    for (QString dir : subDirs) {
        auto fileInfos = utils::image::getImagesInfo(dir, false);
        for (auto fi : fileInfos) {
            const QString path = fi.absoluteFilePath();
            paths << path;
            DBImgInfo dbi;
            dbi.fileName = fi.fileName();
            dbi.filePath = path;
            dbi.time = utils::image::getCreateDateTime(path);

            dbInfos << dbi;

            // Generate thumbnail and storage into cache dir
            if (! utils::image::thumbnailExist(path)) {
                utils::image::generateThumbnail(path);
                generateCount ++;
            }
            else {
                cacheCount ++;
            }

            if (generateCount > 50 || cacheCount > 2000) {
                emit resultReady(dbInfos);
                dbInfos.clear();
                generateCount = 0;
                cacheCount = 0;
            }

            emit currentImport(path);
        }
    }
    emit resultReady(dbInfos);
    if (! m_album.isEmpty())
        emit insertAlbumRequest(m_album, paths);
}

FilesCollectThread::FilesCollectThread(const QStringList &paths, const QString &album)
    : QThread(NULL)
    , m_album(album)
    , m_paths(paths)
{

}

void FilesCollectThread::run()
{
    DBImgInfoList dbInfos;
    QStringList supportPaths;
    using namespace utils::image;

    int generateCount = 0;
    int cacheCount = 0;
    for (auto path : m_paths) {
        if (! imageSupportRead(path)) {
            continue;
        }
        supportPaths << path;
        QFileInfo fi(path);
        DBImgInfo dbi;
        dbi.fileName = fi.fileName();
        dbi.filePath = path;
        dbi.time = utils::image::getCreateDateTime(path);

        dbInfos << dbi;

        // Generate thumbnail and storage into cache dir
        if (! utils::image::thumbnailExist(path)) {
            utils::image::generateThumbnail(path);
            generateCount ++;
        }
        else {
            cacheCount ++;
        }

        if (generateCount > 50 || cacheCount > 2000) {
            emit resultReady(dbInfos);
            dbInfos.clear();
            generateCount = 0;
            cacheCount = 0;
        }

        emit currentImport(path);
    }
    emit resultReady(dbInfos);
    if (! m_album.isEmpty())
        emit insertAlbumRequest(m_album, supportPaths);
}