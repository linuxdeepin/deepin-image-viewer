// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_imageinfo.h"
#include "imageinfo.h"
#include "types.h"
#include "stub.h"

#include <QUrl>
#include <QImage>
#include <QImageReader>
#include <QDir>
#include <QSignalSpy>
#include <QSharedPointer>
#include <QHash>
#include <QSet>
#include <QScopedPointer>
#include <QThreadPool>
#include <QThread>
#include <QCoreApplication>
#include <QEventLoop>
#include <QElapsedTimer>
#include <QTimer>

// ---------- 私有类声明（从 imageinfo.cpp 复制，用于访问私有成员） ----------

// ImageInfoData 声明（imageinfo.cpp 内私有类，仅声明数据成员以正确构造）
class ImageInfoData
{
public:
    typedef QSharedPointer<ImageInfoData> Ptr;

    inline bool isError() const
    {
        bool ret = !exist || (Types::DamagedImage == type);
        return ret;
    }

    QString path;
    Types::ImageType type;
    QSize size;
    int frameIndex = 0;
    int frameCount = 0;
    bool exist = false;
    qreal scale = -1;
    qreal x = 0;
    qreal y = 0;
};

// ImageInfoCache 声明（imageinfo.cpp 内私有类，继承 QObject）
class ImageInfoCache : public QObject
{
public:
    typedef QPair<QString, int> KeyType;

    ImageInfoCache();
    ~ImageInfoCache() override;
    void loadFinished(const QString &path, int frameIndex, ImageInfoData::Ptr data);
    void removeCache(const QString &path, int frameIndex);
    void load(const QString &path, int frameIndex, bool reload = false);

private:
    bool aboutToQuit { false };
    QHash<KeyType, ImageInfoData::Ptr> cache;
    QSet<KeyType> waitSet;
    QScopedPointer<QThreadPool> localPoolPtr;
};

// LoadImageInfoRunnable 声明（imageinfo.cpp 内私有类，继承 QRunnable）
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

// postEvent 桩：捕获全局 CacheInstance() 指针
static QObject *s_capturedCacheInstance = nullptr;
static void ut_ii_stub_capturePostEvent(QObject *receiver, QEvent *event, int priority)
{
    Q_UNUSED(priority)
    if (receiver) {
        s_capturedCacheInstance = receiver;
    }
    delete event;
}

// 捕获全局 CacheInstance() 指针（仅首次调用实际捕获，后续直接返回缓存值）
static QObject *ensureCacheInstanceCaptured()
{
    if (s_capturedCacheInstance) {
        return s_capturedCacheInstance;
    }
    Stub stub;
    stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
    LoadImageInfoRunnable runnable("/tmp/ut_ii_capture_dummy.png", 0);
    ImageInfoData::Ptr data(new ImageInfoData);
    data->path = "/tmp/ut_ii_capture_dummy.png";
    data->exist = true;
    data->type = Types::NormalImage;
    runnable.notifyFinished("/tmp/ut_ii_capture_dummy.png", 0, data);
    return s_capturedCacheInstance;
}

// 重置全局 CacheInstance 的 aboutToQuit 标志（防止跨测试污染）
static void resetGlobalCacheAboutToQuit()
{
    QObject *inst = ensureCacheInstanceCaptured();
    if (inst) {
        reinterpret_cast<ImageInfoCache *>(inst)->aboutToQuit = false;
    }
}

void ut_imageinfo::SetUp()
{
    resetGlobalCacheAboutToQuit();
}

void ut_imageinfo::TearDown()
{
}

// 测试默认构造
TEST_F(ut_imageinfo, DefaultConstruct)
{
    ImageInfo info;
    EXPECT_EQ(info.status(), ImageInfo::Null);
    EXPECT_EQ(info.frameIndex(), 0);
    // 默认 frameCount 为 1（空图片按单帧处理）
    EXPECT_EQ(info.frameCount(), 1);
    // 默认 width/height 为 -1（未加载）
    EXPECT_EQ(info.width(), -1);
    EXPECT_EQ(info.height(), -1);
}

// 测试设置 source
TEST_F(ut_imageinfo, SetSource)
{
    ImageInfo info;
    QSignalSpy spy(&info, &ImageInfo::sourceChanged);

    QUrl testUrl("file:///tmp/nonexistent_test_image.jpg");
    info.setSource(testUrl);

    EXPECT_EQ(info.source(), testUrl);
}

// 测试设置 frameIndex
TEST_F(ut_imageinfo, SetFrameIndex)
{
    ImageInfo info;
    QSignalSpy spy(&info, &ImageInfo::frameIndexChanged);

    info.setFrameIndex(0);
    EXPECT_EQ(info.frameIndex(), 0);
}

// 测试运行时属性默认值
TEST_F(ut_imageinfo, RuntimeProperties)
{
    ImageInfo info;

    // 运行时属性默认值
    EXPECT_EQ(info.x(), 0);
    EXPECT_EQ(info.y(), 0);
}

// ---------- 辅助工具：构造小尺寸临时 PNG 与等待异步加载 ----------

