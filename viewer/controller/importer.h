#ifndef IMPORTER_H
#define IMPORTER_H

#include "dbmanager.h"
#include <QFileInfo>
#include <QFutureWatcher>
#include <QObject>
#include <QThread>

class DirCollectThread : public QThread
{
    Q_OBJECT
public:
    DirCollectThread(const QString &root, const QString &album);
    void run() Q_DECL_OVERRIDE;
    void setStop(bool stop);
    const QString dir() const;

signals:
    void currentImport(const QString &path);
    void resultReady(const DBImgInfoList &infos);
    void insertAlbumRequest(const QString &album, const QStringList &paths);

private:
    QString m_album;
    QString m_root;
    bool m_stop;
};


class FilesCollectThread : public QThread
{
    Q_OBJECT
public:
    FilesCollectThread(const QStringList &paths, const QString &album);
    void run() Q_DECL_OVERRIDE;
    void setStop(bool stop);

signals:
    void currentImport(const QString &path);
    void resultReady(const DBImgInfoList &infos);
    void insertAlbumRequest(const QString &album, const QStringList &paths);
private:
    QString m_album;
    QStringList m_paths;
    bool m_stop;
};


class Importer : public QObject
{
    Q_OBJECT
public:
    explicit Importer(QObject *parent = 0);
    bool isRunning() const;
    void appendDir(const QString &path, const QString &album = "");
    void appendFiles(const QStringList &paths, const QString &album = "");
    void stop();
    void stopDirCollect(const QString &dir);
    void showImportDialog(const QString &album = "");

signals:
    void currentImport(const QString &path);
    void imported(bool success);
    void progressChanged();

private:
    QList<QThread *> m_threads;
    QStringList m_dirs;
};

#endif // IMPORTER_H
