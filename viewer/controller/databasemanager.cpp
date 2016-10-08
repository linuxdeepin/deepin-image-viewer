#include "databasemanager.h"
#include "application.h"
#include "signalmanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

const QString DATABASE_PATH = QDir::homePath() + "/.local/share/deepin/deepin-image-viewer/";
const QString DATABASE_NAME = "deepinimageviewer.db";
const QString IMAGE_TABLE_NAME = "ImageTable";
const QString ALBUM_TABLE_NAME = "AlbumTable";

void DatabaseManager::updateImageInfo(const DatabaseManager::ImageInfo &info)
{
    QMutexLocker locker(&m_mutex);

    QSqlDatabase db = getDatabase();
    if (db.isValid() && imageExist(info.name)) {
        QSqlQuery query( db );
        query.prepare( QString("UPDATE %1 SET "
                               "filepath = :path, "
                               "album = :album, "
                               "label = :label, "
                               "time = :time, "
                               "thumbnail = :thumbnail "
                               "WHERE filename = :name")
                       .arg( IMAGE_TABLE_NAME ) );
        query.bindValue( ":path", info.path );
        query.bindValue( ":album", info.albums );
        query.bindValue( ":label", info.labels );
        query.bindValue( ":time", utils::base::timeToString(info.time) );
        QByteArray inByteArray;
        QBuffer inBuffer( &inByteArray );
        inBuffer.open( QIODevice::WriteOnly );
        if ( !info.thumbnail.save( &inBuffer, "JPG" )) { // write inPixmap into inByteArray
            qDebug() << "Write pixmap to buffer error!" << info.name;
        }
        query.bindValue( ":thumbnail", inByteArray);
        query.bindValue( ":name", info.name );
        if (!query.exec()) {
            qWarning() << "Update image database failed: " << query.lastError();
        }
    }
}

QList<DatabaseManager::ImageInfo> DatabaseManager::getImageInfosByAlbum(
        const QString &album)
{
    // TODO it should read the DB directly
    QStringList nameList = getImageNamesByAlbum(album);
    QList<DatabaseManager::ImageInfo> infoList;

    for (const QString name : nameList) {
        if (name.isEmpty())
            continue;
        infoList << getImageInfoByName(name);
    }

    return infoList;
}

QList<DatabaseManager::ImageInfo> DatabaseManager::getImageInfosByTimeline(const QString &timeline)
{
    QList<ImageInfo> tList;
    QList<ImageInfo> infos = getAllImageInfos();
    for (ImageInfo info : infos) {
        if (utils::base::timeToString(info.time, true) == timeline) {
            tList << info;
        }
    }

    return tList;
}

DatabaseManager::ImageInfo DatabaseManager::getImageInfoByName(const QString &name)
{
    QList<ImageInfo> list = getImageInfos("filename", name);
    if (list.count() != 1) {
        return ImageInfo();
    }
    else {
        return list.first();
    }
}

DatabaseManager::ImageInfo DatabaseManager::getImageInfoByPath(const QString &path)
{
    QList<ImageInfo> list = getImageInfos("filepath", path);
    if (list.count() != 1) {
        return ImageInfo();
    }
    else {
        return list.first();
    }
}

