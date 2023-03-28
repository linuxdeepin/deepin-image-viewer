// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multiimageload.h"
#include "unionimage/unionimage.h"
#include "imagedata/thumbnailcache.h"

#include <QDebug>

/**
   @class MultiImageLoad
   @brief 提供用于 *.tif 等多页图的单独图像加载处理类
        通过分割传入的 id ，判断当前读取的文件的行数和图片索引。
        在 QML 中注册的标识为 "multiimage"
   @warning QQuickImageProvider 派生的接口可能多线程调用，必须保证实现函数是可重入的。
   @threadsafe
 */

MultiImageLoad::MultiImageLoad()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    // 缓存最近3张图片
    imageCache.setMaxCost(3);
}

MultiImageLoad::~MultiImageLoad() {}

/**
   @brief 外部请求图像文件中指定帧的图像，指定帧号通过传入的 \a id 进行区分。
        \a id 格式为 \b{图像路径#frame_帧号_缩略图标识} ，例如 "/home/tmp.tif#frame_3_thumbnail" ，
        表示 tmp.tif 图像文件的第四帧图像缩略图。_thumbnail 可移除，表示需要完整图片
        这个 id 在 QML 文件中组合。
   @param id            图像索引(0 ~ frameCount - 1)
   @param size          图像的原始大小，有需要时可传出
   @param requestedSize 请求的图像大小
   @return 读取的图像数据

   @note 当前需要读取多页图的图像格式仅为 *.tif ，通过默认 QImageReader 即可读取，
        后续其它格式考虑在 LibUnionImage_NameSpace 中添加新的接口。
 */
QImage MultiImageLoad::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    // 拆分id，获取当前读取的文件和图片索引
    static const QString s_tagFrame = "#frame_";
    static const QString s_tagThumbnail = "_thumbnail";
    QString checkId = id;
    bool useThumbnail = checkId.endsWith(s_tagThumbnail);
    if (useThumbnail) {
        checkId.chop(s_tagThumbnail.size());
    }

    // 从后向前查询索引标识
    int index = checkId.lastIndexOf(QRegExp(QString("%1\\d+$").arg(s_tagFrame)));
    int frameIndex = 0;
    QString path;
    if (-1 == index) {
        index = 0;
        path = checkId;
    } else {
        // 移除 "#frame_" 字段
        path = checkId.left(index);
        frameIndex = checkId.right(checkId.size() - index - s_tagFrame.size()).toInt();
    }

    QString tempPath = QUrl(path).toLocalFile();
    QImage image;

    // 判断缓存中是否存在缩略图
    bool hasThumbnail = false;
    if (useThumbnail) {
        hasThumbnail = ThumbnailCache::instance()->contains(tempPath, frameIndex);
        if (hasThumbnail) {
            return ThumbnailCache::instance()->get(tempPath, frameIndex);
        }
    } else {
        image = imageCache.get(tempPath, frameIndex);
    }

    if (image.isNull()) {
        if (frameIndex) {
            image = readMultiImage(tempPath, frameIndex);
        } else {
            image = readNormalImage(tempPath);
        }

        imageCache.add(tempPath, frameIndex, image);
    }

    // 不存在缩略图信息，缓存图片
    if (!hasThumbnail) {
        QImage tmpImage = image.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        ThumbnailCache::instance()->add(tempPath, frameIndex, tmpImage);
    }

    if (size) {
        *size = image.size();
    }

    // 调整图像大小
    if (!image.isNull() && image.size() != requestedSize && requestedSize.isValid()) {
        image = image.scaled(requestedSize);
    }
    return image;
}

/**
   @brief 调用 requestImage() 获取图形信息，返回格式为 QPixmap 。
 */
QPixmap MultiImageLoad::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    return QPixmap::fromImage(requestImage(id, size, requestedSize));
}

/**
   @brief 对缓存的 \a imagePath 图片执行旋转 \a angle 的操作。
   @note 待操作的图片必须在缓存中，当前展示的图片必定在缓存中
 */
void MultiImageLoad::rotateImageCached(int angle, const QString &imagePath, int frameIndex)
{
    // 旋转角度为0时，清除旋转状态缓存，防止外部文件变更后仍使用上一次的旋转状态。
    QMutexLocker _locker(&mutex);
    if (0 == angle) {
        lastRotatePath.clear();
        lastRotateImage = QImage();
        return;
    }

    QImage image;
    if (imagePath != lastRotatePath) {
        image = imageCache.get(imagePath, frameIndex);

        // 仅在首次处理时记录图像数据，防止多次旋转处理导致图片质量降低
        lastRotateImage = image;
        lastRotatePath = imagePath;
    } else {
        image = lastRotateImage;
    }
    _locker.unlock();

    if (!image.isNull()) {
        // 360度不执行旋转
        if (!!(angle % 360)) {
            LibUnionImage_NameSpace::rotateImage(angle, image);
        }

        _locker.relock();
        // 更新图片缓存
        imageCache.add(imagePath, frameIndex, image);
        _locker.unlock();

        // 同样更新缩略图缓存
        QImage tmpImage = image.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        ThumbnailCache::instance()->add(imagePath, frameIndex, tmpImage);
    }
}

/**
   @brief 移除文件路径为 \a imagePath 的图片信息，主要用于文件删除，重命名等变更时重置状态
 */
void MultiImageLoad::removeImageCache(const QString &imagePath)
{
    // 直接缓存的图像信息较少，遍历查询是否包含对应的图片
    QList<ThumbnailCache::Key> keys;
    QMutexLocker _locker(&mutex);
    keys = imageCache.keys();
    _locker.unlock();

    for (const ThumbnailCache::Key &key : keys) {
        if (key.first == imagePath) {
            _locker.relock();
            imageCache.remove(key.first, key.second);
            _locker.unlock();
        }
    }
}

/**
   @brief 移除图像加载器中的缓存数据
 */
void MultiImageLoad::clearCache()
{
    QMutexLocker _locker(&mutex);
    imageCache.clear();
    lastRotatePath.clear();
    lastRotateImage = QImage();
}

/**
   @return 使用 FreeImage 库读取 \a imagePath 的图像数据并返回
 */
QImage MultiImageLoad::readNormalImage(const QString &imagePath)
{
    QImage image;
    QString error;
    if (!LibUnionImage_NameSpace::loadStaticImageFromFile(imagePath, image, error)) {
        qWarning() << QString("Load image %1 error: %2").arg(imagePath).arg(error);
    }
    return image;
}

/**
   @return 读取图像路径 \a imagePath 和 \a frameIndex 指向的图像信息。
 */
QImage MultiImageLoad::readMultiImage(const QString &imagePath, int frameIndex)
{
    QMutexLocker _locker(&mutex);
    // 重新设置图像读取类
    if (imagePath != imageReader.fileName() || !imageReader.canRead()) {
        imageReader.setFileName(imagePath);
    }

    if (imageReader.jumpToImage(frameIndex)) {
        // 读取图像数据
        return imageReader.read();
    }
    return QImage();
}
