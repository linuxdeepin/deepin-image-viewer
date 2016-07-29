#include "databasemanager.h"
#include "signalmanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QApplication>
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

    if (db.isValid()) {
        if (imageExist(info.name)) {
            for (QString album : info.albums) {
                insertImageIntoAlbum(album, info.name,
                                     utils::base::timeToString(info.time));
            }
            return;
        }

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
        query.bindValue( ":label", info.labels );   // TODO not support QStringList
        query.bindValue( ":time", utils::base::timeToString(info.time) );
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
            emit SignalManager::instance()->imageInserted(info);
            emit SignalManager::instance()->imageCountChanged(imageCount());

            // All new image should add to the album "Recent imported"
            insertImageIntoAlbum("Recent imported", info.name,
                                 utils::base::timeToString(info.time));

            // Insert into album when image info is inserted into DB
            for (QString album : info.albums) {
                insertImageIntoAlbum(album, info.name,
                                     utils::base::timeToString(info.time));
            }
        }
    }

}

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

void DatabaseManager::updateThumbnail(const QString &name)
{
    using namespace utils::image;
    QSize ms(THUMBNAIL_MAX_SIZE, THUMBNAIL_MAX_SIZE);
    ImageInfo info = getImageInfoByName(name);
    const QPixmap p = cutSquareImage(scaleImage(info.path), ms);
    info.thumbnail = p;
    updateImageInfo(info);
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

void DatabaseManager::insertImageInfos(const QList<DatabaseManager::ImageInfo> &infos)
{
    if (infos.length() < 1) {
        return;
    }

    QSqlDatabase db = getDatabase();

    if (db.isValid()) {
        const QString recentImport("Recent imported");
        QString albumValues = " VALUES ";
        QString imgValues = " VALUES ";
        for (ImageInfo info : infos) {
            imgValues += "(";
            imgValues += "\"" + info.name + "\",";
            imgValues += "\"" + info.path + "\",";;
            imgValues += "\"" + QVariant(info.albums).toString() + "\",";
            imgValues += "\"" + QVariant(info.labels).toString() + "\",";
            imgValues += "\"" + utils::base::timeToString(info.time) + "\",";
            imgValues += "\"\"";
            imgValues += "),";

            QStringList albums = info.albums << recentImport;
            albums.removeAll("");
            for (QString album : albums) {
                albumValues += "(";
                albumValues += "\"" + album + "\",";
                albumValues += "\"" + info.name + "\",";
                albumValues += "\"" + utils::base::timeToString(info.time) + "\"";
                albumValues += "),";
            }
        }
        imgValues.remove(imgValues.length() - 1, 1);
        albumValues.remove(albumValues.length() - 1, 1);

        // Insert into images table
        QSqlQuery query( db );
        query.exec("BEGIN IMMEDIATE TRANSACTION");
        query.prepare( QString("REPLACE INTO %1 "
            "(filename, filepath, album, label, time, thumbnail) %2")
            .arg( IMAGE_TABLE_NAME ).arg( imgValues ));
        if (! query.exec()) {
            qWarning() << "Insert images into images table failed: "
                       << query.lastError();
            query.exec("COMMIT");
        }
        else {
            query.exec("COMMIT");
            emit SignalManager::instance()->imageCountChanged(imageCount());
        }

        // Clear Recent imported album
        query.clear();
        query.exec("BEGIN IMMEDIATE TRANSACTION");
        query.prepare( QString("DELETE FROM %1 WHERE albumname = :name")
                       .arg(ALBUM_TABLE_NAME) );
        query.bindValue( ":name", recentImport );
        if (! query.exec()) {
            qWarning() << "Remove album from database failed: "
                       << query.lastError();
        }

        // Insert into album table
        query.clear();
        query.exec("BEGIN IMMEDIATE TRANSACTION");
        query.prepare(QString("REPLACE INTO %1 (albumname, filename, time) %2")
                      .arg( ALBUM_TABLE_NAME ).arg( albumValues ));
        if (! query.exec()) {
            qWarning() << "Insert images into album table failed: "
                       << query.lastError();
            query.exec("COMMIT");
        }
        else {
            query.exec("COMMIT");
            emit SignalManager::instance()->imageCountChanged(imageCount());
        }
    }
}

void DatabaseManager::removeImage(const QString &name)
{
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        // TODO remove from labels table

        // Remove from albums table
        query.prepare( QString( "DELETE FROM %1 WHERE filename = :filename" )
                       .arg(ALBUM_TABLE_NAME) );
        query.bindValue( ":filename", name );
        if (! query.exec()) {
            qWarning() << "Remove image from album failed: " << query.lastError();
        }

        // Remove from images table
        query.prepare( QString( "DELETE FROM %1 WHERE filename = :name" )
                       .arg(IMAGE_TABLE_NAME) );
        query.bindValue( ":name", name );
        if (!query.exec()) {
            qWarning() << "Remove image failed: " << query.lastError();
        }

        emit SignalManager::instance()->imageCountChanged(imageCount());
        emit SignalManager::instance()->imageRemoved(name);
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
        const QString queryStr = QString( "INSERT INTO %1 "
                                          "(albumname, filename, time) "
                                          "SELECT '%2', '%3', '%4' "
                                          "WHERE NOT EXISTS ( "
                                          "SELECT * FROM %5 "
                                          "WHERE albumname='%6' "
                                          "AND filename='%7')" )
                .arg(ALBUM_TABLE_NAME)
                .arg(albumname).arg(filename).arg(time)
                .arg(ALBUM_TABLE_NAME).arg(albumname)
                .arg(filename);
        QSqlQuery query( db );
        query.prepare(queryStr);
        if (!query.exec()) {
            qWarning() << "Insert into album failed: " << query.lastError();
        }

        // For UI update
        ImageInfo info = getImageInfoByName(filename);
        info.albums << albumname;
        emit SignalManager::instance()->insertIntoAlbum(info);
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
    emit SignalManager::instance()->removeFromAlbum(albumname, filename);
}

DatabaseManager::AlbumInfo DatabaseManager::getAlbumInfo(const QString &name)
{
    AlbumInfo info;
    info.name = name;

    QStringList nameList;
    QSqlDatabase db = getDatabase();
    if (db.isValid()) {
        QSqlQuery query( db );
        query.prepare( QString("SELECT DISTINCT filename FROM %1 "
                               "WHERE albumname = '%2' AND filename != \"\" "
                               "ORDER BY time DESC")
                       .arg(ALBUM_TABLE_NAME).arg(name) );
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
        query.prepare( QString("SELECT DISTINCT filename FROM %1 "
                               "WHERE albumname = '%2' "
                               "ORDER BY time DESC")
                       .arg(ALBUM_TABLE_NAME).arg(album) );
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
        query.exec("BEGIN IMMEDIATE TRANSACTION");
        query.prepare( QString("SELECT COUNT(*) FROM %1 "
                               "WHERE albumname = '%2' AND filename != \"\" ")
                       .arg(ALBUM_TABLE_NAME).arg(album) );
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
        query.prepare( QString("SELECT "
                               "filename, filepath, album, label, time, thumbnail "
                               "FROM %1 WHERE %2 = \'%3\' ORDER BY time DESC")
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
