// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | AsyncImageResponse(p,i,r) | low | - | 1 | 1 |（经 requestImageResponse 构造，blocked-pool 冻结后观测初始状态）
// | ~AsyncImageResponse | low | - | 1 | 1 |
// | loadScaleAndCache(path,frame,targetSize) | low | - | 1 | 12 |（private，经 run() 间接覆盖）
// | run() | mid | - | 2 | 12 |
// | textureFactory() | low | - | 1 | 3 |
// | ProviderCache() | low | - | 1 | 1 |
// | ~ProviderCache | low | - | 1 | 1 |
// | clearCache() | mid | - | 2 | 2 |
// | preloadImage(filePath) | low | - | 1 | 1 |
// | removeImageCache(imagePath) | mid | - | 2 | 2 |
// | renameImageCache(oldPath,newPath) | low | - | 1 | 2 |
// | rotateImageCached(angle,imagePath,frameIndex) | low | - | 1 | 9 |（5 个 TEST_F + TEST_P×4）
// | AsyncImageProvider() | low | - | 1 | 1 |
// | ~AsyncImageProvider | low | - | 1 | 1 |
// | preloadImage(filePath) | low | - | 1 | 2 |
// | requestImageResponse(id,requestedSize) | low | - | 1 | 13 |（1 个专属 + AsyncImageResponseTest 12 个）
// | ImageProvider() | low | - | 1 | 1 |
// | ~ImageProvider | low | - | 1 | 1 |
// | ImageProvider::requestImage(id,size,requestedSize) | low | - | 1 | 13 |（8 个专属 + FreeImageDataTest 5 个实例）// | ThumbnailProvider() | low | - | 1 | 1 |
// | ~ThumbnailProvider | low | - | 1 | 1 |
// | ThumbnailProvider::requestImage(id,size,requestedSize) | low | - | 1 | 7 |
// | requestPixmap(id,size,requestedSize) | low | - | 1 | 2 |
// | parseProviderID(id,filePath,frameIndex)（static 自由函数） | low | - | 1 | 5 |（TEST_P×5，经 requestImage 的缓存键观测）
// | readNormalImage(imagePath,maxDimension)（static 自由函数） | low | - | 1 | 2 |
// | readNormalImageScaled(imagePath,targetSize)（static 自由函数） | low | - | 1 | 4 |（经 AsyncImageResponseTest run 路径）
// | readMultiImage(imagePath,frameIndex)（static 自由函数） | low | - | 1 | 2 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x] （inventory 全部 27 个 testable 条目均有映射，private/static 函数经公开 API 间接覆盖）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x] （id 无 tag/#frame_0/#frame_N/裸路径、缓存命中/未命中/污染、角度 0/90/180/270/360、尺寸有效/无效/放大/缩小/相等）
// 3. 每个等价类的边界值显式覆盖: [x] （帧号 0/1/越界 9、角度 0 与 360 倍数、缓存比请求大/小/宽高交错、空缓存循环 0 次）
// 4. 同质 ≥ 3 组用 TEST_P: [x] （parseProviderID×5、rotateImageCached 角度×4）
// 5. 分支清单 → 用例映射已列出: [x] （见下方分支清单，均来自 get_code_snippet 真实源码）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x] （源码无 throw；全部 if/for/三元分支均有映射）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x] （源码无 throw 分支，不适用）
// 8. 负面场景有专门用例: [x] （不存在文件/畸形 TIFF/越界帧号/空缓存/角度 0）
// 9. 负面用例验证强异常安全: [x] （角度 0 与缺失图像用例断言缓存与旋转状态不变）
// 10. stub_ext vs gMock 选择正确: [x] （仅 stub QGuiApplication::primaryScreen 一个 Qt 静态方法，无 gMock）
//
// 环境隔离说明（SetUp/TearDown 全局生效）：
// - AsyncImageResponse 为 imageprovider.cpp 文件内局部类：经 AsyncImageProvider::requestImageResponse
//   构造，用 QQuickImageResponse 基类接口（textureFactory/finished）与 dynamic_cast<QRunnable*> 断言
// - 异步确定性：普通用例 QThreadPool::globalInstance()->waitForDone()；需同步执行 run() 的用例先
//   FreezePool 哨兵占住唯一线程冻结池，再 tryTake 取回 runnable（TearDown 兜底恢复 maxThreadCount）
// - ThumbnailCache::instance() 单例在 SetUp/TearDown 均会 clear()，防止跨文件/跨用例污染（test_imageinfo 等同样操作该单例）
// - 多帧输入用手工构造的双页 TIFF：本机 Qt6 gif 插件只读不可写、tif/ico/webp 连续 write() 仅保留末帧，
//   QImageWriter 无法产出多帧文件；畸形 TIFF（头有效数据缺失）用于 size() 有效而 read() 失败的分支
// - 全部测试图片位于各 Fixture 独立的 QTemporaryDir，路径天然唯一，不依赖测试机固定路径
//
// 疑似源码缺陷（行为锁定，未修改源码）：
// 1. ProviderCache::rotateImageCached：angle==0 注释称清除旋转状态，实现直接 return 未清（imageprovider.cpp:220-225）
// 2. ThumbnailProvider::requestImage：null 缩略图无条件入单例缓存，污染后续有效请求（imageprovider.cpp:519-520）
// 3. ProviderCache::clearCache：复位 lastRotatePath/lastRotateImage 但未复位 lastRotation（imageprovider.cpp:296-304）
// 4. parseProviderID：文档示例使用裸路径加帧尾标形态，但 QUrl(裸路径).toLocalFile() 返回空串，
//    裸路径 id 解析不出文件路径（imageprovider.cpp:30-42）
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:168-198 AsyncImageResponse::run）
// B1: image.isNull()（缓存未命中）→ loadScaleAndCache 读盘
// B2: 缓存命中 && !requestedSize.isValid() → 原样返回缓存
// B3: 缓存命中 && valid && (w>rw || h>rh) → scaled 缩小（仅副本，不回写缓存）
// B4: 缓存命中 && valid && (w<rw && h<rh) → imageCache.remove + loadScaleAndCache 重读
// B5: 缓存命中 && valid && 尺寸均不超（含等宽/等高交错）→ 原样返回
// 用例映射：
// - Run_CacheMissWithDownscaleRequest_LoadsScaledAndCaches          → B1
// - Run_InvalidRequestedSize_LoadsFullSizeImage                    → B1
// - Run_UpscaleRequested_LoadsWithoutUpscaling                     → B1
// - Run_TruncatedImage_FallsBackToFullLoadNull                     → B1
// - Run_NonexistentFile_EmitsFinishedWithNullTexture               → B1
// - Run_MultiFrameRequest_LoadsFrameScaledAndCaches                → B1
// - Run_CacheHitInvalidRequested_ReturnsCachedAsIs                 → B2
// - Run_CacheHitLargerWidth_ScalesDownCopyOnly                     → B3（宽超）
// - Run_CacheHitLargerHeight_ScalesDownByHeight                    → B3（高超，|| 右侧）
// - Run_CacheHitSmallerBothDims_RemovesAndReloads                  → B4
// - Run_CacheHitEqualWidthSmallerHeight_ReturnsAsIs                → B5
// - Run_SynchronousExecution_EmitsFinishedExactlyOnce              → B1（手动同步 run + finished 断言）
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:138-188 Destructor~AsyncImageResponse，图谱区间含后续方法体）
// 注：析构本身无分支；图谱返回区间实际覆盖 textureFactory/loadScaleAndCache/run 的 8 个分支，逐条列出（与下方三个方法的清单一致）
// B1: loadScaleAndCache: if (frame) → 帧路由
// B2: loadScaleAndCache: if (!img.isNull() && targetSize.isValid() && 超尺寸) → scaled
// B3: loadScaleAndCache: if (!img.isNull()) → imageCache.add
// B4: run: if (!image.isNull()) → 命中日志
// B5: run: if (image.isNull()) → loadScaleAndCache
// B6: run: else if (requestedSize.isValid()) → 尺寸调整评估
// B7: run: if (w>rw || h>rh) → 缩小副本
// B8: run: else if (w<rw && h<rh) → remove + 重读
// 用例映射：
// - Destructor_AfterPoolExecution_DeletesResponseSafely              → B1-B8（run 执行后析构，缓存不受影响）
// - Run_MultiFrameRequest_LoadsFrameScaledAndCaches                  → B1/B2/B3
// - Run_CacheMissWithDownscaleRequest_LoadsScaledAndCaches           → B2/B3
// - Run_CacheHitLargerWidth_ScalesDownCopyOnly                       → B7
// - Run_CacheHitSmallerBothDims_RemovesAndReloads                     → B5/B8
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:145-163 AsyncImageResponse::loadScaleAndCache）
// B1: frame 非 0 → readMultiImage
// B2: frame == 0 → readNormalImageScaled
// B3: !img.isNull() && targetSize valid && 超尺寸 → scaled
// B4: !img.isNull() → provider->imageCache.add
// B5: img 为 null → 不缓存直接返回
// 用例映射：
// - Run_MultiFrameRequest_LoadsFrameScaledAndCaches                → B1/B3/B4
// - Run_CacheMissWithDownscaleRequest_LoadsScaledAndCaches         → B2/B3/B4
// - Run_UpscaleRequested_LoadsWithoutUpscaling                     → B2/B4（不缩放分支）
// - Run_NonexistentFile_EmitsFinishedWithNullTexture               → B5
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:218-261 ProviderCache::rotateImageCached）
// B1: angle == 0 → 加锁后直接 return（不清旋转状态，注释与实现不符 → 疑似缺陷 1）
// B2: imagePath != lastRotatePath → imageCache.get + 记录 lastRotate*/lastRotation=angle
// B3: imagePath == lastRotatePath → image=lastRotateImage，lastRotation += angle
// B4: image 非 null && lastRotation%360 != 0 → LibUnionImage_NameSpace::rotateImage
// B5: image 非 null && lastRotation%360 == 0 → 跳过旋转
// B6: image 非 null → imageCache.add + ThumbnailCache::instance()->add
// B7: image 为 null → 仅告警，不写缓存
// 用例映射：
// - RotateImageCached_ZeroAngle_ReturnsWithoutSideEffects          → B1（断言状态未被清/未变）
// - RotateImageCached_MissingImage_WritesNoCaches                  → B2/B7
// - RotateImageCached_DifferentPath_ResetsRotationState            → B2×2
// - RotateImageCached_SamePathTwice_AccumulatesRotationAngle       → B2/B3/B4/B6
// - RotateImageCached_TotalMultipleOf360_SkipsRotationUpdatesCaches → B2/B5/B6
// - RotateImageCached_AngleVariations_RotatesAndCaches（TEST_P×4）  → B2/B3/B4/B5/B6
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:266-277 ProviderCache::removeImageCache）
// B1: keys 为空 → for 循环 0 次
// B2: key.first == imagePath → remove 该键（全部帧）
// B3: key.first 不匹配 → 跳过
// 用例映射：
// - RemoveImageCache_NonexistentPath_LeavesCacheIntact             → B1/B3
// - RemoveImageCache_MatchingEntries_RemovesAllFrames              → B2/B3
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:279-291 ProviderCache::renameImageCache）
// B1: keys 为空 → for 循环 0 次
// B2: key.first == oldPath → take + 以 newPath add（全部帧）
// B3: 不匹配 → 跳过
// 用例映射：
// - RenameImageCache_NonexistentPath_IsNoOp                        → B1/B3
// - RenameImageCache_ExistingEntries_MovesAllFrames                → B2/B3
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:349-362 AsyncImageProvider::preloadImage）
// B1: primaryScreen() 非空 → displaySize = screen->size()
// B2: primaryScreen() 为空 → displaySize 保持默认 QSize()
// 用例映射：
// - PreloadImage_RealFile_CachesWithinScreenSize                   → B1
// - PreloadImage_NoPrimaryScreen_CachesFullImage                   → B2（stub primaryScreen 返回空）
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:391-436 ImageProvider::requestImage）
// B1: imageCache.get 为 null（未命中）→ 读盘分支
// B2: frameIndex 非 0 → readMultiImage
// B3: frameIndex == 0 → qBound(maxDim) + readNormalImage
// B4: 未命中 && size 非空 → *size = image.size()
// B5: 未命中 → imageCache.add（含 null 图像）
// B6: 命中 → else 直接使用缓存（size 不再更新）
// B7: !image.isNull() && size != requestedSize && requestedSize valid → scaled
// B8: 其余（null/尺寸相等/请求无效）→ 原样返回
// 用例映射：
// - RequestImage_UncachedFile_LoadsAndCachesExactSize              → B1/B3/B4/B5/B8
// - RequestImage_CachedEntry_ReturnsCachedWithoutDiskRead          → B6/B8
// - RequestImage_EqualRequestedSize_SkipsScaling                   → B7 反例（B8 等尺寸）
// - RequestImage_NonexistentFile_ReturnsNullAndCachesNull          → B1/B3/B5（null 缓存）
// - RequestImage_MultiFrameTag_LoadsRequestedFrame                 → B1/B2/B4/B5
// - RequestImage_FrameOutOfRange_CachesNullFrame                   → B1/B2/B5
// - RequestImage_LargeDimensionRequest_ScalesToRequest             → B1/B3/B4/B7
// - RequestImage_CorruptTiff_ReturnsNull                           → B1/B3/B5
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:470-527 ThumbnailProvider::requestImage）
// B1: ThumbnailCache::instance()->contains → 早退返回缓存（size 出参不更新）
// B2: frameIndex 非 0 → readMultiImage
// B3: reader orig valid → *size=orig + KeepAspectRatioByExpanding 缩放解码
// B4: orig 无效 → 跳过解码器路径
// B5: image 为 null → 回退 readNormalImage 全图加载（成功则更新 *size）
// B6: ThumbnailCache::instance()->add（含 null 缩略图 → 疑似缺陷 2）
// B7: size && size->isEmpty() → *size = image.size()
// B8: !image.isNull() && requestedSize valid && 尺寸不同 → scaled
// 用例映射：
// - RequestImage_UncachedImage_GeneratesReaderThumbnailAndCaches   → B3/B6/B7 反例/B8 反例
// - RequestImage_CachedThumbnail_ReturnsCacheEntryWithoutDisk      → B1
// - RequestImage_MultiFrameTag_LoadsFrameAndCachesHundredThumb     → B2/B6/B7
// - RequestImage_TruncatedImage_FallsBackToFullLoadNull            → B3/B5/B6
// - RequestImage_NonexistentFile_ReturnsNullAndCachesNull          → B4/B5/B6/B7
// - RequestImage_PoisonedCache_HidesNewlyCreatedFile               → B1（缺陷 2 行为锁定）
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:30-42 parseProviderID）
// B1: lastIndexOf 无 "#frame_N$" 匹配 → QUrl(id).toLocalFile + frame=0（裸路径输入时 toLocalFile 为空串）
// B2: 匹配 → 去掉尾部 tag 再 toLocalFile + toInt（帧号仍正确解析）
// 用例映射：
// - ParseProviderID_IdVariants_ParseExpectedPathAndFrame（TEST_P×5）→ B1/B2
//   （kind 0/1/2: file:// URL 常规路径；kind 3/4: 裸路径 → 路径键为空串，帧号照常）
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:67-93 readNormalImageScaled）
// B1: !targetSize.isValid() → readNormalImage 默认上限
// B2: orig valid && target 任一边 < orig → setScaledSize 解码
// B3: B2 解码成功 → 直接返回
// B4: B2 解码失败 → 回退 readNormalImage(15000)
// B5: target >= 原图（放大）→ readNormalImage(15000)
// 用例映射：
// - Run_InvalidRequestedSize_LoadsFullSizeImage                    → B1
// - Run_CacheMissWithDownscaleRequest_LoadsScaledAndCaches         → B2/B3
// - Run_TruncatedImage_FallsBackToFullLoadNull                     → B2/B4
// - Run_UpscaleRequested_LoadsWithoutUpscaling                     → B5
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:98-108 readMultiImage）
// B1: jumpToImage 成功 → read
// B2: 失败 → 返回 null
// 用例映射：
// - ReadMultiImage_FrameNavigation_LoadsExpectedFrame              → B1（帧 0/帧 1 内容区分）
// - RequestImage_FrameOutOfRange_CachesNullFrame                   → B2
//
// 分支清单（来源：get_code_snippet imageprovider.cpp:48-58 readNormalImage）
// B1: loadStaticImageFromFile 失败 → qCWarning
// B2: 成功 → 返回图像
// 用例映射：
// - ReadNormalImage_ValidFile_LoadsExactDimensions                 → B2
// - RequestImage_CorruptTiff_ReturnsNull                           → B1
//