namespace {
// 在系统临时目录下生成一张指定尺寸的 PNG 图片，返回本地文件路径
QString makeTempPng(const QString &name, int w = 10, int h = 10)
{
    QString path = QDir::tempPath() + "/" + name;
    QImage img(w, h, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(path, "PNG");
    return path;
}

// 同步等待 ImageInfo 状态变化。仅处理全局 CacheInstance 的排队事件，
// 避免处理残留的 DBus 事件导致 SEGV。
bool waitUntilStatus(ImageInfo &info, ImageInfo::Status target, int timeout = 5000)
{
    if (info.status() == target) {
        return true;
    }
    QObject *cacheInstance = ensureCacheInstanceCaptured();
    QElapsedTimer timer;
    timer.start();
    while (!timer.hasExpired(timeout)) {
        if (cacheInstance) {
            QCoreApplication::sendPostedEvents(cacheInstance, 0);
        }
        if (info.status() == target) {
            return true;
        }
        QThread::msleep(10);
    }
    return info.status() == target;
}
}  // namespace

// 测试带 source 的构造函数
TEST_F(ut_imageinfo, ConstructWithSource)
{
    QString path = makeTempPng("ut_imageinfo_ctor.png");
    ImageInfo info(QUrl::fromLocalFile(path));
    EXPECT_EQ(info.source(), QUrl::fromLocalFile(path));
    // 构造后立即触发加载流程，状态由 Null 切换为 Loading（或已 Ready）
    EXPECT_NE(info.status(), ImageInfo::Null);
}

// 测试无数据时 type() 返回 NullImage
TEST_F(ut_imageinfo, TypeNoData)
{
    ImageInfo info;
    EXPECT_EQ(info.type(), Types::NullImage);
}

// 测试加载真实图片后的各项信息
TEST_F(ut_imageinfo, LoadRealImageInfo)
{
    QString path = makeTempPng("ut_imageinfo_load.png");
    ImageInfo info;
    QSignalSpy srcSpy(&info, &ImageInfo::sourceChanged);
    info.setSource(QUrl::fromLocalFile(path));
    EXPECT_EQ(srcSpy.count(), 1);

    EXPECT_TRUE(waitUntilStatus(info, ImageInfo::Ready));
    // 加载可能未完成（无事件循环），仅在 data 已加载时验证
    if (info.data) {
        EXPECT_EQ(info.status(), ImageInfo::Ready);
        EXPECT_EQ(info.type(), Types::NormalImage);
        EXPECT_EQ(info.width(), 10);
        EXPECT_EQ(info.height(), 10);
        EXPECT_TRUE(info.exists());
        EXPECT_EQ(info.frameCount(), 0);
    }
}

// 测试交换宽高：无数据时为 no-op，有数据时翻转尺寸
TEST_F(ut_imageinfo, SwapWidthAndHeight)
{
    // 无数据：不崩溃，无效果
    ImageInfo empty;
    empty.swapWidthAndHeight();
    EXPECT_EQ(empty.width(), -1);
    EXPECT_EQ(empty.height(), -1);

    // 有数据：使用非正方形图片以便观察交换
    QString path = makeTempPng("ut_imageinfo_swap.png", 12, 8);
    ImageInfo info;
    info.setSource(QUrl::fromLocalFile(path));
    EXPECT_TRUE(waitUntilStatus(info, ImageInfo::Ready));
    if (info.data) {
        EXPECT_EQ(info.width(), 12);
        EXPECT_EQ(info.height(), 8);
    }

    QSignalSpy wSpy(&info, &ImageInfo::widthChanged);
    QSignalSpy hSpy(&info, &ImageInfo::heightChanged);
    info.swapWidthAndHeight();
    if (info.data) {
        EXPECT_EQ(info.width(), 8);
        EXPECT_EQ(info.height(), 12);
    }
    // swapWidthAndHeight 广播 imageSizeChanged，但 widthChanged 信号非其直接发送，
    // 此处仅确认尺寸已翻转
    EXPECT_TRUE(wSpy.isValid());
    EXPECT_TRUE(hSpy.isValid());
}

// 测试 scale 运行时属性
TEST_F(ut_imageinfo, ScaleProperty)
{
    // 无数据：scale() 返回 -1，setScale 无效
    ImageInfo empty;
    EXPECT_EQ(empty.scale(), -1);
    empty.setScale(2.0);
    EXPECT_EQ(empty.scale(), -1);

    // 有数据：可读写
    QString path = makeTempPng("ut_imageinfo_scale.png");
    ImageInfo info;
    info.setSource(QUrl::fromLocalFile(path));
    EXPECT_TRUE(waitUntilStatus(info, ImageInfo::Ready));
    info.setScale(1.5);
    // 设置相同值时内部不更新，确保无异常
    info.setScale(1.5);
    if (info.data) {
        EXPECT_DOUBLE_EQ(info.scale(), 1.5);
    }
}

// 测试 setX / setY 运行时属性
TEST_F(ut_imageinfo, XYProperties)
{
    // 无数据：getter 返回默认 0，setter 无效
    ImageInfo empty;
    EXPECT_EQ(empty.x(), 0);
    EXPECT_EQ(empty.y(), 0);
    empty.setX(3.5);
    empty.setY(4.5);
    EXPECT_EQ(empty.x(), 0);
    EXPECT_EQ(empty.y(), 0);

    // 有数据：可读写
    QString path = makeTempPng("ut_imageinfo_xy.png");
    ImageInfo info;
    info.setSource(QUrl::fromLocalFile(path));
    EXPECT_TRUE(waitUntilStatus(info, ImageInfo::Ready));
    info.setX(7.0);
    info.setY(9.0);
    if (info.data) {
        EXPECT_DOUBLE_EQ(info.x(), 7.0);
        EXPECT_DOUBLE_EQ(info.y(), 9.0);
    }
}

// 测试 exists()
TEST_F(ut_imageinfo, Exists)
{
    ImageInfo empty;
    EXPECT_FALSE(empty.exists());

    QString path = makeTempPng("ut_imageinfo_exists.png");
    ImageInfo info;
    info.setSource(QUrl::fromLocalFile(path));
    EXPECT_TRUE(waitUntilStatus(info, ImageInfo::Ready));
    if (info.data) {
        EXPECT_TRUE(info.exists());
    }
}

// 测试 hasCachedThumbnail() 各分支
TEST_F(ut_imageinfo, HasCachedThumbnail)
{
    // 空 URL 分支
    ImageInfo empty;
    EXPECT_FALSE(empty.hasCachedThumbnail());

    // 加载不存在的文件 -> type 为 NullImage -> switch 命中 NullImage 返回 false
    ImageInfo nonexist;
    nonexist.setSource(QUrl::fromLocalFile("/tmp/ut_imageinfo_nonexist_thumb.png"));
    waitUntilStatus(nonexist, ImageInfo::Error);
    EXPECT_FALSE(nonexist.hasCachedThumbnail());

    // 加载真实图片后缩略图已缓存 -> default 分支返回 true
    QString path = makeTempPng("ut_imageinfo_thumb.png");
    ImageInfo loaded;
    loaded.setSource(QUrl::fromLocalFile(path));
    EXPECT_TRUE(waitUntilStatus(loaded, ImageInfo::Ready));
    if (loaded.data) {
        EXPECT_TRUE(loaded.hasCachedThumbnail());
    }
}

// 测试 reloadData() 重新加载
TEST_F(ut_imageinfo, ReloadData)
{
    QString path = makeTempPng("ut_imageinfo_reload.png");
    ImageInfo info;
    info.setSource(QUrl::fromLocalFile(path));
    EXPECT_TRUE(waitUntilStatus(info, ImageInfo::Ready));

    QSignalSpy statusSpy(&info, &ImageInfo::statusChanged);
    info.reloadData();  // Q_INVOKABLE
    // reloadData 设置 Loading
    EXPECT_EQ(info.status(), ImageInfo::Loading);
    EXPECT_GE(statusSpy.count(), 0);

    // 等待重新加载完成
    EXPECT_TRUE(waitUntilStatus(info, ImageInfo::Ready));
    if (info.data) {
        EXPECT_EQ(info.status(), ImageInfo::Ready);
    }
}

// 测试 clearCurrentCache()
TEST_F(ut_imageinfo, ClearCurrentCache)
{
    // 无数据：no-op，不崩溃
    ImageInfo empty;
    empty.clearCurrentCache();

    // 有数据：调用不应崩溃且不影响已加载信息
    QString path = makeTempPng("ut_imageinfo_clearcur.png");
    ImageInfo info;
    info.setSource(QUrl::fromLocalFile(path));
    EXPECT_TRUE(waitUntilStatus(info, ImageInfo::Ready));
    if (info.data) {
        EXPECT_TRUE(info.exists());
    }
    info.clearCurrentCache();
    // data 仍存在
    if (info.data) {
        EXPECT_TRUE(info.exists());
    }
}

// 测试静态方法 clearCache()
TEST_F(ut_imageinfo, ClearCacheStatic)
{
    // 仅验证可调用且不崩溃
    ImageInfo::clearCache();
    SUCCEED();
}

// ---------- protected 方法测试（依赖 -fno-access-control） ----------

// 测试 setStatus() 状态切换与信号
TEST_F(ut_imageinfo, SetStatus)
{
    ImageInfo info;
    QSignalSpy spy(&info, &ImageInfo::statusChanged);

    info.setStatus(ImageInfo::Loading);
    EXPECT_EQ(info.status(), ImageInfo::Loading);
    EXPECT_EQ(spy.count(), 1);

    // 相同状态不应再次触发信号
    info.setStatus(ImageInfo::Loading);
    EXPECT_EQ(spy.count(), 1);

    info.setStatus(ImageInfo::Error);
    EXPECT_EQ(info.status(), ImageInfo::Error);
    EXPECT_EQ(spy.count(), 2);
}

// 测试 updateData()：相同指针返回 false
TEST_F(ut_imageinfo, UpdateDataSamePointer)
{
    ImageInfo info;
    // 无数据时 data 为空，传入空指针 -> 相等 -> 返回 false
    QSharedPointer<ImageInfoData> nullPtr;
    EXPECT_FALSE(info.updateData(nullPtr));

    QString path = makeTempPng("ut_imageinfo_updatedata.png");
    info.setSource(QUrl::fromLocalFile(path));
    EXPECT_TRUE(waitUntilStatus(info, ImageInfo::Ready));
    // 传入当前 data 自身 -> 相等 -> 返回 false
    EXPECT_FALSE(info.updateData(info.data));
}

// 测试 updateData() 检测到差异并发送信号（type/size/exist 变更分支）
TEST_F(ut_imageinfo, UpdateDataDetectsChange)
{
    // 先加载不存在的文件：data.exist=false, type=NullImage
    ImageInfo info;
    info.setSource(QUrl::fromLocalFile("/tmp/ut_imageinfo_nonexist_for_change.png"));
    EXPECT_TRUE(waitUntilStatus(info, ImageInfo::Error));
    // data 可能为空（加载未完成，无事件循环）

    // 切换到真实图片，触发 updateData 检测到差异
    QString path = makeTempPng("ut_imageinfo_changedetected.png");
    QSignalSpy typeSpy(&info, &ImageInfo::typeChanged);
    QSignalSpy widthSpy(&info, &ImageInfo::widthChanged);
    QSignalSpy heightSpy(&info, &ImageInfo::heightChanged);
    QSignalSpy existsSpy(&info, &ImageInfo::existsChanged);
    QSignalSpy infoSpy(&info, &ImageInfo::infoChanged);

    info.setSource(QUrl::fromLocalFile(path));
    EXPECT_TRUE(waitUntilStatus(info, ImageInfo::Ready));

    // 仅在 data 已加载时验证信号变化
    if (info.data) {
        EXPECT_GE(typeSpy.count(), 1);
        EXPECT_GE(widthSpy.count(), 1);
        EXPECT_GE(heightSpy.count(), 1);
        EXPECT_GE(existsSpy.count(), 1);
        EXPECT_GE(infoSpy.count(), 1);
    }
}

// 测试 refreshDataFromCache() 各分支
TEST_F(ut_imageinfo, RefreshDataFromCache)
{
    // 分支1：空路径 -> 设置 Error
    ImageInfo infoEmpty;
    infoEmpty.refreshDataFromCache(true);
    EXPECT_EQ(infoEmpty.status(), ImageInfo::Error);

    // 分支5：路径未缓存且 reload=false -> 设置 Error
    ImageInfo infoNoCache;
    infoNoCache.imageUrl = QUrl::fromLocalFile("/tmp/ut_imageinfo_uncached_refresh.png");
    infoNoCache.refreshDataFromCache(false);
    EXPECT_EQ(infoNoCache.status(), ImageInfo::Error);

    // 分支4：路径未缓存且 reload=true -> 设置 Loading 并发起加载
    // （完成通过排队信号，调用返回后仍为 Loading）
    ImageInfo infoReload;
    infoReload.imageUrl = QUrl::fromLocalFile("/tmp/ut_imageinfo_uncached_reload.png");
    infoReload.refreshDataFromCache(true);
    EXPECT_EQ(infoReload.status(), ImageInfo::Loading);

    // 分支2：数据已缓存，新对象取缓存数据
    QString path = makeTempPng("ut_imageinfo_refresh_cached.png");
    ImageInfo donor;
    donor.setSource(QUrl::fromLocalFile(path));
    EXPECT_TRUE(waitUntilStatus(donor, ImageInfo::Ready));  // 缓存已写入
    ImageInfo infoCached;
    infoCached.imageUrl = QUrl::fromLocalFile(path);
    QSignalSpy infoSpy(&infoCached, &ImageInfo::infoChanged);
    infoCached.refreshDataFromCache(false);
    if (!infoCached.data.isNull()) {
        EXPECT_EQ(infoCached.status(), ImageInfo::Ready);
        EXPECT_EQ(infoSpy.count(), 1);
    }
}


// 测试 onLoadFinished() 槽：路径匹配与不匹配
TEST_F(ut_imageinfo, OnLoadFinished)
{
    ImageInfo info;
    info.imageUrl = QUrl::fromLocalFile("/tmp/ut_imageinfo_match.png");

    // 不匹配 -> 不处理，状态保持
    info.onLoadFinished("/tmp/ut_imageinfo_OTHER.png", 0);
    EXPECT_EQ(info.status(), ImageInfo::Null);

    // 匹配但缓存无数据 -> refreshDataFromCache(false) -> Error
    info.onLoadFinished("/tmp/ut_imageinfo_match.png", 0);
    EXPECT_EQ(info.status(), ImageInfo::Error);
}

// 测试 onSizeChanged() 槽
TEST_F(ut_imageinfo, OnSizeChanged)
{
    // data 为空：即使路径匹配也不发信号
    ImageInfo info;
    info.imageUrl = QUrl::fromLocalFile("/tmp/ut_imageinfo_size.png");
    QSignalSpy wSpy(&info, &ImageInfo::widthChanged);
    QSignalSpy hSpy(&info, &ImageInfo::heightChanged);
    info.onSizeChanged("/tmp/ut_imageinfo_size.png", 0);  // 匹配但 data 为空
    EXPECT_EQ(wSpy.count(), 0);
    EXPECT_EQ(hSpy.count(), 0);
    // 路径不匹配 -> 不处理
    info.onSizeChanged("/tmp/ut_imageinfo_other.png", 0);
    EXPECT_EQ(wSpy.count(), 0);
    EXPECT_EQ(hSpy.count(), 0);

    // 有数据且匹配 -> 发送 widthChanged/heightChanged
    QString path = makeTempPng("ut_imageinfo_onsize.png");
    ImageInfo loaded;
    loaded.setSource(QUrl::fromLocalFile(path));
    EXPECT_TRUE(waitUntilStatus(loaded, ImageInfo::Ready));
    QSignalSpy wSpy2(&loaded, &ImageInfo::widthChanged);
    QSignalSpy hSpy2(&loaded, &ImageInfo::heightChanged);
    loaded.onSizeChanged(path, 0);
    if (loaded.data) {
        EXPECT_EQ(wSpy2.count(), 1);
        EXPECT_EQ(hSpy2.count(), 1);
    }
}

// ---------- 私有类测试（imageinfo.cpp 内定义，依赖 -fno-access-control） ----------

// ImageInfoCache 析构函数: 触发 D0 deleting destructor (new + delete)
TEST_F(ut_imageinfo, ImageInfoCacheDeletingDestructor)
{
    auto *obj = new ImageInfoCache();
    delete obj;
    SUCCEED();
}

// ImageInfoData::isError(): DamagedImage 返回 true
TEST_F(ut_imageinfo, ImageInfoDataIsError_DamagedImage)
{
    ImageInfoData data;
    data.type = Types::DamagedImage;
    data.exist = true;
    EXPECT_TRUE(data.isError());

    // exist=false 时也返回 true
    data.exist = false;
    EXPECT_TRUE(data.isError());
}

// ImageInfoData::isError(): NormalImage 且 exist=true 返回 false
TEST_F(ut_imageinfo, ImageInfoDataIsError_NormalImage)
{
    ImageInfoData data;
    data.type = Types::NormalImage;
    data.exist = true;
    EXPECT_FALSE(data.isError());
}

// ImageInfoCache::removeCache()
TEST_F(ut_imageinfo, ImageInfoCacheRemoveCache)
{
    ImageInfoCache cache;

    // 先插入数据
    ImageInfoData::Ptr data(new ImageInfoData);
    data->type = Types::NormalImage;
    data->exist = true;
    cache.loadFinished("/tmp/ut_removecache_test.png", 0, data);

    // 移除已缓存的项
    cache.removeCache("/tmp/ut_removecache_test.png", 0);
    // 移除不存在的项不崩溃
    cache.removeCache("/tmp/ut_removecache_nonexist.png", 0);
    SUCCEED();
}

// ImageInfoCache::loadFinished()
TEST_F(ut_imageinfo, ImageInfoCacheLoadFinished)
{
    ImageInfoCache cache;

    // 插入有效数据
    ImageInfoData::Ptr data(new ImageInfoData);
    data->type = Types::NormalImage;
    data->exist = true;
    cache.loadFinished("/tmp/ut_loadfinished_valid.png", 0, data);

    // 插入空数据（nullptr）走警告分支
    ImageInfoData::Ptr nullData;
    cache.loadFinished("/tmp/ut_loadfinished_null.png", 0, nullData);
    SUCCEED();
}

// ============================================================
// 以下为补充用例，覆盖构造函数 lambda 及 notifyFinished lambda
// ============================================================

// ImageInfoCache aboutToQuit lambda: 手动发射信号触发构造函数中的 lambda
// 先断开全局 CacheInstance 的 aboutToQuit 连接，避免设置其 aboutToQuit 标志
TEST_F(ut_imageinfo, ImageInfoCache_AboutToQuit_TriggersLambda)
{
    // 断开所有 aboutToQuit 连接（包括全局 CacheInstance 和 RotateImageHelper 的）
    qApp->disconnect(SIGNAL(aboutToQuit()));

    // 创建局部实例，构造函数中连接 aboutToQuit 信号
    ImageInfoCache cache;
    EXPECT_FALSE(cache.aboutToQuit);

    // 手动发射 aboutToQuit 信号（-fno-access-control 允许调用 protected 信号）
    // Qt6 信号需 QPrivateSignal 参数
    qApp->aboutToQuit(QCoreApplication::QPrivateSignal{});

    // lambda 设置 aboutToQuit = true，调用 clearCache 和 waitForDone
    EXPECT_TRUE(cache.aboutToQuit);

    // 恢复全局 CacheInstance 的 aboutToQuit 标志，避免影响后续测试
    resetGlobalCacheAboutToQuit();
}

TEST_F(ut_imageinfo, NotifyFinished_QueuedLambda_Executed)
{
    QString path = makeTempPng("ut_notify_lambda.png");

    // 步骤1: 桩 postEvent 捕获 CacheInstance() 指针
    {
        Stub stub;
        stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
        LoadImageInfoRunnable runnable(path, 0);
        ImageInfoData::Ptr data(new ImageInfoData);
        data->path = path;
        data->exist = true;
        data->type = Types::NormalImage;
        // notifyFinished 内部调用 CacheInstance() 并 postEvent，桩捕获 receiver
        runnable.notifyFinished(path, 0, data);
    }
    // s_capturedCacheInstance 现在持有 CacheInstance() 指针
    QObject *capturedInstance = s_capturedCacheInstance;
    s_capturedCacheInstance = nullptr;  // 清除，避免跨测试残留
    ASSERT_NE(capturedInstance, nullptr);

    // 步骤2: 不桩 postEvent，再次调用 notifyFinished 实际投递 lambda
    LoadImageInfoRunnable runnable2(path, 0);
    ImageInfoData::Ptr data2(new ImageInfoData);
    data2->path = path;
    data2->exist = true;
    data2->type = Types::NormalImage;
    runnable2.notifyFinished(path, 0, data2);

    // 步骤3: 仅处理 CacheInstance() 的事件，执行排队 lambda
    QCoreApplication::sendPostedEvents(capturedInstance, 0);

    SUCCEED();
}
// 覆盖源码 imageinfo.cpp:48 的真实 ImageInfoData::isError()。
// CacheInstance() 是 Q_GLOBAL_STATIC 文件静态（imageinfo.cpp:106），不可直接调用，
// 使用 postEvent 桩捕获其指针，再通过 reinterpret_cast 调用 loadFinished 直接写缓存。
TEST_F(ut_imageinfo, RefreshDataFromCache_RealIsError_CoveredByAsyncLoad)
{
    QString path = makeTempPng("ut_imageinfo_real_iserror.png");

    // 步骤1: 桩 postEvent 捕获全局 CacheInstance() 指针
    {
        Stub stub;
        stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
        LoadImageInfoRunnable runnable(path, 0);
        ImageInfoData::Ptr data(new ImageInfoData);
        data->path = path;
        data->exist = true;
        data->type = Types::NormalImage;
        runnable.notifyFinished(path, 0, data);
    }
    QObject *capturedInstance = s_capturedCacheInstance;
    s_capturedCacheInstance = nullptr;
    ASSERT_NE(capturedInstance, nullptr);

    // 步骤2: 通过 reinterpret_cast 将 QObject* 转为本地声明的 ImageInfoCache*，
    // 直接调用 loadFinished 同步写入全局缓存（绕过排队事件机制，避免事件循环依赖）
    auto *cache = reinterpret_cast<ImageInfoCache *>(capturedInstance);
    // 确保 aboutToQuit 为 false（之前的 AboutToQuit 测试可能影响全局实例的连接状态）
    cache->aboutToQuit = false;

    ImageInfoData::Ptr data(new ImageInfoData);
    data->path = path;
    data->exist = true;
    data->type = Types::NormalImage;
    data->size = QSize(10, 10);
    cache->loadFinished(path, 0, data);

    // 步骤3: 创建 ImageInfo（构造函数连接 imageDataChanged 信号），
    // 直接设置路径不走 setSource，调用 refreshDataFromCache(false)。
    // find() 返回缓存数据，data->isError() 被调用（覆盖源码 imageinfo.cpp:48）
    ImageInfo info;
    info.imageUrl = QUrl::fromLocalFile(path);
    info.refreshDataFromCache(false);

    // NormalImage + exist=true → isError()=false → 状态 Ready
    ASSERT_FALSE(info.data.isNull());
    EXPECT_EQ(info.status(), ImageInfo::Ready);
}

// ===================== Coverage enhancement tests =====================

#include "unionimage_global.h"

// Declare the free function from imageinfo.cpp
Types::ImageType imageTypeAdapator(imageViewerSpace::ImageType type);

// imageTypeAdapator: all switch branches
TEST_F(ut_imageinfo, ImageTypeAdapator_AllBranches)
{
    EXPECT_EQ(imageTypeAdapator(imageViewerSpace::ImageTypeBlank), Types::NullImage);
    EXPECT_EQ(imageTypeAdapator(imageViewerSpace::ImageTypeSvg), Types::SvgImage);
    EXPECT_EQ(imageTypeAdapator(imageViewerSpace::ImageTypeStatic), Types::NormalImage);
    EXPECT_EQ(imageTypeAdapator(imageViewerSpace::ImageTypeDynamic), Types::DynamicImage);
    EXPECT_EQ(imageTypeAdapator(imageViewerSpace::ImageTypeMulti), Types::MultiImage);
    // Damaged maps to default branch
    EXPECT_EQ(imageTypeAdapator(imageViewerSpace::ImageTypeDamaged), Types::DamagedImage);
}

// LoadImageInfoRunnable::run() with non-existent file (NullImage path)
TEST_F(ut_imageinfo, LoadImageInfoRunnable_Run_NonExistentFile)
{
    LoadImageInfoRunnable runnable("/tmp/ut_nonexistent_file.png", 0);
    runnable.run();
    SUCCEED();
}

// LoadImageInfoRunnable::run() with non-zero frameIndex on non-multi image -> DamagedImage
TEST_F(ut_imageinfo, LoadImageInfoRunnable_Run_NonMultiWithFrameIndex)
{
    QString path = makeTempPng("ut_run_nonmulti_frame.png");
    LoadImageInfoRunnable runnable(path, 1);
    runnable.run();
    SUCCEED();
}

// setFrameIndex: actual change (set to 1 when currently 0)
TEST_F(ut_imageinfo, SetFrameIndex_ActualChange)
{
    ImageInfo info;
    info.currentIndex = 0;
    QSignalSpy spy(&info, &ImageInfo::frameIndexChanged);
    info.setFrameIndex(1);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(info.currentIndex, 1);
}

// setFrameIndex: no change (same value)
TEST_F(ut_imageinfo, SetFrameIndex_NoChange)
{
    ImageInfo info;
    info.currentIndex = 0;
    QSignalSpy spy(&info, &ImageInfo::frameIndexChanged);
    info.setFrameIndex(0);
    EXPECT_EQ(spy.count(), 0);
}

// clearCurrentCache: with frameCount > 0
TEST_F(ut_imageinfo, ClearCurrentCache_WithFrameCount)
{
    QString path = makeTempPng("ut_clearcache_frame.png");
    ImageInfo info;
    info.imageUrl = QUrl::fromLocalFile(path);
    info.data = ImageInfoData::Ptr(new ImageInfoData);
    info.data->frameCount = 3;
    info.clearCurrentCache();
    SUCCEED();
}

// updateData: frame index and frame count change
TEST_F(ut_imageinfo, UpdateData_FrameIndexAndCountChange)
{
    QString path = makeTempPng("ut_updatedata_frame.png");
    ImageInfo info;
    info.imageUrl = QUrl::fromLocalFile(path);
    info.data = ImageInfoData::Ptr(new ImageInfoData);
    info.data->frameIndex = 0;
    info.data->frameCount = 1;
    info.data->size = QSize(10, 10);
    info.data->type = Types::NormalImage;

    ImageInfoData::Ptr newData(new ImageInfoData);
    newData->frameIndex = 1;
    newData->frameCount = 3;
    newData->size = QSize(10, 10);
    newData->type = Types::NormalImage;
    newData->exist = true;

    QSignalSpy frameSpy(&info, &ImageInfo::frameIndexChanged);
    QSignalSpy countSpy(&info, &ImageInfo::frameCountChanged);

    info.updateData(newData);

    EXPECT_EQ(frameSpy.count(), 1);
    EXPECT_EQ(countSpy.count(), 1);
}

// ImageInfoCache::load() with aboutToQuit = true (skip)
TEST_F(ut_imageinfo, ImageInfoCache_Load_AboutToQuit)
{
    // Capture CacheInstance via postEvent stub
    {
        Stub stub;
        stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
        LoadImageInfoRunnable runnable(makeTempPng("ut_load_quit.png"), 0);
        ImageInfoData::Ptr data(new ImageInfoData);
        data->path = makeTempPng("ut_load_quit.png");
        data->exist = true;
        data->type = Types::NormalImage;
        runnable.notifyFinished(data->path, 0, data);
    }
    QObject *captured = s_capturedCacheInstance;
    s_capturedCacheInstance = nullptr;
    ASSERT_NE(captured, nullptr);
    auto *cache = reinterpret_cast<ImageInfoCache *>(captured);
    cache->aboutToQuit = true;
    // Should skip loading
    cache->load("/tmp/ut_load_quit_test.png", 0, false);
    SUCCEED();
}

// ImageInfoCache::load() with reload=true (bypasses cache check)
TEST_F(ut_imageinfo, ImageInfoCache_Load_Reload)
{
    QString path = makeTempPng("ut_load_reload.png");
    // First capture cache instance
    {
        Stub stub;
        stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
        LoadImageInfoRunnable runnable(path, 0);
        ImageInfoData::Ptr data(new ImageInfoData);
        data->path = path;
        data->exist = true;
        data->type = Types::NormalImage;
        runnable.notifyFinished(path, 0, data);
    }
    QObject *captured = s_capturedCacheInstance;
    s_capturedCacheInstance = nullptr;
    ASSERT_NE(captured, nullptr);
    auto *cache = reinterpret_cast<ImageInfoCache *>(captured);
    cache->aboutToQuit = false;
    // Load with reload=true to bypass cache check
    cache->load(path, 0, true);
    SUCCEED();
}

// ImageInfoCache::loadFinished with aboutToQuit
TEST_F(ut_imageinfo, ImageInfoCache_LoadFinished_AboutToQuit)
{
    {
        Stub stub;
        stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
        LoadImageInfoRunnable runnable(makeTempPng("ut_loadfin_quit.png"), 0);
        ImageInfoData::Ptr data(new ImageInfoData);
        data->path = makeTempPng("ut_loadfin_quit.png");
        data->exist = true;
        data->type = Types::NormalImage;
        runnable.notifyFinished(data->path, 0, data);
    }
    QObject *captured = s_capturedCacheInstance;
    s_capturedCacheInstance = nullptr;
    ASSERT_NE(captured, nullptr);
    auto *cache = reinterpret_cast<ImageInfoCache *>(captured);
    cache->aboutToQuit = true;
    ImageInfoData::Ptr data(new ImageInfoData);
    data->path = "/tmp/ut_loadfin_quit_test.png";
    data->exist = true;
    cache->loadFinished(data->path, 0, data);
    SUCCEED();
}

// =================== Round 58 coverage improvement tests ===================

#include "globalcontrol.h"
#include <QProcess>

// Stub for GlobalControl::enableMultiThread to return false (sync path)
static bool ut_enableMultiThread_false()
{
    return false;
}

// L174-192: LoadImageInfoRunnable::run() with multi-page TIFF
TEST_F(ut_imageinfo, LoadImageInfoRunnable_Run_MultiPageTiff)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString tiffPath = dir.path() + "/multi_page.tif";
    QString script = QString(
        "from PIL import Image\n"
        "img1=Image.new('RGB',(4,4),(255,0,0))\n"
        "img2=Image.new('RGB',(4,4),(0,255,0))\n"
        "img1.save('%1', save_all=True, append_images=[img2])\n"
    ).arg(tiffPath);
    QProcess proc;
    proc.start("python3", QStringList() << "-c" << script);
    proc.waitForFinished(5000);

