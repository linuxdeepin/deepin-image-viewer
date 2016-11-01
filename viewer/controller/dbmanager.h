#ifndef DBMANAGER_H
#define DBMANAGER_H

// ImageTable
//////////////////////////////////////////
//FilePath           | FileName | Time  //
//TEXT primari key   | TEXT     | TEXT  //
//////////////////////////////////////////

// AlbumTable
/////////////////////////////////////////////////////
//AP               | AlbumName         | FilePath  //
//TEXT primari key | TEXT              | TEXT      //
/////////////////////////////////////////////////////

#include <QObject>
#include <QDateTime>
#include <QMutex>

struct DBAlbumInfo {
    QString name;
    int count;
    QDateTime beginTime;
    QDateTime endTime;
};

struct DBImgInfo {
    QString filePath;
    QString fileName;
    QDateTime time;
};
typedef QList<DBImgInfo> DBImgInfoList;

class QSqlDatabase;
class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = 0);

    // TableImage
    const QStringList       getAllPaths() const;
    const DBImgInfoList     getAllInfos() const;
    const QStringList       getAllTimelines() const;
    const DBImgInfoList     getInfosByTimeline(const QString &timeline) const;
    const DBImgInfo         getInfoByName(const QString &name) const;
    const DBImgInfo         getInfoByPath(const QString &path) const;
    int                     getImgsCount() const;
    bool                    isImgExist(const QString &path) const;
    void insertImgInfos(const DBImgInfoList &infos);
    void removeImgInfos(const QStringList &paths);

    // TableAlbum
    const DBAlbumInfo       getAlbumInfo(const QString &album) const;
    const QStringList       getAllAlbumNames() const;
    const QStringList       getPathsByAlbum(const QString &album) const;
    const DBImgInfoList     getInfosByAlbum(const QString &album) const;
    int                     getImgsCountByAlbum(const QString &album) const;
    int                     getAlbumsCount() const;
    bool                    isImgExistInAlbum(const QString &album, const QString &path) const;
    void insertIntoAlbum(const QString &album, const QStringList &paths);
    void removeAlbum(const QString &album);
    void removeFromAlbum(const QString &album, const QStringList &paths);
    void renameAlbum(const QString &oldAlbum, const QString &newAlbum);

private:
    const DBImgInfoList getImgInfos(const QString &key, const QString &value) const;
    const QSqlDatabase getDatabase() const;
    void checkDatabase();
    void importVersion1Data();

private:
    QString m_connectionName;
    QMutex m_mutex;
};

#endif // DBMANAGER_H
