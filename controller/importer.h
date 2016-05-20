#ifndef IMPORTER_H
#define IMPORTER_H

#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-tag.h>
#include <QObject>
#include <QPixmap>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

class Importer : public QObject
{
    Q_OBJECT
public:
    static Importer *instance();
    double getProgress() const;
    void showImportDialog();
    void importFromPath(const QString &path);
    void importSingleFile(const QString &filePath);

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
    QFutureWatcher<QString> m_futureWatcher;
    double m_progress;
    int m_imagesCount;
};

#endif // IMPORTER_H