void DatabaseManager::insertImageInfos(const QList<ImageInfo> &infos)
{
    if (infos.length() < 1) {
        return;
    }
    QSqlDatabase db = getDatabase();

    if (db.isValid()) {
//         const QString recentImport("Recent imported");
         QVariantList filenames, filepaths, albumNames;
         QVariantList labels, times, thumbnails;

         QVariantList albumInfos, albumImgNames, albumInfoTimes;
         for (ImageInfo info : infos) {
             filenames << info.name;
             filepaths << info.path;
             albumNames << QVariant(info.albums).toString();
             labels << QVariant(info.labels).toString();
             times << utils::base::timeToString(info.time);
             thumbnails << info.thumbnail;

             QStringList albums = info.albums/* << recentImport*/;
             albums.removeAll("");
             for (QString album : albums) {
                 albumInfos << album;
                 albumImgNames << info.name;
                 albumInfoTimes << utils::base::timeToString(info.time);
             }
         }
         // Insert into images table
         QSqlQuery query( db );
         query.exec("BEGIN IMMEDIATE TRANSACTION");
         query.prepare(QString("REPLACE INTO %1"
                       "(filename, filepath, album, label, time, thumbnail) "
                       "VALUES (?, ?, ?, ?, ?, ?)").arg(IMAGE_TABLE_NAME));
         query.addBindValue(filenames);
         query.addBindValue(filepaths);
         query.addBindValue(albumNames);
         query.addBindValue(labels);
         query.addBindValue(times);
         query.addBindValue(thumbnails);
         if (! query.execBatch()) {
             qWarning() << "Insert images into images table failed: "
                        << query.lastError();
             query.exec("COMMIT");
         }
         else {
             query.exec("COMMIT");
             emit dApp->signalM->imagesInserted(infos);
         }

         // Clear Recent imported album
//         query.clear();
//         query.exec("BEGIN IMMEDIATE TRANSACTION");
//         query.prepare( QString("DELETE FROM %1 WHERE albumname = :name")
//                        .arg(ALBUM_TABLE_NAME) );
//         query.bindValue( ":name", recentImport );
//         if (! query.exec()) {
//             qWarning() << "Remove album from database failed: "
//                        << query.lastError();
//         }

         // Insert into album table
         query.clear();
         query.exec("BEGIN IMMEDIATE TRANSACTION");
         query.prepare("REPLACE INTO AlbumTable(albumname, filename, time) "
                       "VALUES (?, ?, ?)");
         query.addBindValue(albumInfos);
         query.addBindValue(albumImgNames);
         query.addBindValue(albumInfoTimes);
         if (! query.execBatch()) {
             qWarning() << "Insert images into album table failed: "
                        << query.lastError();
             query.exec("COMMIT");
         }
         else {
             query.exec("COMMIT");
         }
     }
}

void DatabaseManager::removeImages(const QStringList &names)
{
    QSqlDatabase db = getDatabase();
    if (! db.isValid())
        return;
    QString nStr;
    for (QString name : names) {
        nStr += "\"" + name + "\",";
    }
    nStr.remove(nStr.length() - 1, 1);

    QSqlQuery query(db);
    // Remove from albums table
    // Note: the value may contain the % character DONOT use QString::arg()
    QString queryStr = "DELETE FROM " + ALBUM_TABLE_NAME +
            " WHERE filename IN (" + nStr +")";
    query.prepare(queryStr);
    if (! query.exec()) {
        qWarning() << "Remove images from DB failed: " << query.lastError();
    }
    else {
        const QStringList al = getAlbumNameList();
        for (QString album : al) {
            emit dApp->signalM->removedFromAlbum(album, names);
        }
    }

    // Remove from image table
    queryStr = "DELETE FROM " + IMAGE_TABLE_NAME + " WHERE filename IN (" + nStr + ")";
    query.prepare(queryStr);
    if (! query.exec()) {
        qWarning() << "Remove images from DB failed: " << query.lastError();
    }
    else {
        emit dApp->signalM->imagesRemoved(names);
    }
}

bool DatabaseManager::imageExist(const QString &name)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.exec("BEGIN IMMEDIATE TRANSACTION");
        query.prepare( QString("SELECT COUNT(*) FROM %1 WHERE filename = :name")
                       .arg( IMAGE_TABLE_NAME ) );
        query.bindValue( ":name", name );
        if (query.exec()) {
            query.first();
            if (query.value(0).toInt() > 0) {
                query.exec("COMMIT");
                return true;
            }
        }
    }

    return false;
}

int DatabaseManager::imageCount()
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.exec("BEGIN IMMEDIATE TRANSACTION");
        query.prepare( QString("SELECT COUNT(*) FROM %1").arg( IMAGE_TABLE_NAME ) );
        if (query.exec()) {
            query.first();
            int count = query.value(0).toInt();
            query.exec("COMMIT");
            return count;
        }
    }

    return 0;
}

int DatabaseManager::getImagesCountByMonth(const QString &month)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT COUNT(*) FROM %1 WHERE time LIKE \'%2%\'")
                        .arg( IMAGE_TABLE_NAME ).arg( month ) );
        if (query.exec()) {
            query.first();
            int count = query.value(0).toInt();
            query.exec("COMMIT");
            return count;
        }
    }

    return 0;
}

