// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | ImageInfo | mid | in_degree:10 | 2 | 2 |
// | ~ImageInfo | low | destructor | 1 | 1 |
// | clearCache | mid | name_pattern:clearCache | 2 | 2 |
// | clearCurrentCache | mid | name_pattern:clearCurrentCache | 2 | 3 |
// | exists | mid | in_degree:94 | 2 | 2 |
// | frameCount | low | - | 1 | 2 |
// | frameIndex | low | - | 1 | 2 |
// | hasCachedThumbnail | mid | complexity:5 | 2 | 4 |
// | height | mid | recursive,in_degree:24 | 2 | 2 |
// | onLoadFinished | low | - | 1 | 2 |
// | onSizeChanged | low | - | 1 | 2 |
// | refreshDataFromCache | mid | complexity:5 | 2 | 5 |
// | reloadData | mid | in_degree:3 | 2 | 2 |
// | scale | low | - | 1 | 2 |
// | setFrameIndex | low | - | 1 | 2 |
// | setScale | low | - | 1 | 2 |
// | setSource | mid | in_degree:10 | 2 | 2 |
// | setStatus | low | - | 1 | 2 |
// | setX | mid | in_degree:6 | 2 | 2 |
// | setY | mid | in_degree:6 | 2 | 2 |
// | source | mid | in_degree:90 | 2 | 2 |
// | status | mid | in_degree:16 | 2 | 2 |
// | swapWidthAndHeight | low | - | 1 | 2 |
// | type | low | - | 1 | 2 |
// | updateData | mid | complexity:6 | 2 | 3 |
// | width | mid | recursive,in_degree:33 | 2 | 2 |
// | x | low | - | 1 | 1 |
// | y | low | - | 1 | 1 |
// | ImageInfoCache | low | - | 1 | 1 |
// | ~ImageInfoCache | low | destructor | 1 | 1 |
// | ImageInfoCache::clearCache | mid | in_degree:3,name_pattern:clearCache | 2 | 2 |
// | ImageInfoCache::find | mid | in_degree:3 | 2 | 2 |
// | ImageInfoCache::load | mid | in_degree:3 | 2 | 6 |
// | ImageInfoCache::loadFinished | low | - | 1 | 3 |
// | ImageInfoCache::removeCache | mid | name_pattern:removeCache | 2 | 2 |
// | LoadImageInfoRunnable | low | - | 1 | 1 |
// | LoadImageInfoRunnable::loadImage | low | - | 1 | 3 |
// | LoadImageInfoRunnable::notifyFinished | low | - | 1 | 1 |
// | LoadImageInfoRunnable::run | mid | complexity:7,lines:77 | 2 | 8 |
// | ImageInfoData::cloneWithoutFrame | low | - | 1 | 2 |
// | ImageInfoData::isError | mid | in_degree:3 | 2 | 6 |
// | imageTypeAdapator(free) | mid | complexity:7 | 2 | 7 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（4 类 41 方法 + 1 自由函数全部有专属命名用例）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（空/不存在/损坏文件/有效 PNG/多帧/帧越界；
//    isError 类型×存在性 6 组矩阵；imageTypeAdapator 全枚举值 + 越界值）
// 3. 每个等价类的边界值显式覆盖: [x]（frameIndex 0/1/2；scale 同值重复；存在/缺失缩略图；
//    循环边界 0 帧/3 帧；waitSet 空/含键；cache 含/不含键）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（isError 6 组、imageTypeAdapator 7 组）
// 5. 分支清单已列出并映射到用例名: [x]（见下方分支清单段落，来源 get_code_snippet 真实源码）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本集群无显式 throw；错误路径以 Error 状态/DamagedImage
//    类型/空指针数据覆盖并断言）
// 8. 负面场景有专门用例: [x]（空 URL/不存在文件/损坏文件/非多页图带帧索引/closingDown）
// 9. 负面用例验证强异常安全: [x]（SetSource_SameUrl/SetFrameIndex_SameIndex/SetStatus_SameStatus
//    断言状态与信号计数不变；Load_AboutToQuit 断言 find 仍为空）
// 10. stub_ext vs gMock 选择正确: [x]（依赖均为 Qt 类/项目内非虚类/隐藏类镜像声明，全用 stub_ext）
//
// 隐藏类镜像说明（见简报坑 7）：
// - ImageInfoCache/ImageInfoData/LoadImageInfoRunnable 定义于 imageinfo.cpp（无公共头），
//   本文件按源码签名镜像声明后直接链接真实符号（-fno-access-control 下可访问 protected/private）。
// - 镜像类显式声明构造/析构（不定义），避免镜像 TU 发射错误 COMDAT vtable 覆盖真实 vtable
//   （实测：隐式析构会让 string-based connect 失效）。
// - ImageInfoData::cloneWithoutFrame 在源码中为类内 inline 且全工程零调用，符号未发射，
//   无法跨 TU 链接，镜像中按源码逐字内联实现（对 imageinfo.cpp 该函数行覆盖为 0，见汇报）。
// - ImageInfoCache 信号观测：镜像无 Q_OBJECT，PMF 式 connect/QSignalSpy 会被
//   static_assert("No Q_OBJECT in the class with the signal") 拦截（qobject.h:248）；
//   故本文件不定义任何 Q_OBJECT 类（不依赖 AUTOMOC 生成 *.moc），局部实例用
//   QSignalSpy 的 string-based 构造（经真实 vtable/metaobject 解析，实测有效），
//   全局实例的副作用改用 removeCache 拦截计数观测。
//   该构造自 Qt 6.4 起 deprecated，仅产生编译告警；项目未定义 QT_DISABLE_DEPRECATED、
//   未开全局 -Werror（仅 -Werror=format-security），可安全使用。
// - 禁止 stub QCoreApplication::postEvent（曾用于捕获全局缓存实例）：
//   QDBusConnection 后台线程随时调用它，入口补丁与后台线程竞争导致野跳转 SEGV
//   （单跑实测复现）；实例捕获改走 removeCache 拦截，事件投递改用
//   sendPostedEvents(nullptr, QEvent::MetaCall) 类型过滤。

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QFile>
#include <QHash>
#include <QImage>
#include <QList>
#include <QObject>
#include <QPair>
#include <QPointer>
#include <QRunnable>
#include <QScopedPointer>
#include <QSet>
#include <QSharedPointer>
#include <QSignalSpy>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QThread>
#include <QThreadPool>
#include <QUrl>

#include <atomic>
#include <memory>

#include "stub_ext/stubext.h"

#include "imageinfo.h"
#include "thumbnailcache.h"
#include "types.h"
#include "unionimage/unionimage.h"

// ═══════════════ imageinfo.cpp 内部隐藏类镜像声明 ═══════════════

// ImageInfoData 镜像（数据成员顺序与源码一致：path/type/size/frameIndex/frameCount/exist/scale/x/y）
class ImageInfoData
{
public:
    typedef QSharedPointer<ImageInfoData> Ptr;

    // 真实符号：imageinfo.cpp 内 refreshDataFromCache odr-use，已发射，跨 TU 可链
    bool isError() const;

    // 源码为类内 inline 且无调用点（in_degree=0），符号未发射无法链接；
    // 以下为源码逐字拷贝（imageinfo.cpp:31-46），仅供回归，不产生对 imageinfo.cpp 的行覆盖
    Ptr cloneWithoutFrame()
    {
        Ptr other(new ImageInfoData);
        other->path = this->path;
        other->type = this->type;
        other->size = this->size;
        other->frameIndex = this->frameIndex;
        other->frameCount = this->frameCount;

        other->scale = this->scale;
        other->x = this->x;
        other->y = this->y;
        return other;
    }

    QString path;
    Types::ImageType type = Types::NullImage;
    QSize size;
    int frameIndex = 0;
    int frameCount = 0;
    bool exist = false;
    qreal scale = -1;
    qreal x = 0;
    qreal y = 0;
};

// ImageInfoCache 镜像（继承 QObject；构造/析构仅声明 → 调用 imageinfo.cpp 真实符号，
// vptr 指向真实 vtable，string-based connect 可命中真实 metaobject 信号）
class ImageInfoCache : public QObject
{
public:
    typedef QPair<QString, int> KeyType;

    ImageInfoCache();
    ~ImageInfoCache() override;

    ImageInfoData::Ptr find(const QString &path, int frameIndex);
    void loadFinished(const QString &path, int frameIndex, ImageInfoData::Ptr data);
    void removeCache(const QString &path, int frameIndex);
    void clearCache();
    void load(const QString &path, int frameIndex, bool reload = false);

    // 信号定义在 imageinfo.moc（真实符号）；镜像内不得 odr-use（无 Q_OBJECT）
Q_SIGNALS:
    void imageDataChanged(const QString &path, int frameIndex);
    void imageSizeChanged(const QString &path, int frameIndex);

private:
    bool aboutToQuit { false };
    QHash<KeyType, ImageInfoData::Ptr> cache;
    QSet<KeyType> waitSet;
    QScopedPointer<QThreadPool> localPoolPtr;
};

// LoadImageInfoRunnable 镜像（QRunnable 派生，非 QObject，无 meta 风险）
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

// imageinfo.cpp 自由函数（外部链接，直接声明链接真实符号）
Types::ImageType imageTypeAdapator(imageViewerSpace::ImageType type);

// ═══════════════ 公共辅助 ═══════════════

namespace {

// 全局 CacheInstance() 指针缓存。
// 捕获方式：拦截 ImageInfoCache::removeCache 的首次调用取 self（clearCurrentCache 同步触发）。
// 禁止 stub QCoreApplication::postEvent——QDBusConnection 后台线程随时会调它，
// 函数入口补丁与后台线程竞争会导致野跳转 SEGV（本机实测复现，等同简报坑 4 的 DBus 闯入类问题）。
ImageInfoCache *g_globalCache = nullptr;

ImageInfoCache *globalCacheInstance()
{
    if (g_globalCache)
        return g_globalCache;
    stub_ext::StubExt capture;
    capture.set_lamda(VADDR(ImageInfoCache, removeCache),
                      [](ImageInfoCache *self, const QString &, int) { g_globalCache = self; });
    ImageInfo trigger;
    trigger.imageUrl = QUrl::fromLocalFile(QStringLiteral("ut-ii-capture.png"));
    ImageInfoData::Ptr probeData(new ImageInfoData);
    probeData->type = Types::MultiImage;
    probeData->frameCount = 1;
    trigger.data = probeData;
    trigger.clearCurrentCache();  // → CacheInstance()->removeCache(...)，被拦截并捕获 self
    return g_globalCache;
}

// 只投递排队的方法调用事件（QEvent::MetaCall）。
// 不限定接收者也不触碰 DBus 事件：DBus 的 socket/timer 事件不属于 MetaCall，
// 悬空对象的排队事件已由 QObject 析构自动摘除，安全。
void deliverQueuedNotifications()
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::MetaCall);
}

// 同步等待状态变化：sync 加载模式下唯一不确定点是排队投递，循环投递即可在毫秒级收敛
bool waitUntilStatus(ImageInfo &info, ImageInfo::Status target, int timeoutMs = 5000)
{
    if (info.status() == target)
        return true;
    QElapsedTimer timer;
    timer.start();
    while (!timer.hasExpired(timeoutMs)) {
        deliverQueuedNotifications();
        if (info.status() == target)
            return true;
        QThread::msleep(10);
    }
    return info.status() == target;
}

