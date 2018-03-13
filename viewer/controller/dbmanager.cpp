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
#include "dbmanager.h"
#include "application.h"
#include "signalmanager.h"
#include "utils/baseutils.h"
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace {

const QString DATABASE_PATH = QDir::homePath() + "/.local/share/deepin/deepin-image-viewer/";
const QString DATABASE_NAME = "deepinimageviewer.db";
const QString EMPTY_HASH_STR = utils::base::hash(QString(" "));

}  // namespace

DBManager *DBManager::m_dbManager = NULL;

DBManager *DBManager::instance()
{
    if (!m_dbManager) {
        m_dbManager = new DBManager();
    }

    return m_dbManager;
}

DBManager::DBManager(QObject *parent)
    : QObject(parent)
    , m_connectionName("default_connection")
{
    checkDatabase();
}

const QStringList DBManager::getAllPaths() const
{
    QStringList paths;
    const QSqlDatabase db = getDatabase();
    if (! db.isValid())
        return paths;

    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT "
                   "FilePath "
                   "FROM ImageTable3 ORDER BY Time DESC");
    if (! query.exec()) {
        qWarning() << "Get Data from ImageTable3 failed: " << query.lastError();
        return paths;
    }
    else {
        while (query.next()) {
            paths << query.value(0).toString();
        }
    }

    return paths;
}

const DBImgInfoList DBManager::getAllInfos() const
{
    DBImgInfoList infos;
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return infos;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT FilePath, FileName, Dir, Time "
                   "FROM ImageTable3 ORDER BY Time DESC");
    if (! query.exec()) {
        qWarning() << "Get data from ImageTable3 failed: " << query.lastError();
        return infos;
    }
    else {
        using namespace utils::base;
        while (query.next()) {
            DBImgInfo info;
            info.filePath = query.value(0).toString();
            info.fileName = query.value(1).toString();
            info.dirHash = query.value(2).toString();
            info.time = stringToDateTime(query.value(3).toString());

            infos << info;
        }
    }

    return infos;
}

const QStringList DBManager::getAllTimelines() const
{
    QStringList times;
    const QSqlDatabase db = getDatabase();
    if (! db.isValid())
        return times;

    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT DISTINCT Time "
                   "FROM ImageTable3 ORDER BY Time DESC");
    if (! query.exec()) {
        qWarning() << "Get Data from ImageTable3 failed: " << query.lastError();
        return times;
    }
    else {
        while (query.next()) {
            times << query.value(0).toString();
        }
    }

    return times;
}

const DBImgInfoList DBManager::getInfosByTimeline(const QString &timeline) const
{/*
    const DBImgInfoList list = getImgInfos("Time", timeline);
    if (list.count() < 1) {
        return DBImgInfoList();
    }
    else {
        return list;
    }*/
}

const DBImgInfo DBManager::getInfoByName(const QString &name) const
{
    DBImgInfoList list = getImgInfos("FileName", name);
    if (list.count() < 1) {
        return DBImgInfo();
    }
    else {
        return list.first();
    }
}

const DBImgInfo DBManager::getInfoByPath(const QString &path) const
{
    DBImgInfoList list = getImgInfos("FilePath", path);
    if (list.count() != 1) {
        return DBImgInfo();
    }
    else {
        return list.first();
    }
}

const DBImgInfo DBManager::getInfoByPathHash(const QString &pathHash) const
{
    DBImgInfoList list = getImgInfos("PathHash", pathHash);
    if (list.count() != 1) {
        return DBImgInfo();
    }
    else {
        return list.first();
    }
}

int DBManager::getImgsCount() const
{
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return 0;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.exec("BEGIN IMMEDIATE TRANSACTION");
    query.prepare( "SELECT COUNT(*) FROM ImageTable3" );
    if (query.exec()) {
        query.first();
        int count = query.value(0).toInt();
        query.exec("COMMIT");
        return count;
    }

    return 0;
}

