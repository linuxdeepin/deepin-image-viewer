#include "databasemanager.h"
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QBuffer>
#include <QDebug>
#include <QDir>

const QString DATABASE_PATH = QDir::homePath() + "/.local/share/deepin/deepin-image-viewer/";
const QString DATABASE_NAME = "deepinimageviewer.db";
const QString IMAGE_TABLE_NAME = "ImageTable";
const QString ALBUM_TABLE_NAME = "AlbumTable";
const QString DATETIME_FORMAT = "dd.MM.yyyy";

void DatabaseManager::insertImageInfo(const DatabaseManager::ImageInfo &info)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid() && !imageExist(info.name)) {
        QSqlQuery query( db );
        query.prepare( QString("INSERT INTO %1 "
                               "(filename, filepath, album, label, time, thumbnail) "
                               "VALUES (:name, :path, :album, :label, :time, :thumbnail)")
                       .arg( IMAGE_TABLE_NAME ) );
        query.bindValue( ":name", info.name );
        query.bindValue( ":path", info.path );
        query.bindValue( ":album", info.albums );
        query.bindValue( ":label", info.labels );
        query.bindValue( ":time", info.time.toString(DATETIME_FORMAT) );
        QByteArray inByteArray;
        QBuffer inBuffer( &inByteArray );
        inBuffer.open( QIODevice::WriteOnly );
        info.thumbnail.save( &inBuffer ); // write inPixmap into inByteArray
        query.bindValue( ":thumbnail", inByteArray);
        if (!query.exec()) {
            qWarning() << "Insert image into database failed: " << query.lastError();
        }
    }
}

void DatabaseManager::updateImageInfo(const DatabaseManager::ImageInfo &info)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid() && !imageExist(info.name)) {
        QSqlQuery query( db );
        query.prepare( QString("UPDATE %1 SET"
                               "filepath = :path, album = :album, label = :label, time = :time, thumbnail = :thumbnail"
                               "WHERE filename = :name")
                       .arg( IMAGE_TABLE_NAME ) );
        query.bindValue( ":path", info.path );
        query.bindValue( ":album", info.albums );
        query.bindValue( ":label", info.labels );
        query.bindValue( ":time", info.time.toString(DATETIME_FORMAT) );
        QByteArray inByteArray;
        QBuffer inBuffer( &inByteArray );
        inBuffer.open( QIODevice::WriteOnly );
        info.thumbnail.save( &inBuffer ); // write inPixmap into inByteArray
        query.bindValue( ":thumbnail", inByteArray);
        query.bindValue( ":name", info.name );
        if (!query.exec()) {
            qWarning() << "Update image database failed: " << query.lastError();
        }
    }
}

DatabaseManager::ImageInfo DatabaseManager::getImageInfoByAlbum(const QString &album)
{
    return getImageInfo("album", album);
}

DatabaseManager::ImageInfo DatabaseManager::getImageInfoByTime(const QDateTime &time)
{
    return getImageInfo("time", time.toString(DATETIME_FORMAT));
}

DatabaseManager::ImageInfo DatabaseManager::getImageInfoByName(const QString &name)
{
    return getImageInfo("filename", name);
}

void DatabaseManager::removeImage(const QString &name)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString( "DELETE FROM %1 WHERE filename = :name" ).arg(IMAGE_TABLE_NAME) );
        query.bindValue( ":name", name );
        if (!query.exec()) {
            qWarning() << "Remove image record from database failed: " << query.lastError();
        }
    }
}

bool DatabaseManager::imageExist(const QString &name)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT COUNT(*) FROM %1 WHERE filename = :name").arg( IMAGE_TABLE_NAME ) );
        query.bindValue( ":name", name );
        if (query.exec()) {
            query.first();
            if (query.value(0).toInt() > 0) {
                return true;
            }
        }
    }

    return false;
}

void DatabaseManager::insertAlbumInfo(const DatabaseManager::AlbumInfo &info)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid() && !albumExist(info.name)) {
        QSqlQuery query( db );
        query.prepare( QString( "INSERT INTO %1 "
                                "(albumname, count, size, createtime, earliesttime, latesttime) "
                                "VALUES (:name, :count, :size, :createtime, :earliesttime, :latesttime)" )
                       .arg(ALBUM_TABLE_NAME) );
        query.bindValue( ":name", info.name );
        query.bindValue( ":count", info.count );
        query.bindValue( ":size", info.size );
        query.bindValue( ":createtime", info.createTime.toString(DATETIME_FORMAT) );
        query.bindValue( ":earliesttime", info.earliestTime.toString(DATETIME_FORMAT) );
        query.bindValue( ":latesttime", info.latestTime.toString(DATETIME_FORMAT));
        if (!query.exec()) {
            qWarning() << "Insert album into database failed: " << query.lastError();
        }
    }
}

