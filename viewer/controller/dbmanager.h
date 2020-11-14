/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
    QString format;
    QDateTime time;

    bool operator==(const DBImgInfo &other)
    {
        return (filePath == other.filePath &&
                fileName == other.fileName &&
                time == other.time);
    }

    //add by heyi
    void clear()
    {
        filePath.clear();
        fileName.clear();
        dirHash.clear();
    }

    friend QDebug operator<<(QDebug &dbg, const DBImgInfo &info)
    {
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
Q_DECLARE_METATYPE(DBImgInfoList)

class QSqlDatabase;
class DBManager : public QObject
{
    Q_OBJECT
public:
    static DBManager *instance();
    explicit DBManager(QObject *parent = nullptr);

    // TableImage
    const QStringList       getAllPaths() const;
    const DBImgInfoList     getInfosByTimeline(const QString &timeline) const;
    const DBImgInfo         getInfoByPath(const QString &path) const;
    const QStringList       getPathsByDir(const QString &dir) const;
    void insertImgInfos(const DBImgInfoList &infos);
    void removeImgInfos(const QStringList &paths);
    void removeDir(const QString &dir);

    // TableAlbum
    void insertIntoAlbum(const QString &album, const QStringList &paths);

private:
    const DBImgInfoList getImgInfos(const QString &key, const QString &value) const;
    const QSqlDatabase getDatabase() const;
    void checkDatabase();

    static DBManager *m_dbManager;
private:
    QString m_connectionName;
    mutable QMutex m_mutex;
};

#endif // DBMANAGER_H