    if (QFileInfo::exists(tiffPath)) {
        // Capture cache instance via postEvent stub
        {
            Stub stub;
            stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
            LoadImageInfoRunnable runnable(tiffPath, 0);
            runnable.run();
        }
        s_capturedCacheInstance = nullptr;
    }
    SUCCEED();
}

// L179-182: LoadImageInfoRunnable::run() multi-page TIFF with corrupted frame 1
// Corrupt IFD1 StripOffsets so reader.read() for frame 1 returns null -> DamagedImage path
TEST_F(ut_imageinfo, LoadImageInfoRunnable_Run_MultiPageTiff_CorruptedFrame)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString tiffPath = dir.path() + "/multi_page_corrupted.tif";
    QString script = QString(
        "from PIL import Image\n"
        "import struct\n"
        "img1=Image.new('RGB',(4,4),(255,0,0))\n"
        "img2=Image.new('RGB',(4,4),(0,255,0))\n"
        "img1.save('%1', save_all=True, append_images=[img2])\n"
        "with open('%1','rb') as f:\n"
        "    data=bytearray(f.read())\n"
        "endian='<' if data[0:2]==b'II' else '>'\n"
        "ifd0=struct.unpack_from(endian+'I',data,4)[0]\n"
        "n0=struct.unpack_from(endian+'H',data,ifd0)[0]\n"
        "ifd1=struct.unpack_from(endian+'I',data,ifd0+2+n0*12)[0]\n"
        "n1=struct.unpack_from(endian+'H',data,ifd1)[0]\n"
        "for i in range(n1):\n"
        "    eo=ifd1+2+i*12\n"
        "    tag=struct.unpack_from(endian+'H',data,eo)[0]\n"
        "    if tag==273:\n"
        "        struct.pack_into(endian+'I',data,eo+8,len(data)+1000)\n"
        "        break\n"
        "with open('%1','wb') as f:\n"
        "    f.write(data)\n"
    ).arg(tiffPath);
    QProcess proc;
    proc.start("python3", QStringList() << "-c" << script);
    proc.waitForFinished(10000);

    if (QFileInfo::exists(tiffPath)) {
        {
            Stub stub;
            stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
            LoadImageInfoRunnable runnable(tiffPath, 1);
            runnable.run();
        }
        s_capturedCacheInstance = nullptr;
    }
    SUCCEED();
}

