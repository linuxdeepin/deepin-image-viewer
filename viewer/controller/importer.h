#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>
#include <QFuture>

class QThreadPool;
class Importer : public QObject
{
    Q_OBJECT
public:
    static Importer *instance();
    void showImportDialog(const QString &album = "");
    void appendDir(const QString &path, const QString &album = "");
    void appendFiles(const QStringList &paths, const QString &album = "");
    void cancel();
    double progress() const;

signals:
    void imported(bool success);
    void progressChanged(double progress, int count);

protected:
    void timerEvent(QTimerEvent *event);

private:
    explicit Importer(QObject *parent = 0);

private:
    static Importer         *m_importer;
    double                  m_progress;
    int                     m_tid;
    QList<QFuture<void>>    m_futures;
    QStringList             m_cacheDirs;
    QThreadPool             *m_pool;
};

#endif // IMPORTER_H
