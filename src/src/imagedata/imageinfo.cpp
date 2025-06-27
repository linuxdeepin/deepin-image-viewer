// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageinfo.h"
#include "types.h"
#include "thumbnailcache.h"
#include "unionimage/unionimage.h"
#include "globalcontrol.h"

#include <QSet>
#include <QSize>
#include <QFile>
#include <QImageReader>
#include <QThreadPool>
#include <QRunnable>
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

class ImageInfoData
{
public:
    typedef QSharedPointer<ImageInfoData> Ptr;

    ImageInfoData::Ptr cloneWithoutFrame()
    {
        qCDebug(logImageViewer) << "ImageInfoData::cloneWithoutFrame called";
        ImageInfoData::Ptr other(new ImageInfoData);
        other->path = this->path;
        other->type = this->type;
        other->size = this->size;
        other->frameIndex = this->frameIndex;
        other->frameCount = this->frameCount;

        other->scale = this->scale;
        other->x = this->x;
        other->y = this->y;
        qCDebug(logImageViewer) << "ImageInfoData::cloneWithoutFrame finished";
        return other;
    }

    inline bool isError() const
    {
        qCDebug(logImageViewer) << "ImageInfoData::isError called";
        bool ret = !exist || (Types::DamagedImage == type);
        qCDebug(logImageViewer) << "ImageInfoData::isError finished:" << ret;
        return ret;
    }

    QString path;   ///< 图片路径
    Types::ImageType type;   ///< 图片类型
    QSize size;   ///< 图片大小
    int frameIndex = 0;   ///< 当前图片帧号
    int frameCount = 0;   ///< 当前图片总帧数
    bool exist = false;   ///< 图片是否存在

    // runtime property
    qreal scale = -1;   ///< 图片缩放比值
    qreal x = 0;   ///< 相对坐标X轴偏移
    qreal y = 0;   ///< 相对坐标Y轴偏移
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
    typedef QPair<QString, int> KeyType;

    ImageInfoCache();
    ~ImageInfoCache() override;

    ImageInfoData::Ptr find(const QString &path, int frameIndex);
    void load(const QString &path, int frameIndex, bool reload = false);
    void loadFinished(const QString &path, int frameIndex, ImageInfoData::Ptr data);
    void removeCache(const QString &path, int frameIndex);
    void clearCache();

    Q_SIGNAL void imageDataChanged(const QString &path, int frameIndex);
    Q_SIGNAL void imageSizeChanged(const QString &path, int frameIndex);

private:
    bool aboutToQuit { false };
    QHash<KeyType, ImageInfoData::Ptr> cache;
    QSet<KeyType> waitSet;
    QScopedPointer<QThreadPool> localPoolPtr;
};
Q_GLOBAL_STATIC(ImageInfoCache, CacheInstance)

LoadImageInfoRunnable::LoadImageInfoRunnable(const QString &path, int index)
    : frameIndex(index), loadPath(path)
{
}

Types::ImageType imageTypeAdapator(imageViewerSpace::ImageType type)
{
    qCDebug(logImageViewer) << "Adapting image type:" << type;
    switch (type) {
    case imageViewerSpace::ImageTypeBlank:
        qCDebug(logImageViewer) << "Mapped ImageTypeBlank to NullImage.";
        return Types::NullImage;
    case imageViewerSpace::ImageTypeSvg:
        qCDebug(logImageViewer) << "Mapped ImageTypeSvg to SvgImage.";
        return Types::SvgImage;
    case imageViewerSpace::ImageTypeStatic:
        qCDebug(logImageViewer) << "Mapped ImageTypeStatic to NormalImage.";
        return Types::NormalImage;
    case imageViewerSpace::ImageTypeDynamic:
        qCDebug(logImageViewer) << "Mapped ImageTypeDynamic to DynamicImage.";
        return Types::DynamicImage;
    case imageViewerSpace::ImageTypeMulti:
        qCDebug(logImageViewer) << "Mapped ImageTypeMulti to MultiImage.";
        return Types::MultiImage;
    default:
        qCWarning(logImageViewer) << "Unknown image type:" << type << ", mapping to DamagedImage.";
        return Types::DamagedImage;
    }
}