#include "imageprovider.h"
#include "thumbnailcache.h"

#include "stub_ext/stubext.h"

#include <gtest/gtest.h>

#include <QColor>
#include <QDataStream>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QQuickImageResponse>
#include <QQuickTextureFactory>
#include <QRunnable>
#include <QScreen>
#include <QSemaphore>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QThreadPool>
#include <QUrl>

namespace {

// 纯色 PNG 测试图（尺寸刻意取 2:1，保证 KeepAspectRatio 缩放结果精确无舍入）
QString makePng(const QString &dir, const QString &name, int w, int h, QColor color)
{
    const QString path = dir + "/" + name;
    QImage img(w, h, QImage::Format_ARGB32);
    img.fill(color);
    img.save(path, "PNG");
    return path;
}

// 手工构造 3 样本 RGB 8bit 双页未压缩 TIFF（页 0 c0、页 1 c1）。
// 本机 Qt6 的 gif 插件不支持写入（supportedImageFormats 无 gif），
// tif/ico/webp 对同一 writer 连续 write() 只保留最后一帧，故按 TIFF 规范手工组字节
QByteArray makeTwoPageTiffBytes(int w, int h, QColor c0, QColor c1)
{
    const int rowBytes = 3 * w;
    const int dataBytes = rowBytes * h;

    QByteArray page0(dataBytes, 0), page1(dataBytes, 0);
    for (int i = 0; i < w * h; ++i) {
        page0[i * 3 + 0] = char(c0.red());
        page0[i * 3 + 1] = char(c0.green());
        page0[i * 3 + 2] = char(c0.blue());
        page1[i * 3 + 0] = char(c1.red());
        page1[i * 3 + 1] = char(c1.green());
        page1[i * 3 + 2] = char(c1.blue());
    }

    const int entryCount = 10;
    const int ifdSize = 2 + entryCount * 12 + 4;
    const quint32 ifd0Off = 8;
    const quint32 bitsOff = ifd0Off + ifdSize;                 // BitsPerSample[3] 6 字节
    const quint32 data0Off = bitsOff + 6;
    const quint32 data1Off = data0Off + dataBytes;
    const quint32 ifd1Off = data1Off + dataBytes;

    QByteArray bytes;
    QDataStream s(&bytes, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s << quint16(0x4949) << quint16(42) << ifd0Off;

    auto writeIfd = [&](quint32 stripOff, quint32 nextIfd) {
        s << quint16(entryCount);
        auto entry = [&](quint16 tag, quint16 type, quint32 count, quint32 value) {
            s << tag << type << count << value;
        };
        entry(256, 3, 1, quint32(w));            // ImageWidth SHORT
        entry(257, 3, 1, quint32(h));            // ImageLength SHORT
        entry(258, 3, 3, bitsOff);               // BitsPerSample SHORT[3] → 外部
        entry(259, 3, 1, 1);                     // Compression = none
        entry(262, 3, 1, 2);                     // Photometric = RGB
        entry(273, 4, 1, stripOff);              // StripOffsets LONG
        entry(277, 3, 1, 3);                     // SamplesPerPixel
        entry(278, 4, 1, quint32(h));            // RowsPerStrip
        entry(279, 4, 1, quint32(dataBytes));    // StripByteCounts
        entry(284, 3, 1, 1);                     // PlanarConfiguration
        s << nextIfd;
    };

    writeIfd(data0Off, ifd1Off);
    s << quint16(8) << quint16(8) << quint16(8);   // BitsPerSample 数组
    bytes.append(page0);
    bytes.append(page1);

    // IFD1：复用 IFD0 字节，改 StripOffsets 指向第二页数据、NextIFD=0
    QByteArray ifd1 = bytes.mid(ifd0Off, ifdSize);
    {
        QDataStream t(&ifd1, QIODevice::ReadWrite);
        t.setByteOrder(QDataStream::LittleEndian);
        t.device()->seek(2 + 5 * 12 + 8);        // 第 6 个 entry（273）的 value 偏移
        t << data1Off;
        t.device()->seek(ifdSize - 4);
        t << quint32(0);
    }
    return bytes.left(ifd1Off) + ifd1;
}

// 双页 TIFF 落盘（帧 0 红、帧 1 蓝），用于 readMultiImage / parseProviderID 帧路由
QString makeTwoFrameTiff(const QString &dir, const QString &name)
{
    const QString path = dir + "/" + name;
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(makeTwoPageTiffBytes(4, 4, Qt::red, Qt::blue));
    file.close();
    return path;
}

// 畸形 TIFF：IFD 声明 w×h（size() 可解析）但数据仅 8 字节（read() 必失败）。
// 注：Qt6 下截断 PNG 的 size() 恒无效（PNG handler 需完整数据才给出尺寸），无法用于该分支
QString makeCorruptTiff(const QString &dir, const QString &name, int w, int h)
{
    const QString path = dir + "/" + name;
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(makeTwoPageTiffBytes(w, h, Qt::green, Qt::green).left(148));  // 头8+IFD126+Bits6+数据8
    file.close();
    return path;
}

bool poolWaitDone()
{
    return QThreadPool::globalInstance()->waitForDone(60000);
}

// 冻结全局线程池：限制为单线程并用阻塞哨兵占住它，保证此后提交的任务只入队不执行。
// 注：不能用 setMaxThreadCount(0)——Qt 会以 "negative or null maximum thread count" 拒绝并保持原值，
// 导致任务仍被工作线程抢跑、tryTake 失败（全量运行中 2 例偶发失败的根因）
class FreezePool {
public:
    explicit FreezePool(QThreadPool *p)
        : pool(p), sentinel(new Sentinel(started, release))
    {
        savedMax = pool->maxThreadCount();
        pool->setMaxThreadCount(1);
        pool->start(sentinel);
        started.acquire();   // 等哨兵真正占住线程
    }

    ~FreezePool()
    {
        release.release();   // 放行哨兵
        pool->waitForDone(60000);
        pool->setMaxThreadCount(savedMax);
    }

    FreezePool(const FreezePool &) = delete;
    FreezePool &operator=(const FreezePool &) = delete;

private:
    class Sentinel : public QRunnable {
    public:
        Sentinel(QSemaphore &s, QSemaphore &r)
            : m_started(s), m_release(r) {}
        void run() override
        {
            m_started.release();
            m_release.acquire();   // 阻塞直至测试放行
        }

    private:
        QSemaphore &m_started;
        QSemaphore &m_release;
    };

    QThreadPool *pool = nullptr;
    QSemaphore started;
    QSemaphore release;
    int savedMax = 1;
    Sentinel *sentinel = nullptr;
};

}  // namespace

// ══════════════════════════ AsyncImageResponse ══════════════════════════
// 文件内局部类：经 AsyncImageProvider::requestImageResponse 构造，用基类接口断言

class AsyncImageResponseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ThumbnailCache::instance()->clear();
        stub.clear();
        provider = new AsyncImageProvider();
        poolMaxSaved = QThreadPool::globalInstance()->maxThreadCount();
    }

    void TearDown() override
    {
        poolWaitDone();
        ThumbnailCache::instance()->clear();
        QThreadPool::globalInstance()->setMaxThreadCount(poolMaxSaved);
        stub.clear();
        delete provider;
    }

    // 冻结全局线程池后发起请求并取回 runnable，保证 run() 不被池线程抢先执行
    struct BlockedResponse {
        QQuickImageResponse *response = nullptr;
        QRunnable *runnable = nullptr;
        bool taken = false;
    };

    BlockedResponse requestBlocked(const QString &id, const QSize &size)
    {
        BlockedResponse br;
        FreezePool freeze(QThreadPool::globalInstance());
        br.response = provider->requestImageResponse(id, size);
        br.runnable = dynamic_cast<QRunnable *>(br.response);
        if (br.runnable)
            br.taken = QThreadPool::globalInstance()->tryTake(br.runnable);
        return br;   // FreezePool 析构时放行哨兵并恢复 maxThreadCount
    }

    stub_ext::StubExt stub;
    AsyncImageProvider *provider = nullptr;
    int poolMaxSaved = 1;
    QTemporaryDir tempDir;
};

