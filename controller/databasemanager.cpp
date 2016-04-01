#include "databasemanager.h"
#include "signalmanager.h"
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include <QFileInfo>
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QMutexLocker>

const QString DATABASE_PATH = QDir::homePath() + "/.local/share/deepin/deepin-image-viewer/";
const QString DATABASE_NAME = "deepinimageviewer.db";
const QString IMAGE_TABLE_NAME = "ImageTable";
const QString ALBUM_TABLE_NAME = "AlbumTable";

void DatabaseManager::insertImageInfo(const DatabaseManager::ImageInfo &info)
{
    QMutexLocker locker(&m_mutex);

    QSqlDatabase db = getDatabase();

    if (db.isValid() && !imageExist(info.name)) {
        QSqlQuery query( db );
        /*NOTE: '"BEGIN IMMEDIATE TRANSACTION"' try to avoid the error like "database is locked",
         * but it not work good
         * The error "database is locked" is typically happens when someone begins a transaction,
         * and tries to write to a database while other person is reading from the
         * database (in another transaction). */
        query.exec("BEGIN IMMEDIATE TRANSACTION");
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
        if ( !info.thumbnail.save( &inBuffer, "JPG" )) { // write inPixmap into inByteArray
            qDebug() << "Write pixmap to buffer error!" << info.name;
        }
        query.bindValue( ":thumbnail", inByteArray);
        if (!query.exec()) {
            qWarning() << "Insert image into database failed: " << query.lastError();
            query.exec("COMMIT");
        }
        else {
            query.exec("COMMIT");
            emit SignalManager::instance()->imageCountChanged();
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

QList<DatabaseManager::ImageInfo> DatabaseManager::getImageInfoByAlbum(const QString &album)
{
    return getImageInfos("album", album);
}

QList<DatabaseManager::ImageInfo> DatabaseManager::getImageInfoByTime(const QDateTime &time)
{
    return getImageInfos("time", time.toString(DATETIME_FORMAT));
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
        else {
            emit SignalManager::instance()->imageCountChanged();
        }
    }
}

bool DatabaseManager::imageExist(const QString &name)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.exec("BEGIN IMMEDIATE TRANSACTION");
        query.prepare( QString("SELECT COUNT(*) FROM %1 WHERE filename = :name").arg( IMAGE_TABLE_NAME ) );
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
        query.prepare( QString("SELECT DISTINCT time FROM %1 ORDER BY time").arg(IMAGE_TABLE_NAME) );
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

const QStringList DatabaseManager::supportImageType()
{
    QStringList origin;
    origin<< "BMP";
    origin<< "GIF";
    origin<< "JPG";
    origin<< "PNG";
    origin<< "PBM";
    origin<< "PGM";
    origin<< "PPM";
    origin<< "XBM";
    origin<< "XPM";
    origin<< "SVG";

    origin<< "DDS";
    origin<< "ICNS";
    origin<< "JP2";
    origin<< "MNG";
    origin<< "TGA";
    origin<< "TIFF";
    origin<< "WBMP";
    origin<< "WEBP";

    QStringList list;
    for (QString v : origin) {
        list << v.toLower();
    }
    list += origin;

    return list;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent),m_connectionName("default_connection")
{
    checkDatabase();
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

QList<DatabaseManager::ImageInfo> DatabaseManager::getImageInfos(const QString &key, const QString &value)
{
    QList<ImageInfo> infoList;
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT "
                               "filename, filepath, album, label, time, thumbnail "
                               "FROM %1 WHERE %2 = \'%3\' ORDER BY filename")
                       .arg( IMAGE_TABLE_NAME ).arg( key ).arg( value ) );
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
                info.time = QDateTime::fromString(query.value(4).toString(), DATETIME_FORMAT);
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