/**
   @brief 在线程中读取及构造图片信息，包含图片路径、类型、大小等，并读取图片内容创建缩略图。
 */
void LoadImageInfoRunnable::run()
{
    qCDebug(logImageViewer) << "LoadImageInfoRunnable::run() entered for path:" << loadPath << ", frameIndex:" << frameIndex;
    if (qApp->closingDown()) {
        qCDebug(logImageViewer) << "Application is closing down, LoadImageInfoRunnable exiting.";
        return;
    }

    ImageInfoData::Ptr data(new ImageInfoData);
    data->path = loadPath;
    data->exist = QFileInfo::exists(loadPath);
    qCDebug(logImageViewer) << "Image file existence check:" << data->exist;

    if (!data->exist) {
        // 缓存中存在数据，则图片为加载后删除
        data->type = (ThumbnailCache::instance()->contains(data->path)) ? Types::NonexistImage : Types::NullImage;
        qCDebug(logImageViewer) << "Image file does not exist. Type set to:" << data->type;
        notifyFinished(data->path, frameIndex, data);
        return;
    }

    imageViewerSpace::ImageType type = LibUnionImage_NameSpace::getImageType(loadPath);
    data->type = imageTypeAdapator(type);
    qCDebug(logImageViewer) << "Image type adapted to:" << data->type;

    if (Types::NullImage == data->type) {
        qCWarning(logImageViewer) << "Image type is NullImage, skipping further processing.";
        notifyFinished(data->path, frameIndex, data);
        return;
    }

    QImageReader reader(loadPath);
    if (Types::MultiImage == data->type) {
        qCDebug(logImageViewer) << "Image is multi-image type. Jumping to image frame:" << frameIndex;
        reader.jumpToImage(frameIndex);
        QImage image = reader.read();
        if (image.isNull()) {
            // 数据获取异常
            data->type = Types::DamagedImage;
            qCWarning(logImageViewer) << "Failed to read multi-image frame" << frameIndex << ", setting type to DamagedImage.";
            notifyFinished(data->path, frameIndex, data);
            return;
        }

        data->size = image.size();
        data->frameCount = reader.imageCount();
        qCDebug(logImageViewer) << "Multi-image size:" << data->size << ", frame count:" << data->frameCount;
        // 保存图片比例缩放
        image = image.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // 缓存缩略图信息
        ThumbnailCache::instance()->add(data->path, frameIndex, image);
        qCDebug(logImageViewer) << "Thumbnail added to cache for multi-image.";

    } else if (0 != frameIndex) {
        // 非多页图类型，但指定了索引，存在异常
        data->type = Types::DamagedImage;
        qCWarning(logImageViewer) << "Non-multi-image type with non-zero frame index, setting type to DamagedImage.";
        notifyFinished(data->path, frameIndex, data);
        return;

    } else {
        QImage image;
        if (loadImage(image, data->size)) {
            qCDebug(logImageViewer) << "Image loaded successfully. Size:" << data->size;
            // 缓存缩略图信息
            ThumbnailCache::instance()->add(data->path, frameIndex, image);
            qCDebug(logImageViewer) << "Thumbnail added to cache.";
        } else {
            // 读取图片数据存在异常，调整图片类型
            data->type = Types::DamagedImage;
            qCWarning(logImageViewer) << "Failed to load image data, setting type to DamagedImage.";
        }
    }

    notifyFinished(data->path, frameIndex, data);
    qCDebug(logImageViewer) << "LoadImageInfoRunnable::run() finished for path:" << loadPath;
}

/**
   @brief 加载图片数据
   @param image 读取的图片源数据
   @param sourceSize 源图片大小
   @return 是否正常加载图片数据
 */
bool LoadImageInfoRunnable::loadImage(QImage &image, QSize &sourceSize) const
{
    qCDebug(logImageViewer) << "LoadImageInfoRunnable::loadImage() entered for path:" << loadPath;
    QString error;
    bool ret = LibUnionImage_NameSpace::loadStaticImageFromFile(loadPath, image, error);
    if (ret) {
        sourceSize = image.size();
        // 保存图片比例缩放
        image = image.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        qCDebug(logImageViewer) << "Static image loaded successfully. Source size:" << sourceSize;
    } else {
        qCWarning(logImageViewer) << "Failed to load image:" << loadPath << "Error:" << error;
    }

    qCDebug(logImageViewer) << "LoadImageInfoRunnable::loadImage() returning:" << ret;
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
    qCDebug(logImageViewer) << "LoadImageInfoRunnable::notifyFinished() entered for path:" << path << ", frameIndex:" << frameIndex;
    QMetaObject::invokeMethod(
            CacheInstance(), [=]() { CacheInstance()->loadFinished(path, frameIndex, data); }, Qt::QueuedConnection);
    qCDebug(logImageViewer) << "Invoked CacheInstance()->loadFinished in queued connection.";
}