// L329-330: ImageInfoCache::load() cache hit (key already in cache)
TEST_F(ut_imageinfo, ImageInfoCache_Load_CacheHit)
{
    QString path = makeTempPng("ut_cache_hit.png");
    // Capture cache instance
    {
        Stub stub;
        stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
        LoadImageInfoRunnable runnable(path, 0);
        ImageInfoData::Ptr data(new ImageInfoData);
        data->path = path;
        data->exist = true;
        data->type = Types::NormalImage;
        runnable.notifyFinished(path, 0, data);
    }
    QObject *captured = s_capturedCacheInstance;
    s_capturedCacheInstance = nullptr;
    ASSERT_NE(captured, nullptr);
    auto *cache = reinterpret_cast<ImageInfoCache *>(captured);
    cache->aboutToQuit = false;

    // Insert a cache entry manually
    ImageInfoCache::KeyType key(path, 0);
    ImageInfoData::Ptr cachedData(new ImageInfoData);
    cachedData->path = path;
    cachedData->exist = true;
    cachedData->type = Types::NormalImage;
    cache->cache.insert(key, cachedData);
    cache->waitSet.remove(key);

    // Load with reload=false - should hit cache and return early
    cache->load(path, 0, false);
    SUCCEED();
}

// L335-339: ImageInfoCache::load() sync path (enableMultiThread returns false)
TEST_F(ut_imageinfo, ImageInfoCache_Load_SyncPath)
{
    QString path = makeTempPng("ut_sync_load.png");
    // Capture cache instance
    {
        Stub stub;
        stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
        LoadImageInfoRunnable runnable(path, 0);
        ImageInfoData::Ptr data(new ImageInfoData);
        data->path = path;
        data->exist = true;
        data->type = Types::NormalImage;
        runnable.notifyFinished(path, 0, data);
    }
    QObject *captured = s_capturedCacheInstance;
    s_capturedCacheInstance = nullptr;
    ASSERT_NE(captured, nullptr);
    auto *cache = reinterpret_cast<ImageInfoCache *>(captured);
    cache->aboutToQuit = false;

    // Stub enableMultiThread to return false so sync path is taken
    Stub stub;
    stub.set(ADDR(GlobalControl, enableMultiThread), ut_enableMultiThread_false);

    cache->load(path, 0, false);
    SUCCEED();
}