void DatabaseManager::insertImageIntoAlbum(const QString &albumname,
                                           const QString &filename,
                                           const QString &time)
{
    //insert into test(albumname, filename, time)
    //select "album4", "file4", "2016.1.2"
    //where not exists(
    //select * from test where albumname="album4" and filename="file4");
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare(QString("INSERT OR IGNORE INTO %1(albumname, filename, time) "
                      "SELECT :albumname, :filename, :time "
                      "WHERE NOT EXISTS ( "
                      "SELECT * FROM AlbumTable "
                      "WHERE albumname=:albumname AND filename=:filename)")
                      .arg(ALBUM_TABLE_NAME));
        query.bindValue(":albumname", albumname);
        query.bindValue(":filename", filename);
        query.bindValue(":time", time);
        if (!query.exec()) {
            qWarning() << "Insert into album failed: " << query.lastError();
        }

        // For UI update
        ImageInfo info = getImageInfoByName(filename);
        info.albums << albumname;
        emit dApp->signalM->insertIntoAlbum(info);
    }
}

void DatabaseManager::removeImageFromAlbum(const QString &albumname,
                                           const QString &filename)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString( "DELETE FROM %1 WHERE albumname = :albumname "
                                "AND filename = :filename" )
                       .arg(ALBUM_TABLE_NAME) );
        query.bindValue( ":albumname", albumname );
        query.bindValue( ":filename", filename );
        if (!query.exec()) {
            qWarning() << "Remove image record from album failed: "
                       << query.lastError();
        }
    }

    // For UI update
    emit dApp->signalM->removedFromAlbum(albumname, QStringList(filename));
}

void DatabaseManager::removeImagesFromAlbum(const QString &album,
                                            const QStringList &names)
{
    QSqlDatabase db = getDatabase();
    if (! db.isValid())
        return;
    QString nStr;
    for (QString name : names) {
        //TODO: there will be better way to filter the imagename include
        //special char " () &etc.
        if (name.contains("\"") || name.contains("(")
                || name.contains(")"))
            removeImageFromAlbum(album, name);
        else
            nStr += "\"" + name + "\",";
    }
    nStr.remove(nStr.length() - 1, 1);

    QSqlQuery query(db);
    // Remove from albums table
    // Note: the value may contain the % character DONOT use QString::arg()
    const QString queryStr = "DELETE FROM " + ALBUM_TABLE_NAME +
            " WHERE albumname = \"" + album + "\"AND filename IN (" + nStr + ")";
    query.prepare(queryStr);
    if (! query.exec()) {
        qWarning() << "Remove images from DB failed: " << query.lastError();
    }
    else {
        emit dApp->signalM->removedFromAlbum(album, names);
    }
}

DatabaseManager::AlbumInfo DatabaseManager::getAlbumInfo(const QString &name)
{
    AlbumInfo info;
    info.name = name;

    QStringList nameList;
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare(QString("SELECT DISTINCT filename FROM %1 "
            "WHERE albumname =:name AND filename != \"\" ORDER BY time DESC")
                      .arg(ALBUM_TABLE_NAME));
        query.bindValue(":name", name);
        if ( !query.exec() ) {
            qWarning() << "Get images from AlbumTable failed: "
                       << query.lastError();
        }
        else {
            while (query.next()) {
                nameList << query.value(0).toString();
            }
        }
    }

    info.count = nameList.length();
    if (nameList.length() == 1) {
        info.endTime = getImageInfoByName(nameList.first()).time;
        info.beginTime = info.endTime;
    }
    else if (nameList.length() > 1) {
        info.endTime = getImageInfoByName(nameList.first()).time;
        info.beginTime = getImageInfoByName(nameList.last()).time;
    }

    return info;
}

void DatabaseManager::removeAlbum(const QString &name)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("DELETE FROM %1 WHERE albumname = :name")
                       .arg(ALBUM_TABLE_NAME) );
        query.bindValue( ":name", name );
        if (!query.exec()) {
            qWarning() << "Remove album from database failed: "
                       << query.lastError();
        }
    }
}

void DatabaseManager::renameAlbum(const QString &oldName, const QString &newName)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("UPDATE %1 SET "
                               "albumname = :newName "
                               "WHERE albumname = :oldName ")
                       .arg( ALBUM_TABLE_NAME ) );
        query.bindValue( ":newName", newName );
        query.bindValue( ":oldName", oldName );
        if (!query.exec()) {
            qWarning() << "Update album name failed: " << query.lastError();
        }
    }
}

void DatabaseManager::clearRecentImported()
{
    const QString a = "Recent imported";
    removeAlbum(a);
    // Make sure Recent imported always show in UI
    insertImageIntoAlbum(a, "", "");
}