ImageInfoCache::ImageInfoCache()
    : localPoolPtr(new QThreadPool)
{
    qCDebug(logImageViewer) << "ImageInfoCache constructor entered.";
    // 调整后台线程，由于imageprovider部分也存在子线程调用
    localPoolPtr->setMaxThreadCount(qMax(2, QThread::idealThreadCount() / 2));
    qCDebug(logImageViewer) << "ImageInfoCache thread pool max count set to:" << localPoolPtr->maxThreadCount();

    // 退出时清理线程状态
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        qCDebug(logImageViewer) << "QCoreApplication::aboutToQuit signal received. Clearing cache and waiting for thread pool.";
        aboutToQuit = true;
        clearCache();

        localPoolPtr->waitForDone();
        qCDebug(logImageViewer) << "ImageInfoCache thread pool finished.";
    });
    qCDebug(logImageViewer) << "Connected QCoreApplication::aboutToQuit signal.";
}

ImageInfoCache::~ImageInfoCache() {
    qCDebug(logImageViewer) << "ImageInfoCache destructor entered.";
}

/**
   @return 返回缓存中文件路径为 \a path 和帧索引为 \a frameIndex 的缓存数据
 */
ImageInfoData::Ptr ImageInfoCache::find(const QString &path, int frameIndex)
{
    qCDebug(logImageViewer) << "ImageInfoCache::find() called for path:" << path << ", frameIndex:" << frameIndex;
    ThumbnailCache::Key key = ThumbnailCache::toFindKey(path, frameIndex);
    return cache.value(key);
}

/**
   @brief 加载文件路径 \a path 指向的帧索引为 \a frameIndex 的图像文件，
    \a reload 标识用于重新加载图片文件数据
 */
void ImageInfoCache::load(const QString &path, int frameIndex, bool reload)
{
    qCDebug(logImageViewer) << "ImageInfoCache::load() called for path:" << path << ", frameIndex:" << frameIndex << ", reload:" << reload;
    if (aboutToQuit) {
        qCDebug(logImageViewer) << "Skipping image load during application shutdown:" << path;
        return;
    }

    ThumbnailCache::Key key = ThumbnailCache::toFindKey(path, frameIndex);

    if (waitSet.contains(key)) {
        qCDebug(logImageViewer) << "Image already in loading queue:" << path << "frame:" << frameIndex;
        return;
    }
    if (!reload && cache.contains(key)) {
        qCDebug(logImageViewer) << "Image already cached:" << path << "frame:" << frameIndex;
        return;
    }
    waitSet.insert(key);

    if (!GlobalControl::enableMultiThread()) {
        qCDebug(logImageViewer) << "Loading image synchronously:" << path << "frame:" << frameIndex;
        // 低于2逻辑线程，直接加载，防止部分平台出现卡死等情况
        LoadImageInfoRunnable runnable(path, frameIndex);
        runnable.run();
    } else {
        qCDebug(logImageViewer) << "Loading image asynchronously:" << path << "frame:" << frameIndex;
        LoadImageInfoRunnable *runnable = new LoadImageInfoRunnable(path, frameIndex);
        localPoolPtr->start(runnable, QThread::LowPriority);
    }
}

/**
   @brief 图像信息加载完成，接收来自 LoadImageInfoRunnable 的完成信号，根据文件路径 \a path
    和图像帧索引 \a frameIndex 区分加载的数据 \a data ，并保存至缓存中
 */
