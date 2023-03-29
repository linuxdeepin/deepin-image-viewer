// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include "thumbnailcache.h"

#include <QQuickImageProvider>
#include <QImageReader>
#include <QImage>
#include <QMutex>

// 异步图片加载器
class AsyncImageProvider : public QQuickAsyncImageProvider
{
public:
    explicit AsyncImageProvider();
    ~AsyncImageProvider() override;

    virtual QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;

    void rotateImageCached(int angle, const QString &imagePath, int frameIndex = 0);
    void removeImageCache(const QString &imagePath);
    void clearCache();

private:
    QMutex mutex;
    ThumbnailCache imageCache;  ///< 图像数据缓存(已存在锁保护)
    QString lastRotatePath;  ///< 缓存的旋转文件路径
    QImage lastRotateImage;  ///< 缓存的旋转图像信息

    friend class AsyncImageResponse;
};

// 缩略图加载器
class ThumbnailProvider : public QQuickImageProvider
{
public:
    explicit ThumbnailProvider();
    ~ThumbnailProvider() override;

    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
};

#endif  // IMAGEPROVIDER_H