// 在临时目录生成指定尺寸实心 PNG，返回本地绝对路径
QString makeTempPng(const QTemporaryDir &dir, const QString &name, int w = 12, int h = 8)
{
    const QString path = dir.filePath(name);
    QImage img(w, h, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(path, "PNG");
    return path;
}

// 生成存在但内容非图片的坏文件
QString makeGarbageFile(const QTemporaryDir &dir, const QString &name)
{
    const QString path = dir.filePath(name);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
        file.write("this is not an image payload");
    return path;
}

// 重置全局隐藏单例状态，防止跨用例污染（aboutToQuit 复位 + 双缓存清空）
void resetGlobalCacheState()
{
    if (ImageInfoCache *inst = globalCacheInstance())
        inst->aboutToQuit = false;
    ImageInfo::clearCache();
}

// 拦截 ImageInfoCache::removeCache 计数（clearCurrentCache 用例的观测量；
// 无需拿到全局实例指针，也无须 string-spy 全局信号）
struct RemoveCacheCalls
{
    int count = 0;
    QStringList paths;
    QList<int> frameIndexes;
};

void stubRemoveCacheCalls(stub_ext::StubExt &stub, RemoveCacheCalls &calls)
{
    stub.set_lamda(VADDR(ImageInfoCache, removeCache),
                   [&calls](ImageInfoCache *, const QString &path, int frameIndex) {
                       calls.paths.append(path);
                       calls.frameIndexes.append(frameIndex);
                       ++calls.count;
                   });
}

// 捕获 notifyFinished 排队送达的 (path, frameIndex, data) 三元组
struct CapturedNotification
{
    int count = 0;
    QString path;
    int frameIndex = -1;
    bool got() const { return count > 0; }
    ImageInfoData::Ptr data;
};

// imageTypeAdapator 参数化用例：枚举值 → 期望映射
struct ImageTypeCase
{
    imageViewerSpace::ImageType in;
    Types::ImageType expected;
};

// ImageInfoData::isError 参数化用例：类型 × 存在性 → 期望
struct IsErrorCase
{
    Types::ImageType type;
    bool exist;
    bool expected;
};

}  // namespace

// ═══════════════ 复杂方法分支清单（来源：get_code_snippet 真实源码）═══════════════

// 分支清单（来源：ImageInfo::clearCurrentCache，共 2 分支）
// B1: if (data) → 进入清理；data 为空 → 整体跳过
// B2: for (int i = 0; i < data->frameCount; ++i) → 逐帧 removeCache（0 帧即 0 次）
// 用例映射：
// - ClearCurrentCache_WithoutData_SkipsRemovalLoop → B1 为假
// - ClearCurrentCache_LoadedStaticImage_NoFrameEntriesRemoved → B1 真 + B2 循环 0 次（frameCount=0）
// - ClearCurrentCache_MultiFrameData_RemovesEachFrameEntry → B1 真 + B2 循环 3 次
//
// 分支清单（来源：ImageInfo::exists，共 1 分支）
// B1: data ? data->exist : false（空数据 → false）
// 用例映射：
// - Exists_WithoutData_ReturnsFalse → B1 假侧
// - Exists_LoadedImage_ReturnsTrue → B1 真侧
//
// 分支清单（来源：ImageInfo::frameCount，共 1 分支）
// B1: data ? data->frameCount : 1（空数据按单帧处理）
// 用例映射：
// - FrameCount_WithoutData_ReturnsOne → B1 假侧
// - FrameCount_LoadedStaticImage_ReturnsZero → B1 真侧（静态图 run() 不写 frameCount，保持默认 0）
//
// 分支清单（来源：ImageInfo::hasCachedThumbnail，共 6 分支）
// B1: imageUrl.isEmpty() → return false
// B2: case Types::NullImage → return false
// B3: case Types::DamagedImage → return false
// B4: default → ThumbnailCache::contains(toLocalFile, frameIndex)
// B5: B1 命中后的提前 return false
// B6: 末尾 return ret（B4 查询结果直接透传）
// 用例映射：
// - HasCachedThumbnail_EmptyUrl_ReturnsFalse → B1+B5
// - HasCachedThumbnail_MissingImageFile_ReturnsFalse → B2+B5
// - HasCachedThumbnail_DamagedImageType_ReturnsFalse → B3+B5
// - HasCachedThumbnail_LoadedImage_ReturnsTrue → B4+B6（contains 真）
//
// 分支清单（来源：ImageInfo::height，共 1 分支）
// B1: data ? data->size.height() : -1
// 用例映射：
// - Height_WithoutData_ReturnsMinusOne → B1 假侧
// - Height_LoadedImage_ReturnsSourceHeight → B1 真侧
//
// 分支清单（来源：ImageInfo::onLoadFinished，共 1 分支）
// B1: imageUrl.toLocalFile() == path && currentIndex == frameIndex → refreshDataFromCache(false)
// 用例映射：
// - OnLoadFinished_MatchingPathAndFrame_RefreshesFromCache → B1 真
// - OnLoadFinished_MismatchedPath_SkipsRefresh → B1 假
//
// 分支清单（来源：ImageInfo::onSizeChanged，共 2 分支）
// B1: imageUrl.toLocalFile() == path && currentIndex == frameIndex → 进入
// B2: if (data) → Q_EMIT widthChanged/heightChanged
// 用例映射：
// - OnSizeChanged_MatchingPathWithData_EmitsSizeSignals → B1 真 + B2 真
// - OnSizeChanged_WithoutMatchOrData_EmitsNothing → B1 假 / B2 假
//
// 分支清单（来源：ImageInfo::refreshDataFromCache，共 5 分支）
// B1: localPath.isEmpty() → setStatus(Error) 后 return
// B2: if (newData) → 有缓存数据
// B3: if (data)（旧数据存在）→ updateData(newData)，变更才 emit infoChanged
// B4: if (updateData(newData)) → emit infoChanged（B3 假时 data=newData 并直接 emit）
// B5: if (reload) → setStatus(Loading) + 发起 load；否则 setStatus(Error)
// 用例映射：
// - RefreshDataFromCache_EmptyPath_SetsErrorImmediately → B1
// - RefreshDataFromCache_CachedDataFirstLoad_EmitsInfoChangedAndReady → B2 真 + B3 假 + B4 直接分支
// - RefreshDataFromCache_UpdatedCacheData_EmitsInfoChangedOnDifference → B2 真 + B3 真 + B4 真
// - RefreshDataFromCache_UncachedPathNoReload_SetsError → B2 假 + B5 假
// - RefreshDataFromCache_UncachedPathWithReload_EntersLoading → B2 假 + B5 真
//
// 分支清单（来源：ImageInfo::scale，共 1 分支）
// B1: data ? data->scale : -1
// 用例映射：
// - Scale_WithoutData_ReturnsMinusOne → B1 假侧
// - Scale_LoadedData_ReturnsConfiguredValue → B1 真侧
//
// 分支清单（来源：ImageInfo::setFrameIndex，共 1 分支）
// B1: currentIndex != index → 赋值 + emit frameIndexChanged + refreshDataFromCache(true)
// 用例映射：
// - SetFrameIndex_NewIndex_EmitsSignalAndReloadsFrame → B1 真
// - SetFrameIndex_SameIndex_DoesNotEmitSignal → B1 假
//
// 分支清单（来源：ImageInfo::setScale，共 1 分支）
// B1: data && data->scale != s → data->scale = s
// 用例映射：
// - SetScale_WithLoadedData_UpdatesScaleValue → B1 真（含同值短路）
// - SetScale_WithoutData_HasNoEffect → B1 假
//
// 分支清单（来源：ImageInfo::setSource，共 1 分支）
// B1: imageUrl != source → 赋值 + emit sourceChanged + refreshDataFromCache(true)
// 用例映射：
// - SetSource_NewUrl_EmitsSignalAndStartsLoading → B1 真
// - SetSource_SameUrl_SkipsRefreshAndSignal → B1 假
//
// 分支清单（来源：ImageInfo::setStatus，共 1 分支）
// B1: imageStatus != status → 赋值 + emit statusChanged
// 用例映射：
// - SetStatus_NewStatus_EmitsStatusChanged → B1 真
// - SetStatus_SameStatus_DoesNotEmitAgain → B1 假
//
// 分支清单（来源：ImageInfo::setX，共 1 分支）
// B1: if (data) → data->x = x
// 用例映射：
// - SetX_WithLoadedData_UpdatesPosition → B1 真
// - SetX_WithoutData_KeepsZero → B1 假
//
// 分支清单（来源：ImageInfo::setY，共 1 分支）
// B1: if (data) → data->y = y
// 用例映射：
// - SetY_WithLoadedData_UpdatesPosition → B1 真
// - SetY_WithoutData_KeepsZero → B1 假
//
// 分支清单（来源：ImageInfo::swapWidthAndHeight，共 1 分支）
// B1: if (data) → size 交换 + 广播 CacheInstance()->imageSizeChanged
// 用例映射：
// - SwapWidthAndHeight_WithoutData_HasNoEffect → B1 假
// - SwapWidthAndHeight_LoadedImage_SwapsDimensionsAndNotifies → B1 真
//
// 分支清单（来源：ImageInfo::type，共 1 分支）
// B1: data ? data->type : Types::NullImage
// 用例映射：
// - Type_WithoutData_ReturnsNullImage → B1 假侧
// - Type_LoadedPng_ReturnsNormalImage → B1 真侧
//
// 分支清单（来源：ImageInfo::updateData，共 7 分支）
// B1: newData == data → return false（同指针/双空）
// B2: oldData->type != newData->type → emit typeChanged
// B3: oldData->size != newData->size → emit widthChanged + heightChanged
// B4: oldData->frameIndex != newData->frameIndex → emit frameIndexChanged
// B5: oldData->frameCount != newData->frameCount → emit frameCountChanged
// B6: oldData->exist != newData->exist → emit existsChanged
// B7: B1 命中 → 提前 return false（无任何信号）
// 用例映射：
// - UpdateData_SamePointer_ReturnsFalseWithoutSignals → B1+B7
// - UpdateData_IdenticalContent_ReturnsFalseWithoutSignals → B2~B6 全假
// - UpdateData_FieldDifferences_EmitsMatchingSignalsAndReturnsTrue → B2~B6 全真
//
// 分支清单（来源：ImageInfo::width，共 1 分支）
// B1: data ? data->size.width() : -1
// 用例映射：
// - Width_WithoutData_ReturnsMinusOne → B1 假侧
// - Width_LoadedImage_ReturnsSourceWidth → B1 真侧
//
// 分支清单（来源：ImageInfo::x，共 1 分支）
// B1: data ? data->x : 0
// 用例映射：
// - X_WithoutData_ReturnsZero → B1 假侧（真侧由 SetX_WithLoadedData 断言）
//
// 分支清单（来源：ImageInfo::y，共 1 分支）
// B1: data ? data->y : 0
// 用例映射：
// - Y_WithoutData_ReturnsZero → B1 假侧（真侧由 SetY_WithLoadedData 断言）
//
// 分支清单（来源：ImageInfoCache::load，共 6 分支）
// B1: aboutToQuit → return（退出期不再调度）
// B2: waitSet.contains(key) → return（去重，加载中不重复入队）
// B3: !reload && cache.contains(key) → return（已有缓存且不强制重载）
// B4: !GlobalControl::enableMultiThread() → 栈上 LoadImageInfoRunnable 同步 run()
// B5: else → localPoolPtr->start(runnable, LowPriority) 异步执行
// B6: reload == true 绕过 B3 缓存命中，强制重新加载
// 用例映射：
// - Load_AboutToQuit_SkipsScheduling → B1
// - Load_AlreadyWaiting_SkipsDuplicateLoad → B2（含不同 frameIndex 键不同边界）
// - Load_CachedWithoutReload_SkipsLoad → B3
// - Load_CachedWithReload_ForcesReload → B6
// - Load_SyncMode_RunsLoaderInline → B4
// - Load_AsyncMode_ExecutesRunnableInPool → B5
//
// 分支清单（来源：ImageInfoCache::loadFinished，共 3 分支）
// B1: aboutToQuit → return（退出期不更新缓存）
// B2: if (data) → cache.insert；else 仅告警
// B3: waitSet.size() % 8 == 0 → ::malloc_trim(0)（空 waitSet 的 0%8==0 也命中）
// 用例映射：
// - LoadFinished_AboutToQuit_SkipsUpdate → B1
// - LoadFinished_ValidData_CachesAndEmitsSignal → B2 真（+B3）
// - LoadFinished_NullData_EmitsWithoutCaching → B2 假（+B3）
//
// 分支清单（来源：LoadImageInfoRunnable::loadImage，共 4 分支）
// B1: orig.isValid()（QImageReader 能给尺寸）→ setScaledSize 缩略解码
// B2: !image.isNull()（缩略解码成功）→ return true
// B3: if (ret)（LibUnionImage 全图加载成功）→ 缩放 100x100 + return true
// B4: B2 命中 → 提前 return true（不走回退路径）
// 用例映射：
// - LoadImage_ValidPng_ReturnsTrueWithSourceSizeAndThumbnail → B1+B2+B4
// - LoadImage_MissingFile_ReturnsFalseWithNullImage → B1 假（+B3 假）
// - LoadImage_UnreadableContent_FallsBackAndFails → B1 假 + B3 假
//
// 分支清单（来源：LoadImageInfoRunnable::run，共 12 分支）
// B1: qApp->closingDown() → return（应用退出中）
// B2: !data->exist（文件不存在）→ 判定类型后 notifyFinished 提前返回
// B3: ThumbnailCache::contains(path) ? NonexistImage : NullImage（不存在文件的两类）
// B4: Types::NullImage == data->type → notifyFinished 提前返回
// B5: Types::MultiImage == data->type → jumpToImage 帧读取路径
// B6: 多页图帧读取失败（image.isNull()）→ type = DamagedImage 提前返回
// B7: else if (0 != frameIndex)（非多页图带帧索引）→ type = DamagedImage 提前返回
// B8: loadImage 成功 → ThumbnailCache::add
// B9: loadImage 失败 → type = DamagedImage（继续走 notifyFinished）
// B10: B2 命中后的提前 return
// B11: B4 命中后的提前 return
// B12: B6 命中后的提前 return
// 用例映射：
// - Run_ApplicationClosingDown_SkipsNotification → B1
// - Run_MissingFile_NotifiesNullImageType → B2+B3(contains 假)+B10
// - Run_DeletedFileWithCachedThumbnail_NotifiesNonexistImage → B2+B3(contains 真)+B10
// - Run_ValidPng_NotifiesLoadedImageData → B8（B5/B7 假）
// - Run_NonMultiImageWithNonZeroFrame_NotifiesDamagedImage → B7
// - Run_UnreadableStaticContent_NotifiesDamagedImage → B9
// - Run_ForcedMultiImage_ReadsFrameCountAndCachesThumbnail → B5（读帧成功）
// - Run_ForcedMultiImageUnreadableFrame_NotifiesDamagedImage → B5+B6+B12
//
// 分支清单（来源：imageTypeAdapator，共 7 分支）
// B1: case ImageTypeBlank → Types::NullImage
// B2: case ImageTypeSvg → Types::SvgImage
// B3: case ImageTypeStatic → Types::NormalImage
// B4: case ImageTypeDynamic → Types::DynamicImage
// B5: case ImageTypeMulti → Types::MultiImage
// B6: default（含 ImageTypeDamaged 及越界值）→ Types::DamagedImage
// B7: 每个 case/default 提前 return
// 用例映射：
// - ImageTypeAdapator_AllValues_MapToExpectedTypes（TEST_P ×7）→ B1~B7

