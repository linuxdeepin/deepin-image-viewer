// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_imageprovider.h"
#include "imageprovider.h"

#include <QImage>
#include <QUrl>
#include <QDir>
#include <QSize>
#include <QPixmap>
#include <QCoreApplication>
#include <QQuickImageResponse>
#include <QQuickTextureFactory>
#include <QThreadPool>

// 生成临时 PNG 文件，返回绝对路径
static QString makeProviderTempImage(const QString &name, int side = 64)
{
    QString dir = QDir::tempPath() + "/ut_imageprovider_" +
                  QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(dir);
    QString path = dir + "/" + name;
    QImage img(side, side, QImage::Format_ARGB32);
    img.fill(Qt::green);
    img.save(path, "PNG");
    return path;
}

void ut_imageprovider::SetUp()
{
}

void ut_imageprovider::TearDown()
{
}

// ==================== ProviderCache ====================

// 测试 ProviderCache 构造与析构
TEST_F(ut_imageprovider, ProviderCacheConstruct)
{
    ProviderCache cache;
    cache.clearCache();  // 确认可调用
    SUCCEED();
}

// 测试 rotateImageCached 角度为 0 提前返回
TEST_F(ut_imageprovider, RotateImageCachedZeroAngle)
{
    ProviderCache cache;
    cache.rotateImageCached(0, "/tmp/ut_zero_angle.png");
    SUCCEED();
}

// 测试 rotateImageCached 缓存中无图像（image 为 null 走警告分支）
TEST_F(ut_imageprovider, RotateImageCachedNoCachedImage)
{
    ProviderCache cache;
    cache.rotateImageCached(90, "/tmp/ut_no_cached_image.png", 0);
    SUCCEED();
}

// 测试 rotateImageCached 对已缓存图像执行旋转
TEST_F(ut_imageprovider, RotateImageCachedWithCachedImage)
{
    ProviderCache cache;
    QString path = makeProviderTempImage("rot.png");
    QImage img(path);
    ASSERT_FALSE(img.isNull());

    // 先把图像放入缓存（imageCache 为 protected 成员，借由 -fno-access-control 直接访问）
    cache.imageCache.add(path, 0, img);
    cache.rotateImageCached(90, path, 0);

    // 同一路径连续旋转（走 lastRotatePath 分支）
    cache.rotateImageCached(90, path, 0);
    // 360 度不旋转分支
    cache.rotateImageCached(180, path, 0);
    SUCCEED();
}

// 测试 removeImageCache
TEST_F(ut_imageprovider, RemoveImageCache)
{
    ProviderCache cache;
    QString path = makeProviderTempImage("rm.png");
    QImage img(path);
    cache.imageCache.add(path, 0, img);
    cache.imageCache.add(path, 1, img);
    cache.removeImageCache(path);
    EXPECT_FALSE(cache.imageCache.contains(path, 0));

    // 移除不存在的项不崩溃
    cache.removeImageCache("/tmp/ut_not_in_cache.png");
}

// 测试 renameImageCache
TEST_F(ut_imageprovider, RenameImageCache)
{
    ProviderCache cache;
    QString oldPath = makeProviderTempImage("old.png");
    QString newPath = QFileInfo(oldPath).absolutePath() + "/new.png";
    QImage img(oldPath);
    cache.imageCache.add(oldPath, 0, img);
    cache.renameImageCache(oldPath, newPath);
    EXPECT_TRUE(cache.imageCache.contains(newPath, 0));
    EXPECT_FALSE(cache.imageCache.contains(oldPath, 0));

    // 重命名不存在的项不崩溃
    cache.renameImageCache("/tmp/ut_no_old.png", "/tmp/ut_no_new.png");
}

// 测试 clearCache 清空状态
TEST_F(ut_imageprovider, ClearCache)
{
    ProviderCache cache;
    QString path = makeProviderTempImage("clr.png");
    QImage img(path);
    cache.imageCache.add(path, 0, img);
    cache.rotateImageCached(90, path, 0);

    cache.clearCache();
    EXPECT_FALSE(cache.imageCache.contains(path, 0));
    EXPECT_TRUE(cache.lastRotatePath.isEmpty());
}

// 测试 preloadImage 默认实现（空操作）
TEST_F(ut_imageprovider, PreloadImageDefault)
{
    ProviderCache cache;
    cache.preloadImage("/tmp/ut_preload.png");
    SUCCEED();
}

// ==================== ImageProvider ====================

// 测试 ImageProvider 构造与析构
TEST_F(ut_imageprovider, ImageProviderConstruct)
{
    ImageProvider provider;
    SUCCEED();
}

// 测试 ImageProvider::requestImage 读取真实文件(frame 0)
TEST_F(ut_imageprovider, ImageProviderRequestImage)
{
    ImageProvider provider;
    QString path = makeProviderTempImage("req.png", 80);
    QString id = QUrl::fromLocalFile(path).toString();  // id 需为 file:// URL
    QSize outSize;
    QImage img = provider.requestImage(id, &outSize, QSize(40, 40));
    EXPECT_FALSE(img.isNull());
}

// 测试 ImageProvider::requestImage 缓存命中
TEST_F(ut_imageprovider, ImageProviderRequestImageCached)
{
    ImageProvider provider;
    QString path = makeProviderTempImage("cached.png");
    QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize1;
    provider.requestImage(id, &outSize1, QSize());  // 首次加载并缓存
    QSize outSize2;
    QImage img = provider.requestImage(id, &outSize2, QSize());  // 走缓存
    EXPECT_FALSE(img.isNull());
}

