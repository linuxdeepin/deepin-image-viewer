#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QDateTime>
#include <QSqlDatabase>
const QString DATETIME_FORMAT = "dd.MM.yyyy";

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    struct ImageInfo {
        QString name;
        QString path;
        QStringList albums;
        QStringList labels;
        QDateTime time;
        QPixmap thumbnail;
    };
    struct AlbumInfo {
        QString name;
        QString size;
        int count;
        QDateTime createTime;
        QDateTime earliestTime;
        QDateTime latestTime;
    };

    static DatabaseManager *instance();
    ~DatabaseManager();

    QList<ImageInfo> getImageInfoByAlbum(const QString &album);
    QList<ImageInfo> getImageInfoByTime(const QDateTime &time);
    ImageInfo getImageInfoByName(const QString &name);
    void insertImageInfo(const ImageInfo &info);
    void updateImageInfo(const ImageInfo &info);
    void removeImage(const QString &name);
    bool imageExist(const QString &name);
    int imageCount();
    AlbumInfo getAlbumInfo(const QString &name);
    void insertAlbumInfo(const AlbumInfo &info);
    void updateAlbumInfo(const AlbumInfo &info);
    void removeAlbum(const QString &name);
    bool albumExist(const QString &name);
    QStringList getAlbumNameList();
    QStringList getTimeLineList();
    const QStringList supportImageType();

private:
    explicit DatabaseManager(QObject *parent = 0);

    QList<ImageInfo> getImageInfos(const QString &key, const QString &value);
    QSqlDatabase getDatabase();
    void checkDatabase();

private:
    static DatabaseManager *m_databaseManager;
    QString m_connectionName;
};

#endif // DATABASEMANAGER_H