// ═══════════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F/TEST_P 均包含 // Arrange / // Act / // Assert 三段注释

// ─────────────────────── ImageInfo ───────────────────────

class ImageInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        resetGlobalCacheState();
        // 默认强制同步加载（idealThreadCount=1 → enableMultiThread()=false），消除线程池时序抖动
        stub.set_lamda(static_cast<int (*)()>(&QThread::idealThreadCount), []() { return 1; });
        info = nullptr;
    }

    void TearDown() override
    {
        delete info;
        info = nullptr;
        ImageInfo::clearCache();
        stub.clear();
    }

    // 构造 + 同步加载一张真实 PNG 并等待 Ready
    ImageInfo *makeLoadedInfo(const QString &path)
    {
        info = new ImageInfo(QUrl::fromLocalFile(path));
        EXPECT_TRUE(waitUntilStatus(*info, ImageInfo::Ready));
        return info;
    }

    stub_ext::StubExt stub;
    ImageInfo *info = nullptr;
    QTemporaryDir tempDir;
};

// ── 构造 / 析构 ──

TEST_F(ImageInfoTest, ImageInfo_ConstructionWithSource_TriggersLoadAndReachesReady)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("ctor.png"));

    // Act
    ImageInfo loaded(QUrl::fromLocalFile(path));

    // Assert：构造即触发加载链（Null → Loading → 投递后 Ready）
    EXPECT_EQ(loaded.source(), QUrl::fromLocalFile(path));
    EXPECT_NE(loaded.status(), ImageInfo::Null);
    ASSERT_TRUE(waitUntilStatus(loaded, ImageInfo::Ready));
    EXPECT_EQ(loaded.type(), Types::NormalImage);
    EXPECT_EQ(loaded.width(), 12);
    EXPECT_EQ(loaded.height(), 8);
    EXPECT_TRUE(loaded.exists());
}

TEST_F(ImageInfoTest, ImageInfo_DefaultConstruction_ReturnsUnloadedDefaults)
{
    // Arrange
    ImageInfo blank;
    const int frameIndex = blank.frameIndex();

    // Act
    const ImageInfo::Status status = blank.status();

    // Assert：空数据下各 getter 的默认分支
    EXPECT_EQ(status, ImageInfo::Null);
    EXPECT_TRUE(blank.source().isEmpty());
    EXPECT_EQ(frameIndex, 0);
    EXPECT_EQ(blank.frameCount(), 1);
    EXPECT_EQ(blank.type(), Types::NullImage);
    EXPECT_FALSE(blank.exists());
}

TEST_F(ImageInfoTest, ImageInfo_DestructionWithLoadedData_CompletesCleanly)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("dtor.png"));
    ImageInfo *victim = new ImageInfo(QUrl::fromLocalFile(path));
    ASSERT_TRUE(waitUntilStatus(*victim, ImageInfo::Ready));
    QPointer<ImageInfo> victimWatcher(victim);
    QSignalSpy destroyedSpy(victim, &QObject::destroyed);

    // Act
    delete victim;

    // Assert：destroyed(QObject*) 的指针载荷经 QVariant→toLongLong 转换不可靠
    // （实测返回 0），改用 QPointer 置空证明对象确实析构
    EXPECT_EQ(destroyedSpy.count(), 1);
    EXPECT_TRUE(victimWatcher.isNull());
}

// ── setSource / source ──

TEST_F(ImageInfoTest, SetSource_NewUrl_EmitsSignalAndStartsLoading)
{
    // Arrange
    ImageInfo blank;
    QSignalSpy sourceSpy(&blank, &ImageInfo::sourceChanged);

    // Act
    blank.setSource(QUrl::fromLocalFile(makeTempPng(tempDir, QStringLiteral("setsrc.png"))));

    // Assert：B1 真 → 信号 + 立即进入 Loading
    EXPECT_EQ(sourceSpy.count(), 1);
    EXPECT_EQ(blank.status(), ImageInfo::Loading);
    EXPECT_FALSE(blank.source().isEmpty());
}

TEST_F(ImageInfoTest, SetSource_SameUrl_SkipsRefreshAndSignal)
{
    // Arrange
    const QUrl url = QUrl::fromLocalFile(makeTempPng(tempDir, QStringLiteral("same.png")));
    ImageInfo blank(url);
    QSignalSpy sourceSpy(&blank, &ImageInfo::sourceChanged);
    const ImageInfo::Status before = blank.status();

    // Act
    blank.setSource(url);

    // Assert：B1 假 → 无信号、状态不变（强异常安全）
    EXPECT_EQ(sourceSpy.count(), 0);
    EXPECT_EQ(blank.status(), before);
}

TEST_F(ImageInfoTest, Source_DefaultConstruction_ReturnsEmptyUrl)
{
    // Arrange
    ImageInfo blank;

    // Act
    const QUrl ret = blank.source();

    // Assert
    EXPECT_TRUE(ret.isEmpty());
    EXPECT_EQ(ret.toLocalFile(), QString());
}

TEST_F(ImageInfoTest, Source_AfterSetSource_ReturnsGivenUrl)
{
    // Arrange
    ImageInfo blank;
    const QUrl url = QUrl::fromLocalFile(makeTempPng(tempDir, QStringLiteral("getsrc.png")));

    // Act
    blank.setSource(url);

    // Assert
    EXPECT_EQ(blank.source(), url);
    EXPECT_EQ(blank.source().toLocalFile(), url.toLocalFile());
}

// ── status / setStatus ──

TEST_F(ImageInfoTest, Status_DefaultConstruction_ReturnsNull)
{
    // Arrange
    ImageInfo blank;

    // Act
    const ImageInfo::Status ret = blank.status();

    // Assert
    EXPECT_EQ(ret, ImageInfo::Null);
    EXPECT_NE(ret, ImageInfo::Ready);
}

TEST_F(ImageInfoTest, Status_AfterLoading_TransitionsToReady)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("st.png"));
    ImageInfo loaded(QUrl::fromLocalFile(path));

    // Act
    ASSERT_TRUE(waitUntilStatus(loaded, ImageInfo::Ready));

    // Assert
    EXPECT_EQ(loaded.status(), ImageInfo::Ready);
    EXPECT_TRUE(loaded.exists());
}

TEST_F(ImageInfoTest, SetStatus_NewStatus_EmitsStatusChanged)
{
    // Arrange
    ImageInfo blank;
    QSignalSpy statusSpy(&blank, &ImageInfo::statusChanged);

    // Act
    blank.setStatus(ImageInfo::Loading);

    // Assert：B1 真
    EXPECT_EQ(statusSpy.count(), 1);
    EXPECT_EQ(blank.status(), ImageInfo::Loading);
}

TEST_F(ImageInfoTest, SetStatus_SameStatus_DoesNotEmitAgain)
{
    // Arrange
    ImageInfo blank;
    blank.setStatus(ImageInfo::Loading);
    QSignalSpy statusSpy(&blank, &ImageInfo::statusChanged);

    // Act
    blank.setStatus(ImageInfo::Loading);

    // Assert：B1 假 → 无信号且状态保持
    EXPECT_EQ(statusSpy.count(), 0);
    EXPECT_EQ(blank.status(), ImageInfo::Loading);
}

