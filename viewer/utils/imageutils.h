#ifndef IMGUTIL_H
#define IMGUTIL_H

#include "baseutils.h"
#include <QDateTime>
#include <QFileInfo>
#include <QPixmap>

namespace utils {

namespace image {

const int THUMBNAIL_MAX_SIZE = 192;

const QPixmap                       cutSquareImage(const QPixmap &pixmap,
                                                   const QSize &size);
const QMap<QString, QString>        getAllMetaData(const QString &path);
const QDateTime                     getCreateDateTime(const QString &path);
const QFileInfoList                 getImagesInfo(const QString &dir,
                                                  bool recursive = true);
const QString                       getOrientation(const QString &path);
const QImage                        getRotatedImage(const QString &path);
const QPixmap                       getThumbnail(const QString &path,
                                                 bool exifOnly = false);
bool                                isImageSupported(const QString &path);
bool                                rotate(const QString &path, int degree);
const QPixmap                       scaleImage(const QString &path,
                                               const QSize &size = QSize(384, 383));

}  // namespace image

}  // namespace utils

#endif // IMGUTIL_H
