﻿// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageinfo.h"
#include "types.h"
#include "thumbnailcache.h"
#include "unionimage/unionimage.h"

#include <QSet>
#include <QSize>
#include <QFile>
#include <QImageReader>
#include <QThreadPool>
#include <QRunnable>
#include <QDebug>

class ImageInfoData
{
public:
    typedef QSharedPointer<ImageInfoData> Ptr;

    ImageInfoData::Ptr cloneWithoutFrame()
    {
        ImageInfoData::Ptr other(new ImageInfoData);
        other->path = this->path;
        other->type = this->type;
        other->size = this->size;
        other->frameIndex = this->frameIndex;
        other->frameCount = this->frameCount;

        return other;
    }

    inline bool isError() const { return !exist || (Types::DamagedImage == type); }

    QString path;           ///< 图片路径
    Types::ImageType type;  ///< 图片类型
    QSize size;             ///< 图片大小
    int frameIndex = 0;     ///< 当前图片帧号
    int frameCount = 0;     ///< 当前图片总帧数
    bool exist = false;     ///< 图片是否存在
};

class LoadImageInfoRunnable : public QRunnable
{
public:
    explicit LoadImageInfoRunnable(const QString &path, int index = 0);
    void run() override;
    bool loadImage(QImage &image, QSize &sourceSize) const;
    void notifyFinished(const QString &path, int frameIndex, ImageInfoData::Ptr data) const;

private:
    int frameIndex = 0;
    QString loadPath;
};

class ImageInfoCache : public QObject
{
    Q_OBJECT
public:
    typedef QString KeyType;

    ImageInfoCache();
    ~ImageInfoCache();

    ImageInfoData::Ptr find(const KeyType &path, int frameIndex);
    void load(const KeyType &path, int frameIndex, bool reload = false);
    void loadFinished(const KeyType &path, int frameIndex, ImageInfoData::Ptr data);

    void clearCache();

    Q_SIGNAL void imageDataChanged(const KeyType &path, int frameIndex);

private:
    QHash<KeyType, ImageInfoData::Ptr> cache;
    QSet<KeyType> waitSet;
};
Q_GLOBAL_STATIC(ImageInfoCache, CacheInstance)

LoadImageInfoRunnable::LoadImageInfoRunnable(const QString &path, int index)
    : frameIndex(index)
    , loadPath(path)
{
}

Types::ImageType imageTypeAdapator(imageViewerSpace::ImageType type)
{
    switch (type) {
        case imageViewerSpace::ImageTypeBlank:
            return Types::NullImage;
        case imageViewerSpace::ImageTypeSvg:
            return Types::SvgImage;
        case imageViewerSpace::ImageTypeStatic:
            return Types::NormalImage;
        case imageViewerSpace::ImageTypeDynamic:
            return Types::DynamicImage;
        case imageViewerSpace::ImageTypeMulti:
            return Types::MultiImage;
        default:
            return Types::DamagedImage;
    }
}

/**
   @brief 在线程中读取及构造图片信息，包含图片路径、类型、大小等，并读取图片内容创建缩略图。
 */
void LoadImageInfoRunnable::run()
{
    ImageInfoData::Ptr data(new ImageInfoData);
    data->path = loadPath;
    data->exist = QFileInfo::exists(loadPath);

    if (!data->exist) {
        // 缓存中存在数据，则图片为加载后删除
        data->type = (ThumbnailCache::instance()->contains(data->path)) ? Types::NonexistImage : Types::NullImage;
        notifyFinished(data->path, frameIndex, data);
        return;
    }

    imageViewerSpace::ImageType type = LibUnionImage_NameSpace::getImageType(loadPath);
    data->type = imageTypeAdapator(type);

    if (Types::NullImage == data->type) {
        notifyFinished(data->path, frameIndex, data);
        return;
    }

    QImageReader reader(loadPath);
    if (Types::MultiImage == data->type) {
        reader.jumpToImage(frameIndex);
        QImage image = reader.read();
        if (image.isNull()) {
            // 数据获取异常
            data->type = Types::DamagedImage;
            notifyFinished(data->path, frameIndex, data);
            return;
        }

        // 缓存缩略图信息
        ThumbnailCache::instance()->add(data->path, frameIndex, image);
        data->size = image.size();
        data->frameCount = reader.imageCount();

    } else if (0 != frameIndex) {
        // 非多页图类型，但指定了索引，存在异常
        data->type = Types::DamagedImage;
        notifyFinished(data->path, frameIndex, data);
        return;
    } else {
        QImage image;
        if (loadImage(image, data->size)) {
            // 缓存缩略图信息
            ThumbnailCache::instance()->add(data->path, frameIndex, image);
        } else {
            // 读取图片数据存在异常，调整图片类型
            data->type = Types::DamagedImage;
        }
    }

    notifyFinished(data->path, frameIndex, data);
}

