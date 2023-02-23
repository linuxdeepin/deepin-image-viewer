// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailcache.h"

ThumbnailCache::ThumbnailCache() {}

ThumbnailCache::~ThumbnailCache() {}

ThumbnailCache *ThumbnailCache::instance()
{
    static ThumbnailCache ins;
    return &ins;
}

/**
   @return 返回缓存中是否存在文件路径为 \a path 和图片帧索引为 \a frameIndex 的缩略图
 */
bool ThumbnailCache::contains(const QString &path, int frameIndex)
{
    QMutexLocker _locker(&mutex);
    return cache.contains(toFindKey(path, frameIndex));
}

/**
   @return 返回缓存中文件路径为 \a path 和图片帧索引为 \a frameIndex 的缩略图
 */
QImage ThumbnailCache::get(const QString &path, int frameIndex)
{
    QMutexLocker _locker(&mutex);
    return cache.value(toFindKey(path, frameIndex));
}

/**
   @brief 添加文件路径为 \a path 和图片帧索引为 \a frameIndex 的缩略图
 */
void ThumbnailCache::add(const QString &path, int frameIndex, const QImage &image)
{
    QMutexLocker _locker(&mutex);
    cache.insert(toFindKey(path, frameIndex), image);
}

/**
   @brief 清空缩略图信息
 */
void ThumbnailCache::clear()
{
    QMutexLocker _locker(&mutex);
    cache.clear();
}

/**
   @return 组合图像文件路径 \a path 和图像帧索引 \a frameIndex 为缩略图缓存处理的 key
 */
QString ThumbnailCache::toFindKey(const QString &path, int frameIndex)
{
    return frameIndex ? (path + QString::number(frameIndex)) : path;
}