bool DatabaseManager::imageExistAlbum(const QString &name, const QString &album)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.exec("BEGIN IMMEDIATE TRANSACTION");
        query.prepare( QString("SELECT COUNT(*) FROM %1 WHERE filename = :name "
                               "AND albumname = :album")
                       .arg( ALBUM_TABLE_NAME ) );
        query.bindValue( ":name", name );
        query.bindValue( ":album", album );
        if (query.exec()) {
            query.first();
            if (query.value(0).toInt() == 1) {
                query.exec("COMMIT");
                return true;
            }
        }
    }

    return false;
}

QStringList DatabaseManager::getAlbumNameList()
{
    QStringList list;
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT DISTINCT albumname FROM %1")
                       .arg(ALBUM_TABLE_NAME) );
        if ( !query.exec() ) {
            qWarning() << "Get AlbumNames failed: " << query.lastError();
        }
        else {
            while (query.next()) {
                list << query.value(0).toString();
            }
        }
    }

    return list;
}

QStringList DatabaseManager::getImageNamesByAlbum(const QString &album)
{
    QStringList nameList;
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        QString queryStr = "SELECT DISTINCT filename FROM " + ALBUM_TABLE_NAME
            + " WHERE albumname = \"" + album +"\" ORDER BY time DESC";
        query.prepare(queryStr);
        if ( !query.exec() ) {
            qWarning() << "Get images from AlbumTable failed: "
                       << query.lastError();
        }
        else {
            while (query.next()) {
                nameList << query.value(0).toString();
            }
        }
    }

    return nameList;
}

int DatabaseManager::albumsCount()
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.exec("BEGIN IMMEDIATE TRANSACTION");
        query.prepare( QString("SELECT COUNT(DISTINCT albumname) FROM %1")
                       .arg( ALBUM_TABLE_NAME ) );
        if (query.exec()) {
            query.first();
            int count = query.value(0).toInt();
            query.exec("COMMIT");
            return count;
        }
    }

    return 0;
}

int DatabaseManager::getImagesCountByAlbum(const QString &album)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare(QString("SELECT COUNT(*) FROM %1 "
                      "WHERE albumname =:albumname AND filename !=\"\"")
                      .arg(ALBUM_TABLE_NAME));
        query.bindValue(":albumname", album);
        if (query.exec()) {
            query.first();
            int count = query.value(0).toInt();
            query.exec("COMMIT");
            return count;
        }
        else {
            qDebug() << "Get images count error :" << query.lastError();
        }
    }

    return 0;
}

QStringList DatabaseManager::getTimeLineList(bool ascending)
{
    QStringList list;
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare(QString("SELECT DISTINCT time FROM %1 ORDER BY time %2").
                      arg(IMAGE_TABLE_NAME).
                      arg(ascending ? "ASC" : "DESC"));
        if ( !query.exec() ) {
            qWarning() << "Get TimeLine failed: " << query.lastError();
        }
        else {
            for ( int i = 0; query.next(); i ++ ) {
                using namespace utils::base;
                const QString tl = timeToString(
                            stringToDateTime(query.value(0).toString()), true);
                if (list.indexOf(tl) == -1)
                    list << tl;
            }
        }
    }

    return list;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent),m_connectionName("default_connection")
{
    checkDatabase();

    // Destruct current instance before process exits.
    // Or else DatabaseManager::~DatabaseManager() will never be called.
    connect(qApp, &QApplication::aboutToQuit,
            [&]() {
        delete this;
    });
}

DatabaseManager *DatabaseManager::m_databaseManager = NULL;
DatabaseManager *DatabaseManager::instance()
{
    if (!m_databaseManager) {
        m_databaseManager = new DatabaseManager;
    }

    return m_databaseManager;
}

DatabaseManager::~DatabaseManager()
{
    QSqlDatabase db = getDatabase();
    if (db.isOpen()) {
        db.close();
    }
}

const QStringList DatabaseManager::getAllImagesName()
{
    QStringList nameList;
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT "
                               "filename "
                               "FROM %1 ORDER BY time DESC")
                       .arg( IMAGE_TABLE_NAME ));
        if (!query.exec()) {
            qWarning() << "Get Image from database failed: " << query.lastError();
            return nameList;
        }
        else {
            while (query.next()) {
                nameList << query.value(0).toString();
            }
        }
    }

    return nameList;
}

const QStringList DatabaseManager::getAllImagesPath()
{
    QStringList pathList;
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT "
                               "filepath "
                               "FROM %1 ORDER BY time DESC")
                       .arg( IMAGE_TABLE_NAME ));
        if (!query.exec()) {
            qWarning() << "Get Image from database failed: " << query.lastError();
            return pathList;
        }
        else {
            while (query.next()) {
                pathList << query.value(0).toString();
            }
        }
    }

    return pathList;
}