/**
   @brief 加载图片数据
   @param image 读取的图片源数据
   @param sourceSize 源图片大小
   @return 是否正常加载图片数据
 */
bool LoadImageInfoRunnable::loadImage(QImage &image, QSize &sourceSize) const
{
    QString error;
    bool ret = LibUnionImage_NameSpace::loadStaticImageFromFile(loadPath, image, error);
    if (ret) {
        sourceSize = image.size();
        // 保存图片比例缩放
        image = image.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    } else {
        qWarning() << "Load image " << loadPath << "error:" << error;
    }

    return ret;
}

/**
   @brief 提示缓存管理图像数据已加载完成
   @param path 图片文件路径
   @param frameIndex 多页图图片索引
   @param data 图像数据
 */
void LoadImageInfoRunnable::notifyFinished(const QString &path, int frameIndex, ImageInfoData::Ptr data) const
{
    QMetaObject::invokeMethod(
        CacheInstance(), [=]() { CacheInstance()->loadFinished(path, frameIndex, data); }, Qt::QueuedConnection);
}

ImageInfoCache::ImageInfoCache() {}

ImageInfoCache::~ImageInfoCache() {}

/**
   @return 返回缓存中文件路径为 \a path 和帧索引为 \a frameIndex 的缓存数据
 */
ImageInfoData::Ptr ImageInfoCache::find(const KeyType &path, int frameIndex)
{
    QString key = ThumbnailCache::toFindKey(path, frameIndex);
    return cache.value(key);
}

/**
   @brief 加载文件路径 \a path 指向的帧索引为 \a frameIndex 的图像文件，
    \a reload 标识用于重新加载图片文件数据
 */
void ImageInfoCache::load(const KeyType &path, int frameIndex, bool reload)
{
    QString key = ThumbnailCache::toFindKey(path, frameIndex);

    if (waitSet.contains(key)) {
        return;
    }
    if (!reload && cache.contains(key)) {
        return;
    }
    waitSet.insert(key);

    LoadImageInfoRunnable *runnable = new LoadImageInfoRunnable(path, frameIndex);
    QThreadPool::globalInstance()->start(runnable);
}

/**
   @brief 图像信息加载完成，接收来自 LoadImageInfoRunnable 的完成信号，根据文件路径 \a path
    和图像帧索引 \a frameIndex 区分加载的数据 \a data ，并保存至缓存中
 */
void ImageInfoCache::loadFinished(const KeyType &path, int frameIndex, ImageInfoData::Ptr data)
{
    QString key = ThumbnailCache::toFindKey(path, frameIndex);

    waitSet.remove(key);
    if (data) {
        cache.insert(key, data);
    }

    Q_EMIT imageDataChanged(path, frameIndex);
}

/**
   @brief 清空缓存信息，用于重新载入图像时使用
 */
void ImageInfoCache::clearCache()
{
    waitSet.clear();
    cache.clear();
}

/**
   @class ImageInfo
   @brief 图像信息管理类
   @detail 用于后台异步加载图像数据并缓存，此缓存在内部进行复用，可在 C++/QML
    中调用 ImageInfo 取得基础的图像信息。详细的图像信息参见 ExtraImageInfo
 */

ImageInfo::ImageInfo(QObject *parent)
    : QObject(parent)
{
    connect(CacheInstance(), &ImageInfoCache::imageDataChanged, this, &ImageInfo::onLoadFinished);
}

ImageInfo::~ImageInfo() {}

ImageInfo::Status ImageInfo::status() const
{
    return imageStatus;
}

/**
   @brief 设置图片路径为 \a path .
        若缓存中不存在对应图片的信息，将尝试加载对应的数据
 */
void ImageInfo::setSource(const QUrl &source)
{
    if (imageUrl != source) {
        imageUrl = source;
        Q_EMIT sourceChanged();

        QString localPath = imageUrl.toLocalFile();
        ImageInfoData::Ptr newData = CacheInstance()->find(localPath, currentIndex);
        if (newData) {
            data = newData;
            Q_EMIT infoChanged();

            setStatus(data->isError() ? Error : Ready);
        } else {
            setStatus(Loading);
            CacheInstance()->load(localPath, currentIndex);
        }
    }
}

