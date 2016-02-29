#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QDateTime>
#include <QSqlDatabase>

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

    explicit DatabaseManager(const QString &connectionName, QObject *parent = 0);
    ~DatabaseManager();

    ImageInfo getImageInfoByAlbum(const QString &album);
    ImageInfo getImageInfoByTime(const QDateTime &time);
    ImageInfo getImageInfoByName(const QString &name);
    void insertImageInfo(const ImageInfo &info);
    void updateImageInfo(const ImageInfo &info);
    void removeImage(const QString &name);
    bool imageExist(const QString &name);
    AlbumInfo getAlbumInfo(const QString &name);
    void insertAlbumInfo(const AlbumInfo &info);
    void updateAlbumInfo(const AlbumInfo &info);
    void removeAlbum(const QString &name);
    bool albumExist(const QString &name);
    QStringList getAlbumNameList();
    QStringList getTimeLineList();

private:

    ImageInfo getImageInfo(const QString &key, const QString &value);
    QSqlDatabase getDatabase();
    void checkDatabase();

    QString m_connectionName;
};

#endif // DATABASEMANAGER_H