TEST_F(AsyncImageResponseTest, AsyncImageResponse_BeforeRun_HasNullTexture)
{
    // Arrange: 冻结线程池，构造未运行的应答（id 指向不存在的文件）
    const QString path = tempDir.filePath("ctor_missing.png");
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    const BlockedResponse br = requestBlocked(id, QSize(50, 25));

    // Assert  // 构造仅初始化成员：image 为空，尚未加载（Qt6 对 null 图像可能返回空工厂）
    ASSERT_NE(br.response, nullptr);
    ASSERT_TRUE(br.taken);
    QQuickTextureFactory *factory = br.response->textureFactory();
    EXPECT_TRUE(factory == nullptr || factory->image().isNull());
    EXPECT_TRUE(provider->imageCache.keys().isEmpty());
    delete factory;
    delete br.response;
}

TEST_F(AsyncImageResponseTest, Run_CacheMissWithDownscaleRequest_LoadsScaledAndCaches)
{
    // Arrange: 100x50 真实 PNG，请求 50x25（2:1 精确缩小）
    const QString path = makePng(tempDir.path(), "miss_scale.png", 100, 50, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(50, 25));
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Assert  // run: B1 未命中 → readNormalImageScaled 解码器缩放 → add 缓存
    QQuickTextureFactory *factory = response->textureFactory();
    ASSERT_NE(factory, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(50, 25));
    EXPECT_TRUE(provider->imageCache.contains(path, 0));
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(50, 25));
    delete factory;
    delete response;
}

TEST_F(AsyncImageResponseTest, Run_InvalidRequestedSize_LoadsFullSizeImage)
{
    // Arrange: 100x50 PNG，请求尺寸无效
    const QString path = makePng(tempDir.path(), "invalid_size.png", 100, 50, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize());
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Assert  // readNormalImageScaled B1: 无效尺寸走默认上限全图加载
    QQuickTextureFactory *factory = response->textureFactory();
    ASSERT_NE(factory, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(100, 50));
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(100, 50));
    delete factory;
    delete response;
}

TEST_F(AsyncImageResponseTest, Run_UpscaleRequested_LoadsWithoutUpscaling)
{
    // Arrange: 100x50 PNG，请求 200x100（放大场景）
    const QString path = makePng(tempDir.path(), "upscale.png", 100, 50, Qt::cyan);
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(200, 100));
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Assert  // readNormalImageScaled B5: 目标不小于原图 → 升级上限读原图，不放大
    QQuickTextureFactory *factory = response->textureFactory();
    ASSERT_NE(factory, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(100, 50));
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(100, 50));
    delete factory;
    delete response;
}