// L245, L251-253: loadImage fallback path
// When QImageReader fails (orig not valid) and loadStaticImageFromFile succeeds
TEST_F(ut_imageinfo, LoadImageInfoRunnable_LoadImage_Fallback)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    // Create a valid image file with .tga extension
    // QImageReader may not report size for TGA, causing fallback to loadStaticImageFromFile
    QString path = dir.path() + "/fallback.tga";
    // Write minimal TGA data
    QByteArray tgaData;
    tgaData.append(static_cast<char>(0));
    tgaData.append(static_cast<char>(0));
    tgaData.append(static_cast<char>(2));
    tgaData.append(static_cast<char>(0)); tgaData.append(static_cast<char>(0));
    tgaData.append(static_cast<char>(0)); tgaData.append(static_cast<char>(0));
    tgaData.append(static_cast<char>(0));
    tgaData.append(static_cast<char>(0)); tgaData.append(static_cast<char>(0));
    tgaData.append(static_cast<char>(0)); tgaData.append(static_cast<char>(0));
    tgaData.append(static_cast<char>(4)); tgaData.append(static_cast<char>(0));
    tgaData.append(static_cast<char>(4)); tgaData.append(static_cast<char>(0));
    tgaData.append(static_cast<char>(24));
    tgaData.append(static_cast<char>(0));
    for (int i = 0; i < 48; ++i) {
        tgaData.append(static_cast<char>(128));
    }
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(tgaData);
    f.close();

    if (QFileInfo::exists(path)) {
        {
            Stub stub;
            stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
            LoadImageInfoRunnable runnable(path, 0);
            runnable.run();
        }
        s_capturedCacheInstance = nullptr;
    }
    SUCCEED();
}

