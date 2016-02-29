#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>
#include <QPixmap>

class Importer : public QObject
{
    Q_OBJECT
public:
    explicit Importer(QObject *parent = 0);
    void importFromPath(const QString &path);
    void importSingleFile(const QString &filePath);

public slots:
    void importThreadFinish(const QString &filePath);

private:
    QStringList m_importList;
    int m_readCount;
};

#endif // IMPORTER_H