void ImageInfoCache::loadFinished(const QString &path, int frameIndex, ImageInfoData::Ptr data)
{
    qCDebug(logImageViewer) << "ImageInfoCache::loadFinished called for path:" << path << "frameIndex:" << frameIndex;
    if (aboutToQuit) {
        qCDebug(logImageViewer) << "Skipping load finished during application shutdown:" << path;
        return;
    }

    ThumbnailCache::Key key = ThumbnailCache::toFindKey(path, frameIndex);

    if (data && waitSet.contains(key)) {
        cache.insert(key, data);
        qCDebug(logImageViewer) << "Image loaded successfully:" << path << "frame:" << frameIndex
                                << "type:" << data->type << "size:" << data->size;
    } else {
        qCWarning(logImageViewer) << "Failed to load image data:" << path << "frame:" << frameIndex;
    }
    waitSet.remove(key);

    Q_EMIT imageDataChanged(path, frameIndex);
    qCDebug(logImageViewer) << "Emitted imageDataChanged signal.";
}

/**
   @brief 移除文件路径为 \a path 图片的第 \a frameIndex 帧缓存数据
 */
void ImageInfoCache::removeCache(const QString &path, int frameIndex)
{
    qCDebug(logImageViewer) << "Removing image cache:" << path << "frame:" << frameIndex;
    cache.remove(ThumbnailCache::toFindKey(path, frameIndex));
    // 同时移除缓存的图像数据
    ThumbnailCache::instance()->remove(path, frameIndex);

    Q_EMIT imageDataChanged(path, frameIndex);
}

/**
   @brief 清空缓存信息，用于重新载入图像时使用
 */
void ImageInfoCache::clearCache()
{
    qCDebug(logImageViewer) << "Clearing all image caches";
    // 清理还未启动的线程任务
    localPoolPtr->clear();
    waitSet.clear();
    cache.clear();
}

/**
   @class ImageInfo
   @brief 图像信息管理类
   @details 用于后台异步加载图像数据并缓存，此缓存在内部进行复用，可在 C++/QML
    中调用 ImageInfo 取得基础的图像信息。详细的图像信息参见 ExtraImageInfo
   @warning 非线程安全，仅在GUI线程调用
 */

ImageInfo::ImageInfo(QObject *parent)
    : QObject(parent)
{
    qCDebug(logImageViewer) << "ImageInfo constructor called";
    // TODO(renbin): 这种方式效率不佳，应调整为记录文件对应的 ImageInfo 对象进行直接调用(均在同一线程)
    connect(CacheInstance(), &ImageInfoCache::imageDataChanged, this, &ImageInfo::onLoadFinished);
    connect(CacheInstance(), &ImageInfoCache::imageSizeChanged, this, &ImageInfo::onSizeChanged);
    qCDebug(logImageViewer) << "Connected ImageInfoCache signals to onLoadFinished and onSizeChanged.";
}

ImageInfo::ImageInfo(const QUrl &source, QObject *parent)
    : QObject(parent)
{
    qCDebug(logImageViewer) << "ImageInfo constructor called with source:" << source;
    connect(CacheInstance(), &ImageInfoCache::imageDataChanged, this, &ImageInfo::onLoadFinished);
    connect(CacheInstance(), &ImageInfoCache::imageSizeChanged, this, &ImageInfo::onSizeChanged);
    setSource(source);
}

ImageInfo::~ImageInfo()
{
    qCDebug(logImageViewer) << "ImageInfo destructor called";
}

ImageInfo::Status ImageInfo::status() const
{
    qCDebug(logImageViewer) << "ImageInfo::status called, returning:" << imageStatus;
    return imageStatus;
}

/**
   @brief 设置图片路径为 \a path .
        若缓存中不存在对应图片的信息，将尝试加载对应的数据
 */
void ImageInfo::setSource(const QUrl &source)
{
    qCDebug(logImageViewer) << "ImageInfo::setSource called with source:" << source;
    if (imageUrl != source) {
        qCDebug(logImageViewer) << "ImageInfo::setSource changed source from:" << imageUrl << "to:" << source;
        imageUrl = source;
        Q_EMIT sourceChanged();

        // 刷新数据
        refreshDataFromCache(true);
    }
}

/**
   @return 返回图片路径信息
 */
QUrl ImageInfo::source() const
{
    qCDebug(logImageViewer) << "ImageInfo::source called, returning:" << imageUrl;
    return imageUrl;
}