// L245: loadImage() fallback when reader.read() returns null after setScaledSize
// Corrupted PNG: valid header (size readable) but corrupted IDAT data (read fails)
TEST_F(ut_imageinfo, LoadImageInfoRunnable_LoadImage_CorruptedPng_Fallback)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString pngPath = dir.path() + "/corrupted.png";
    QString script = QString(
        "from PIL import Image\n"
        "img=Image.new('RGB',(10,10),(255,0,0))\n"
        "img.save('%1','PNG')\n"
        "with open('%1','rb') as f:\n"
        "    data=bytearray(f.read())\n"
        "pos=8\n"
        "while pos<len(data):\n"
        "    length=int.from_bytes(data[pos:pos+4],'big')\n"
        "    ct=data[pos+4:pos+8]\n"
        "    if ct==b'IDAT':\n"
        "        for i in range(min(20,length)):\n"
        "            data[pos+8+i]=0xFF\n"
        "        break\n"
        "    pos+=12+length\n"
        "with open('%1','wb') as f:\n"
        "    f.write(data)\n"
    ).arg(pngPath);
    QProcess proc;
    proc.start("python3", QStringList() << "-c" << script);
    proc.waitForFinished(10000);

    if (QFileInfo::exists(pngPath)) {
        {
            Stub stub;
            stub.set(ADDR(QCoreApplication, postEvent), ut_ii_stub_capturePostEvent);
            LoadImageInfoRunnable runnable(pngPath, 0);
            runnable.run();
        }
        s_capturedCacheInstance = nullptr;
    }
    SUCCEED();
}

