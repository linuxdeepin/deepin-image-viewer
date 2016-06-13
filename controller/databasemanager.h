#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QDateTime>
#include <QSqlDatabase>
#include <QMutex>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    struct ImageInfo {
        QString name;
        QString path;
        QStringList albums; // Discard
        QStringList labels;
        QDateTime time;
        QPixmap thumbnail;
    };
    struct AlbumInfo {
        QString name;
        int count;
        QDateTime beginTime;
        QDateTime endTime;
    };

    static DatabaseManager *instance();
    ~DatabaseManager();

    QList<ImageInfo> getImageInfosByAlbum(const QString &album);
    QList<ImageInfo> getImageInfosByTime(const QDateTime &time);
    ImageInfo getImageInfoByName(const QString &name);
    ImageInfo getImageInfoByPath(const QString &path);
    void insertImageInfo(const ImageInfo &info);
    void updateImageInfo(const ImageInfo &info);
    void updateThumbnail(const QString &name);
    void removeImage(const QString &name);
    bool imageExist(const QString &name);
    int getImagesCountByMonth(const QString &month);
    int imageCount();

    AlbumInfo getAlbumInfo(const QString &name);
    QStringList getAlbumNameList();
    QStringList getImageNamesByAlbum(const QString &album);
    void insertImageIntoAlbum(const QString &albumname,
                              const QString &filename,
                              const QString &time);
    void removeImageFromAlbum(const QString &albumname, const QString &filename);
    void removeAlbum(const QString &name);
    void renameAlbum(const QString &oldName, const QString &newName);
    void clearRecentImported();
    bool imageExistAlbum(const QString &name, const QString &album);
    int getImagesCountByAlbum(const QString &album);
    int albumsCount();

    QStringList getTimeLineList(bool ascending = true);

private:
    explicit DatabaseManager(QObject *parent = 0);

    QList<ImageInfo> getImageInfos(const QString &key, const QString &value);
    QSqlDatabase getDatabase();
    void checkDatabase();

private:
    static DatabaseManager *m_databaseManager;
    QString m_connectionName;
    QMutex m_mutex;
};

#endif // DATABASEMANAGER_H
