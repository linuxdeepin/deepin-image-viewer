#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>
#include <QPixmap>

class Importer : public QObject
{
    Q_OBJECT
public:
    static Importer *instance();
    double getProgress() const;
    void importFromPath(const QString &path);
    void importSingleFile(const QString &filePath);

public slots:
    void importThreadFinish(const QString &filePath);

signals:
    void importProgressChanged(int importedCount, double progress);

private:
    explicit Importer(QObject *parent = 0);

private:
    static Importer *m_importer;
    QStringList m_importList;
    int m_readCount;
    double m_progress;
};

#endif // IMPORTER_H