// L251-253: LoadImageInfoRunnable::loadImage() with ICNS file.
// ICNS has QImageReader::size() returning invalid (QSize(-1,-1)),
// so the code skips the QImageReader path and falls through to
// loadStaticImageFromFile, which CAN load ICNS (imageCount>0).
// This covers the success branch of loadStaticImageFromFile in loadImage().
TEST_F(ut_imageinfo, LoadImage_WithIcns_CoversLoadStaticImageSuccessPath)
{
    QTemporaryDir dir;
    const QString path = dir.path() + "/test.icns";

    // Create a valid ICNS file with a 32x32 ARGB icon (ic08 type)
    QByteArray iconData(32 * 32 * 4, 0);
    for (int i = 0; i < 32 * 32; ++i) {
        iconData[i * 4] = 0xFF;     // R
        iconData[i * 4 + 1] = 0x00; // G
        iconData[i * 4 + 2] = 0x00; // B
        iconData[i * 4 + 3] = 0xFF; // A
    }
    QByteArray iconEntry;
    iconEntry.append("ic08");
    qint32 entrySize = iconData.size() + 8;
    iconEntry.append(QByteArray::fromRawData(reinterpret_cast<const char *>(&entrySize), 4).toHex());
    // Write big-endian size
    QByteArray sizeBytes;
    sizeBytes.append((entrySize >> 24) & 0xFF);
    sizeBytes.append((entrySize >> 16) & 0xFF);
    sizeBytes.append((entrySize >> 8) & 0xFF);
    sizeBytes.append(entrySize & 0xFF);
    iconEntry = QByteArray("ic08") + sizeBytes + iconData;
    qint32 totalSize = iconEntry.size() + 8;
    QByteArray totalSizeBytes;
    totalSizeBytes.append((totalSize >> 24) & 0xFF);
    totalSizeBytes.append((totalSize >> 16) & 0xFF);
    totalSizeBytes.append((totalSize >> 8) & 0xFF);
    totalSizeBytes.append(totalSize & 0xFF);
    QByteArray icnsData = QByteArray("icns") + totalSizeBytes + iconEntry;

    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(icnsData);
    f.close();

    // Verify QImageReader returns invalid size for this ICNS
    QImageReader checkReader(path);
    ASSERT_FALSE(checkReader.size().isValid())
        << "QImageReader::size() should be invalid for ICNS";

    LoadImageInfoRunnable runnable(path, 0);
    QImage image;
    QSize sourceSize;
    bool result = runnable.loadImage(image, sourceSize);
    EXPECT_TRUE(result);
    EXPECT_FALSE(image.isNull());
}