int DBManager::getImgsCountByDir(const QString &dir) const
{
    const QSqlDatabase db = getDatabase();
    if (dir.isEmpty() || ! db.isValid()) {
        return 0;
    }

    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare("SELECT COUNT(*) FROM ImageTable3 "
                          "WHERE Dir=:Dir AND FilePath !=\" \"");
    query.bindValue(":Dir", utils::base::hash(dir));
    if (query.exec()) {
        query.first();
        return query.value(0).toInt();
    }
    else {
        qDebug() << "Get images count by Dir failed :" << query.lastError();
        return 0;
    }
}

const QStringList DBManager::getPathsByDir(const QString &dir) const
{
    QStringList list;
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return list;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT DISTINCT FilePath FROM ImageTable3 "
                   "WHERE Dir=:dir " );
    query.bindValue(":dir", utils::base::hash(dir));
    if (! query.exec() ) {
        qWarning() << "Get Paths from ImageTable3 failed: " << query.lastError();
    }
    else {
        while (query.next()) {
            list << query.value(0).toString();
        }
    }

    return list;
}

bool DBManager::isImgExist(const QString &path) const
{
    const QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        return false;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.exec("BEGIN IMMEDIATE TRANSACTION");
    query.prepare( "SELECT COUNT(*) FROM ImageTable3 WHERE FilePath = :path" );
    query.bindValue( ":path", path );
    if (query.exec()) {
        query.first();
        if (query.value(0).toInt() > 0) {
            query.exec("COMMIT");
            return true;
        }
    }

    return false;
}

void DBManager::insertImgInfos(const DBImgInfoList &infos)
{
    const QSqlDatabase db = getDatabase();
    if (infos.isEmpty() || ! db.isValid()) {
        return;
    }

    QVariantList pathhashs, filenames, filepaths, dirs, times;

    for (DBImgInfo info : infos) {
        filenames << info.fileName;
        filepaths << info.filePath;
        pathhashs << utils::base::hash(info.filePath);
        dirs << info.dirHash;
        times << utils::base::timeToString(info.time, true);
    }

    // Insert into ImageTable3
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.exec("BEGIN IMMEDIATE TRANSACTION");
    query.prepare( "REPLACE INTO ImageTable3 "
                   "(PathHash, FilePath, FileName, Dir, Time) VALUES (?, ?, ?, ?, ?)" );
    query.addBindValue(pathhashs);
    query.addBindValue(filepaths);
    query.addBindValue(filenames);
    query.addBindValue(dirs);
    query.addBindValue(times);
    if (! query.execBatch()) {
        qWarning() << "Insert data into ImageTable3 failed: "
                   << query.lastError();
        query.exec("COMMIT");
    }
    else {
        query.exec("COMMIT");
        emit dApp->signalM->imagesInserted(infos);
    }
}

void DBManager::removeImgInfos(const QStringList &paths)
{
    QSqlDatabase db = getDatabase();
    if (paths.isEmpty() || ! db.isValid()) {
        return;
    }

    // Collect info before removing data
    DBImgInfoList infos;
    QStringList pathHashs;
    for (QString path : paths) {
        pathHashs << utils::base::hash(path);
        infos << getInfoByPath(path);
    }

    QSqlQuery query(db);
    // Remove from albums table
    query.setForwardOnly(true);
    query.exec("BEGIN IMMEDIATE TRANSACTION");
    QString qs = "DELETE FROM AlbumTable3 WHERE PathHash=?";
    query.prepare(qs);
    query.addBindValue(pathHashs);
    if (! query.execBatch()) {
        qWarning() << "Remove data from AlbumTable3 failed: "
                   << query.lastError();
        query.exec("COMMIT");
    }
    else {
        query.exec("COMMIT");
    }

    // Remove from image table
    query.exec("BEGIN IMMEDIATE TRANSACTION");
    qs = "DELETE FROM ImageTable3 WHERE PathHash=?";
    query.prepare(qs);
    query.addBindValue(pathHashs);
    if (! query.execBatch()) {
        qWarning() << "Remove data from ImageTable3 failed: "
                   << query.lastError();
        query.exec("COMMIT");
    }
    else {
        emit dApp->signalM->imagesRemoved(infos);
        query.exec("COMMIT");
    }
}