void DatabaseManager::updateAlbumInfo(const DatabaseManager::AlbumInfo &info)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid() && !albumExist(info.name)) {
        QSqlQuery query( db );
        query.prepare( QString("UPDATE %1 SET"
                               "count = :count, size = :size, createtime = :createtime, earliesttime = :earliesttime, latesttime = :latesttime"
                               "WHERE albumname = :name")
                       .arg( ALBUM_TABLE_NAME ) );
        query.bindValue( ":count", info.count );
        query.bindValue( ":size", info.size );
        query.bindValue( ":createtime", info.createTime.toString(DATETIME_FORMAT) );
        query.bindValue( ":earliesttime", info.earliestTime.toString(DATETIME_FORMAT) );
        query.bindValue( ":latesttime", info.latestTime.toString(DATETIME_FORMAT));
        query.bindValue( ":name", info.name );
        if (!query.exec()) {
            qWarning() << "Update AlbumInfo failed: " << query.lastError();
        }
    }
}

DatabaseManager::AlbumInfo DatabaseManager::getAlbumInfo(const QString &name)
{
    QSqlDatabase db = getDatabase();
    AlbumInfo info;
    if (db.isValid() && albumExist(info.name)) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT "
                               "albumname, count, size, createtime, earliesttime, latesttime "
                               "FROM %1 WHERE albumname = :name ORDER BY :name ").arg(ALBUM_TABLE_NAME) );
        query.bindValue( ":name", name );
        if (!query.exec()) {
            qWarning() << "Get Album from database failed: " << query.lastError();
            return info;
        }
        else {
            if (query.record().count() > 1) {
                qWarning() << "Got duplicate data!";
                return info;
            }
            query.first();
            info.name = query.value(0).toString();
            info.count = query.value(1).toInt();
            info.size = query.value(2).toString();
            info.createTime = QDateTime::fromString( query.value(3).toString(), DATETIME_FORMAT );
            info.earliestTime = QDateTime::fromString( query.value(4).toString(), DATETIME_FORMAT );
            info.latestTime = QDateTime::fromString( query.value(5).toString(), DATETIME_FORMAT );
            return info;
        }
    }

    return info;
}

void DatabaseManager::removeAlbum(const QString &name)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("DELETE FROM %1 WHERE albumname = :name").arg(ALBUM_TABLE_NAME) );
        query.bindValue( ":name", name );
        if (!query.exec()) {
            qWarning() << "Remove album record from database failed: " << query.lastError();
        }
    }
}

bool DatabaseManager::albumExist(const QString &name)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT * FROM %1 WHERE albumname = :name").arg(ALBUM_TABLE_NAME) );
        query.bindValue( ":name", name );
        if (query.exec()) {
            query.first();
            if (query.value(0).toInt() > 0) {
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
        query.prepare( QString("SELECT DISTINCT album FROM %1").arg(IMAGE_TABLE_NAME) );
        if ( !query.exec() ) {
            qWarning() << "Get AlbumNames failed: " << query.lastError();
        }
        else {
            QSqlRecord rec = query.record();
            for ( int i = 0; query.next(); i ++ ) {
                list << query.value(0).toString();
            }
        }
    }

    return list;
}

QStringList DatabaseManager::getTimeLineList()
{
    QStringList list;
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT DISTINCT time FROM %1").arg(IMAGE_TABLE_NAME) );
        if ( !query.exec() ) {
            qWarning() << "Get TimeLine failed: " << query.lastError();
        }
        else {
            QSqlRecord rec = query.record();
            for ( int i = 0; query.next(); i ++ ) {
                list << query.value(0).toString();
            }
        }
    }

    return list;
}

DatabaseManager::DatabaseManager(const QString &connectionName, QObject *parent)
    : QObject(parent),m_connectionName(connectionName)
{
    checkDatabase();
}

DatabaseManager::~DatabaseManager()
{
    QSqlDatabase db = getDatabase();
    if (db.isOpen()) {
        db.close();
    }
}

DatabaseManager::ImageInfo DatabaseManager::getImageInfo(const QString &key, const QString &value)
{
    QSqlDatabase db = getDatabase();
    ImageInfo info;
    if (db.isValid() && imageExist(info.name)) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT "
                               "filename, filepath, album, label, time, thumbnail"
                               "FROM %1 WHERE %2 = :value")
                       .arg( IMAGE_TABLE_NAME ).arg( key ) );
        query.bindValue( ":value", value );
        if (!query.exec()) {
            qWarning() << "Get Image from database failed: " << query.lastError();
            return info;
        }
        else {
            if (query.record().count() > 1) {
                qWarning() << "Got duplicate data!";
                return info;
            }
            query.first();
            info.name = query.value(0).toString();
            info.path = query.value(1).toString();
            info.albums = query.value(2).toStringList();
            info.labels = query.value(3).toStringList();
            info.time = QDateTime::fromString(query.value(4).toString(), DATETIME_FORMAT);
            info.thumbnail.loadFromData(query.value(5).toByteArray());
            return info;
        }
    }

    return info;
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
    //image_file
    query.exec( QString("CREATE TABLE IF NOT EXISTS %1 ( "
                        "filename TEXT primary key, "
                        "filepath TEXT, "
                        "album TEXT, "      //array
                        "label TEXT,"       //array
                        "time TEXT, "
                        "thumbnail BLOB )").arg(IMAGE_TABLE_NAME) );
    //album
    query.exec( QString("CREATE TABLE IF NOT EXISTS %1 ( "
                        "albumname TEXT primary key, "
                        "count INTEGER, "
                        "size TEXT,"
                        "createtime TEXT, "
                        "earliesttime TEXT,"
                        "latesttime TEXT )").arg(ALBUM_TABLE_NAME) );
}
