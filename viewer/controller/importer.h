#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>

class Importer : public QObject
{
    Q_OBJECT
public:
    static Importer *instance();
    void showImportDialog(const QString &album = "");
    void importDir(const QString &path, const QString &album = "");
    void importFiles(const QStringList &paths, const QString &album = "");

signals:
    void imported(bool success);

private:
    explicit Importer(QObject *parent = 0);

private:
    static Importer *m_importer;
};

#endif // IMPORTER_H
