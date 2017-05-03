#ifndef DBMANAGER_H
#define DBMANAGER_H

// ImageTable
///////////////////////////////////////////////////////
//FilePath           | FileName | Dir        | Time  //
//TEXT primari key   | TEXT     | TEXT       | TEXT  //
///////////////////////////////////////////////////////

// AlbumTable
/////////////////////////////////////////////////////
//AP               | AlbumName         | FilePath  //
//TEXT primari key | TEXT              | TEXT      //
/////////////////////////////////////////////////////

#include <QObject>
#include <QDateTime>
#include <QMutex>
#include <QDebug>

struct DBAlbumInfo {
    QString name;
    int count;
    QDateTime beginTime;
    QDateTime endTime;
};

struct DBImgInfo {
    QString filePath;
    QString fileName;
    QString dirHash;
    QDateTime time;

    bool operator==(const DBImgInfo& other)
    {
        return (filePath == other.filePath &&
                fileName == other.fileName &&
                time == other.time);
    }

    friend QDebug operator<<(QDebug &dbg, const DBImgInfo& info) {
        dbg << "(DBImgInfo)["
            << "Path:" << info.filePath
            << "Name:" << info.fileName
            << "Dir:" << info.dirHash
            << "Time:" << info.time
            << "]";
        return dbg;
    }
};
typedef QList<DBImgInfo> DBImgInfoList;

class QSqlDatabase;
class DBManager : public QObject
{
    Q_OBJECT
public:
    static DBManager* instance();
    explicit DBManager(QObject *parent = 0);

    // TableImage
    const QStringList       getAllPaths() const;
    const DBImgInfoList     getAllInfos() const;
    const QStringList       getAllTimelines() const;
    const DBImgInfoList     getInfosByTimeline(const QString &timeline) const;
    const DBImgInfo         getInfoByName(const QString &name) const;
    const DBImgInfo         getInfoByPath(const QString &path) const;
    const DBImgInfo         getInfoByPathHash(const QString &pathHash) const;
    int                     getImgsCount() const;
    int                     getImgsCountByDir(const QString &dir) const;
    const QStringList       getPathsByDir(const QString &dir) const;
    bool                    isImgExist(const QString &path) const;
    void insertImgInfos(const DBImgInfoList &infos);
    void removeImgInfos(const QStringList &paths);
    void removeDir(const QString &dir);

    // TableAlbum
    const DBAlbumInfo       getAlbumInfo(const QString &album) const;
    const QStringList       getAllAlbumNames() const;
    const QStringList       getPathsByAlbum(const QString &album) const;
    const DBImgInfoList     getInfosByAlbum(const QString &album) const;
    int                     getImgsCountByAlbum(const QString &album) const;
    int                     getAlbumsCount() const;
    bool                    isAlbumExistInDB(const QString &album) const;
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
    void importVersion2Data();

    static DBManager* m_dbManager;
private:
    QString m_connectionName;
    QMutex m_mutex;
};

#endif // DBMANAGER_H