void DBManager::removeDir(const QString &dir)
{
    QSqlDatabase db = getDatabase();
    if (dir.isEmpty() || ! db.isValid()) {
        return;
    }

    const QString dirHash = utils::base::hash(dir);
    // Collect info before removing data
    DBImgInfoList infos = getImgInfos("Dir", dirHash);
    QStringList pathHashs;
    for (auto info : infos) {
        pathHashs << utils::base::hash(info.filePath);
    }

    QSqlQuery query(db);
    // Remove from albums table
    query.setForwardOnly(true);
    query.exec("BEGIN IMMEDIATE TRANSACTION");
    QString qs = "DELETE FROM AlbumTable3 WHERE PathHash=?";
    query.prepare(qs);
    query.addBindValue(pathHashs);
    if (! query.execBatch()) {
        qWarning() << "Remove data from AlbumTable3 failed: "
                   << query.lastError();
    }
    query.exec("COMMIT");

    // Remove from image table
    query.exec("BEGIN IMMEDIATE TRANSACTION");
    qs = "DELETE FROM ImageTable3 WHERE Dir=:Dir";
    query.prepare(qs);
    query.bindValue(":Dir", dirHash);
    if (! query.exec()) {
        qWarning() << "Remove dir's images from ImageTable3 failed: "
                   << query.lastError();
        query.exec("COMMIT");
    }
    else {
        query.exec("COMMIT");
        emit dApp->signalM->imagesRemoved(infos);
    }
}

const DBAlbumInfo DBManager::getAlbumInfo(const QString &album) const
{
    DBAlbumInfo info;
    QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return info;
    }

    info.name = album;
    QStringList pathHashs;

    QSqlQuery query( db );
    query.setForwardOnly(true);
    QString ps = "SELECT DISTINCT PathHash FROM AlbumTable3 "
                 "WHERE AlbumName =:name AND PathHash != \"%1\" ";
    query.prepare( ps.arg(EMPTY_HASH_STR) );
    query.bindValue(":name", album);
    if ( ! query.exec() ) {
        qWarning() << "Get data from AlbumTable3 failed: "
                   << query.lastError();
    }
    else {
        while (query.next()) {
            pathHashs << query.value(0).toString();
        }
    }

    info.count = pathHashs.length();
    if (pathHashs.length() == 1) {
        info.endTime = getInfoByPathHash(pathHashs.first()).time;
        info.beginTime = info.endTime;
    }
    else if (pathHashs.length() > 1) {
        //TODO: The images' info in AlbumTable need dateTime
        //If: without those, need to loop access dateTime
        foreach (QString pHash,  pathHashs) {
            QDateTime tmpTime = getInfoByPathHash(pHash).time;
            if (tmpTime < info.beginTime || info.beginTime.isNull()) {
                info.beginTime = tmpTime;
            }

            if (tmpTime > info.endTime || info.endTime.isNull()) {
                info.endTime = tmpTime;
            }
        }
    }

    return info;
}

const QStringList DBManager::getAllAlbumNames() const
{
    QStringList list;
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return list;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT DISTINCT AlbumName FROM AlbumTable3" );
    if ( !query.exec() ) {
        qWarning() << "Get AlbumNames failed: " << query.lastError();
    }
    else {
        while (query.next()) {
            list << query.value(0).toString();
        }
    }

    return list;
}

const QStringList DBManager::getPathsByAlbum(const QString &album) const
{
    QStringList list;
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return list;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare("SELECT DISTINCT i.FilePath "
                  "FROM ImageTable3 AS i, AlbumTable3 AS a "
                  "WHERE i.PathHash=a.PathHash "
                  "AND a.AlbumName=:album "
                  "AND FilePath != \" \" ");
    query.bindValue(":album", album);
    if (! query.exec() ) {
        qWarning() << "Get Paths from AlbumTable3 failed: " << query.lastError();
    }
    else {
        while (query.next()) {
            list << query.value(0).toString();
        }
    }

    return list;
}