/**
   @return 返回图片路径信息
 */
QUrl ImageInfo::source() const
{
    return imageUrl;
}

/**
   @return 返回图片的类型，参考 Types::ImageType
 */
int ImageInfo::type() const
{
    return data ? data->type : Types::NullImage;
}

/**
   @return 返回图片宽度
 */
int ImageInfo::width() const
{
    return data ? data->size.width() : -1;
}

/**
   @return 返回图片高度
 */
int ImageInfo::height() const
{
    return data ? data->size.height() : -1;
}

/**
   @brief 设置当前图片的帧索引，仅对多页图有效，若成功设置，进行异步加载图片
   @param index 图片帧索引
 */
void ImageInfo::setFrameIndex(int index)
{
    if (currentIndex != index) {
        currentIndex = index;
        Q_EMIT frameIndexChanged();

        QString localPath = imageUrl.toLocalFile();
        if (localPath.isEmpty()) {
            return;
        }

        ImageInfoData::Ptr newData = CacheInstance()->find(localPath, currentIndex);
        if (newData) {
            data = newData;
            Q_EMIT infoChanged();

            setStatus(data->isError() ? Error : Ready);
        } else {
            setStatus(Loading);
            CacheInstance()->load(localPath, currentIndex);
        }
    }
}

/**
   @return 返回当前图片的帧索引，对多页图有效
 */
int ImageInfo::frameIndex() const
{
    return currentIndex;
}

/**
   @return 返回当前图片的总帧数，默认为1
 */
int ImageInfo::frameCount() const
{
    return data ? data->frameCount : 1;
}

/**
   @return 返回当前图片是否存在，图片可能在展示过程中被销毁
 */
bool ImageInfo::exists() const
{
    return data ? data->exist : false;
}

/**
   @return 返回是否存在已缓存的缩略图数据
 */
bool ImageInfo::hasCachedThumbnail() const
{
    if (imageUrl.isEmpty()) {
        return false;
    } else {
        return ThumbnailCache::instance()->contains(imageUrl.toLocalFile(), frameIndex());
    }
}

/**
   @brief 刷新当前图片信息
 */
void ImageInfo::refresh()
{
    setStatus(Loading);
    CacheInstance()->load(imageUrl.toLocalFile(), currentIndex, true);
}

/**
   @brief 清空缓存数据
   @note 这不会影响处于加载队列中的任务
 */
void ImageInfo::clearCache()
{
    CacheInstance()->clearCache();
}

/**
   @brief 设置图片信息状态为 \a status
 */
void ImageInfo::setStatus(ImageInfo::Status status)
{
    if (imageStatus != status) {
        imageStatus = status;
        Q_EMIT statusChanged();
    }
}

/**
   @brief 更新图像数据，将发送部分关键数据的更新信号
   @param newData 新图像数据
 */
void ImageInfo::updateData(const QSharedPointer<ImageInfoData> &newData)
{
    if (newData == data) {
        return;
    }

    ImageInfoData::Ptr oldData = data;
    data = newData;

    if (oldData->type != newData->type) {
        Q_EMIT typeChanged();
    }
    if (oldData->size != newData->size) {
        Q_EMIT widthChanged();
        Q_EMIT heightChanged();
    }
    if (oldData->frameIndex != newData->frameIndex) {
        Q_EMIT frameIndexChanged();
    }
    if (oldData->frameCount != newData->frameCount) {
        Q_EMIT frameCountChanged();
    }
    if (oldData->exist != newData->exist) {
        Q_EMIT existsChanged();
    }
}

/**
   @brief 内部数据异步加载完成，返回处理的文件路径 \a path ,
        根据加载结果，设置图片信息状态
 */
void ImageInfo::onLoadFinished(const QString &path, int frameIndex)
{
    if (imageUrl.toLocalFile() == path && currentIndex == frameIndex) {
        ImageInfoData::Ptr newData = CacheInstance()->find(path, currentIndex);
        if (newData) {
            if (data) {
                // 刷新旧数据，需发送部分更新信号
                updateData(newData);
            } else {
                data = newData;
            }
            Q_EMIT infoChanged();

            setStatus(data->isError() ? Error : Ready);
        } else {
            setStatus(Error);
        }
    }
}

#include "imageinfo.moc"