/**
   @return 返回图片的类型，参考 Types::ImageType
 */
int ImageInfo::type() const
{
    int ret = data ? data->type : Types::NullImage;
    qCDebug(logImageViewer) << "ImageInfo::type called, returning:" << ret;
    return ret;
}

/**
   @return 返回图片宽度
 */
int ImageInfo::width() const
{
    int ret = data ? data->size.width() : -1;
    qCDebug(logImageViewer) << "ImageInfo::width called, returning:" << ret;
    return ret;
}

/**
   @return 返回图片高度
 */
int ImageInfo::height() const
{
    int ret = data ? data->size.height() : -1;
    qCDebug(logImageViewer) << "ImageInfo::height called, returning:" << ret;
    return ret;
}

/**
   @brief 交换宽高，用于在图片旋转时使用
    数据通过共享指针存储，仅在单处修改即可
 */
void ImageInfo::swapWidthAndHeight()
{
    qCDebug(logImageViewer) << "ImageInfo::swapWidthAndHeight called";
    if (data) {
        qCDebug(logImageViewer) << "ImageInfo::swapWidthAndHeight swapping width and height";
        data->size = QSize(data->size.height(), data->size.width());
        // 广播大小变更信号
        Q_EMIT CacheInstance()->imageSizeChanged(imageUrl.toLocalFile(), currentIndex);
    }
}

/**
   @brief 设置当前图片的帧索引，仅对多页图有效，若成功设置，进行异步加载图片
   @param index 图片帧索引
 */
void ImageInfo::setFrameIndex(int index)
{
    qCDebug(logImageViewer) << "ImageInfo::setFrameIndex called with index:" << index;
    if (currentIndex != index) {
        qCDebug(logImageViewer) << "ImageInfo::setFrameIndex changing currentIndex from:" << currentIndex << "to:" << index;
        currentIndex = index;
        Q_EMIT frameIndexChanged();

        // 刷新数据
        refreshDataFromCache(true);
    }
}

/**
   @return 返回当前图片的帧索引，对多页图有效
 */
int ImageInfo::frameIndex() const
{
    qCDebug(logImageViewer) << "ImageInfo::frameIndex called, returning:" << currentIndex;
    return currentIndex;
}

/**
   @return 返回当前图片的总帧数，默认为1
 */
int ImageInfo::frameCount() const
{
    int ret = data ? data->frameCount : 1;
    qCDebug(logImageViewer) << "ImageInfo::frameCount called, returning:" << ret;
    return ret;
}

/**
   @brief 设置图片运行时属性缩放为 \a s ，除缩放外，还有图片组件在界面上的偏移值 x y 。
    这些属性不会用于状态的实时同步或抛出信号，仅在初始化图片展示时取缓存数据复位状态。
 */
void ImageInfo::setScale(qreal s)
{
    qCDebug(logImageViewer) << "ImageInfo::setScale called with scale:" << s;
    if (data && data->scale != s) {
        qCDebug(logImageViewer) << "ImageInfo::setScale changing scale from:" << data->scale << "to:" << s;
        data->scale = s;
    }
}

qreal ImageInfo::scale() const
{
    qreal ret = data ? data->scale : -1;
    qCDebug(logImageViewer) << "ImageInfo::scale called, returning:" << ret;
    return ret;
}

void ImageInfo::setX(qreal x)
{
    qCDebug(logImageViewer) << "ImageInfo::setX called with x:" << x;
    if (data) {
        qCDebug(logImageViewer) << "ImageInfo::setX changing x from:" << data->x << "to:" << x;
        data->x = x;
    }
}

qreal ImageInfo::x() const
{
    qreal ret = data ? data->x : 0;
    qCDebug(logImageViewer) << "ImageInfo::x called, returning:" << ret;
    return ret;
}

void ImageInfo::setY(qreal y)
{
    qCDebug(logImageViewer) << "ImageInfo::setY called with y:" << y;
    if (data) {
        qCDebug(logImageViewer) << "ImageInfo::setY changing y from:" << data->y << "to:" << y;
        data->y = y;
    }
}

qreal ImageInfo::y() const
{
    qreal ret = data ? data->y : 0;
    qCDebug(logImageViewer) << "ImageInfo::y called, returning:" << ret;
    return ret;
}