const DBImgInfoList DBManager::getInfosByAlbum(const QString &album) const
{
    DBImgInfoList infos;
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return infos;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare("SELECT DISTINCT i.FilePath, i.FileName, i.Dir, i.Time "
                  "FROM ImageTable3 AS i, AlbumTable3 AS a "
                  "WHERE i.PathHash=a.PathHash AND a.AlbumName=:album");
    query.bindValue(":album", album);
    if (! query.exec()) {
        qWarning() << "Get ImgInfo by album failed: " << query.lastError();
    }
    else {
        using namespace utils::base;
        while (query.next()) {
            DBImgInfo info;
            info.filePath = query.value(0).toString();
            info.fileName = query.value(1).toString();
            info.dirHash = query.value(2).toString();
            info.time = stringToDateTime(query.value(3).toString());

            infos << info;
        }
    }
    return infos;
}

int DBManager::getImgsCountByAlbum(const QString &album) const
{
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return 0;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    QString ps = "SELECT COUNT(*) FROM AlbumTable3 "
                 "WHERE AlbumName =:album AND PathHash != \"%1\" ";
    query.prepare( ps.arg(EMPTY_HASH_STR) );
    query.bindValue(":album", album);
    if (query.exec()) {
        query.first();
        return query.value(0).toInt();
    }
    else {
        qDebug() << "Get images count error :" << query.lastError();
        return 0;
    }
}

int DBManager::getAlbumsCount() const
{
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return 0;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare("SELECT COUNT(DISTINCT AlbumName) FROM AlbumTable3");
    if (query.exec()) {
        query.first();
        return query.value(0).toInt();
    }
    else {
        return 0;
    }
}

bool DBManager::isImgExistInAlbum(const QString &album, const QString &path) const
{
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return false;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT COUNT(*) FROM AlbumTable3 WHERE PathHash = :hash "
                   "AND AlbumName = :album");
    query.bindValue( ":hash", utils::base::hash(path) );
    query.bindValue( ":album", album );
    if (query.exec()) {
        query.first();
        return (query.value(0).toInt() == 1);
    }
    else {
        return false;
    }
}

bool DBManager::isAlbumExistInDB(const QString &album) const
{
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return false;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT COUNT(*) FROM AlbumTable3 WHERE AlbumName = :album");
    query.bindValue( ":album", album );
    if (query.exec()) {
        query.first();
        return (query.value(0).toInt() >= 1);
    }
    else {
        return false;
    }
}

void DBManager::insertIntoAlbum(const QString &album, const QStringList &paths)
{
    const QSqlDatabase db = getDatabase();
    if (! db.isValid() || album.isEmpty()) {
        return;
    }
    QStringList nameRows, pathHashRows;
    for (QString path : paths) {
        nameRows << album;
        pathHashRows << utils::base::hash(path);
    }

    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.exec("BEGIN IMMEDIATE TRANSACTION");
    query.prepare("REPLACE INTO AlbumTable3 (AlbumId, AlbumName, PathHash) "
                  "VALUES (null, ?, ?)");
    query.addBindValue(nameRows);
    query.addBindValue(pathHashRows);
    if (! query.execBatch()) {
        qWarning() << "Insert data into AlbumTable3 failed: "
                   << query.lastError();
    }
    query.exec("COMMIT");

    //FIXME: Don't insert the repeated filepath into the same album
    //Delete the same data
    QString ps = "DELETE FROM AlbumTable3 where AlbumId NOT IN"
                 "(SELECT min(AlbumId) FROM AlbumTable3 GROUP BY"
                 " AlbumName, PathHash) AND PathHash != \"%1\" ";
    query.prepare(ps.arg(EMPTY_HASH_STR));
    if (!query.exec()) {
        qDebug() << "delete same date failed!";
    }
    query.exec("COMMIT");

    emit dApp->signalM->insertedIntoAlbum(album, paths);
}

void DBManager::removeAlbum(const QString &album)
{
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare("DELETE FROM AlbumTable3 WHERE AlbumName=:album");
    query.bindValue(":album", album);
    if (!query.exec()) {
        qWarning() << "Remove album from database failed: " << query.lastError();
    }
}

void DBManager::removeFromAlbum(const QString &album, const QStringList &paths)
{
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return;
    }

    QStringList pathHashs;
    for (QString path : paths) {
        pathHashs << utils::base::hash(path);
    }
    QSqlQuery query(db);
    query.setForwardOnly(true);
    query.exec("BEGIN IMMEDIATE TRANSACTION");
    // Remove from albums table
    QString qs("DELETE FROM AlbumTable3 WHERE AlbumName=\"%1\" AND PathHash=?");
    query.prepare(qs.arg(album));
    query.addBindValue(pathHashs);
    if (! query.execBatch()) {
        qWarning() << "Remove images from DB failed: " << query.lastError();
    }
    else {
        emit dApp->signalM->removedFromAlbum(album, paths);
    }
    query.exec("COMMIT");
}