// 测试 ImageProvider::requestImage 不存在文件返回空图像
TEST_F(ut_imageprovider, ImageProviderRequestImageNonexist)
{
    ImageProvider provider;
    QImage img = provider.requestImage("/tmp/ut_not_exist.png", nullptr, QSize());
    EXPECT_TRUE(img.isNull());
}

// 测试 ImageProvider::requestImage 带 frame 索引（多页图，非 0 帧）
TEST_F(ut_imageprovider, ImageProviderRequestImageWithFrame)
{
    ImageProvider provider;
    // 单页 PNG 指定非 0 帧，readMultiImage 返回空
    QImage img = provider.requestImage("file:///tmp/ut_frame.png#frame_1", nullptr, QSize());
    // 非存在文件，frame != 0，返回空
    EXPECT_TRUE(img.isNull());
}

// ==================== ThumbnailProvider ====================

// 测试 ThumbnailProvider 构造
TEST_F(ut_imageprovider, ThumbnailProviderConstruct)
{
    ThumbnailProvider provider;
    SUCCEED();
}

// 测试 ThumbnailProvider::requestImage 读取真实文件
TEST_F(ut_imageprovider, ThumbnailProviderRequestImage)
{
    ThumbnailProvider provider;
    QString path = makeProviderTempImage("thumb.png", 100);
    QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;
    QImage img = provider.requestImage(id, &outSize, QSize(50, 50));
    EXPECT_FALSE(img.isNull());
}

// 测试 ThumbnailProvider::requestImage 缓存命中
TEST_F(ut_imageprovider, ThumbnailProviderRequestImageCached)
{
    ThumbnailProvider provider;
    QString path = makeProviderTempImage("thumb_cached.png", 100);
    QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize1;
    provider.requestImage(id, &outSize1, QSize());  // 首次，写入缩略图缓存
    QImage img = provider.requestImage(id, nullptr, QSize());  // 走 ThumbnailCache
    EXPECT_FALSE(img.isNull());
}

// 测试 ThumbnailProvider::requestPixmap
TEST_F(ut_imageprovider, ThumbnailProviderRequestPixmap)
{
    ThumbnailProvider provider;
    QString path = makeProviderTempImage("pixmap.png", 100);
    QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;
    QPixmap pix = provider.requestPixmap(id, &outSize, QSize(50, 50));
    EXPECT_FALSE(pix.isNull());
}

// ==================== AsyncImageProvider ====================

// 测试 AsyncImageProvider 构造
TEST_F(ut_imageprovider, AsyncImageProviderConstruct)
{
    AsyncImageProvider provider;
    SUCCEED();
}

// 测试 AsyncImageProvider::requestImageResponse 返回应答并完成
TEST_F(ut_imageprovider, AsyncImageProviderRequestImageResponse)
{
    AsyncImageProvider provider;
    QString path = makeProviderTempImage("async.png", 100);
    QString id = QUrl::fromLocalFile(path).toString();

    QQuickImageResponse *response = provider.requestImageResponse(id, QSize(50, 50));
    ASSERT_NE(response, nullptr);

    // waitForDone 是线程 join，不处理事件循环，避免 ASAN 下 DBus 残留事件 SEGV
    QThreadPool::globalInstance()->waitForDone();
    delete response;
}

// 测试 AsyncImageProvider::preloadImage 不崩溃
TEST_F(ut_imageprovider, AsyncImageProviderPreloadImage)
{
    AsyncImageProvider provider;
    QString path = makeProviderTempImage("preload.png", 100);
    provider.preloadImage(QUrl::fromLocalFile(path).toString());

    // 等待后台线程完成，避免 provider 析构后线程访问已释放资源
    QThreadPool::globalInstance()->waitForDone();
    SUCCEED();
}

// ==================== Deleting Destructor (new + delete) ====================

// ImageProvider 析构函数: 触发 D0 deleting destructor
TEST_F(ut_imageprovider, ImageProviderDeletingDestructor)
{
    auto *obj = new ImageProvider();
    delete obj;
    SUCCEED();
}

// ThumbnailProvider 析构函数: 触发 D0 deleting destructor
TEST_F(ut_imageprovider, ThumbnailProviderDeletingDestructor)
{
    auto *obj = new ThumbnailProvider();
    delete obj;
    SUCCEED();
}

// AsyncImageProvider 析构函数: 触发 D0 deleting destructor
TEST_F(ut_imageprovider, AsyncImageProviderDeletingDestructor)
{
    auto *obj = new AsyncImageProvider();
    delete obj;
    SUCCEED();
}

// ProviderCache 析构函数: 触发 D0 deleting destructor
TEST_F(ut_imageprovider, ProviderCacheDeletingDestructor)
{
    auto *obj = new ProviderCache();
    delete obj;
    SUCCEED();
}

// AsyncImageResponse::textureFactory(): 通过基类虚函数调用
TEST_F(ut_imageprovider, AsyncImageResponseTextureFactory)
{
    AsyncImageProvider provider;
    QString path = makeProviderTempImage("texture.png", 64);
    QString id = QUrl::fromLocalFile(path).toString();

    QQuickImageResponse *response = provider.requestImageResponse(id, QSize(32, 32));
    ASSERT_NE(response, nullptr);
    // 等待后台加载完成，避免数据竞争
    QThreadPool::globalInstance()->waitForDone();
    // textureFactory() 是 QQuickImageResponse 的虚函数，实际调用 AsyncImageResponse::textureFactory
    QQuickTextureFactory *factory = response->textureFactory();
    EXPECT_NE(factory, nullptr);
    delete factory;
    delete response;
}