// ── 数据型 getter（空/已加载两侧）──

TEST_F(ImageInfoTest, FrameCount_WithoutData_ReturnsOne)
{
    // Arrange
    ImageInfo blank;

    // Act
    const int ret = blank.frameCount();

    // Assert：B1 假侧 → 空数据按单帧
    EXPECT_EQ(ret, 1);
    EXPECT_GT(ret, 0);
}

TEST_F(ImageInfoTest, FrameCount_LoadedStaticImage_ReturnsZero)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("fc.png"));
    ImageInfo *loaded = makeLoadedInfo(path);

    // Act
    const int ret = loaded->frameCount();

    // Assert：B1 真侧 → run() 静态路径不写 frameCount，保持默认 0
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(loaded->type(), Types::NormalImage);
}

TEST_F(ImageInfoTest, FrameIndex_DefaultConstruction_ReturnsZero)
{
    // Arrange
    ImageInfo blank;

    // Act
    const int ret = blank.frameIndex();

    // Assert
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(blank.data.isNull());
}

TEST_F(ImageInfoTest, FrameIndex_AfterUpdate_ReturnsCurrentIndex)
{
    // Arrange
    ImageInfo blank;

    // Act
    blank.currentIndex = 2;

    // Assert
    EXPECT_EQ(blank.frameIndex(), 2);
    EXPECT_NE(blank.frameIndex(), 0);
}

TEST_F(ImageInfoTest, Width_WithoutData_ReturnsMinusOne)
{
    // Arrange
    ImageInfo blank;

    // Act
    const int ret = blank.width();

    // Assert：B1 假侧
    EXPECT_EQ(ret, -1);
    EXPECT_LT(ret, 0);
}

TEST_F(ImageInfoTest, Width_LoadedImage_ReturnsSourceWidth)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("w.png"), 20, 10);
    ImageInfo *loaded = makeLoadedInfo(path);

    // Act
    const int ret = loaded->width();

    // Assert：B1 真侧
    EXPECT_EQ(ret, 20);
    EXPECT_EQ(loaded->height(), 10);
}

TEST_F(ImageInfoTest, Height_WithoutData_ReturnsMinusOne)
{
    // Arrange
    ImageInfo blank;

    // Act
    const int ret = blank.height();

    // Assert：B1 假侧
    EXPECT_EQ(ret, -1);
    EXPECT_LT(ret, 0);
}

TEST_F(ImageInfoTest, Height_LoadedImage_ReturnsSourceHeight)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("h.png"), 20, 10);
    ImageInfo *loaded = makeLoadedInfo(path);

    // Act
    const int ret = loaded->height();

    // Assert：B1 真侧
    EXPECT_EQ(ret, 10);
    EXPECT_EQ(loaded->width(), 20);
}

TEST_F(ImageInfoTest, Type_WithoutData_ReturnsNullImage)
{
    // Arrange
    ImageInfo blank;

    // Act
    const int ret = blank.type();

    // Assert：B1 假侧
    EXPECT_EQ(ret, Types::NullImage);
    EXPECT_NE(ret, Types::NormalImage);
}

TEST_F(ImageInfoTest, Type_LoadedPng_ReturnsNormalImage)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("t.png"));
    ImageInfo *loaded = makeLoadedInfo(path);

    // Act
    const int ret = loaded->type();

    // Assert：B1 真侧
    EXPECT_EQ(ret, Types::NormalImage);
    EXPECT_NE(ret, Types::NullImage);
}

TEST_F(ImageInfoTest, Scale_WithoutData_ReturnsMinusOne)
{
    // Arrange
    ImageInfo blank;

    // Act
    const qreal ret = blank.scale();

    // Assert：B1 假侧
    EXPECT_DOUBLE_EQ(ret, -1.0);
    EXPECT_LT(ret, 0);
}

TEST_F(ImageInfoTest, Scale_LoadedData_ReturnsConfiguredValue)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("sc.png"));
    ImageInfo *loaded = makeLoadedInfo(path);

    // Act
    loaded->setScale(2.5);

    // Assert：B1 真侧（setX/setScale 写入后 getter 读回）
    EXPECT_DOUBLE_EQ(loaded->scale(), 2.5);
    EXPECT_GT(loaded->scale(), 0);
}

TEST_F(ImageInfoTest, X_WithoutData_ReturnsZero)
{
    // Arrange
    ImageInfo blank;

    // Act
    const qreal ret = blank.x();

    // Assert：B1 假侧
    EXPECT_DOUBLE_EQ(ret, 0.0);
    EXPECT_TRUE(blank.data.isNull());
}

TEST_F(ImageInfoTest, Y_WithoutData_ReturnsZero)
{
    // Arrange
    ImageInfo blank;

    // Act
    const qreal ret = blank.y();

    // Assert：B1 假侧
    EXPECT_DOUBLE_EQ(ret, 0.0);
    EXPECT_TRUE(blank.data.isNull());
}

TEST_F(ImageInfoTest, Exists_WithoutData_ReturnsFalse)
{
    // Arrange
    ImageInfo blank;

    // Act
    const bool ret = blank.exists();

    // Assert：B1 假侧
    EXPECT_FALSE(ret);  // branch: data 为空 → exist=false
    EXPECT_EQ(blank.data.data(), nullptr);
}

TEST_F(ImageInfoTest, Exists_LoadedImage_ReturnsTrue)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("ex.png"));
    ImageInfo *loaded = makeLoadedInfo(path);

    // Act
    const bool ret = loaded->exists();

    // Assert：B1 真侧
    EXPECT_TRUE(ret);   // branch: data->exist == true
    EXPECT_EQ(loaded->status(), ImageInfo::Ready);
}

// ── setFrameIndex / setScale / setX / setY ──

TEST_F(ImageInfoTest, SetFrameIndex_NewIndex_EmitsSignalAndReloadsFrame)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("sfi.png"));
    ImageInfo *loaded = makeLoadedInfo(path);
    QSignalSpy frameSpy(loaded, &ImageInfo::frameIndexChanged);

    // Act
    loaded->setFrameIndex(1);  // 非多页图带非 0 帧索引 → run() 判 DamagedImage

    // Assert：B1 真 → 信号 + 重载（Loading → Error）
    EXPECT_EQ(frameSpy.count(), 1);
    EXPECT_EQ(loaded->frameIndex(), 1);
    ASSERT_TRUE(waitUntilStatus(*loaded, ImageInfo::Error));
    EXPECT_EQ(loaded->type(), Types::DamagedImage);
}

TEST_F(ImageInfoTest, SetFrameIndex_SameIndex_DoesNotEmitSignal)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("sfi0.png"));
    ImageInfo *loaded = makeLoadedInfo(path);
    QSignalSpy frameSpy(loaded, &ImageInfo::frameIndexChanged);
    const ImageInfo::Status before = loaded->status();

    // Act
    loaded->setFrameIndex(0);  // 当前已是 0

    // Assert：B1 假 → 无信号、状态不变
    EXPECT_EQ(frameSpy.count(), 0);
    EXPECT_EQ(loaded->status(), before);
}

TEST_F(ImageInfoTest, SetScale_WithLoadedData_UpdatesScaleValue)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("ssc.png"));
    ImageInfo *loaded = makeLoadedInfo(path);

    // Act
    loaded->setScale(1.5);
    const qreal afterFirst = loaded->scale();
    loaded->setScale(1.5);  // 同值重复：短路不更新

    // Assert：B1 真 + 同值路径
    EXPECT_DOUBLE_EQ(afterFirst, 1.5);
    EXPECT_DOUBLE_EQ(loaded->scale(), 1.5);
}

TEST_F(ImageInfoTest, SetScale_WithoutData_HasNoEffect)
{
    // Arrange
    ImageInfo blank;

    // Act
    blank.setScale(3.0);

    // Assert：B1 假 → 无数据可写
    EXPECT_DOUBLE_EQ(blank.scale(), -1.0);
    EXPECT_DOUBLE_EQ(blank.x(), 0.0);
}

TEST_F(ImageInfoTest, SetX_WithLoadedData_UpdatesPosition)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("sx.png"));
    ImageInfo *loaded = makeLoadedInfo(path);

    // Act
    loaded->setX(7.5);

    // Assert：B1 真
    EXPECT_DOUBLE_EQ(loaded->x(), 7.5);
    EXPECT_DOUBLE_EQ(loaded->y(), 0.0);
}

TEST_F(ImageInfoTest, SetX_WithoutData_KeepsZero)
{
    // Arrange
    ImageInfo blank;

    // Act
    blank.setX(9.0);

    // Assert：B1 假 → getter 保持默认
    EXPECT_DOUBLE_EQ(blank.x(), 0.0);
    EXPECT_DOUBLE_EQ(blank.y(), 0.0);
}

TEST_F(ImageInfoTest, SetY_WithLoadedData_UpdatesPosition)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("sy.png"));
    ImageInfo *loaded = makeLoadedInfo(path);

    // Act
    loaded->setY(-4.25);

    // Assert：B1 真
    EXPECT_DOUBLE_EQ(loaded->y(), -4.25);
    EXPECT_DOUBLE_EQ(loaded->x(), 0.0);
}

TEST_F(ImageInfoTest, SetY_WithoutData_KeepsZero)
{
    // Arrange
    ImageInfo blank;

    // Act
    blank.setY(-1.0);

    // Assert：B1 假
    EXPECT_DOUBLE_EQ(blank.y(), 0.0);
    EXPECT_DOUBLE_EQ(blank.x(), 0.0);
}

// ── swapWidthAndHeight ──

TEST_F(ImageInfoTest, SwapWidthAndHeight_WithoutData_HasNoEffect)
{
    // Arrange
    ImageInfo blank;

    // Act
    blank.swapWidthAndHeight();

    // Assert：B1 假 → 不崩溃且保持默认
    EXPECT_EQ(blank.width(), -1);
    EXPECT_EQ(blank.height(), -1);
}

TEST_F(ImageInfoTest, SwapWidthAndHeight_LoadedImage_SwapsDimensionsAndNotifies)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("swap.png"), 12, 8);
    ImageInfo *loaded = makeLoadedInfo(path);
    QSignalSpy widthSpy(loaded, &ImageInfo::widthChanged);
    QSignalSpy heightSpy(loaded, &ImageInfo::heightChanged);

    // Act
    loaded->swapWidthAndHeight();

    // Assert：B1 真 → 尺寸交换；imageSizeChanged 广播回到自身 onSizeChanged → 双信号
    EXPECT_EQ(loaded->width(), 8);
    EXPECT_EQ(loaded->height(), 12);
    EXPECT_EQ(widthSpy.count(), 1);
    EXPECT_EQ(heightSpy.count(), 1);
}

// ── onLoadFinished / onSizeChanged ──

TEST_F(ImageInfoTest, OnLoadFinished_MatchingPathAndFrame_RefreshesFromCache)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("olf.png"));
    ImageInfo *donor = makeLoadedInfo(path);  // 先让全局缓存持有数据
    ASSERT_EQ(donor->status(), ImageInfo::Ready);
    ImageInfo fresh;
    fresh.imageUrl = QUrl::fromLocalFile(path);  // 直写 protected 成员，绕开自动加载
    QSignalSpy infoSpy(&fresh, &ImageInfo::infoChanged);

    // Act
    fresh.onLoadFinished(path, 0);

    // Assert：B1 真 → 从缓存取数、首次挂载数据发 infoChanged
    EXPECT_EQ(fresh.status(), ImageInfo::Ready);
    EXPECT_EQ(infoSpy.count(), 1);
    EXPECT_EQ(fresh.width(), 12);
}