const QList<DatabaseManager::ImageInfo> DatabaseManager::getAllImageInfos()
{
    QList<ImageInfo> infoList;
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT "
                               "filename, filepath, album, label, time, thumbnail "
                               "FROM %1 ORDER BY time DESC")
                       .arg( IMAGE_TABLE_NAME ));
        if (!query.exec()) {
            qWarning() << "Get Image from database failed: " << query.lastError();
            return infoList;
        }
        else {
            while (query.next()) {
                ImageInfo info;
                info.name = query.value(0).toString();
                info.path = query.value(1).toString();
                info.albums = query.value(2).toStringList();
                info.labels = query.value(3).toStringList();
                info.time = utils::base::stringToDateTime(query.value(4).toString());
                info.thumbnail.loadFromData(query.value(5).toByteArray());

                infoList << info;
            }
        }
    }

    return infoList;
}

QList<DatabaseManager::ImageInfo> DatabaseManager::getImageInfos(const QString &key, const QString &value)
{
    QMutexLocker locker(&m_mutex);

    QList<ImageInfo> infoList;
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        // Note: the value may contain the % character DONOT use QString::arg()
        // const QString queryStr = QString() + "SELECT " +
        //     " filename, filepath, album, label, time, thumbnail " +
        //     " FROM " + IMAGE_TABLE_NAME +
        //     " WHERE " + key + " = \"" + value + "\" ORDER BY time DESC";

        query.prepare(QString("SELECT "
                      "filename, filepath, album, label, time, thumbnail "
                      "FROM %1 WHERE %2= :value "
                      "ORDER BY time DESC").arg(IMAGE_TABLE_NAME).arg(key));

        query.bindValue(":value", value);
        if (!query.exec()) {
            qWarning() << "Get Image from database failed: " << query.lastError();
            return infoList;
        }
        else {
            while (query.next()) {
                ImageInfo info;
                info.name = query.value(0).toString();
                info.path = query.value(1).toString();
                info.albums = query.value(2).toStringList();
                info.labels = query.value(3).toStringList();
                info.time = utils::base::stringToDateTime(query.value(4).toString());
                info.thumbnail.loadFromData(query.value(5).toByteArray());

                infoList << info;
            }
        }
    }

    return infoList;
}

QSqlDatabase DatabaseManager::getDatabase()
{
    if( QSqlDatabase::contains(m_connectionName) )
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        return db;
    }
    else {
        //if database not open, open it.
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);//not dbConnection
        db.setDatabaseName(DATABASE_PATH + DATABASE_NAME);
        if (!db.open()) {
            qWarning()<< "Open database error:" << db.lastError();
            return QSqlDatabase();
        }
        else {
            //            qDebug() << "Database Opended!";
            return db;
        }
    }
}

void DatabaseManager::checkDatabase()
{
    //if database not exist, create it.
    QDir dp(DATABASE_PATH);
    if ( !dp.exists() ) {
        dp.mkpath(DATABASE_PATH);
    }

    QSqlDatabase db = getDatabase();
    if (!db.isValid()) {
        return;
    }

    QSqlQuery query(db);
    //////////////////////////////////////////////////////////////////////////
    //filename              | filepath | album | label | time | thumbnail   //
    //TEXT primari key      | TEXT     | TEXT  | TEXT  | TEXT | BLOB        //
    //////////////////////////////////////////////////////////////////////////

    //image_file
    query.exec( QString("CREATE TABLE IF NOT EXISTS %1 ( "
                        "filename TEXT primary key, "
                        "filepath TEXT, "
                        "album TEXT, "      //array
                        "label TEXT,"       //array
                        "time TEXT, "
                        "thumbnail BLOB )").arg(IMAGE_TABLE_NAME) );

    //////////////////////////////////////////////////////////////////////////
    //albumid             | albumname         | filename      | time        //
    //INTEGER primari key | TEXT              | TEXT          | TEXT        //
    //////////////////////////////////////////////////////////////////////////

    //album
    query.exec( QString("CREATE TABLE IF NOT EXISTS %1 ( "
                        "albumid INTEGER primary key, "
                        "albumname TEXT, "
                        "filename TEXT, "
                        "time TEXT )").arg(ALBUM_TABLE_NAME) );
}