/**
   @return 返回当前图片是否存在，图片可能在展示过程中被销毁
 */
bool ImageInfo::exists() const
{
    bool ret = data ? data->exist : false;
    qCDebug(logImageViewer) << "ImageInfo::exists() called, returning: " << ret;
    return ret;
}

/**
   @return 返回是否存在已缓存的缩略图数据
   @warning 缓存空间有限，已缓存的缩略图数据可能后续被移除，需重新加载缩略图
 */
bool ImageInfo::hasCachedThumbnail() const
{
    qCDebug(logImageViewer) << "ImageInfo::hasCachedThumbnail called";
    if (imageUrl.isEmpty()) {
        return false;
    } else {
        switch (type()) {
        case Types::NullImage:
        case Types::DamagedImage:
            qCDebug(logImageViewer) << "ImageInfo::hasCachedThumbnail returning false for damaged image";
            return false;
        default:
            break;
        }

        bool ret = ThumbnailCache::instance()->contains(imageUrl.toLocalFile(), frameIndex());
        qCDebug(logImageViewer) << "ImageInfo::hasCachedThumbnail returning:" << ret;
        return ret;
    }
}

/**
   @brief 强制重新加载当前图片信息
 */
void ImageInfo::reloadData()
{
    qCDebug(logImageViewer) << "Reloading image data:" << imageUrl.toLocalFile() << "frame:" << currentIndex;
    setStatus(Loading);
    CacheInstance()->load(imageUrl.toLocalFile(), currentIndex, true);
}

/**
   @brief 清除当前文件的缓存，多页图将清空所有帧图像的缓存
 */
void ImageInfo::clearCurrentCache()
{
    qCDebug(logImageViewer) << "ImageInfo::clearCurrentCache called";
    if (data) {
        qCDebug(logImageViewer) << "Clearing current image cache:" << imageUrl.toLocalFile()
                                << "frames:" << data->frameCount;
        for (int i = 0; i < data->frameCount; ++i) {
            CacheInstance()->removeCache(imageUrl.toLocalFile(), i);
        }
    }
}

/**
   @brief 清空缓存数据，包括缩略图缓存和图片属性缓存
   @note 这不会影响处于加载队列中的任务
 */
void ImageInfo::clearCache()
{
    qCDebug(logImageViewer) << "Clearing all image caches";
    CacheInstance()->clearCache();
    ThumbnailCache::instance()->clear();
}

/**
   @brief 设置图片信息状态为 \a status
 */
void ImageInfo::setStatus(ImageInfo::Status status)
{
    qCDebug(logImageViewer) << "ImageInfo::setStatus called with status:" << status;
    if (imageStatus != status) {
        qCDebug(logImageViewer) << "ImageInfo::setStatus changing status from:" << imageStatus << "to:" << status;
        imageStatus = status;
        Q_EMIT statusChanged();
    }
}

/**
   @brief 更新图像数据，将发送部分关键数据的更新信号
   @param newData 新图像数据
   @return 返回数据是否确实存在变更
 */
bool ImageInfo::updateData(const QSharedPointer<ImageInfoData> &newData)
{
    qCDebug(logImageViewer) << "ImageInfo::updateData called with newData:" << newData;
    if (newData == data) {
        qCDebug(logImageViewer) << "ImageInfo::updateData returning false because newData is the same as data";
        return false;
    }
    ImageInfoData::Ptr oldData = data;
    data = newData;

    bool change = false;
    if (oldData->type != newData->type) {
        qCDebug(logImageViewer) << "ImageInfo::updateData changing type from:" << oldData->type << "to:" << newData->type;
        Q_EMIT typeChanged();
        change = true;
    }
    if (oldData->size != newData->size) {
        qCDebug(logImageViewer) << "ImageInfo::updateData changing size from:" << oldData->size << "to:" << newData->size;
        Q_EMIT widthChanged();
        Q_EMIT heightChanged();
        change = true;
    }
    if (oldData->frameIndex != newData->frameIndex) {
        qCDebug(logImageViewer) << "ImageInfo::updateData changing frameIndex from:" << oldData->frameIndex << "to:" << newData->frameIndex;
        Q_EMIT frameIndexChanged();
        change = true;
    }
    if (oldData->frameCount != newData->frameCount) {
        qCDebug(logImageViewer) << "ImageInfo::updateData changing frameCount from:" << oldData->frameCount << "to:" << newData->frameCount;
        Q_EMIT frameCountChanged();
        change = true;
    }
    if (oldData->exist != newData->exist) {
        qCDebug(logImageViewer) << "ImageInfo::updateData changing exist from:" << oldData->exist << "to:" << newData->exist;
        Q_EMIT existsChanged();
        change = true;
    }

    qCDebug(logImageViewer) << "ImageInfo::updateData returning:" << change;
    return change;
}