TEST_F(ImageInfoTest, OnLoadFinished_MismatchedPath_SkipsRefresh)
{
    // Arrange
    ImageInfo blank;
    blank.imageUrl = QUrl::fromLocalFile(makeTempPng(tempDir, QStringLiteral("olf2.png")));
    QSignalSpy infoSpy(&blank, &ImageInfo::infoChanged);

    // Act
    blank.onLoadFinished(QStringLiteral("a-totally-different-path.png"), 0);

    // Assert：B1 假 → 不刷新、状态保持 Null
    EXPECT_EQ(blank.status(), ImageInfo::Null);
    EXPECT_EQ(infoSpy.count(), 0);
}

TEST_F(ImageInfoTest, OnSizeChanged_MatchingPathWithData_EmitsSizeSignals)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("osc.png"));
    ImageInfo *loaded = makeLoadedInfo(path);
    QSignalSpy widthSpy(loaded, &ImageInfo::widthChanged);
    QSignalSpy heightSpy(loaded, &ImageInfo::heightChanged);

    // Act
    loaded->onSizeChanged(path, 0);

    // Assert：B1 真 + B2 真 → 双信号
    EXPECT_EQ(widthSpy.count(), 1);
    EXPECT_EQ(heightSpy.count(), 1);
    EXPECT_EQ(loaded->width(), 12);
}

TEST_F(ImageInfoTest, OnSizeChanged_WithoutMatchOrData_EmitsNothing)
{
    // Arrange
    ImageInfo blank;
    blank.imageUrl = QUrl::fromLocalFile(makeTempPng(tempDir, QStringLiteral("osc2.png")));
    QSignalSpy widthSpy(&blank, &ImageInfo::widthChanged);
    QSignalSpy heightSpy(&blank, &ImageInfo::heightChanged);

    // Act
    blank.onSizeChanged(QStringLiteral("mismatch-path.png"), 0);  // B1 假
    blank.onSizeChanged(blank.imageUrl.toLocalFile(), 0);         // B1 真 + B2 假（无数据）

    // Assert
    EXPECT_EQ(widthSpy.count(), 0);
    EXPECT_EQ(heightSpy.count(), 0);
}

// ── refreshDataFromCache ──

TEST_F(ImageInfoTest, RefreshDataFromCache_EmptyPath_SetsErrorImmediately)
{
    // Arrange
    ImageInfo blank;  // imageUrl 为空
    QSignalSpy statusSpy(&blank, &ImageInfo::statusChanged);

    // Act
    blank.refreshDataFromCache(true);

    // Assert：B1 → Error
    EXPECT_EQ(blank.status(), ImageInfo::Error);
    EXPECT_GE(statusSpy.count(), 1);
}

TEST_F(ImageInfoTest, RefreshDataFromCache_CachedDataFirstLoad_EmitsInfoChangedAndReady)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("rfd.png"));
    ImageInfo *donor = makeLoadedInfo(path);  // 缓存已写入
    ASSERT_EQ(donor->status(), ImageInfo::Ready);
    ImageInfo fresh;
    fresh.imageUrl = QUrl::fromLocalFile(path);
    QSignalSpy infoSpy(&fresh, &ImageInfo::infoChanged);

    // Act
    fresh.refreshDataFromCache(false);

    // Assert：B2 真 + B3 假 → data 首挂 + infoChanged + Ready
    EXPECT_EQ(infoSpy.count(), 1);
    EXPECT_EQ(fresh.status(), ImageInfo::Ready);
    EXPECT_EQ(fresh.width(), 12);
}

TEST_F(ImageInfoTest, RefreshDataFromCache_UpdatedCacheData_EmitsInfoChangedOnDifference)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("rfd2.png"));
    ImageInfo *observer = makeLoadedInfo(path);   // data 已挂（走 updateData 对比路径）
    ImageInfoData::Ptr updated(new ImageInfoData);
    updated->path = path;
    updated->type = Types::NormalImage;
    updated->size = QSize(20, 30);
    updated->exist = true;
    QSignalSpy infoSpy(observer, &ImageInfo::infoChanged);
    QSignalSpy typeSpy(observer, &ImageInfo::typeChanged);
    auto *globalCache = globalCacheInstance();

    // Act
    globalCache->loadFinished(path, 0, updated);  // 缓存更新 → imageDataChanged → onLoadFinished

    // Assert：B2 真 + B3 真 + B4 真 → 尺寸变更触发 infoChanged，类型未变不发 typeChanged
    EXPECT_EQ(infoSpy.count(), 1);
    EXPECT_EQ(typeSpy.count(), 0);
    EXPECT_EQ(observer->width(), 20);
    EXPECT_EQ(observer->height(), 30);
}

TEST_F(ImageInfoTest, RefreshDataFromCache_UncachedPathNoReload_SetsError)
{
    // Arrange
    ImageInfo blank;
    blank.imageUrl = QUrl::fromLocalFile(makeTempPng(tempDir, QStringLiteral("rfd3.png")));

    // Act
    blank.refreshDataFromCache(false);

    // Assert：B2 假 + B5 假 → Error
    EXPECT_EQ(blank.status(), ImageInfo::Error);
    EXPECT_TRUE(blank.data.isNull());
}

TEST_F(ImageInfoTest, RefreshDataFromCache_UncachedPathWithReload_EntersLoading)
{
    // Arrange
    ImageInfo blank;
    blank.imageUrl = QUrl::fromLocalFile(makeTempPng(tempDir, QStringLiteral("rfd4.png")));
    QSignalSpy statusSpy(&blank, &ImageInfo::statusChanged);

    // Act
    blank.refreshDataFromCache(true);  // 同步 run 已执行、通知仍在队列 → 停在 Loading

    // Assert：B2 假 + B5 真 → Loading + 发起加载
    EXPECT_EQ(blank.status(), ImageInfo::Loading);
    EXPECT_GE(statusSpy.count(), 1);
}

// ── updateData ──

TEST_F(ImageInfoTest, UpdateData_SamePointer_ReturnsFalseWithoutSignals)
{
    // Arrange
    ImageInfo blank;  // data 为空
    QSignalSpy typeSpy(&blank, &ImageInfo::typeChanged);
    ImageInfoData::Ptr nullPtr;

    // Act
    const bool retNull = blank.updateData(nullPtr);
    const QString path = makeTempPng(tempDir, QStringLiteral("ud.png"));
    ImageInfo *loaded = makeLoadedInfo(path);
    const bool retSelf = loaded->updateData(loaded->data);

    // Assert：B1+B7 → 同指针（含双空）直接 false
    EXPECT_FALSE(retNull);
    EXPECT_FALSE(retSelf);
    EXPECT_EQ(typeSpy.count(), 0);
}

TEST_F(ImageInfoTest, UpdateData_IdenticalContent_ReturnsFalseWithoutSignals)
{
    // Arrange
    ImageInfo observer;
    ImageInfoData::Ptr a(new ImageInfoData);
    a->type = Types::NormalImage;
    a->size = QSize(10, 10);
    a->frameIndex = 0;
    a->frameCount = 1;
    a->exist = true;
    ImageInfoData::Ptr b(new ImageInfoData);
    b->path = a->path;
    b->type = a->type;
    b->size = a->size;
    b->frameIndex = a->frameIndex;
    b->frameCount = a->frameCount;
    b->exist = a->exist;
    observer.data = a;
    QSignalSpy typeSpy(&observer, &ImageInfo::typeChanged);
    QSignalSpy widthSpy(&observer, &ImageInfo::widthChanged);

    // Act
    const bool ret = observer.updateData(b);

    // Assert：B2~B6 全假 → false 无信号，但 data 已替换为新指针
    EXPECT_FALSE(ret);
    EXPECT_EQ(typeSpy.count(), 0);
    EXPECT_EQ(widthSpy.count(), 0);
}

TEST_F(ImageInfoTest, UpdateData_FieldDifferences_EmitsMatchingSignalsAndReturnsTrue)
{
    // Arrange
    ImageInfo observer;
    ImageInfoData::Ptr a(new ImageInfoData);
    a->type = Types::NormalImage;
    a->size = QSize(10, 10);
    a->frameIndex = 0;
    a->frameCount = 1;
    a->exist = true;
    ImageInfoData::Ptr b(new ImageInfoData);
    b->type = Types::SvgImage;
    b->size = QSize(20, 30);
    b->frameIndex = 2;
    b->frameCount = 5;
    b->exist = false;
    observer.data = a;
    QSignalSpy typeSpy(&observer, &ImageInfo::typeChanged);
    QSignalSpy widthSpy(&observer, &ImageInfo::widthChanged);
    QSignalSpy heightSpy(&observer, &ImageInfo::heightChanged);
    QSignalSpy frameIdxSpy(&observer, &ImageInfo::frameIndexChanged);
    QSignalSpy frameCntSpy(&observer, &ImageInfo::frameCountChanged);
    QSignalSpy existsSpy(&observer, &ImageInfo::existsChanged);

    // Act
    const bool ret = observer.updateData(b);

    // Assert：B2~B6 全真 → 各字段信号 + true + 新值生效
    EXPECT_TRUE(ret);
    EXPECT_EQ(typeSpy.count(), 1);
    EXPECT_EQ(widthSpy.count(), 1);
    EXPECT_EQ(heightSpy.count(), 1);
    EXPECT_EQ(frameIdxSpy.count(), 1);
    EXPECT_EQ(frameCntSpy.count(), 1);
    EXPECT_EQ(existsSpy.count(), 1);
    EXPECT_EQ(observer.type(), Types::SvgImage);
    EXPECT_EQ(observer.width(), 20);
    EXPECT_EQ(observer.frameCount(), 5);
    EXPECT_FALSE(observer.exists());
}

// ── clearCurrentCache / clearCache ──

TEST_F(ImageInfoTest, ClearCurrentCache_WithoutData_SkipsRemovalLoop)
{
    // Arrange
    ImageInfo blank;
    RemoveCacheCalls calls;
    stubRemoveCacheCalls(stub, calls);

    // Act
    blank.clearCurrentCache();

    // Assert：B1 假 → 无 removeCache 调用
    EXPECT_EQ(calls.count, 0);
    EXPECT_TRUE(blank.data.isNull());
}

TEST_F(ImageInfoTest, ClearCurrentCache_LoadedStaticImage_NoFrameEntriesRemoved)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("ccc.png"));
    ImageInfo *loaded = makeLoadedInfo(path);
    RemoveCacheCalls calls;
    stubRemoveCacheCalls(stub, calls);

    // Act
    loaded->clearCurrentCache();

    // Assert：B1 真 + B2 循环 0 次（静态图 frameCount=0，缺陷：单帧缓存条目不会被移除）
    EXPECT_EQ(loaded->frameCount(), 0);
    EXPECT_EQ(calls.count, 0);
    EXPECT_TRUE(loaded->hasCachedThumbnail());  // 缓存条目仍在
}