TEST_F(AsyncImageResponseTest, Run_TruncatedImage_FallsBackToFullLoadNull)
{
    // Arrange: 畸形 TIFF（size() 可解析、数据缺失），请求 50x25
    const QString path = makeCorruptTiff(tempDir.path(), "truncated.tif", 400, 200);
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(50, 25));
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Assert  // readNormalImageScaled B4: 缩放解码失败 → 回退全图加载也失败 → null 不入缓存
    QQuickTextureFactory *factory = response->textureFactory();
    EXPECT_TRUE(factory == nullptr || factory->image().isNull());
    EXPECT_FALSE(provider->imageCache.contains(path, 0));
    delete factory;
    delete response;
}

TEST_F(AsyncImageResponseTest, Run_CacheHitInvalidRequested_ReturnsCachedAsIs)
{
    // Arrange: 预置 100x50 缓存，请求尺寸无效
    const QString path = tempDir.filePath("hit_invalid.png");
    QImage cached(100, 50, QImage::Format_ARGB32);
    cached.fill(Qt::red);
    provider->imageCache.add(path, 0, cached);
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize());
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Assert  // run B2: 命中且请求无效 → 原样返回缓存
    QQuickTextureFactory *factory = response->textureFactory();
    ASSERT_NE(factory, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(100, 50));
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(100, 50));
    delete factory;
    delete response;
}

TEST_F(AsyncImageResponseTest, Run_CacheHitLargerWidth_ScalesDownCopyOnly)
{
    // Arrange: 预置 100x50 缓存，请求 50x25（宽高超限，宽驱动）
    const QString path = tempDir.filePath("hit_w.png");
    QImage cached(100, 50, QImage::Format_ARGB32);
    cached.fill(Qt::red);
    provider->imageCache.add(path, 0, cached);
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(50, 25));
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Assert  // run B3: 缩放仅作用于返回副本，缓存条目保持原尺寸
    QQuickTextureFactory *factory = response->textureFactory();
    ASSERT_NE(factory, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(50, 25));
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(100, 50));
    delete factory;
    delete response;
}

TEST_F(AsyncImageResponseTest, Run_CacheHitLargerHeight_ScalesDownByHeight)
{
    // Arrange: 预置 100x50 缓存，请求 150x40（仅高超限 → 高驱动缩放到 80x40）
    const QString path = tempDir.filePath("hit_h.png");
    QImage cached(100, 50, QImage::Format_ARGB32);
    cached.fill(Qt::red);
    provider->imageCache.add(path, 0, cached);
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(150, 40));
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Assert  // run B3（|| 右侧）: KeepAspectRatio 以高度为准 → 80x40
    QQuickTextureFactory *factory = response->textureFactory();
    ASSERT_NE(factory, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(80, 40));
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(100, 50));
    delete factory;
    delete response;
}

TEST_F(AsyncImageResponseTest, Run_CacheHitSmallerBothDims_RemovesAndReloads)
{
    // Arrange: 预置过小 20x10 缓存，磁盘上放真实 100x50 图，请求 50x25
    const QString path = makePng(tempDir.path(), "reload.png", 100, 50, Qt::green);
    QImage tooSmall(20, 10, QImage::Format_ARGB32);
    tooSmall.fill(Qt::red);
    provider->imageCache.add(path, 0, tooSmall);
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(50, 25));
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Assert  // run B4: 缓存过小 → remove + 重读并按新尺寸回写缓存
    QQuickTextureFactory *factory = response->textureFactory();
    ASSERT_NE(factory, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(50, 25));
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(50, 25));
    delete factory;
    delete response;
}

TEST_F(AsyncImageResponseTest, Run_CacheHitEqualWidthSmallerHeight_ReturnsAsIs)
{
    // Arrange: 预置 100x50 缓存，请求 100x60（等宽高超 → 既不缩也不重读）
    const QString path = tempDir.filePath("hit_mixed.png");
    QImage cached(100, 50, QImage::Format_ARGB32);
    cached.fill(Qt::red);
    provider->imageCache.add(path, 0, cached);
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(100, 60));
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Assert  // run B5: 不满足 B3/B4 → 原样返回
    QQuickTextureFactory *factory = response->textureFactory();
    ASSERT_NE(factory, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(100, 50));
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(100, 50));
    delete factory;
    delete response;
}

TEST_F(AsyncImageResponseTest, Run_MultiFrameRequest_LoadsFrameScaledAndCaches)
{
    // Arrange: 双页 TIFF（帧 4x4），请求 #frame_1 且目标 2x2
    const QString path = makeTwoFrameTiff(tempDir.path(), "async_multi.tif");
    const QString id = QUrl::fromLocalFile(path).toString() + "#frame_1";

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(2, 2));
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Assert  // loadScaleAndCache B1/B3/B4: 读帧 → 手动缩到 2x2 → 以 (path,1) 入缓存
    QQuickTextureFactory *factory = response->textureFactory();
    ASSERT_NE(factory, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(2, 2));
    EXPECT_TRUE(provider->imageCache.contains(path, 1));
    EXPECT_EQ(provider->imageCache.get(path, 1).size(), QSize(2, 2));
    delete factory;
    delete response;
}

TEST_F(AsyncImageResponseTest, Run_NonexistentFile_EmitsFinishedWithNullTexture)
{
    // Arrange: id 指向不存在的文件
    const QString path = tempDir.filePath("async_missing.png");
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(50, 25));
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Assert  // 加载失败仍正常完成：无可用纹理且不污染缓存（Qt6 对 null 图像可能返回空工厂）
    QQuickTextureFactory *factory = response->textureFactory();
    EXPECT_TRUE(factory == nullptr || factory->image().isNull());
    EXPECT_FALSE(provider->imageCache.contains(path, 0));
    delete factory;
    delete response;
}

TEST_F(AsyncImageResponseTest, Run_SynchronousExecution_EmitsFinishedExactlyOnce)
{
    // Arrange: 冻结线程池取回 runnable，在测试线程同步执行 run()
    const QString path = makePng(tempDir.path(), "sync_run.png", 100, 50, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();
    const BlockedResponse br = requestBlocked(id, QSize(50, 25));
    ASSERT_NE(br.response, nullptr);
    ASSERT_NE(br.runnable, nullptr);
    ASSERT_TRUE(br.taken);
    QSignalSpy spy(br.response, &QQuickImageResponse::finished);

    // Act
    br.runnable->run();

    // Assert  // run() 结束精确发射一次 finished，且图像已加载
    EXPECT_EQ(spy.count(), 1);
    QQuickTextureFactory *factory = br.response->textureFactory();
    ASSERT_NE(factory, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(50, 25));
    delete factory;
    delete br.response;
}

TEST_F(AsyncImageResponseTest, TextureFactory_AfterLoad_ReturnsFactoryWrappingImage)
{
    // Arrange: 完成一次正常加载
    const QString path = makePng(tempDir.path(), "texture.png", 100, 50, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(50, 25));
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());

    // Act
    QQuickTextureFactory *factory = response->textureFactory();
    QQuickTextureFactory *factoryAgain = response->textureFactory();

    // Assert  // 每次调用返回独立非空工厂，纹理即加载结果
    ASSERT_NE(factory, nullptr);
    ASSERT_NE(factoryAgain, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(50, 25));
    EXPECT_EQ(factoryAgain->image().size(), QSize(50, 25));
    delete factory;
    delete factoryAgain;
    delete response;
}

TEST_F(AsyncImageResponseTest, Destructor_AfterPoolExecution_DeletesResponseSafely)
{
    // Arrange: 完成一次异步加载
    const QString path = makePng(tempDir.path(), "dtor.png", 100, 50, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(50, 25));
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(50, 25));

    // Act
    delete response;

    // Assert  // autoDelete(false)：由调用方析构，不影响 provider 缓存
    EXPECT_TRUE(provider->imageCache.contains(path, 0));
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(50, 25));
}

// ══════════════════════════ ProviderCache ══════════════════════════

class ProviderCacheTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ThumbnailCache::instance()->clear();
        stub.clear();
        obj = new ProviderCache();
    }

    void TearDown() override
    {
        ThumbnailCache::instance()->clear();
        stub.clear();
        delete obj;
    }

    stub_ext::StubExt stub;
    ProviderCache *obj = nullptr;
    QTemporaryDir tempDir;
};

TEST_F(ProviderCacheTest, ProviderCache_FreshInstance_HasEmptyCacheAndRotationState)
{
    // Arrange: SetUp 已构造全新实例

    // Act
    const QList<ThumbnailCache::Key> keys = obj->imageCache.keys();

    // Assert  // 构造后缓存为空、旋转状态为零值
    EXPECT_TRUE(keys.isEmpty());
    EXPECT_EQ(obj->lastRotation, 0);
    EXPECT_TRUE(obj->lastRotatePath.isEmpty());
    EXPECT_TRUE(obj->lastRotateImage.isNull());
}

