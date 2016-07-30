#ifndef IMPORTER_H
#define IMPORTER_H

#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-tag.h>
#include <QObject>
#include <QPixmap>
#include <QFuture>
#include <QFutureWatcher>
#include <QMap>
#include <QtConcurrent>

class QTimer;
class Importer : public QObject
{
    Q_OBJECT
public:
    static Importer *instance();
    bool isRunning() const;
    double getProgress() const;
    int finishedCount() const;
    void nap();
    void showImportDialog();
    void stopImport();
    void importDir(const QString &path, const QString &album = "");
    void importFiles(const QStringList &files, const QString &album = "");

    QStringList getAlbums(const QString &path) const;

signals:
    void importProgressChanged(double progress);

private slots:
    void onFutureWatcherFinish();
    void onFutureResultReady(int index);

private:
    explicit Importer(QObject *parent = 0);
    void loadCacheImages();

private:
    static Importer *m_importer;
    QStringList m_cacheImportList;
    QMap<QString, QString> m_albums;  // <path, album>
    QFutureWatcher<QString> m_futureWatcher;
    double m_progress;
    int m_imagesCount;
};

#endif // IMPORTER_H
