/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "importer.h"
#include "application.h"
#include "dirwatcher/scanpathsdialog.h"
#include "utils/imageutils.h"
#include <QDirIterator>
#include <DFileDialog>
#include <QTimer>
#include <QDebug>
#include <QStorageInfo>

DWIDGET_USE_NAMESPACE
typedef DFileDialog QFDToDFileDialog;

namespace {

const int REFRESH_DELAY = 3000;

QStringList collectSubDirs(const QString &path)
{
    QStringList dirs;
    QDirIterator dirIterator(path,
                             QDir::Dirs | QDir::NoDotDot | QDir::NoDot,
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
       DBManager::instance()->insertIntoAlbum(album, paths);
    }
}

}  // namespace

Importer *Importer::m_importer = NULL;

Importer *Importer::instance()
{
    if (!m_importer) {
        m_importer = new Importer();
    }

    return m_importer;
}


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
            DBManager::instance(), &DBManager::insertImgInfos);
    connect(dt, &DirCollectThread::insertAlbumRequest,
            DBManager::instance(), &DBManager::insertIntoAlbum);
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
            DBManager::instance(), &DBManager::insertImgInfos);
    connect(ft, &FilesCollectThread::insertAlbumRequest,
            DBManager::instance(), &DBManager::insertIntoAlbum);
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

void Importer::stopDirCollect(const QString &dir)
{
    for (auto t : m_threads) {
        DirCollectThread *dc = dynamic_cast<DirCollectThread*>(t);
        if (dc && dc->dir() == dir) {
            qDebug() << "Stopping dir collect thread..." << dir;
            dc->setStop(true);
            t->quit();
            t->wait();
            t->deleteLater();
        }
    }

    emit imported(true);
}

void Importer::showImportDialog(const QString &album)
{
    QString dir = QFDToDFileDialog::getExistingDirectory(
                nullptr, tr("Open Directory"), QDir::homePath(),
                QFDToDFileDialog::ShowDirsOnly | QFDToDFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) return;
    if (ScanPathsDialog::instance()->addPath(dir)) {
        appendDir(dir, album);
    }
}

DirCollectThread::DirCollectThread(const QString &root, const QString &album)
    :QThread(NULL)
    , m_album(album)
    , m_root(root)
    , m_stop(false)
{

}

void DirCollectThread::run()
{
    QStringList subDirs = collectSubDirs(m_root);
    DBImgInfoList dbInfos;
    QStringList paths;

    qint64 bt = QDateTime::currentMSecsSinceEpoch();
    for (QString dir : subDirs) {
        // Remove the subdir's images to avoid repeat insert
        DBManager::instance()->removeDir(dir);
    }

    subDirs << m_root;
    QStringList dbPaths = DBManager::instance()->getAllPaths();
    for (QString dir : subDirs) {
        auto fileInfos = utils::image::getImagesInfo(dir, false);
        for (auto fi : fileInfos) {
            if (m_stop) {
                return;
            }
            const QString path = fi.absoluteFilePath();
            paths << path;

            if (dbPaths.contains(path)) {
                continue;
            }

            // Generate thumbnail and storage into cache dir
            if (! utils::image::thumbnailExist(path)) {
                // Generate thumbnail failed, do not insert into DB
                if (! utils::image::generateThumbnail(path)) {
                    continue;
                }
            }

            DBImgInfo dbi;
            dbi.fileName = fi.fileName();
            dbi.filePath = path;
            dbi.dirHash = utils::base::hash(m_root);
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

void DirCollectThread::setStop(bool stop)
{
    m_stop = stop;
}

const QString DirCollectThread::dir() const
{
    return m_root;
}

FilesCollectThread::FilesCollectThread(const QStringList &paths, const QString &album)
    : QThread(NULL)
    , m_album(album)
    , m_paths(paths)
    , m_stop(false)
{

}

void FilesCollectThread::run()
{
    DBImgInfoList dbInfos;
    QStringList supportPaths;
    QStringList dbPaths = DBManager::instance()->getAllPaths();
    using namespace utils::image;

    qint64 bt = QDateTime::currentMSecsSinceEpoch();
    for (auto path : m_paths) {
        if (m_stop) {
            return;
        }
        if (! imageSupportRead(path)) {
            continue;
        }
        supportPaths << path;

        if (dbPaths.contains(path)) {
            continue;
        }

        // Generate thumbnail and storage into cache dir
        if (! utils::image::thumbnailExist(path)) {
            // Generate thumbnail failed, do not insert into DB
            if (! utils::image::generateThumbnail(path)) {
                continue;
            }
        }

        QFileInfo fi(path);
        DBImgInfo dbi;
        dbi.fileName = fi.fileName();
        dbi.filePath = path;
        dbi.dirHash = utils::base::hash(QString());
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
    if (! dbInfos.isEmpty())
        emit resultReady(dbInfos);
    if (! m_album.isEmpty())
        emit insertAlbumRequest(m_album, supportPaths);
}

void FilesCollectThread::setStop(bool stop)
{
    m_stop = stop;
}