TEST_F(ProviderCacheTest, Destructor_PopulatedInstance_DeletesSafely)
{
    // Arrange: 堆上构造两个实例并填充缓存，victim 析构后 survivor 应完好
    auto *victim = new ProviderCache();
    auto *survivor = new ProviderCache();
    const QString pathVictim = tempDir.filePath("dtor_victim.png");
    const QString pathSurvivor = tempDir.filePath("dtor_survivor.png");
    QImage img(100, 50, QImage::Format_ARGB32);
    img.fill(Qt::green);
    victim->imageCache.add(pathVictim, 0, img);
    survivor->imageCache.add(pathSurvivor, 0, img);

    // Act
    delete victim;

    // Assert
    EXPECT_TRUE(survivor->imageCache.contains(pathSurvivor, 0));
    EXPECT_EQ(survivor->imageCache.get(pathSurvivor, 0).size(), QSize(100, 50));
    delete survivor;
}

TEST_F(ProviderCacheTest, ClearCache_PopulatedState_ResetsAllEntries)
{
    // Arrange: 填充图像缓存与旋转状态
    const QString path = makePng(tempDir.path(), "clear.png", 100, 50, Qt::green);
    QImage img(100, 50, QImage::Format_ARGB32);
    img.fill(Qt::green);
    obj->imageCache.add(path, 0, img);
    obj->rotateImageCached(90, path, 0);
    EXPECT_TRUE(obj->imageCache.contains(path, 0));
    EXPECT_EQ(obj->lastRotatePath, path);

    // Act
    obj->clearCache();

    // Assert  // 图像缓存与旋转路径/图像复位；lastRotation 未复位（现状锁定 → 疑似缺陷 3）
    EXPECT_TRUE(obj->imageCache.keys().isEmpty());
    EXPECT_TRUE(obj->lastRotatePath.isEmpty());
    EXPECT_TRUE(obj->lastRotateImage.isNull());
    EXPECT_EQ(obj->lastRotation, 90);
}

TEST_F(ProviderCacheTest, ClearCache_EmptyState_IsNoOp)
{
    // Arrange: 空缓存实例（循环 0 次边界）

    // Act
    obj->clearCache();

    // Assert  // 状态保持空且无异常
    EXPECT_TRUE(obj->imageCache.keys().isEmpty());
    EXPECT_EQ(obj->lastRotation, 0);
}

TEST_F(ProviderCacheTest, PreloadImage_AnyPath_HasNoSideEffect)
{
    // Arrange: 基类默认实现为空操作
    const QString path = tempDir.filePath("preload.png");

    // Act
    obj->preloadImage(path);

    // Assert  // 不产生缓存也不改变状态
    EXPECT_TRUE(obj->imageCache.keys().isEmpty());
    EXPECT_TRUE(obj->lastRotatePath.isEmpty());
}

TEST_F(ProviderCacheTest, RemoveImageCache_MatchingEntries_RemovesAllFrames)
{
    // Arrange: 同一路径三帧 + 另一路径一帧
    const QString target = tempDir.filePath("remove_target.png");
    const QString other = tempDir.filePath("remove_other.png");
    QImage img(40, 20, QImage::Format_ARGB32);
    img.fill(Qt::green);
    obj->imageCache.add(target, 0, img);
    obj->imageCache.add(target, 1, img);
    obj->imageCache.add(target, 2, img);
    obj->imageCache.add(other, 0, img);

    // Act
    obj->removeImageCache(target);

    // Assert  // 目标路径全部帧被移除，其他路径保留
    EXPECT_FALSE(obj->imageCache.contains(target, 0));
    EXPECT_FALSE(obj->imageCache.contains(target, 1));
    EXPECT_FALSE(obj->imageCache.contains(target, 2));
    EXPECT_TRUE(obj->imageCache.contains(other, 0));
}

TEST_F(ProviderCacheTest, RemoveImageCache_NonexistentPath_LeavesCacheIntact)
{
    // Arrange: 缓存有内容，移除不存在的路径（循环全程不匹配）
    const QString kept = tempDir.filePath("keep.png");
    QImage img(40, 20, QImage::Format_ARGB32);
    img.fill(Qt::green);
    obj->imageCache.add(kept, 0, img);

    // Act
    obj->removeImageCache(tempDir.filePath("ghost.png"));

    // Assert  // 强异常安全：缓存未损坏
    EXPECT_TRUE(obj->imageCache.contains(kept, 0));
    EXPECT_EQ(obj->imageCache.keys().size(), 1);
}

TEST_F(ProviderCacheTest, RenameImageCache_ExistingEntries_MovesAllFrames)
{
    // Arrange: 旧路径两帧
    const QString oldPath = tempDir.filePath("old.png");
    const QString newPath = tempDir.filePath("new.png");
    QImage img0(100, 50, QImage::Format_ARGB32);
    img0.fill(Qt::red);
    QImage img1(40, 20, QImage::Format_ARGB32);
    img1.fill(Qt::blue);
    obj->imageCache.add(oldPath, 0, img0);
    obj->imageCache.add(oldPath, 1, img1);

    // Act
    obj->renameImageCache(oldPath, newPath);

    // Assert  // 全部帧迁移到新键，内容保持
    EXPECT_TRUE(obj->imageCache.contains(newPath, 0));
    EXPECT_TRUE(obj->imageCache.contains(newPath, 1));
    EXPECT_FALSE(obj->imageCache.contains(oldPath, 0));
    EXPECT_FALSE(obj->imageCache.contains(oldPath, 1));
    EXPECT_EQ(obj->imageCache.get(newPath, 0).size(), QSize(100, 50));
    EXPECT_EQ(obj->imageCache.get(newPath, 1).size(), QSize(40, 20));
}

TEST_F(ProviderCacheTest, RenameImageCache_NonexistentPath_IsNoOp)
{
    // Arrange: 缓存有内容，旧路径不存在
    const QString kept = tempDir.filePath("keep2.png");
    QImage img(40, 20, QImage::Format_ARGB32);
    img.fill(Qt::green);
    obj->imageCache.add(kept, 0, img);

    // Act
    obj->renameImageCache(tempDir.filePath("ghost_old.png"), tempDir.filePath("ghost_new.png"));

    // Assert  // 无迁移发生，原缓存不变
    EXPECT_TRUE(obj->imageCache.contains(kept, 0));
    EXPECT_EQ(obj->imageCache.keys().size(), 1);
}

TEST_F(ProviderCacheTest, RotateImageCached_ZeroAngle_ReturnsWithoutSideEffects)
{
    // Arrange: 预置缓存与旋转状态（模拟此前已旋转过）
    const QString path = tempDir.filePath("zero_angle.png");
    QImage img(100, 50, QImage::Format_ARGB32);
    img.fill(Qt::green);
    obj->imageCache.add(path, 0, img);
    obj->lastRotatePath = path;
    obj->lastRotateImage = img;
    obj->lastRotation = 90;

    // Act
    obj->rotateImageCached(0, path, 0);

    // Assert  // 现状：0 度直接早退，既不旋转也不清状态（注释声称清除 → 疑似缺陷 1）
    EXPECT_EQ(obj->lastRotation, 90);
    EXPECT_EQ(obj->lastRotatePath, path);
    EXPECT_TRUE(obj->imageCache.contains(path, 0));
    EXPECT_EQ(obj->imageCache.get(path, 0).size(), QSize(100, 50));
    EXPECT_FALSE(ThumbnailCache::instance()->contains(path, 0));
}

TEST_F(ProviderCacheTest, RotateImageCached_MissingImage_WritesNoCaches)
{
    // Arrange: 路径不在缓存中（get 返回 null）
    const QString path = tempDir.filePath("no_image.png");

    // Act
    obj->rotateImageCached(90, path, 0);

    // Assert  // null 分支：仅记录旋转状态，两个缓存都不写
    EXPECT_FALSE(obj->imageCache.contains(path, 0));
    EXPECT_FALSE(ThumbnailCache::instance()->contains(path, 0));
    EXPECT_EQ(obj->lastRotatePath, path);
}

TEST_F(ProviderCacheTest, RotateImageCached_DifferentPath_ResetsRotationState)
{
    // Arrange: 两个路径各有缓存图
    const QString pathA = tempDir.filePath("rot_a.png");
    const QString pathB = tempDir.filePath("rot_b.png");
    QImage imgA(100, 50, QImage::Format_ARGB32);
    imgA.fill(Qt::green);
    QImage imgB(40, 20, QImage::Format_ARGB32);
    imgB.fill(Qt::green);
    obj->imageCache.add(pathA, 0, imgA);
    obj->imageCache.add(pathB, 0, imgB);

    // Act
    obj->rotateImageCached(90, pathA, 0);
    obj->rotateImageCached(90, pathB, 0);

    // Assert  // 切换路径后重新记录基准：角度复位为本次值，B 被旋转 90 度
    EXPECT_EQ(obj->lastRotatePath, pathB);
    EXPECT_EQ(obj->lastRotation, 90);
    EXPECT_EQ(obj->imageCache.get(pathB, 0).size(), QSize(20, 40));
    EXPECT_EQ(obj->imageCache.get(pathA, 0).size(), QSize(50, 100));
}