void DBManager::renameAlbum(const QString &oldAlbum, const QString &newAlbum)
{
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare("UPDATE AlbumTable3 SET "
                  "AlbumName = :newName "
                  "WHERE AlbumName = :oldName ");
    query.bindValue( ":newName", newAlbum );
    query.bindValue( ":oldName", oldAlbum );
    if (! query.exec()) {
        qWarning() << "Update album name failed: " << query.lastError();
    }
}

const DBImgInfoList DBManager::getImgInfos(const QString &key, const QString &value) const
{
    DBImgInfoList infos;
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return infos;
    }
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare(QString("SELECT FilePath, FileName, Dir, Time FROM ImageTable3 "
                          "WHERE %1= :value ORDER BY Time DESC").arg(key));
    query.bindValue(":value", value);
    if (!query.exec()) {
        qWarning() << "Get Image from database failed: " << query.lastError();
    }
    else {
        using namespace utils::base;
        while (query.next()) {
            DBImgInfo info;
            info.filePath = query.value(0).toString();
            info.fileName = query.value(1).toString();
            info.dirHash = query.value(2).toString();
            info.time = stringToDateTime(query.value(3).toString());

            infos << info;
        }
    }

    return infos;
}

const QSqlDatabase DBManager::getDatabase() const
{
    if( QSqlDatabase::contains(m_connectionName) ) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        return db;
    }
    else {
        //if database not open, open it.
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);//not dbConnection
        db.setDatabaseName(DATABASE_PATH + DATABASE_NAME);
        if (! db.open()) {
            qWarning()<< "Open database error:" << db.lastError();
            return QSqlDatabase();
        }
        else {
            return db;
        }
    }
}

void DBManager::checkDatabase()
{
    QDir dd(DATABASE_PATH);
    if (! dd.exists()) {
        qDebug() << "create database paths";
        dd.mkpath(DATABASE_PATH);
        if (dd.exists())
            qDebug() << "create database succeed!";
        else
            qDebug() << "create database failed!";
    } else {
        qDebug() << "database is exist!";
    }
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return;
    }
    bool tableExist = false;
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT name FROM sqlite_master "
                   "WHERE type=\"table\" AND name = \"ImageTable3\"");
    if (query.exec() && query.first()) {
        tableExist = ! query.value(0).toString().isEmpty();
    }
    //if tables not exist, create it.
    if ( ! tableExist ) {
        QSqlQuery query(db);
        // ImageTable3
        //////////////////////////////////////////////////////////////
        //PathHash           | FilePath | FileName   | Dir  | Time  //
        //TEXT primari key   | TEXT     | TEXT       | TEXT | TEXT  //
        //////////////////////////////////////////////////////////////
        query.exec( QString("CREATE TABLE IF NOT EXISTS ImageTable3 ( "
                            "PathHash TEXT primary key, "
                            "FilePath TEXT, "
                            "FileName TEXT, "
                            "Dir TEXT, "
                            "Time TEXT )"));

        // AlbumTable3
        //////////////////////////////////////////////////////////
        //AlbumId               | AlbumName         | PathHash  //
        //INTEGER primari key   | TEXT              | TEXT      //
        //////////////////////////////////////////////////////////
        query.exec( QString("CREATE TABLE IF NOT EXISTS AlbumTable3 ( "
                            "AlbumId INTEGER primary key, "
                            "AlbumName TEXT, "
                            "PathHash TEXT)") );

//        // Check if there is an old version table exist or not

//        //TODO: AlbumTable's primary key is changed, need to importVersion again
//        importVersion1Data();
//        importVersion2Data();
    }

}

