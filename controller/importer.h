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

class DatabaseManager;
class Importer : public QObject
{
    Q_OBJECT
public:
    static Importer *instance();
    double getProgress() const;
    int finishedCount() const;
    void showImportDialog();
    void stopImport();
    void importFromPath(const QString &path, const QString &album = "");
    void importSingleFile(const QString &filePath, const QString &album = "");

    QStringList getAlbums(const QString &path) const;

signals:
    void importProgressChanged(double progress);
    void importStart();
private slots:
    void onFutureWatcherFinish();
    void onFutureResultReady(int index);

private:
    explicit Importer(QObject *parent = 0);
    void loadCacheImages();

private:
    static Importer *m_importer;
    DatabaseManager *m_dbManager;
    QStringList m_cacheImportList;
    QMap<QString, QString> m_albums;  // <path, album>
    QFutureWatcher<QString> m_futureWatcher;
    double m_progress;
    int m_imagesCount;
};

#endif // IMPORTER_H