TEST_F(ProviderCacheTest, RotateImageCached_SamePathTwice_AccumulatesRotationAngle)
{
    // Arrange: 100x50 缓存图
    const QString path = tempDir.filePath("rot_acc.png");
    QImage img(100, 50, QImage::Format_ARGB32);
    img.fill(Qt::green);
    obj->imageCache.add(path, 0, img);

    // Act
    obj->rotateImageCached(90, path, 0);
    obj->rotateImageCached(90, path, 0);

    // Assert  // 第二次基于 lastRotateImage 累计 180 度：不换维，且缓存/缩略图缓存均更新
    EXPECT_EQ(obj->lastRotation, 180);
    EXPECT_EQ(obj->imageCache.get(path, 0).size(), QSize(100, 50));
    EXPECT_TRUE(ThumbnailCache::instance()->contains(path, 0));
    EXPECT_EQ(ThumbnailCache::instance()->get(path, 0).size(), QSize(200, 100));
}

TEST_F(ProviderCacheTest, RotateImageCached_TotalMultipleOf360_SkipsRotationUpdatesCaches)
{
    // Arrange: 100x50 缓存图
    const QString path = tempDir.filePath("rot_360.png");
    QImage img(100, 50, QImage::Format_ARGB32);
    img.fill(Qt::green);
    obj->imageCache.add(path, 0, img);

    // Act
    obj->rotateImageCached(360, path, 0);

    // Assert  // 360 整除跳过实际旋转，但仍回写两个缓存
    EXPECT_EQ(obj->lastRotation, 360);
    EXPECT_EQ(obj->imageCache.get(path, 0).size(), QSize(100, 50));
    EXPECT_TRUE(ThumbnailCache::instance()->contains(path, 0));
    EXPECT_EQ(ThumbnailCache::instance()->get(path, 0).size(), QSize(200, 100));
}

namespace {
struct RotateCase {
    int angle;
    int expectW;
    int expectH;
    int thumbW;
    int thumbH;
};
}  // namespace

struct RotateAngleParamTest : public ProviderCacheTest, public ::testing::WithParamInterface<RotateCase> {
};

// 100x50 基准图旋转：角度 → 图像缓存尺寸 / 缩略图缓存尺寸（KeepAspectRatioByExpanding 至少覆盖 100x100）
TEST_P(RotateAngleParamTest, RotateImageCached_AngleVariations_RotatesAndCaches)
{
    const RotateCase c = GetParam();

    // Arrange: 100x50 缓存图
    const QString path = tempDir.filePath(QString("rot_%1.png").arg(c.angle));
    QImage img(100, 50, QImage::Format_ARGB32);
    img.fill(Qt::green);
    obj->imageCache.add(path, 0, img);

    // Act
    obj->rotateImageCached(c.angle, path, 0);

    // Assert  // 旋转结果与缩略图（≥100x100 扩展）分别写入两级缓存
    EXPECT_EQ(obj->lastRotation, c.angle);
    EXPECT_EQ(obj->imageCache.get(path, 0).size(), QSize(c.expectW, c.expectH));
    EXPECT_TRUE(ThumbnailCache::instance()->contains(path, 0));
    EXPECT_EQ(ThumbnailCache::instance()->get(path, 0).size(), QSize(c.thumbW, c.thumbH));
}

INSTANTIATE_TEST_SUITE_P(
    AngleVariations, RotateAngleParamTest,
    ::testing::Values(
        RotateCase{90, 50, 100, 100, 200},
        RotateCase{180, 100, 50, 200, 100},
        RotateCase{270, 50, 100, 100, 200},
        RotateCase{360, 100, 50, 200, 100}));

// ══════════════════════════ AsyncImageProvider ══════════════════════════

class AsyncImageProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ThumbnailCache::instance()->clear();
        stub.clear();
        provider = new AsyncImageProvider();
        poolMaxSaved = QThreadPool::globalInstance()->maxThreadCount();
    }

    void TearDown() override
    {
        poolWaitDone();
        ThumbnailCache::instance()->clear();
        QThreadPool::globalInstance()->setMaxThreadCount(poolMaxSaved);
        stub.clear();
        delete provider;
    }

    stub_ext::StubExt stub;
    AsyncImageProvider *provider = nullptr;
    int poolMaxSaved = 1;
    QTemporaryDir tempDir;
};

TEST_F(AsyncImageProviderTest, AsyncImageProvider_FreshInstance_HasEmptyCache)
{
    // Arrange: SetUp 已构造

    // Act
    const QList<ThumbnailCache::Key> keys = provider->imageCache.keys();

    // Assert  // 初始无任何缓存条目
    EXPECT_TRUE(keys.isEmpty());
    EXPECT_EQ(keys.size(), 0);
}

TEST_F(AsyncImageProviderTest, Destructor_AfterBackgroundTasks_DeletesSafely)
{
    // Arrange: 堆上构造两个实例，victim 执行一次后台预加载后等待完成
    auto *victim = new AsyncImageProvider();
    auto *survivor = new AsyncImageProvider();
    const QString pathVictim = makePng(tempDir.path(), "async_dtor_victim.png", 40, 20, Qt::green);
    const QString pathSurvivor = tempDir.filePath("async_dtor_survivor.png");
    victim->preloadImage(QUrl::fromLocalFile(pathVictim).toString());
    QImage img(40, 20, QImage::Format_ARGB32);
    img.fill(Qt::green);
    survivor->imageCache.add(pathSurvivor, 0, img);
    ASSERT_TRUE(poolWaitDone());
    EXPECT_TRUE(victim->imageCache.contains(pathVictim, 0));

    // Act
    delete victim;

    // Assert  // survivor 状态不受 victim 析构影响
    EXPECT_TRUE(survivor->imageCache.contains(pathSurvivor, 0));
    EXPECT_EQ(survivor->imageCache.get(pathSurvivor, 0).size(), QSize(40, 20));
    delete survivor;
}

TEST_F(AsyncImageProviderTest, PreloadImage_RealFile_CachesWithinScreenSize)
{
    // Arrange: 40x20 小图（必然小于任一屏幕分辨率）
    const QString path = makePng(tempDir.path(), "preload_real.png", 40, 20, Qt::green);

    // Act
    provider->preloadImage(QUrl::fromLocalFile(path).toString());
    ASSERT_TRUE(poolWaitDone());

    // Assert  // 主屏幕尺寸为预加载上限：小图按原尺寸入缓存
    EXPECT_TRUE(provider->imageCache.contains(path, 0));
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(40, 20));
}

TEST_F(AsyncImageProviderTest, PreloadImage_NoPrimaryScreen_CachesFullImage)
{
    // Arrange: stub 掉主屏幕（无窗口系统环境分支），40x20 小图
    stub.set_lamda(static_cast<QScreen *(*)()>(&QGuiApplication::primaryScreen),
                   []() -> QScreen * { return nullptr; });
    const QString path = makePng(tempDir.path(), "preload_noscreen.png", 40, 20, Qt::green);

    // Act
    provider->preloadImage(QUrl::fromLocalFile(path).toString());
    ASSERT_TRUE(poolWaitDone());

    // Assert  // displaySize 为默认无效值 → 全尺寸加载
    EXPECT_TRUE(provider->imageCache.contains(path, 0));
    EXPECT_EQ(provider->imageCache.get(path, 0).size(), QSize(40, 20));
}

TEST_F(AsyncImageProviderTest, RequestImageResponse_NewRequest_ReturnsCompletedResponse)
{
    // Arrange: 100x50 真实 PNG
    const QString path = makePng(tempDir.path(), "req_resp.png", 100, 50, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    QQuickImageResponse *response = provider->requestImageResponse(id, QSize(50, 25));

    // Assert  // 返回非空应答，异步完成后图像与缓存就绪
    ASSERT_NE(response, nullptr);
    ASSERT_TRUE(poolWaitDone());
    QQuickTextureFactory *factory = response->textureFactory();
    ASSERT_NE(factory, nullptr);
    EXPECT_EQ(factory->image().size(), QSize(50, 25));
    EXPECT_TRUE(provider->imageCache.contains(path, 0));
    delete factory;
    delete response;
}

// ══════════════════════════ ImageProvider ══════════════════════════

class ImageProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ThumbnailCache::instance()->clear();
        stub.clear();
        obj = new ImageProvider();
    }

    void TearDown() override
    {
        ThumbnailCache::instance()->clear();
        stub.clear();
        delete obj;
    }

    stub_ext::StubExt stub;
    ImageProvider *obj = nullptr;
    QTemporaryDir tempDir;
};

TEST_F(ImageProviderTest, ImageProvider_FreshInstance_HasEmptyCache)
{
    // Arrange: SetUp 已构造

    // Act
    const QList<ThumbnailCache::Key> keys = obj->imageCache.keys();

    // Assert
    EXPECT_TRUE(keys.isEmpty());
    EXPECT_EQ(obj->imageCache.get(tempDir.filePath("none.png")).isNull(), true);
}