void DBManager::importVersion1Data()
{
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return;
    }
    bool tableExist = false;
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT name FROM sqlite_master "
                   "WHERE type=\"table\" AND name = \"ImageTable\"");
    if (query.exec() && query.first()) {
        tableExist = ! query.value(0).toString().isEmpty();
    }

    if (tableExist) {
        // Import ImageTable into ImageTable3
        query.clear();
        query.setForwardOnly(true);
        query.prepare( "SELECT filename, filepath, time "
                       "FROM ImageTable ORDER BY time DESC");
        if (! query.exec()) {
            qWarning() << "Import ImageTable into ImageTable3 failed: "
                       << query.lastError();
        }
        else {
            DBImgInfoList infos;
            using namespace utils::base;
            while (query.next()) {
                DBImgInfo info;
                info.fileName = query.value(0).toString();
                info.filePath = query.value(1).toString();
                info.time = stringToDateTime(query.value(2).toString());

                infos << info;
            }
            insertImgInfos(infos);
        }

        // Import AlbumTable into AlbumTable3
        query.clear();
        query.prepare("SELECT DISTINCT a.albumname, i.filepath "
                      "FROM ImageTable AS i, AlbumTable AS a "
                      "WHERE i.filename=a.filename ");
        if (! query.exec()) {
            qWarning() << "Import AlbumTable into AlbumTable3 failed: "
                       << query.lastError();
        }
        else {
            // <Album-Paths>
            QMap<QString, QStringList> aps;
            using namespace utils::base;
            while (query.next()) {
                QString album = query.value(0).toString();
                QString path = query.value(1).toString();
                if (aps.keys().contains(album)) {
                    aps[album].append(path);
                }
                else {
                    aps.insert(album, QStringList(path));
                }
            }
            for (QString album : aps.keys()) {
                insertIntoAlbum(album, aps[album]);
            }
        }

        // Drop old table
        query.clear();
        query.prepare("DROP TABLE AlbumTable");
        if (! query.exec()) {
            qWarning() << "Drop old tables failed: " << query.lastError();
        }
        query.prepare("DROP TABLE ImageTable");
        if (! query.exec()) {
            qWarning() << "Drop old tables failed: " << query.lastError();
        }
    }
}

void DBManager::importVersion2Data()
{
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return;
    }
    bool tableExist = false;
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT name FROM sqlite_master "
                   "WHERE type=\"table\" AND name = \"ImageTable2\"");
    if (query.exec() && query.first()) {
        tableExist = ! query.value(0).toString().isEmpty();
    }

    if (tableExist) {
        // Import ImageTable2 into ImageTable3
        query.clear();
        query.prepare( "SELECT FileName, FilePath, Time "
                       "FROM ImageTable2 ORDER BY Time DESC");
        if (! query.exec()) {
            qWarning() << "Import ImageTable2 into ImageTable3 failed: "
                       << query.lastError();
        }
        else {
            DBImgInfoList infos;
            using namespace utils::base;
            while (query.next()) {
                DBImgInfo info;
                info.fileName = query.value(0).toString();
                info.filePath = query.value(1).toString();
                info.time = stringToDateTime(query.value(2).toString());

                infos << info;
            }
            insertImgInfos(infos);
        }

        // Import AlbumTable2 into AlbumTable3
        query.clear();
        query.prepare(" SELECT AlbumName, FilePath FROM AlbumTable2 ");
        if (! query.exec()) {
            qWarning() << "Import AlbumTable2 into AlbumTable3 failed: "
                       << query.lastError();
        }
        else {
            // <Album-Paths>
            QMap<QString, QStringList> aps;
            using namespace utils::base;
            while (query.next()) {
                QString album = query.value(0).toString();
                QString path = query.value(1).toString();
                if (aps.keys().contains(album)) {
                    aps[album].append(path);
                }
                else {
                    aps.insert(album, QStringList(path));
                }
            }
            for (QString album : aps.keys()) {
                insertIntoAlbum(album, aps[album]);
            }
        }

        // Drop old table
        query.clear();
        query.prepare("DROP TABLE AlbumTable2");
        if (! query.exec()) {
            qWarning() << "Drop old tables failed: " << query.lastError();
        }
        query.prepare("DROP TABLE ImageTable2");
        if (! query.exec()) {
            qWarning() << "Drop old tables failed: " << query.lastError();
        }
    }
}