TEST_F(ImageInfoTest, ClearCurrentCache_MultiFrameData_RemovesEachFrameEntry)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("ccc3.png"));
    ImageInfo observer;
    observer.imageUrl = QUrl::fromLocalFile(path);
    ImageInfoData::Ptr multi(new ImageInfoData);
    multi->path = path;
    multi->type = Types::MultiImage;
    multi->size = QSize(12, 8);
    multi->frameCount = 3;
    multi->exist = true;
    observer.data = multi;
    RemoveCacheCalls calls;
    stubRemoveCacheCalls(stub, calls);

    // Act
    observer.clearCurrentCache();

    // Assert：B1 真 + B2 循环 3 次 → removeCache(path, 0/1/2)
    EXPECT_EQ(calls.count, 3);
    EXPECT_EQ(calls.frameIndexes, (QList<int>() << 0 << 1 << 2));
    EXPECT_EQ(calls.paths.at(0), path);
}

TEST_F(ImageInfoTest, ClearCache_LoadedImage_DropsCacheAndThumbnail)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("cc.png"));
    ImageInfo *loaded = makeLoadedInfo(path);
    EXPECT_TRUE(loaded->hasCachedThumbnail());  // 前置：缓存已建立

    // Act
    ImageInfo::clearCache();

    // Assert：全局缓存 + 缩略图缓存均被清空
    ImageInfo fresh;
    fresh.imageUrl = QUrl::fromLocalFile(path);
    fresh.refreshDataFromCache(false);
    EXPECT_EQ(fresh.status(), ImageInfo::Error);
    EXPECT_FALSE(loaded->hasCachedThumbnail());
}

TEST_F(ImageInfoTest, ClearCache_WhenAlreadyEmpty_KeepsDefaultState)
{
    // Arrange
    ImageInfo blank;

    // Act
    ImageInfo::clearCache();

    // Assert：空缓存下调用无副作用
    EXPECT_EQ(blank.status(), ImageInfo::Null);
    EXPECT_FALSE(blank.hasCachedThumbnail());
}

// ── hasCachedThumbnail ──

TEST_F(ImageInfoTest, HasCachedThumbnail_EmptyUrl_ReturnsFalse)
{
    // Arrange
    ImageInfo blank;

    // Act
    const bool ret = blank.hasCachedThumbnail();

    // Assert：B1
    EXPECT_FALSE(ret);  // branch: imageUrl.isEmpty()
    EXPECT_EQ(blank.source(), QUrl());
}

TEST_F(ImageInfoTest, HasCachedThumbnail_LoadedImage_ReturnsTrue)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("hct.png"));
    ImageInfo *loaded = makeLoadedInfo(path);

    // Act
    const bool ret = loaded->hasCachedThumbnail();

    // Assert：B4 default 分支 → contains 真
    EXPECT_TRUE(ret);   // branch: ThumbnailCache 命中
    EXPECT_EQ(loaded->type(), Types::NormalImage);
}

TEST_F(ImageInfoTest, HasCachedThumbnail_MissingImageFile_ReturnsFalse)
{
    // Arrange
    ImageInfo missing;
    missing.setSource(QUrl::fromLocalFile(tempDir.filePath(QStringLiteral("no-such-file.png"))));

    // Act
    ASSERT_TRUE(waitUntilStatus(missing, ImageInfo::Error));
    const bool ret = missing.hasCachedThumbnail();

    // Assert：B2 case NullImage（文件不存在且无缩略图）
    EXPECT_FALSE(ret);  // branch: type() == NullImage
    EXPECT_EQ(missing.type(), Types::NullImage);
}

TEST_F(ImageInfoTest, HasCachedThumbnail_DamagedImageType_ReturnsFalse)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("hctd.png"));
    ImageInfo *loaded = makeLoadedInfo(path);
    ASSERT_TRUE(loaded->hasCachedThumbnail());
    ImageInfoData::Ptr damaged(new ImageInfoData);
    damaged->path = path;
    damaged->type = Types::DamagedImage;
    damaged->exist = true;

    // Act
    loaded->data = damaged;  // 直写数据为 Damaged 类型
    const bool ret = loaded->hasCachedThumbnail();

    // Assert：B3 case DamagedImage（即便缩略图已缓存也返回 false）
    EXPECT_FALSE(ret);  // branch: type() == DamagedImage
    EXPECT_EQ(ThumbnailCache::instance()->contains(path, 0), true);
}

// ── reloadData ──

TEST_F(ImageInfoTest, ReloadData_LoadedImage_SetsLoadingThenReadyAgain)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("rl.png"));
    ImageInfo *loaded = makeLoadedInfo(path);
    QSignalSpy statusSpy(loaded, &ImageInfo::statusChanged);

    // Act
    loaded->reloadData();

    // Assert：强制重载 → Loading →（投递后）Ready
    EXPECT_EQ(loaded->status(), ImageInfo::Loading);
    ASSERT_TRUE(waitUntilStatus(*loaded, ImageInfo::Ready));
    EXPECT_GE(statusSpy.count(), 2);
}

TEST_F(ImageInfoTest, ReloadData_MissingFile_EndsInErrorState)
{
    // Arrange
    ImageInfo missing(QUrl::fromLocalFile(tempDir.filePath(QStringLiteral("gone.png"))));
    ASSERT_TRUE(waitUntilStatus(missing, ImageInfo::Error));

    // Act
    missing.reloadData();
    ASSERT_TRUE(waitUntilStatus(missing, ImageInfo::Error));

    // Assert：不存在文件重载仍收敛到 Error
    EXPECT_EQ(missing.type(), Types::NullImage);
    EXPECT_FALSE(missing.exists());
}

// ─────────────────────── ImageInfoCache ───────────────────────

class ImageInfoCacheTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        resetGlobalCacheState();
        obj = new ImageInfoCache();  // 真实构造符号：vptr 指向真实 vtable
    }

    void TearDown() override
    {
        delete obj;  // 真实析构符号（镜像仅声明，避免错误 vtable 发射）
        obj = nullptr;
        stub.clear();
    }

    // 强制同步（idealThreadCount=1 → enableMultiThread()=false）
    void forceSyncMode()
    {
        stub.set_lamda(static_cast<int (*)()>(&QThread::idealThreadCount), []() { return 1; });
    }

    // 强制异步（线程池路径）
    void forceAsyncMode()
    {
        stub.set_lamda(static_cast<int (*)()>(&QThread::idealThreadCount), []() { return 8; });
    }

    // 计数 LoadImageInfoRunnable::run 的执行（桩掉真实解码）
    std::atomic<int> *stubRunCounter()
    {
        runCalls = 0;
        stub.set_lamda(VADDR(LoadImageInfoRunnable, run),
                       [this](LoadImageInfoRunnable *) { ++runCalls; });
        return &runCalls;
    }

    ImageInfoData::Ptr makeData(const QString &path, Types::ImageType type = Types::NormalImage)
    {
        ImageInfoData::Ptr data(new ImageInfoData);
        data->path = path;
        data->type = type;
        data->size = QSize(12, 8);
        data->exist = true;
        return data;
    }

    stub_ext::StubExt stub;
    ImageInfoCache *obj = nullptr;
    std::atomic<int> runCalls { 0 };
    QTemporaryDir tempDir;
};

TEST_F(ImageInfoCacheTest, ImageInfoCache_Construction_InitializesPoolAndQuitHook)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：读取构造产物状态
    const int maxThreads = obj->localPoolPtr->maxThreadCount();

    // Assert：本地线程池按 qMax(2, idealThreadCount/2) 初始化；aboutToQuit 关闭
    EXPECT_GE(maxThreads, 2);
    EXPECT_FALSE(obj->aboutToQuit);
}

TEST_F(ImageInfoCacheTest, ImageInfoCache_Destruction_DisposesInstanceCleanly)
{
    // Arrange
    QSignalSpy destroyedSpy(obj, &QObject::destroyed);
    EXPECT_FALSE(obj->aboutToQuit);

    // Act
    delete obj;
    obj = nullptr;

    // Assert：真实析构符号触发 QObject::destroyed
    EXPECT_EQ(destroyedSpy.count(), 1);
}

TEST_F(ImageInfoCacheTest, Find_EmptyCache_ReturnsNullPointer)
{
    // Arrange
    const QString path = tempDir.filePath(QStringLiteral("a.png"));

    // Act
    const ImageInfoData::Ptr ret0 = obj->find(path, 0);
    const ImageInfoData::Ptr retNeg = obj->find(path, 3);

    // Assert
    EXPECT_EQ(ret0.data(), nullptr);
    EXPECT_EQ(retNeg.data(), nullptr);
}

TEST_F(ImageInfoCacheTest, Find_AfterLoadFinished_ReturnsStoredData)
{
    // Arrange
    const QString path = tempDir.filePath(QStringLiteral("cached.png"));
    const ImageInfoData::Ptr stored = makeData(path);
    obj->loadFinished(path, 0, stored);

    // Act
    const ImageInfoData::Ptr found = obj->find(path, 0);
    const ImageInfoData::Ptr otherFrame = obj->find(path, 1);

    // Assert：同键命中返回同一份数据，异键不命中
    EXPECT_EQ(found.data(), stored.data());
    EXPECT_TRUE(otherFrame.isNull());
}

TEST_F(ImageInfoCacheTest, Load_AboutToQuit_SkipsScheduling)
{
    // Arrange
    forceSyncMode();
    std::atomic<int> *calls = stubRunCounter();
    obj->aboutToQuit = true;

    // Act
    obj->load(tempDir.filePath(QStringLiteral("quit.png")), 0);

    // Assert：B1 → 不调度任何加载
    EXPECT_EQ(calls->load(), 0);
    EXPECT_TRUE(obj->find(tempDir.filePath(QStringLiteral("quit.png")), 0).isNull());
}

TEST_F(ImageInfoCacheTest, Load_AlreadyWaiting_SkipsDuplicateLoad)
{
    // Arrange
    forceSyncMode();
    std::atomic<int> *calls = stubRunCounter();
    const QString path = tempDir.filePath(QStringLiteral("dup.png"));

    // Act
    obj->load(path, 0);
    const int afterFirst = calls->load();
    obj->load(path, 0);            // B2：waitSet 已含该键
    obj->load(path, 1);            // 边界：不同 frameIndex 是不同键

    // Assert
    EXPECT_EQ(afterFirst, 1);
    EXPECT_EQ(calls->load(), 2);
}

TEST_F(ImageInfoCacheTest, Load_CachedWithoutReload_SkipsLoad)
{
    // Arrange
    forceSyncMode();
    std::atomic<int> *calls = stubRunCounter();
    const QString path = tempDir.filePath(QStringLiteral("warm.png"));
    obj->loadFinished(path, 0, makeData(path));  // 预热缓存

    // Act
    obj->load(path, 0, false);  // B3：cache 命中且不强制重载

    // Assert
    EXPECT_EQ(calls->load(), 0);
    EXPECT_FALSE(obj->find(path, 0).isNull());
}

TEST_F(ImageInfoCacheTest, Load_CachedWithReload_ForcesReload)
{
    // Arrange
    forceSyncMode();
    std::atomic<int> *calls = stubRunCounter();
    const QString path = tempDir.filePath(QStringLiteral("rewarm.png"));
    obj->loadFinished(path, 0, makeData(path));  // 预热缓存

    // Act
    obj->load(path, 0, true);  // B6：reload 绕过缓存命中

    // Assert
    EXPECT_EQ(calls->load(), 1);
    EXPECT_FALSE(obj->find(path, 0).isNull());  // run 被桩掉，旧缓存数据仍在
}