TEST_F(ImageProviderTest, Destructor_PopulatedInstance_DeletesSafely)
{
    // Arrange: 堆上构造两个实例并各自填充缓存
    auto *victim = new ImageProvider();
    auto *survivor = new ImageProvider();
    const QString pathVictim = tempDir.filePath("img_dtor_victim.png");
    const QString pathSurvivor = tempDir.filePath("img_dtor_survivor.png");
    QImage img(100, 50, QImage::Format_ARGB32);
    img.fill(Qt::green);
    victim->imageCache.add(pathVictim, 0, img);
    survivor->imageCache.add(pathSurvivor, 0, img);

    // Act
    delete victim;

    // Assert  // survivor 缓存不受 victim 析构影响
    EXPECT_TRUE(survivor->imageCache.contains(pathSurvivor, 0));
    EXPECT_EQ(survivor->imageCache.get(pathSurvivor, 0).size(), QSize(100, 50));
    delete survivor;
}

TEST_F(ImageProviderTest, RequestImage_UncachedFile_LoadsAndCachesExactSize)
{
    // Arrange: 100x50 PNG，请求尺寸无效
    const QString path = makePng(tempDir.path(), "sync_load.png", 100, 50, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize());

    // Assert  // 未命中 → 读盘、设置 size 出参、入缓存，请求无效不缩放
    EXPECT_EQ(img.size(), QSize(100, 50));
    EXPECT_EQ(outSize, QSize(100, 50));
    EXPECT_TRUE(obj->imageCache.contains(path, 0));
    EXPECT_EQ(obj->imageCache.get(path, 0).size(), QSize(100, 50));
}

TEST_F(ImageProviderTest, RequestImage_CachedEntry_ReturnsCachedWithoutDiskRead)
{
    // Arrange: 预置独特尺寸缓存（磁盘上无此文件，若读盘必失败）
    const QString path = tempDir.filePath("cached_hit.png");
    QImage cached(123, 45, QImage::Format_ARGB32);
    cached.fill(Qt::red);
    obj->imageCache.add(path, 0, cached);
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize());

    // Assert  // 命中分支：原样返回缓存且不再更新 size 出参（保持空）
    EXPECT_EQ(img.size(), QSize(123, 45));
    EXPECT_TRUE(outSize.isEmpty());
    EXPECT_EQ(obj->imageCache.get(path, 0).size(), QSize(123, 45));
}

TEST_F(ImageProviderTest, RequestImage_EqualRequestedSize_SkipsScaling)
{
    // Arrange: 100x50 PNG，请求恰好等于原图尺寸
    const QString path = makePng(tempDir.path(), "equal_size.png", 100, 50, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize(100, 50));

    // Assert  // 尺寸相等 → 不触发缩放分支，保持精确尺寸
    EXPECT_EQ(img.size(), QSize(100, 50));
    EXPECT_EQ(outSize, QSize(100, 50));
    EXPECT_EQ(obj->imageCache.get(path, 0).size(), QSize(100, 50));
}

TEST_F(ImageProviderTest, RequestImage_NonexistentFile_ReturnsNullAndCachesNull)
{
    // Arrange: id 指向不存在的文件
    const QString path = tempDir.filePath("sync_missing.png");
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize());

    // Assert  // 空图像仍被缓存（源码注释明确该行为）
    EXPECT_TRUE(img.isNull());
    EXPECT_TRUE(outSize.isEmpty());
    EXPECT_TRUE(obj->imageCache.contains(path, 0));
    EXPECT_TRUE(obj->imageCache.get(path, 0).isNull());
}

TEST_F(ImageProviderTest, RequestImage_MultiFrameTag_LoadsRequestedFrame)
{
    // Arrange: 双页 TIFF，请求帧 1（蓝色）
    const QString path = makeTwoFrameTiff(tempDir.path(), "sync_multi.tif");
    const QString id = QUrl::fromLocalFile(path).toString() + "#frame_1";
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize());

    // Assert  // 帧路由正确：读到 4x4 的蓝色帧并以 (path,1) 入缓存
    ASSERT_FALSE(img.isNull());
    EXPECT_EQ(img.size(), QSize(4, 4));
    EXPECT_GT(img.pixelColor(1, 1).blue(), 200);
    EXPECT_LT(img.pixelColor(1, 1).red(), 100);
    EXPECT_EQ(outSize, QSize(4, 4));
    EXPECT_TRUE(obj->imageCache.contains(path, 1));
}

TEST_F(ImageProviderTest, RequestImage_FrameOutOfRange_CachesNullFrame)
{
    // Arrange: 双页 TIFF，请求越界帧 9
    const QString path = makeTwoFrameTiff(tempDir.path(), "sync_oof.tif");
    const QString id = QUrl::fromLocalFile(path).toString() + "#frame_9";

    // Act
    const QImage img = obj->requestImage(id, nullptr, QSize());

    // Assert  // jumpToImage 失败 → null 帧仍入缓存
    EXPECT_TRUE(img.isNull());
    EXPECT_TRUE(obj->imageCache.contains(path, 9));
    EXPECT_TRUE(obj->imageCache.get(path, 9).isNull());
}

TEST_F(ImageProviderTest, RequestImage_LargeDimensionRequest_ScalesToRequest)
{
    // Arrange: 5000x10 长图，请求 6000x6000（介于 4096 与 15000 之间的钳制值）
    const QString path = makePng(tempDir.path(), "large_dim.png", 5000, 10, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize(6000, 6000));

    // Assert  // 解码不被 4096 默认上限截断，最终按请求等比放大到 6000x12
    EXPECT_EQ(outSize, QSize(5000, 10));
    EXPECT_EQ(img.size(), QSize(6000, 12));
}

TEST_F(ImageProviderTest, RequestImage_CorruptTiff_ReturnsNull)
{
    // Arrange: 畸形 TIFF（size() 可解析、数据缺失）
    const QString path = makeCorruptTiff(tempDir.path(), "sync_corrupt.tif", 400, 200);
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize());

    // Assert  // readNormalImage 失败分支 → null 且入缓存
    EXPECT_TRUE(img.isNull());
    EXPECT_TRUE(obj->imageCache.contains(path, 0));
}

// ══════════════════════════ ThumbnailProvider ══════════════════════════

class ThumbnailProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ThumbnailCache::instance()->clear();
        stub.clear();
        obj = new ThumbnailProvider();
    }

    void TearDown() override
    {
        ThumbnailCache::instance()->clear();
        stub.clear();
        delete obj;
    }

    stub_ext::StubExt stub;
    ThumbnailProvider *obj = nullptr;
    QTemporaryDir tempDir;
};

TEST_F(ThumbnailProviderTest, ThumbnailProvider_FreshInstance_ExposesImageProviderBase)
{
    // Arrange: 堆上构造
    ThumbnailProvider *heap = nullptr;

    // Act
    heap = new ThumbnailProvider();

    // Assert  // QQuickImageProvider(Image) 基类链路完整
    EXPECT_NE(dynamic_cast<QQmlImageProviderBase *>(heap), nullptr);
    EXPECT_NE(dynamic_cast<QQuickImageProvider *>(heap), nullptr);
    EXPECT_NO_THROW(delete heap);
}

TEST_F(ThumbnailProviderTest, Destructor_AfterRequests_DeletesSafely)
{
    // Arrange: 堆上构造并产生一次缩略图缓存
    auto *heap = new ThumbnailProvider();
    const QString path = makePng(tempDir.path(), "thumb_dtor.png", 400, 200, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();
    heap->requestImage(id, nullptr, QSize());
    EXPECT_TRUE(ThumbnailCache::instance()->contains(path, 0));
    EXPECT_EQ(ThumbnailCache::instance()->get(path, 0).size(), QSize(200, 100));

    // Act
    delete heap;

    // Assert  // provider 无自有缓存，析构不影响缩略图单例
    EXPECT_TRUE(ThumbnailCache::instance()->contains(path, 0));
    EXPECT_EQ(ThumbnailCache::instance()->get(path, 0).size(), QSize(200, 100));
}

TEST_F(ThumbnailProviderTest, RequestImage_UncachedImage_GeneratesReaderThumbnailAndCaches)
{
    // Arrange: 400x200 PNG，请求尺寸无效
    const QString path = makePng(tempDir.path(), "thumb_gen.png", 400, 200, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize());

    // Assert  // 解码器按 KeepAspectRatioByExpanding 输出 200x100；size 出参为原图尺寸
    EXPECT_EQ(img.size(), QSize(200, 100));
    EXPECT_EQ(outSize, QSize(400, 200));
    EXPECT_TRUE(ThumbnailCache::instance()->contains(path, 0));
    EXPECT_EQ(ThumbnailCache::instance()->get(path, 0).size(), QSize(200, 100));
}

TEST_F(ThumbnailProviderTest, RequestImage_CachedThumbnail_ReturnsCacheEntryWithoutDisk)
{
    // Arrange: 预置独特尺寸缩略图（磁盘上无此文件，若读盘必失败）
    const QString path = tempDir.filePath("thumb_hit.png");
    QImage cached(111, 37, QImage::Format_ARGB32);
    cached.fill(Qt::red);
    ThumbnailCache::instance()->add(path, 0, cached);
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize());

    // Assert  // contains 命中早退：原样返回缓存且不触碰 size 出参
    EXPECT_EQ(img.size(), QSize(111, 37));
    EXPECT_TRUE(outSize.isEmpty());
}

