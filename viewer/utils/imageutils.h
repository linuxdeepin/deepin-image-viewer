#ifndef IMGUTIL_H
#define IMGUTIL_H

#include "baseutils.h"
#include <QDateTime>
#include <QFileInfo>
#include <QPixmap>

namespace utils {

namespace image {

const int THUMBNAIL_MAX_SIZE = 291;
const int THUMBNAIL_NORMAL_SIZE = 128;

enum ThumbnailType {
    ThumbNormal,
    ThumbLarge,
    ThumbFail
};

const QPixmap                       cachePixmap(const QString &path);
const QPixmap                       cutSquareImage(const QPixmap &pixmap);
const QPixmap                       cutSquareImage(const QPixmap &pixmap,
                                                   const QSize &size);
const QMap<QString, QString>        getAllMetaData(const QString &path);
const QDateTime                     getCreateDateTime(const QString &path);
const QFileInfoList                 getImagesInfo(const QString &dir,
                                                  bool recursive = true);
const QString                       getOrientation(const QString &path);
const QImage                        getRotatedImage(const QString &path);
bool                                imageSupportRead(const QString &path);
bool                                imageSupportSave(const QString &path);
bool                                imageSupportWrite(const QString &path);
bool                                rotate(const QString &path, int degree);
const QImage                        scaleImage(const QString &path,
                                               const QSize &size = QSize(384, 383));

bool                                generateThumbnail(const QString &path);
const QPixmap                       getThumbnail(const QString &path,
                                                 bool cacheOnly = false);
void                                removeThumbnail(const QString &path);
const QString                       thumbnailCachePath();
const QString                       thumbnailPath(const QString &path,
                                                  ThumbnailType type = ThumbLarge);
bool                                thumbnailExist(const QString &path);

}  // namespace image

}  // namespace utils

#endif // IMGUTIL_H
