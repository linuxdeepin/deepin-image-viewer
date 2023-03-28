// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTIIMAGELOAD_H
#define MULTIIMAGELOAD_H

#include "thumbnailcache.h"

#include <QQuickImageProvider>
#include <QImageReader>
#include <QImage>
#include <QMutex>

class MultiImageLoad : public QQuickImageProvider
{
public:
    explicit MultiImageLoad();
    ~MultiImageLoad() override;

    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

    void rotateImageCached(int angle, const QString &imagePath, int frameIndex = 0);
    void removeImageCache(const QString &imagePath);
    void clearCache();

private:
    QImage readNormalImage(const QString &imagePath);
    QImage readMultiImage(const QString &imagePath, int frameIndex);

private:
    QMutex mutex;
    ThumbnailCache imageCache;  ///< 图像数据缓存
    QImageReader imageReader;   ///< 图像读取类

    QString lastRotatePath;  ///< 缓存的旋转文件路径
    QImage lastRotateImage;  ///< 缓存的旋转图像信息
};

#endif  // MULTIIMAGELOAD_H