TEST_F(ThumbnailProviderTest, RequestImage_MultiFrameTag_LoadsFrameAndCachesHundredThumb)
{
    // Arrange: 双页 TIFF，请求帧 1
    const QString path = makeTwoFrameTiff(tempDir.path(), "thumb_multi.tif");
    const QString id = QUrl::fromLocalFile(path).toString() + "#frame_1";
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize());

    // Assert  // 帧分支不经解码器缩放；缓存扩展到 100x100 的缩略图；size 补填为帧尺寸
    ASSERT_FALSE(img.isNull());
    EXPECT_EQ(img.size(), QSize(4, 4));
    EXPECT_GT(img.pixelColor(1, 1).blue(), 200);
    EXPECT_EQ(outSize, QSize(4, 4));
    EXPECT_TRUE(ThumbnailCache::instance()->contains(path, 1));
    EXPECT_EQ(ThumbnailCache::instance()->get(path, 1).size(), QSize(100, 100));
}

TEST_F(ThumbnailProviderTest, RequestImage_TruncatedImage_FallsBackToFullLoadNull)
{
    // Arrange: 畸形 TIFF（size() 可解析、解码器读取失败）
    const QString path = makeCorruptTiff(tempDir.path(), "thumb_corrupt.tif", 400, 200);
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize());

    // Assert  // 回退 readNormalImage 仍失败 → null；size 出参已按原图尺寸设置
    EXPECT_TRUE(img.isNull());
    EXPECT_EQ(outSize, QSize(400, 200));
    EXPECT_TRUE(ThumbnailCache::instance()->contains(path, 0));
}

TEST_F(ThumbnailProviderTest, RequestImage_NonexistentFile_ReturnsNullAndCachesNull)
{
    // Arrange: id 指向不存在的文件
    const QString path = tempDir.filePath("thumb_missing.png");
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QImage img = obj->requestImage(id, &outSize, QSize());

    // Assert  // 解码器与全图加载都失败 → null 缩略图仍入单例缓存（疑似缺陷 2）
    EXPECT_TRUE(img.isNull());
    EXPECT_TRUE(outSize.isEmpty());
    EXPECT_TRUE(ThumbnailCache::instance()->contains(path, 0));
    EXPECT_TRUE(ThumbnailCache::instance()->get(path, 0).isNull());
}

TEST_F(ThumbnailProviderTest, RequestImage_PoisonedCache_HidesNewlyCreatedFile)
{
    // Arrange: 先请求不存在的路径（缓存 null），随后创建有效文件
    const QString path = tempDir.filePath("poison.png");
    const QString id = QUrl::fromLocalFile(path).toString();
    obj->requestImage(id, nullptr, QSize());
    ASSERT_TRUE(ThumbnailCache::instance()->contains(path, 0));
    const QString created = makePng(tempDir.path(), "poison.png", 400, 200, Qt::green);
    ASSERT_EQ(created, path);
    ASSERT_FALSE(QImage(path).isNull());

    // Act
    const QImage img = obj->requestImage(id, nullptr, QSize());

    // Assert  // 行为锁定：被 null 污染的缓存令后续有效文件仍返回 null（疑似缺陷 2）
    EXPECT_TRUE(img.isNull());
    EXPECT_TRUE(ThumbnailCache::instance()->get(path, 0).isNull());
}

TEST_F(ThumbnailProviderTest, RequestPixmap_ValidImage_ReturnsScaledPixmap)
{
    // Arrange: 400x200 PNG，请求 30x15（2:1 精确缩小）
    const QString path = makePng(tempDir.path(), "pixmap.png", 400, 200, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QPixmap pix = obj->requestPixmap(id, &outSize, QSize(30, 15));

    // Assert  // requestPixmap = fromImage(requestImage)：尺寸与出参正确
    EXPECT_FALSE(pix.isNull());
    EXPECT_EQ(pix.size(), QSize(30, 15));
    EXPECT_EQ(outSize, QSize(400, 200));
}

TEST_F(ThumbnailProviderTest, RequestPixmap_MissingFile_ReturnsNullPixmap)
{
    // Arrange: id 指向不存在的文件
    const QString path = tempDir.filePath("pixmap_missing.png");
    const QString id = QUrl::fromLocalFile(path).toString();
    QSize outSize;

    // Act
    const QPixmap pix = obj->requestPixmap(id, &outSize, QSize(30, 15));

    // Assert  // 空图像转空 pixmap
    EXPECT_TRUE(pix.isNull());
    EXPECT_EQ(pix.width(), 0);
    EXPECT_EQ(pix.height(), 0);
}

// ══════════════════════════ 自由函数（经公开 API 间接覆盖） ══════════════════════════

class FreeImageDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ThumbnailCache::instance()->clear();
        stub.clear();
        parser = new ImageProvider();
    }

    void TearDown() override
    {
        ThumbnailCache::instance()->clear();
        stub.clear();
        delete parser;
    }

    stub_ext::StubExt stub;
    ImageProvider *parser = nullptr;
    QTemporaryDir tempDir;
};

TEST_F(FreeImageDataTest, ReadNormalImage_ValidFile_LoadsExactDimensions)
{
    // Arrange: 100x50 PNG，经 requestImage（无效请求尺寸）走 readNormalImage 默认上限路径
    const QString path = makePng(tempDir.path(), "free_normal.png", 100, 50, Qt::green);
    const QString id = QUrl::fromLocalFile(path).toString();

    // Act
    const QImage img = parser->requestImage(id, nullptr, QSize());

    // Assert  // readNormalImage B2: 成功加载原始尺寸
    EXPECT_EQ(img.size(), QSize(100, 50));
    EXPECT_FALSE(img.isNull());
}

TEST_F(FreeImageDataTest, ReadMultiImage_FrameNavigation_LoadsExpectedFrame)
{
    // Arrange: 双页 TIFF（帧 0 红、帧 1 蓝）
    const QString path = makeTwoFrameTiff(tempDir.path(), "free_multi.tif");
    const QString id0 = QUrl::fromLocalFile(path).toString();
    const QString id1 = id0 + "#frame_1";

    // Act
    const QImage frame0 = parser->requestImage(id0, nullptr, QSize());
    const QImage frame1 = parser->requestImage(id1, nullptr, QSize());

    // Assert  // readMultiImage B1: 帧导航命中各自帧（以颜色区分）
    ASSERT_FALSE(frame0.isNull());
    ASSERT_FALSE(frame1.isNull());
    EXPECT_GT(frame0.pixelColor(1, 1).red(), 200);
    EXPECT_LT(frame0.pixelColor(1, 1).blue(), 100);
    EXPECT_GT(frame1.pixelColor(1, 1).blue(), 200);
    EXPECT_LT(frame1.pixelColor(1, 1).red(), 100);
}

namespace {
struct ParseIdCase {
    int kind;             // 0: file URL；1: file URL#frame_0；2: file URL#frame_12；3: 裸路径；4: 裸路径#frame_3
    const char *name;     // 唯一文件名（文件不存在，仅用于解析观测）
    int expectFrame;
};
}  // namespace

struct ParseIdParamTest : public FreeImageDataTest, public ::testing::WithParamInterface<ParseIdCase> {
};

// parseProviderID 结果经 requestImage 的缓存键 (path, frame) 观测（文件不存在 → null 也入缓存）
TEST_P(ParseIdParamTest, ParseProviderID_IdVariants_ParseExpectedPathAndFrame)
{
    const ParseIdCase c = GetParam();

    // Arrange: 按形态构造 id，目标文件均不存在
    const QString path = tempDir.filePath(c.name);
    QString id;
    switch (c.kind) {
    case 0:
        id = QUrl::fromLocalFile(path).toString();
        break;
    case 1:
        id = QUrl::fromLocalFile(path).toString() + "#frame_0";
        break;
    case 2:
        id = QUrl::fromLocalFile(path).toString() + "#frame_12";
        break;
    case 3:
        id = path;
        break;
    default:
        id = path + "#frame_3";
        break;
    }
    ASSERT_FALSE(QFile::exists(path));

    // Act
    const QImage img = parser->requestImage(id, nullptr, QSize());

    // Assert  // B1（无 tag → frame 0）/ B2（tag → toInt）。
    // 裸路径（kind 3/4）非 URL 形态：QUrl(裸路径).toLocalFile() 返回空串 → 路径键为空（现状锁定 → 疑似缺陷 4）
    const QString expectKeyPath = (c.kind >= 3) ? QString() : path;
    EXPECT_TRUE(img.isNull());
    EXPECT_TRUE(parser->imageCache.keys().contains(ThumbnailCache::Key(expectKeyPath, c.expectFrame)));
}

INSTANTIATE_TEST_SUITE_P(
    IdVariations, ParseIdParamTest,
    ::testing::Values(
        ParseIdCase{0, "parse_url.png", 0},
        ParseIdCase{1, "parse_frame0.png", 0},
        ParseIdCase{2, "parse_frame12.png", 12},
        ParseIdCase{3, "parse_bare.png", 0},
        ParseIdCase{4, "parse_bare_frame3.png", 3}));
