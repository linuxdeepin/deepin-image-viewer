#include "importer.h"
#include "application.h"
#include "controller/dbmanager.h"
#include "controller/signalmanager.h"
#include "utils/imageutils.h"
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QMutex>
#include <QtConcurrent>
#include <QVariant>

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

QMutex mutex;
void importDir(const QString &path, const QString &album)
{
    QMutexLocker locker(&mutex);
    const QFileInfoList fileInfos = utils::image::getImagesInfo(path, false);
    if(fileInfos.isEmpty() ) {
        return;
    }

    QStringList ePaths;
    DBImgInfoList imgInfos;
    for (QFileInfo finfo : fileInfos) {
        QString ep = finfo.absoluteFilePath().toUtf8().toPercentEncoding("/");
        DBImgInfo imgInfo;
        imgInfo.fileName = finfo.fileName().toUtf8().toPercentEncoding();
        imgInfo.filePath = ep;
        imgInfo.time = utils::image::getCreateDateTime(finfo.absoluteFilePath());

        ePaths << ep;
        imgInfos << imgInfo;
    }
    dApp->dbM->insertImgInfos(imgInfos);
    dApp->dbM->insertIntoAlbum(album, ePaths);
}

void importFiles(const QStringList &dPaths, const QString &album)
{
    QMutexLocker locker(&mutex);
    DBImgInfoList imgInfos;
    QStringList ePaths;
    for (QString dp : dPaths) {
        if (! utils::image::imageSupportRead(dp)) {
            continue;
        }
        QFileInfo info(dp);
        QString ep = info.absoluteFilePath().toUtf8().toPercentEncoding("/");
        DBImgInfo imgInfo;
        imgInfo.fileName = info.fileName().toUtf8().toPercentEncoding();
        imgInfo.filePath = ep;
        imgInfo.time = utils::image::getCreateDateTime(dp);

        ePaths << ep;
        imgInfos << imgInfo;

    }
    dApp->dbM->insertImgInfos(imgInfos);
    dApp->dbM->insertIntoAlbum(album, ePaths);
}

}  // namespace

Importer::Importer(QObject *parent)
    : QObject(parent)
    , m_progress(1)
    , m_tid(0)
    , m_pool(new QThreadPool())
{
    m_pool->setMaxThreadCount(1);
    m_pool->setExpiryTimeout(60000*30);
}

Importer *Importer::m_importer = NULL;
Importer *Importer::instance()
{
    if (!m_importer) {
        m_importer = new Importer();
    }

    return m_importer;
}

void Importer::showImportDialog(const QString &album)
{
    QString dir = QFileDialog::getExistingDirectory(
                nullptr, tr("Open Directory"), QDir::homePath(),
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) return;
    appendDir(dir, album);
}

void Importer::appendDir(const QString &path, const QString &album)
{
    if (! QDir(path).exists() || m_cacheDirs.contains(path)) {
        return;
    }
    if (m_progress == 1) {
        emit progressChanged(0, 0);
    }
    m_cacheDirs << path;
    // If path is root or something like that, collect sub-dirs will spend a lot of time
    QFuture<QStringList> df = QtConcurrent::run(collectSubDirs, path);
    QTimer *t = new QTimer(this);
    t->setSingleShot(false);
    connect(t, &QTimer::timeout, this, [=] {
        if (df.isFinished()) {
            if (m_tid == 0) {
                m_tid = startTimer(1000);
            }
            QStringList dirs = df.result();
            for (QString dir : dirs) {
                QFuture<void> future = QtConcurrent::run(m_pool, importDir, dir, album);
                m_futures.append(future);
            }

            t->deleteLater();
        }
    });
    t->start(200);
}

void Importer::appendFiles(const QStringList &paths, const QString &album)
{
    if (paths.isEmpty()) {
        return;
    }
    if (m_tid == 0) {
        m_tid = startTimer(1000);
    }
    QFuture<void> future = QtConcurrent::run(m_pool, importFiles, paths, album);
    m_futures.append(future);
}

void Importer::cancel()
{
    m_pool->clear();
    killTimer(m_tid);
    m_tid = 0;
    m_progress = 1;
    m_futures.clear();
    m_cacheDirs.clear();
    emit imported(true);
    emit progressChanged(1, 0);
}

double Importer::progress() const
{
    return m_progress;
}

void Importer::timerEvent(QTimerEvent *event)
{
    // Check if import progress is finish
    if (event->timerId() == m_tid) {
        bool allFinished = true;
        int dcount = 0;
        for (auto f : m_futures) {
            if (f.isRunning()) {
                allFinished = false;
            }
            else if (f.isFinished()) {
                dcount ++;
            }
        }
        if (allFinished) {
            killTimer(m_tid);
            m_tid = 0;
            m_progress = 1;
            m_futures.clear();
            m_cacheDirs.clear();
            emit imported(true);
            emit progressChanged(1, dcount);
        }
        else {
            m_progress = 1.0 * dcount / m_futures.count();
            emit progressChanged(m_progress, dcount);
        }
        // Note: if m_futures is too large, the m_pool will auto reserveThread
        // and i do not know why
        m_pool->releaseThread();
    }
}
