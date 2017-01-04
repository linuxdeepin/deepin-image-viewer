#include "importer.h"
#include "application.h"
#include "utils/imageutils.h"
#include <QDirIterator>
#include <QFileDialog>
#include <QTimer>
#include <QDebug>
#include <QStorageInfo>

namespace {

const int REFRESH_DELAY = 3000;


QStringList mountPoints()
{
    QStringList paths;
    QList<QStorageInfo> infos = QStorageInfo::mountedVolumes();
    for (auto info : infos) {
        const QString rp = info.rootPath();
        if (rp.startsWith("/media") || rp.startsWith("/run/media")) {
            paths << rp;
        }
    }

    return paths;
}

QString mountPoint(const QString &path, const QStringList &points)
{
    for (QString point : points) {
        if (path.startsWith(point))
            return point;
    }

    return QString();
}

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
            dApp->dbM, &DBManager::insertImgInfos);
    connect(dt, &DirCollectThread::insertAlbumRequest,
            dApp->dbM, &DBManager::insertIntoAlbum);
    connect(dt, &DirCollectThread::currentImport,
            this, &Importer::currentImport);
    connect(dt, &DirCollectThread::finished, this, [=] {
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
    connect(ft, &FilesCollectThread::finished, this, [=] {
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
        t->deleteLater();
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
    QStringList points = mountPoints();

    qint64 bt = QDateTime::currentMSecsSinceEpoch();
    for (QString dir : subDirs) {
        auto fileInfos = utils::image::getImagesInfo(dir, false);
        for (auto fi : fileInfos) {
            const QString path = fi.absoluteFilePath();

            // Generate thumbnail and storage into cache dir
            if (! utils::image::thumbnailExist(path)) {
                // Generate thumbnail failed, do not insert into DB
                if (! utils::image::generateThumbnail(path)) {
                    continue;
                }
            }

            paths << path;
            DBImgInfo dbi;
            dbi.fileName = fi.fileName();
            dbi.filePath = path;
            dbi.mountPoint = mountPoint(path, points);
            dbi.time = utils::image::getCreateDateTime(path);

            dbInfos << dbi;

            qint64 et = QDateTime::currentMSecsSinceEpoch();
            if (et - bt > REFRESH_DELAY) {
                bt = et;
                if (! dbInfos.isEmpty()) {
                    emit resultReady(dbInfos);
                    dbInfos.clear();
                }
            }

            emit currentImport(path);
        }
    }
    if (! dbInfos.isEmpty())
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
    QStringList points = mountPoints();
    using namespace utils::image;

    qint64 bt = QDateTime::currentMSecsSinceEpoch();
    for (auto path : m_paths) {
        if (! imageSupportRead(path)) {
            continue;
        }

        // Generate thumbnail and storage into cache dir
        if (! utils::image::thumbnailExist(path)) {
            // Generate thumbnail failed, do not insert into DB
            if (! utils::image::generateThumbnail(path)) {
                continue;
            }
        }

        supportPaths << path;
        QFileInfo fi(path);
        DBImgInfo dbi;
        dbi.fileName = fi.fileName();
        dbi.filePath = path;
        dbi.mountPoint = mountPoint(path, points);
        dbi.time = utils::image::getCreateDateTime(path);

        dbInfos << dbi;

        qint64 et = QDateTime::currentMSecsSinceEpoch();
        if (et - bt > REFRESH_DELAY) {
            bt = et;
            if (! dbInfos.isEmpty()) {
                emit resultReady(dbInfos);
                dbInfos.clear();
            }
        }

        emit currentImport(path);
    }
    emit resultReady(dbInfos);
    if (! m_album.isEmpty())
        emit insertAlbumRequest(m_album, supportPaths);
}