TEST_F(ImageInfoCacheTest, Load_SyncMode_RunsLoaderInline)
{
    // Arrange
    forceSyncMode();
    std::atomic<int> *calls = stubRunCounter();
    const QString path = tempDir.filePath(QStringLiteral("sync.png"));

    // Act
    obj->load(path, 0);

    // Assert：B4 → 调用线程内同步执行（调用返回即完成）
    EXPECT_EQ(calls->load(), 1);
    EXPECT_TRUE(obj->find(path, 0).isNull());  // run 被桩掉，无数据入库
}

TEST_F(ImageInfoCacheTest, Load_AsyncMode_ExecutesRunnableInPool)
{
    // Arrange
    forceAsyncMode();
    std::atomic<int> *calls = stubRunCounter();
    const QString path = tempDir.filePath(QStringLiteral("async.png"));

    // Act
    obj->load(path, 0);
    QElapsedTimer timer;
    timer.start();
    while (calls->load() < 1 && !timer.hasExpired(5000))
        QThread::msleep(10);
    obj->load(path, 0);  // 池任务已执行完，waitSet 仍持键 → 去重

    // Assert：B5 → 线程池异步执行且不重复调度
    EXPECT_GE(calls->load(), 1);
    EXPECT_EQ(calls->load(), 1);
}

TEST_F(ImageInfoCacheTest, LoadFinished_ValidData_CachesAndEmitsSignal)
{
    // Arrange
    QSignalSpy cacheSpy(obj, SIGNAL(imageDataChanged(QString,int)));
    ASSERT_TRUE(cacheSpy.isValid());
    const QString path = tempDir.filePath(QStringLiteral("lf.png"));
    const ImageInfoData::Ptr data = makeData(path);

    // Act
    obj->loadFinished(path, 4, data);

    // Assert：B2 真 → 入缓存 + 发信号（并清理 waitSet）
    EXPECT_EQ(cacheSpy.count(), 1);
    EXPECT_EQ(cacheSpy.at(0).at(1).toInt(), 4);
    EXPECT_EQ(obj->find(path, 4).data(), data.data());
}

TEST_F(ImageInfoCacheTest, LoadFinished_NullData_EmitsWithoutCaching)
{
    // Arrange
    QSignalSpy cacheSpy(obj, SIGNAL(imageDataChanged(QString,int)));
    ASSERT_TRUE(cacheSpy.isValid());
    const QString path = tempDir.filePath(QStringLiteral("lfn.png"));

    // Act
    obj->loadFinished(path, 0, ImageInfoData::Ptr());

    // Assert：B2 假 → 不入缓存但仍发信号
    EXPECT_EQ(cacheSpy.count(), 1);
    EXPECT_EQ(obj->find(path, 0).data(), nullptr);
}

TEST_F(ImageInfoCacheTest, LoadFinished_AboutToQuit_SkipsUpdate)
{
    // Arrange
    QSignalSpy cacheSpy(obj, SIGNAL(imageDataChanged(QString,int)));
    ASSERT_TRUE(cacheSpy.isValid());
    const QString path = tempDir.filePath(QStringLiteral("lfq.png"));
    obj->aboutToQuit = true;

    // Act
    obj->loadFinished(path, 0, makeData(path));

    // Assert：B1 → 不缓存、不发信号
    EXPECT_EQ(cacheSpy.count(), 0);
    EXPECT_EQ(obj->find(path, 0).data(), nullptr);
}

TEST_F(ImageInfoCacheTest, RemoveCache_ExistingEntry_RemovesAndEmitsSignal)
{
    // Arrange
    const QString path = tempDir.filePath(QStringLiteral("rm.png"));
    obj->loadFinished(path, 0, makeData(path));  // 预置缓存（此步自身会发一次信号）
    ASSERT_FALSE(obj->find(path, 0).isNull());
    QSignalSpy cacheSpy(obj, SIGNAL(imageDataChanged(QString,int)));  // 预置后再装 spy：只计 removeCache 的发射
    ASSERT_TRUE(cacheSpy.isValid());

    // Act
    obj->removeCache(path, 0);

    // Assert
    EXPECT_EQ(cacheSpy.count(), 1);
    EXPECT_EQ(obj->find(path, 0).data(), nullptr);
}

TEST_F(ImageInfoCacheTest, RemoveCache_UnknownEntry_EmitsSignalWithoutCrash)
{
    // Arrange
    QSignalSpy cacheSpy(obj, SIGNAL(imageDataChanged(QString,int)));
    ASSERT_TRUE(cacheSpy.isValid());
    const QString ghost = tempDir.filePath(QStringLiteral("ghost.png"));

    // Act
    obj->removeCache(ghost, 0);

    // Assert：不存在条目仍广播信号（供界面刷新），缓存保持为空
    EXPECT_EQ(cacheSpy.count(), 1);
    EXPECT_EQ(obj->find(ghost, 0).data(), nullptr);
}

TEST_F(ImageInfoCacheTest, ClearCache_DropsCachedAndPendingEntries_Completely)
{
    // Arrange
    forceSyncMode();
    std::atomic<int> *calls = stubRunCounter();
    const QString cachedPath = tempDir.filePath(QStringLiteral("c1.png"));
    const QString pendingPath = tempDir.filePath(QStringLiteral("c2.png"));
    obj->loadFinished(cachedPath, 0, makeData(cachedPath));
    obj->load(pendingPath, 0);  // run 被桩 → waitSet 留下 pendingPath 键
    ASSERT_EQ(calls->load(), 1);

    // Act
    obj->clearCache();

    // Assert：缓存与 waitSet 均清空（load 不再被 waitSet 拦截）
    EXPECT_TRUE(obj->find(cachedPath, 0).isNull());
    obj->load(pendingPath, 0, false);
    EXPECT_EQ(calls->load(), 2);
}

TEST_F(ImageInfoCacheTest, ClearCache_EmptyInstance_KeepsFindEmpty)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act
    obj->clearCache();

    // Assert
    EXPECT_EQ(obj->find(tempDir.filePath(QStringLiteral("none.png")), 0).data(), nullptr);
    EXPECT_FALSE(obj->aboutToQuit);
}

// ─────────────────────── LoadImageInfoRunnable ───────────────────────

class LoadImageInfoRunnableTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        resetGlobalCacheState();
        cap = std::make_shared<CapturedNotification>();
        // 默认拦截 CacheInstance()->loadFinished，捕获排队送达的通知（隔离全局副作用）
        stub.set_lamda(VADDR(ImageInfoCache, loadFinished),
                       [holder = cap.get()](ImageInfoCache *, const QString &path, int frameIndex,
                                            ImageInfoData::Ptr data) {
                           holder->path = path;
                           holder->frameIndex = frameIndex;
                           holder->data = data;
                           ++holder->count;
                       });
    }

    void TearDown() override
    {
        ImageInfo::clearCache();
        stub.clear();
    }

    stub_ext::StubExt stub;
    std::shared_ptr<CapturedNotification> cap;
    QTemporaryDir tempDir;
};

TEST_F(LoadImageInfoRunnableTest, LoadImageInfoRunnable_Construction_PassesPathAndFrameToNotification)
{
    // Arrange
    const QString path = tempDir.filePath(QStringLiteral("ctor-missing.png"));
    LoadImageInfoRunnable runnable(path, 3);

    // Act
    runnable.run();
    deliverQueuedNotifications();

    // Assert：构造参数完整透传到通知回调
    ASSERT_TRUE(cap->got());
    EXPECT_EQ(cap->path, path);
    EXPECT_EQ(cap->frameIndex, 3);
    EXPECT_FALSE(cap->data.isNull());
}

TEST_F(LoadImageInfoRunnableTest, LoadImage_MissingFile_ReturnsFalseWithNullImage)
{
    // Arrange
    LoadImageInfoRunnable runnable(tempDir.filePath(QStringLiteral("no-file.png")), 0);
    QImage image;
    QSize sourceSize;

    // Act
    const bool ret = runnable.loadImage(image, sourceSize);

    // Assert：B1 假（读不到尺寸）→ 回退加载亦失败。
    // 失败路径上 res 可能保持 null（size 为 (-1,-1)）也可能被清成 0x0（size 为 (0,0)），
    // 取决于 unionimage 失败分支实现，两者都属"无有效图像"，用 isEmpty() 兼容
    EXPECT_FALSE(ret);
    EXPECT_TRUE(image.isNull() || image.size().isEmpty());
    EXPECT_EQ(sourceSize, QSize());
}

TEST_F(LoadImageInfoRunnableTest, LoadImage_ValidPng_ReturnsTrueWithSourceSizeAndThumbnail)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("li.png"), 12, 8);
    LoadImageInfoRunnable runnable(path, 0);
    QImage image;
    QSize sourceSize;

    // Act
    const bool ret = runnable.loadImage(image, sourceSize);

    // Assert：B1+B2+B4 → 解码器直出缩略图：源尺寸保留、输出按 12:8 扩展到 150x100
    EXPECT_TRUE(ret);
    EXPECT_EQ(sourceSize, QSize(12, 8));
    EXPECT_EQ(image.size(), QSize(150, 100));
}

TEST_F(LoadImageInfoRunnableTest, LoadImage_UnreadableContent_FallsBackAndFails)
{
    // Arrange
    const QString path = makeGarbageFile(tempDir, QStringLiteral("bad-li.png"));
    LoadImageInfoRunnable runnable(path, 0);
    QImage image;
    QSize sourceSize;

    // Act
    const bool ret = runnable.loadImage(image, sourceSize);

    // Assert：B1 假 + B3 假（LibUnionImage 回退也失败）；res 为 null 或被清为 0x0 均视为无有效图像
    EXPECT_FALSE(ret);
    EXPECT_TRUE(image.isNull() || image.size().isEmpty());
    EXPECT_EQ(sourceSize, QSize());
}

TEST_F(LoadImageInfoRunnableTest, NotifyFinished_QueuedConnection_DeliversExactlyOnce)
{
    // Arrange
    LoadImageInfoRunnable runnable(tempDir.filePath(QStringLiteral("nf.png")), 5);
    const int beforeDeliver = cap->count;

    // Act
    runnable.notifyFinished(tempDir.filePath(QStringLiteral("nf.png")), 5, ImageInfoData::Ptr());

    // Assert：排队语义——投递前不触发、投递后恰好一次且参数一致
    EXPECT_EQ(cap->count, beforeDeliver);
    deliverQueuedNotifications();
    EXPECT_EQ(cap->count, beforeDeliver + 1);
    EXPECT_EQ(cap->frameIndex, 5);
}

TEST_F(LoadImageInfoRunnableTest, Run_ApplicationClosingDown_SkipsNotification)
{
    // Arrange
    stub.set_lamda(static_cast<bool (*)()>(&QCoreApplication::closingDown),
                   []() { return true; });
    LoadImageInfoRunnable runnable(tempDir.filePath(QStringLiteral("closing.png")), 0);

    // Act
    runnable.run();
    deliverQueuedNotifications();

    // Assert：B1 → 直接返回，不产生任何通知
    EXPECT_EQ(cap->count, 0);
    EXPECT_FALSE(cap->got());
}