/**
   @brief 从缓存中刷新数据，若数据变更，将触发相关数据更新信号。
    \a reload 标识此次刷新是否为重新加载数据，重新加载在无数据时将请求刷新数据。
 */
void ImageInfo::refreshDataFromCache(bool reload)
{
    qCDebug(logImageViewer) << "ImageInfo::refreshDataFromCache called with reload:" << reload;
    QString localPath = imageUrl.toLocalFile();
    if (localPath.isEmpty()) {
        qCWarning(logImageViewer) << "Empty image path";
        qCDebug(logImageViewer) << "ImageInfo::refreshDataFromCache setting status to Error";
        setStatus(Error);
        return;
    }

    ImageInfoData::Ptr newData = CacheInstance()->find(localPath, currentIndex);
    qCDebug(logImageViewer) << "ImageInfo::refreshDataFromCache newData:" << newData;
    if (newData) {
        if (data) {
            // 刷新旧数据，需发送部分更新信号，确有数据变更再发送 infoChanged()
            if (updateData(newData)) {
                qCDebug(logImageViewer) << "Image data updated:" << localPath << "frame:" << currentIndex;
                qCDebug(logImageViewer) << "ImageInfo::refreshDataFromCache emitting infoChanged";
                Q_EMIT infoChanged();
            }
        } else {
            data = newData;
            qCDebug(logImageViewer) << "New image data loaded:" << localPath << "frame:" << currentIndex;
            Q_EMIT infoChanged();
        }

        setStatus(data->isError() ? Error : Ready);
    } else {
        qCDebug(logImageViewer) << "ImageInfo::refreshDataFromCache no newData found";
        if (reload) {
            qCDebug(logImageViewer) << "Requesting image reload:" << localPath << "frame:" << currentIndex;
            setStatus(Loading);
            CacheInstance()->load(localPath, currentIndex);
        } else {
            qCWarning(logImageViewer) << "Image data not found:" << localPath << "frame:" << currentIndex;
            setStatus(Error);
        }
    }
    qCDebug(logImageViewer) << "ImageInfo::refreshDataFromCache finished";
}

/**
   @brief 内部数据异步加载完成，返回处理的文件路径 \a path ,
        根据加载结果，设置图片信息状态
 */
void ImageInfo::onLoadFinished(const QString &path, int frameIndex)
{
    qCDebug(logImageViewer) << "ImageInfo::onLoadFinished called with path:" << path << "frameIndex:" << frameIndex;
    if (imageUrl.toLocalFile() == path && currentIndex == frameIndex) {
        // 从缓存刷新数据，不重新加载
        refreshDataFromCache(false);
    } else {
        qCDebug(logImageViewer) << "Loaded image does not match current image, skipping update.";
    }
    qCDebug(logImageViewer) << "ImageInfo::onLoadFinished finished";
}

/**
   @brief 图片 \a path 的第 \a frameIndex 帧的图片大小出现变更
 */
void ImageInfo::onSizeChanged(const QString &path, int frameIndex)
{
    qCDebug(logImageViewer) << "ImageInfo::onSizeChanged called with path:" << path << "frameIndex:" << frameIndex;
    if (imageUrl.toLocalFile() == path && currentIndex == frameIndex) {
        qCDebug(logImageViewer) << "ImageInfo::onSizeChanged updating width and height";
        if (data) {
            qCDebug(logImageViewer) << "ImageInfo::onSizeChanged emitting widthChanged and heightChanged";
            Q_EMIT widthChanged();
            Q_EMIT heightChanged();
        }
    }
    qCDebug(logImageViewer) << "ImageInfo::onSizeChanged finished";
}

#include "imageinfo.moc"
