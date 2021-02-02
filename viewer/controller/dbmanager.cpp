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

DBManager *DBManager::m_dbManager = nullptr;

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

    QMutexLocker mutex(&m_mutex);
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT "
                   "FilePath "
                   "FROM ImageTable3 ORDER BY Time DESC");
    if (! query.exec()) {
        qWarning() << "Get Data from ImageTable3 failed: " << query.lastError();
        mutex.unlock();
        return paths;
    } else {
        while (query.next()) {
            paths << query.value(0).toString();
        }
    }
    mutex.unlock();

    return paths;
}


const DBImgInfoList DBManager::getInfosByTimeline(const QString &timeline) const
{
    Q_UNUSED(timeline);
    DBImgInfoList ret;
    /*
       const DBImgInfoList list = getImgInfos("Time", timeline);
       if (list.count() < 1) {
           return DBImgInfoList();
       }
       else {
           return list;
       }*/

    return ret;
}


const DBImgInfo DBManager::getInfoByPath(const QString &path) const
{
    DBImgInfoList list = getImgInfos("FilePath", path);
    if (list.count() != 1) {
        return DBImgInfo();
    } else {
        return list.first();
    }
}



const QStringList DBManager::getPathsByDir(const QString &dir) const
{
    QStringList list;
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return list;
    }
    QMutexLocker mutex(&m_mutex);
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT DISTINCT FilePath FROM ImageTable3 "
                   "WHERE Dir=:dir " );
    query.bindValue(":dir", utils::base::hash(dir));
    if (! query.exec() ) {
        qWarning() << "Get Paths from ImageTable3 failed: " << query.lastError();
        mutex.unlock();
    } else {
        while (query.next()) {
            list << query.value(0).toString();
        }
    }
    mutex.unlock();

    return list;
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

    QMutexLocker mutex(&m_mutex);
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
        mutex.unlock();
    } else {
        query.exec("COMMIT");
        mutex.unlock();
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

    QMutexLocker mutex(&m_mutex);
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
    } else {
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
    } else {
        emit dApp->signalM->imagesRemoved(infos);
        query.exec("COMMIT");
    }
    mutex.unlock();
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

    QMutexLocker mutex(&m_mutex);
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
    } else {
        query.exec("COMMIT");
        emit dApp->signalM->imagesRemoved(infos);
    }
    mutex.unlock();
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

    QMutexLocker mutex(&m_mutex);
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
    mutex.unlock();

    emit dApp->signalM->insertedIntoAlbum(album, paths);
}

const DBImgInfoList DBManager::getImgInfos(const QString &key, const QString &value) const
{
    DBImgInfoList infos;
    const QSqlDatabase db = getDatabase();
    if (! db.isValid()) {
        return infos;
    }
    QMutexLocker mutex(&m_mutex);
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare(QString("SELECT FilePath, FileName, Dir, Time FROM ImageTable3 "
                          "WHERE %1= :value ORDER BY Time DESC").arg(key));
    query.bindValue(":value", value);
    if (!query.exec()) {
        qWarning() << "Get Image from database failed: " << query.lastError();
        mutex.unlock();
    } else {
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
    mutex.unlock();
    return infos;
}

const QSqlDatabase DBManager::getDatabase() const
{
    QMutexLocker mutex(&m_mutex);
    if ( QSqlDatabase::contains(m_connectionName) ) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        mutex.unlock();
        return db;
    } else {
        //if database not open, open it.
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);//not dbConnection
        db.setDatabaseName(DATABASE_PATH + DATABASE_NAME);
        if (! db.open()) {
            qWarning() << "Open database error:" << db.lastError();
            mutex.unlock();
            return QSqlDatabase();
        } else {
            mutex.unlock();
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
    QMutexLocker mutex(&m_mutex);
    QSqlQuery query( db );
    query.setForwardOnly(true);
    query.prepare( "SELECT name FROM sqlite_master "
                   "WHERE type=\"table\" AND name = \"ImageTable3\"");
    if (query.exec() && query.first()) {
        tableExist = ! query.value(0).toString().isEmpty();
    }
    //if tables not exist, create it.
    if ( ! tableExist ) {
//        QSqlQuery query(db);
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
    mutex.unlock();

}
