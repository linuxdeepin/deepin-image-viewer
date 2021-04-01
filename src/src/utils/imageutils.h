/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
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
#ifndef IMGUTIL_H
#define IMGUTIL_H

#include "baseutils.h"
#include <QDateTime>
#include <QFileInfo>
#include <QPixmap>
#include <QDir>

#define VAULT_DECRYPT_DIR_NAME          "vault_unlocked"
#define VAULT_BASE_PATH (QDir::homePath() + QString("/.local/share/applications"))  //! 获取保险箱创建的目录地址

namespace utils {

namespace image {

const int THUMBNAIL_MAX_SIZE = 291 * 2;
const int THUMBNAIL_NORMAL_SIZE = 128 * 2;

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
const QImage loadTga(QString filePath, bool &success);
/*
 * lmh0901，根据后缀是否是图片
**/
bool                                suffixisImage(const QString &path);
bool                                imageSupportRead(const QString &path);
bool                                imageSupportSave(const QString &path);
//bool                                imageSupportWrite(const QString &path);
bool                                imageSupportWallPaper(const QString &path);
bool                                rotate(const QString &path, int degree);
const QImage                        scaleImage(const QString &path,
                                               const QSize &size = QSize(384, 383));

bool                                generateThumbnail(const QString &path);
const QPixmap                       getThumbnail(const QString &path,
                                                 bool cacheOnly = false);
void                                removeThumbnail(const QString &path);
const QString                       thumbnailCachePath();
const QString                       thumbnailPath(const QString &path, ThumbnailType type = ThumbLarge);
bool                                thumbnailExist(const QString &path, ThumbnailType type = ThumbLarge);

QStringList                         supportedImageFormats();
//保险箱地址判断
QString                             makeVaultLocalPath(const QString &path, const QString &base);
bool                                isVaultFile(const QString &path);
//增加该地址能否被删除的api
bool                                isCanRemove(const QString &path);
QPixmap getDamagePixmap(bool bLight = true);





}  // namespace image

}  // namespace utils

#endif // IMGUTIL_H