TEST_F(LoadImageInfoRunnableTest, Run_MissingFile_NotifiesNullImageType)
{
    // Arrange
    const QString path = tempDir.filePath(QStringLiteral("missing.png"));
    LoadImageInfoRunnable runnable(path, 0);

    // Act
    runnable.run();
    deliverQueuedNotifications();

    // Assert：B2+B3(contains 假)+B10 → NullImage
    ASSERT_TRUE(cap->got());
    ImageInfo probe;
    probe.imageUrl = QUrl::fromLocalFile(cap->path);
    probe.data = cap->data;
    EXPECT_EQ(probe.type(), Types::NullImage);
    EXPECT_FALSE(probe.exists());
}

TEST_F(LoadImageInfoRunnableTest, Run_DeletedFileWithCachedThumbnail_NotifiesNonexistImage)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("deleted.png"));
    LoadImageInfoRunnable primer(path, 0);
    primer.run();                    // 第一次 run：加载成功并写入缩略图缓存
    deliverQueuedNotifications();
    ASSERT_TRUE(cap->got());
    ASSERT_TRUE(QFile::remove(path));  // 模拟加载后文件被删除
    *cap = CapturedNotification();

    // Act
    LoadImageInfoRunnable second(path, 0);
    second.run();
    deliverQueuedNotifications();

    // Assert：B2+B3(contains 真)+B10 → NonexistImage
    ASSERT_TRUE(cap->got());
    ImageInfo probe;
    probe.imageUrl = QUrl::fromLocalFile(cap->path);
    probe.data = cap->data;
    EXPECT_EQ(probe.type(), Types::NonexistImage);
    EXPECT_FALSE(probe.exists());
}

TEST_F(LoadImageInfoRunnableTest, Run_ValidPng_NotifiesLoadedImageData)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("ok.png"), 12, 8);
    LoadImageInfoRunnable runnable(path, 0);

    // Act
    runnable.run();
    deliverQueuedNotifications();

    // Assert：B8 → 数据完整且缩略图入库
    ASSERT_TRUE(cap->got());
    ImageInfo probe;
    probe.imageUrl = QUrl::fromLocalFile(cap->path);
    probe.data = cap->data;
    EXPECT_EQ(probe.type(), Types::NormalImage);
    EXPECT_EQ(probe.width(), 12);
    EXPECT_EQ(probe.height(), 8);
    EXPECT_TRUE(probe.exists());
    EXPECT_TRUE(ThumbnailCache::instance()->contains(path, 0));
}

TEST_F(LoadImageInfoRunnableTest, Run_NonMultiImageWithNonZeroFrame_NotifiesDamagedImage)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("frame.png"));
    LoadImageInfoRunnable runnable(path, 2);  // 非多页图却指定帧索引

    // Act
    runnable.run();
    deliverQueuedNotifications();

    // Assert：B7 → DamagedImage
    ASSERT_TRUE(cap->got());
    ImageInfo probe;
    probe.imageUrl = QUrl::fromLocalFile(cap->path);
    probe.data = cap->data;
    EXPECT_EQ(probe.type(), Types::DamagedImage);
    EXPECT_TRUE(probe.exists());
}

TEST_F(LoadImageInfoRunnableTest, Run_UnreadableStaticContent_NotifiesDamagedImage)
{
    // Arrange
    const QString path = makeGarbageFile(tempDir, QStringLiteral("bad.png"));
    stub.set_lamda(&LibUnionImage_NameSpace::getImageType,
                   [](const QString &) { return imageViewerSpace::ImageTypeStatic; });
    LoadImageInfoRunnable runnable(path, 0);

    // Act
    runnable.run();
    deliverQueuedNotifications();

    // Assert：B9 → 静态类型但数据读取失败 → DamagedImage
    ASSERT_TRUE(cap->got());
    ImageInfo probe;
    probe.imageUrl = QUrl::fromLocalFile(cap->path);
    probe.data = cap->data;
    EXPECT_EQ(probe.type(), Types::DamagedImage);
    EXPECT_TRUE(probe.exists());
}

TEST_F(LoadImageInfoRunnableTest, Run_ForcedMultiImage_ReadsFrameCountAndCachesThumbnail)
{
    // Arrange
    const QString path = makeTempPng(tempDir, QStringLiteral("multi.png"));
    stub.set_lamda(&LibUnionImage_NameSpace::getImageType,
                   [](const QString &) { return imageViewerSpace::ImageTypeMulti; });
    LoadImageInfoRunnable runnable(path, 0);

    // Act
    runnable.run();
    deliverQueuedNotifications();

    // Assert：B5 读帧成功 → MultiImage + 缩略图入库。
    // frameCount 来自 read() 之后的 reader.imageCount()：单文件格式在设备被消耗后归 0
    // （本地 Qt6.8 实测 plain=1 / after-read=0），断言 0 以固化真实行为
    ASSERT_TRUE(cap->got());
    ImageInfo probe;
    probe.imageUrl = QUrl::fromLocalFile(cap->path);
    probe.data = cap->data;
    EXPECT_EQ(probe.type(), Types::MultiImage);
    EXPECT_EQ(probe.frameCount(), 0);
    EXPECT_EQ(probe.width(), 12);
    EXPECT_TRUE(ThumbnailCache::instance()->contains(path, 0));
}

TEST_F(LoadImageInfoRunnableTest, Run_ForcedMultiImageUnreadableFrame_NotifiesDamagedImage)
{
    // Arrange
    const QString path = makeGarbageFile(tempDir, QStringLiteral("badmulti.png"));
    stub.set_lamda(&LibUnionImage_NameSpace::getImageType,
                   [](const QString &) { return imageViewerSpace::ImageTypeMulti; });
    LoadImageInfoRunnable runnable(path, 0);

    // Act
    runnable.run();
    deliverQueuedNotifications();

    // Assert：B5+B6+B12 → 帧读取失败 → DamagedImage
    ASSERT_TRUE(cap->got());
    ImageInfo probe;
    probe.imageUrl = QUrl::fromLocalFile(cap->path);
    probe.data = cap->data;
    EXPECT_EQ(probe.type(), Types::DamagedImage);
    EXPECT_TRUE(probe.exists());
}

// ─────────────────────── ImageInfoData ───────────────────────

class ImageInfoDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        resetGlobalCacheState();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// isError 参数化子 Fixture（主 Fixture 保持 ::testing::Test，见简报坑 1）
struct ImageInfoDataIsErrorParamTest : public ImageInfoDataTest,
                                       public ::testing::WithParamInterface<IsErrorCase> {
};

// 用例映射：6 组实例覆盖 B1（!exist 短路左右两侧）与 B2（DamagedImage == type）
TEST_P(ImageInfoDataIsErrorParamTest, IsError_TypeExistenceMatrix_MatchesExpected)
{
    // Arrange
    const IsErrorCase &c = GetParam();
    ImageInfoData::Ptr data(new ImageInfoData);
    data->type = c.type;
    data->exist = c.exist;
    ImageInfo probe;  // 用真实 getter 交叉验证 exist 字段（同时校验镜像布局）

    // Act
    probe.data = data;
    const bool ret = data->isError();

    // Assert
    EXPECT_EQ(ret, c.expected);
    EXPECT_EQ(probe.exists(), c.exist);
}

INSTANTIATE_TEST_SUITE_P(
    TypeExistenceMatrix, ImageInfoDataIsErrorParamTest,
    ::testing::Values(
        IsErrorCase{Types::NormalImage, true, false},   // 正常图 → 非错误
        IsErrorCase{Types::SvgImage, true, false},      // SVG 正常 → 非错误
        IsErrorCase{Types::NormalImage, false, true},   // 文件缺失 → 错误
        IsErrorCase{Types::MultiImage, false, true},    // 缺失短路右侧不再看类型
        IsErrorCase{Types::DamagedImage, true, true},   // 损坏类型 → 错误
        IsErrorCase{Types::DamagedImage, false, true}   // 双重错误条件
        ));

TEST_F(ImageInfoDataTest, CloneWithoutFrame_CopiesAllFields_ReturnsEqualData)
{
    // Arrange
    ImageInfoData src;
    src.path = QStringLiteral("/ut-clone-src.png");
    src.type = Types::MultiImage;
    src.size = QSize(32, 16);
    src.frameIndex = 2;
    src.frameCount = 5;
    src.scale = 1.25;
    src.x = -3.5;
    src.y = 4.75;

    // Act
    const ImageInfoData::Ptr clone = src.cloneWithoutFrame();

    // Assert：被拷贝字段逐一相等
    ASSERT_FALSE(clone.isNull());
    EXPECT_EQ(clone->path, src.path);
    EXPECT_EQ(clone->type, src.type);
    EXPECT_EQ(clone->size, src.size);
    EXPECT_EQ(clone->frameIndex, src.frameIndex);
    EXPECT_EQ(clone->frameCount, src.frameCount);
    EXPECT_DOUBLE_EQ(clone->scale, src.scale);
    EXPECT_DOUBLE_EQ(clone->x, src.x);
    EXPECT_DOUBLE_EQ(clone->y, src.y);
}

TEST_F(ImageInfoDataTest, CloneWithoutFrame_DropsExistFlag_CloneBecomesError)
{
    // Arrange
    ImageInfoData src;
    src.type = Types::NormalImage;
    src.exist = true;  // 源数据健康

    // Act
    const ImageInfoData::Ptr clone = src.cloneWithoutFrame();

    // Assert：源数据 copy 漏掉 exist 字段（默认 false）→ 克隆体被判为错误数据
    // 疑似源码缺陷：imageinfo.cpp cloneWithoutFrame 未拷贝 exist（详见汇报）
    ASSERT_FALSE(clone.isNull());
    EXPECT_EQ(clone->exist, false);
    EXPECT_EQ(clone->isError(), true);
}

// ─────────────────────── imageTypeAdapator（自由函数）───────────────────────

class ImageTypeAdapatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// 参数化子 Fixture（见简报坑 1）
struct ImageTypeAdapatorParamTest : public ImageTypeAdapatorTest,
                                    public ::testing::WithParamInterface<ImageTypeCase> {
};

TEST_P(ImageTypeAdapatorParamTest, ImageTypeAdapator_AllValues_MapToExpectedTypes)
{
    // Arrange
    const ImageTypeCase &c = GetParam();

    // Act
    const Types::ImageType ret = imageTypeAdapator(c.in);

    // Assert：枚举映射精确匹配 + 固定锚点（Static → Normal）复核映射表本身
    EXPECT_EQ(ret, c.expected);
    EXPECT_EQ(imageTypeAdapator(imageViewerSpace::ImageTypeStatic), Types::NormalImage);
}

INSTANTIATE_TEST_SUITE_P(
    EnumMapping, ImageTypeAdapatorParamTest,
    ::testing::Values(
        ImageTypeCase{imageViewerSpace::ImageTypeBlank, Types::NullImage},
        ImageTypeCase{imageViewerSpace::ImageTypeSvg, Types::SvgImage},
        ImageTypeCase{imageViewerSpace::ImageTypeStatic, Types::NormalImage},
        ImageTypeCase{imageViewerSpace::ImageTypeDynamic, Types::DynamicImage},
        ImageTypeCase{imageViewerSpace::ImageTypeMulti, Types::MultiImage},
        ImageTypeCase{imageViewerSpace::ImageTypeDamaged, Types::DamagedImage},  // 走 default
        ImageTypeCase{static_cast<imageViewerSpace::ImageType>(999), Types::DamagedImage}  // 越界 → default
        ));

