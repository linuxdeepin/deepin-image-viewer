#ifndef EXPORTER_H
#define EXPORTER_H

#include <QObject>
#include <QMap>

class Exporter : public QObject {
    Q_OBJECT
public:
    static Exporter *instance();

public slots:
    void exportImage(const QStringList imagePaths);
    void exportAlbum(const QString &albumname);
    void popupDialogSaveImage(const QStringList imagePaths);
private:
    explicit Exporter(QObject *parent = 0);
    static Exporter *m_exporter;
    QMap<QString, QString> m_picFormatMap;

    void initValidFormatMap();
    QString getOrderFormat(QString defaultFormat);
};

#endif // EXPORTER_H
