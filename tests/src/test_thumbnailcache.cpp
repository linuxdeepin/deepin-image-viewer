// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | ThumbnailCache() | low | - | 1 | 2 |
// | ~ThumbnailCache | low | - | 1 | 1 |
// | add(path,frame,image) | mid | - | 2 | 6 |
// | clear() | mid | - | 2 | 2 |
// | contains(path,frame) | mid | - | 2 | 5 |
// | get(path,frame) | mid | - | 2 | 7 |
// | instance() | low | - | 1 | 1 |
// | keys() | mid | - | 2 | 2 |
// | remove(path,frame) | mid | - | 2 | 3 |
// | setMaxCost(maxCost) | mid | - | 2 | 2 |
// | take(path,frame) | mid | - | 2 | 3 |
// | toFindKey(path,frame) | mid | - | 2 | 2 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（12 个方法全覆盖，含构造/析构）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（空路径/普通路径/多级路径/Unicode 路径；frame 0/正/负）
// 3. 每个等价类的边界值显式覆盖: [x]（maxCost 240/241 逐位边界、frameIndex 负值与 0、重复 key）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（Add_RoundtripEntry_ParamSet 4 组）
// 5. 分支清单 → 用例映射已列出: [x]（本类方法分支数均 <3 且 complexity<10，属简单方法，
//    按 §4.1 允许省略清单；get/take 的 image 空/非空两路均由 AddAndGet/Take 系列用例覆盖）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（get/take 的 if(image) 两侧、
//    其余方法无分支）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无显式 throw；错误路径以空 QImage 返回值断言覆盖）
// 8. 负面场景有专门用例: [x]（Missing/Null/Duplicate/ZeroCost 系列）
// 9. 负面用例验证强异常安全: [x]（remove/take 缺失 key 后其余条目保持断言）
// 10. stub_ext vs gMock 选择正确: [x]（纯内存 QCache 封装，无外部依赖，无需 stub）
//
// 说明：
// - ThumbnailCache 为进程级单例（instance() 内 static 局部对象），基于单例的用例在
//   Arrange 首先调用 clear() 保证自足；TearDown 统一 clear() 并恢复默认 maxCost(240)。
// - 构造/析构与驱逐边界类用例使用栈上独立对象，避免与单例互相污染。
// - 后台图片加载线程（其它测试文件的 imageinfo 链路）可能向单例 add 条目，
//   故涉及 instance() 的断言只检查本用例写入的 key，不断言 keys() 总数。

// ─────────────────────────────────────────────────────────────
// 分支清单（来源：析构~ThumbnailCache，图谱 snippet 实际横跨 13-63 行，
// 覆盖 instance/contains/get/take/add 邻接方法，实数 7 分支）
// ─────────────────────────────────────────────────────────────
// B1: get() 中 image 非空 → return *image（缓存命中）
// B2: get() 中 image 为空 → return QImage()（缓存未命中）
// B3: take() 中 image 非空 → 拷贝并 delete 后返回
// B4: take() 中 image 为空 → return QImage()
// B5: instance() → return &ins（静态局部单例）
// B6: contains() → 透传 cache.contains 的存在性
// B7: ~ThumbnailCache() → 空实现（片段内早退 return 由上述兄弟方法贡献）
// 映射： Get_MissingEntry_ReturnsNullImage → B2
//        Add_RoundtripEntry_ParamSet → B1+B6
//        Take_ExistingEntry_ReturnsImageAndEvicts → B3
//        Take_MissingEntry_ReturnsNullImage → B4
//        Take_SecondCallOnSameKey_ReturnsNullImage → B4
//        Instance_RepeatedCalls_ReturnSameSingleton → B5
//        ThumbnailCache_Destructor_ScopedCacheEndsCleanly → B7

#include <gtest/gtest.h>

#include <QImage>
#include <QStringList>

#include "stub_ext/stubext.h"
#include "thumbnailcache.h"

class ThumbnailCacheTest : public ::testing::Test {
protected:
    void SetUp() override {}

    void TearDown() override
    {
        // 单例状态复位：清空条目并恢复构造函数默认容量，防止污染其它用例
        ThumbnailCache::instance()->clear();
        ThumbnailCache::instance()->setMaxCost(240);
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// ── instance：单例语义 ────────────────────────────────────────────

TEST_F(ThumbnailCacheTest, Instance_RepeatedCalls_ReturnSameSingleton)
{
    // Arrange
    ThumbnailCache *first = ThumbnailCache::instance();

    // Act
    ThumbnailCache *second = ThumbnailCache::instance();

    // Assert
    EXPECT_NE(first, nullptr);
    EXPECT_EQ(first, second);
}

// ── 构造/析构 ─────────────────────────────────────────────────────

TEST_F(ThumbnailCacheTest, ThumbnailCache_Constructor_FreshCacheStartsEmpty)
{
    // Arrange
    const QImage frame(4, 4, QImage::Format_RGB32);

    // Act
    ThumbnailCache cache;

    // Assert
    EXPECT_TRUE(cache.keys().isEmpty());
    EXPECT_FALSE(cache.contains(QStringLiteral("any.png"), 0));
    EXPECT_EQ(cache.keys().size(), 0);
}

TEST_F(ThumbnailCacheTest, ThumbnailCache_Constructor_DefaultCostLimit240)
{
    // Arrange：独立栈上对象，验证构造函数设定的默认容量（源码 cache.setMaxCost(240)）
    ThumbnailCache cache;
    const QImage frame(4, 4, QImage::Format_RGB32);
    for (int i = 0; i < 240; ++i)
        cache.add(QStringLiteral("shot%1.png").arg(i), 0, frame);

    // Act：再插入第 241 条（每条 cost=1，超出默认 maxCost=240）
    cache.add(QStringLiteral("shot240.png"), 0, frame);

    // Assert：最旧条目被驱逐，最新条目保留，总数收敛于 240
    EXPECT_FALSE(cache.contains(QStringLiteral("shot0.png"), 0));    // 边界：超限后被驱逐
    EXPECT_TRUE(cache.contains(QStringLiteral("shot239.png"), 0));   // 边界：限内最新
    EXPECT_TRUE(cache.contains(QStringLiteral("shot240.png"), 0));
    EXPECT_EQ(cache.keys().size(), 240);
}

TEST_F(ThumbnailCacheTest, ThumbnailCache_Destructor_ScopedCacheEndsCleanly)
{
    // Arrange
    const QImage frame(2, 2, QImage::Format_RGB32);

    // Act：作用域结束触发析构（~ThumbnailCache 释放 QCache 托管的 QImage*）
    EXPECT_NO_THROW({
        ThumbnailCache scoped;
        scoped.add(QStringLiteral("scoped.png"), 0, frame);
        scoped.setMaxCost(8);
    });

    // Assert：析构后独立对象行为不受影响
    ThumbnailCache followUp;
    EXPECT_TRUE(followUp.keys().isEmpty());
    EXPECT_FALSE(followUp.contains(QStringLiteral("scoped.png"), 0));
    EXPECT_EQ(followUp.keys().size(), 0);
}

// ── add / get / contains：参数化回写 ──────────────────────────────

namespace {
struct CacheEntryCase {
    QString path;
    int frame;
};
}  // namespace

class ThumbnailCacheEntryParamTest : public ThumbnailCacheTest,
                                     public ::testing::WithParamInterface<CacheEntryCase> {};

TEST_P(ThumbnailCacheEntryParamTest, Add_RoundtripEntry_ParamSet)
{
    // Arrange
    ThumbnailCache *cache = ThumbnailCache::instance();
    cache->clear();
    QImage image(8, 8, QImage::Format_RGB32);
    image.fill(Qt::green);
    const CacheEntryCase entry = GetParam();

    // Act
    cache->add(entry.path, entry.frame, image);

    // Assert
    EXPECT_TRUE(cache->contains(entry.path, entry.frame));
    const QImage stored = cache->get(entry.path, entry.frame);
    EXPECT_FALSE(stored.isNull());
    EXPECT_EQ(stored.size(), image.size());
}

INSTANTIATE_TEST_SUITE_P(
    CacheEntries, ThumbnailCacheEntryParamTest,
    ::testing::Values(
        CacheEntryCase{QStringLiteral(""), 0},                        // 边界：空路径
        CacheEntryCase{QStringLiteral("img.png"), 0},                 // 单文件名
        CacheEntryCase{QStringLiteral("dir/img.png"), 5},             // 多级路径 + 正帧号
        CacheEntryCase{QStringLiteral("图片/照片 1.png"), -1}));      // Unicode + 负帧号

TEST_F(ThumbnailCacheTest, Add_DuplicateKey_ReplacesCachedImage)
{
    // Arrange
    ThumbnailCache cache;
    QImage first(2, 2, QImage::Format_RGB32);
    first.fill(Qt::red);
    QImage second(2, 2, QImage::Format_RGB32);
    second.fill(Qt::blue);

    // Act：同一路径 + 帧号重复写入
    cache.add(QStringLiteral("dup.png"), 0, first);
    cache.add(QStringLiteral("dup.png"), 0, second);

    // Assert
    EXPECT_EQ(cache.keys().size(), 1);
    const QImage stored = cache.get(QStringLiteral("dup.png"), 0);
    EXPECT_EQ(stored.pixel(0, 0), second.pixel(0, 0));  // 后写入的图像生效
}

TEST_F(ThumbnailCacheTest, Add_NullImage_StillCachesEntry)
{
    // Arrange
    ThumbnailCache cache;

    // Act
    cache.add(QStringLiteral("nullimg.png"), 0, QImage());

    // Assert：QCache 照常记录条目，get 返回空图
    EXPECT_TRUE(cache.contains(QStringLiteral("nullimg.png"), 0));
    EXPECT_TRUE(cache.get(QStringLiteral("nullimg.png"), 0).isNull());
    EXPECT_EQ(cache.keys().size(), 1);
}

TEST_F(ThumbnailCacheTest, Get_MissingEntry_ReturnsNullImage)
{
    // Arrange
    ThumbnailCache cache;
    cache.add(QStringLiteral("present.png"), 0, QImage(2, 2, QImage::Format_RGB32));

    // Act
    const QImage missing = cache.get(QStringLiteral("absent.png"), 0);

    // Assert
    EXPECT_TRUE(missing.isNull());
    EXPECT_FALSE(cache.contains(QStringLiteral("absent.png"), 0));
    EXPECT_EQ(cache.keys().size(), 1);  // 既有条目保持
}

TEST_F(ThumbnailCacheTest, Contains_OtherFrameIndex_ReturnsFalse)
{
    // Arrange
    ThumbnailCache cache;
    cache.add(QStringLiteral("multi.png"), 0, QImage(2, 2, QImage::Format_RGB32));

    // Act
    const bool sameFrame = cache.contains(QStringLiteral("multi.png"), 0);
    const bool otherFrame = cache.contains(QStringLiteral("multi.png"), 1);

    // Assert：帧号是 key 的一部分，不同帧号互不命中
    EXPECT_TRUE(sameFrame);
    EXPECT_FALSE(otherFrame);
    EXPECT_EQ(cache.keys().size(), 1);
}

// ── take ──────────────────────────────────────────────────────────

TEST_F(ThumbnailCacheTest, Take_ExistingEntry_ReturnsImageAndEvicts)
{
    // Arrange
    ThumbnailCache cache;
    QImage source(3, 3, QImage::Format_RGB32);
    source.fill(Qt::cyan);
    cache.add(QStringLiteral("take.png"), 0, source);

    // Act
    const QImage taken = cache.take(QStringLiteral("take.png"), 0);

    // Assert
    EXPECT_EQ(taken.pixel(1, 1), source.pixel(1, 1));
    EXPECT_FALSE(cache.contains(QStringLiteral("take.png"), 0));  // take 后条目消失
}

TEST_F(ThumbnailCacheTest, Take_MissingEntry_ReturnsNullImage)
{
    // Arrange
    ThumbnailCache cache;

    // Act
    const QImage taken = cache.take(QStringLiteral("ghost.png"), 7);

    // Assert
    EXPECT_TRUE(taken.isNull());
    EXPECT_TRUE(cache.keys().isEmpty());
    EXPECT_EQ(cache.keys().size(), 0);
}

TEST_F(ThumbnailCacheTest, Take_SecondCallOnSameKey_ReturnsNullImage)
{
    // Arrange
    ThumbnailCache cache;
    QImage source(2, 2, QImage::Format_RGB32);
    cache.add(QStringLiteral("once.png"), 0, source);
    const QImage firstTake = cache.take(QStringLiteral("once.png"), 0);

    // Act：同一 key 连续第二次 take（边界：条目已消费）
    const QImage secondTake = cache.take(QStringLiteral("once.png"), 0);

    // Assert
    EXPECT_FALSE(firstTake.isNull());
    EXPECT_TRUE(secondTake.isNull());
    EXPECT_EQ(firstTake.width(), 2);  // 首次 take 返回原图尺寸
}

// ── remove ────────────────────────────────────────────────────────

TEST_F(ThumbnailCacheTest, Remove_ExistingEntry_DropsFromCache)
{
    // Arrange
    ThumbnailCache cache;
    cache.add(QStringLiteral("rm.png"), 0, QImage(2, 2, QImage::Format_RGB32));

    // Act
    cache.remove(QStringLiteral("rm.png"), 0);

    // Assert
    EXPECT_FALSE(cache.contains(QStringLiteral("rm.png"), 0));
    EXPECT_TRUE(cache.keys().isEmpty());
    EXPECT_EQ(cache.keys().size(), 0);
}

TEST_F(ThumbnailCacheTest, Remove_MissingEntry_LeavesCacheIntact)
{
    // Arrange
    ThumbnailCache cache;
    cache.add(QStringLiteral("keep.png"), 0, QImage(2, 2, QImage::Format_RGB32));

    // Act：移除不存在的 key（负面：无此条目）
    cache.remove(QStringLiteral("ghost.png"), 3);

    // Assert：既有条目不受影响（强异常安全）
    EXPECT_TRUE(cache.contains(QStringLiteral("keep.png"), 0));
    EXPECT_EQ(cache.keys().size(), 1);
}

TEST_F(ThumbnailCacheTest, Remove_OneOfManyEntries_KeepsOthers)
{
    // Arrange
    ThumbnailCache cache;
    const QImage frame(2, 2, QImage::Format_RGB32);
    cache.add(QStringLiteral("a.png"), 0, frame);
    cache.add(QStringLiteral("b.png"), 0, frame);
    cache.add(QStringLiteral("c.png"), 0, frame);

    // Act
    cache.remove(QStringLiteral("b.png"), 0);

    // Assert
    EXPECT_FALSE(cache.contains(QStringLiteral("b.png"), 0));
    EXPECT_TRUE(cache.contains(QStringLiteral("a.png"), 0));
    EXPECT_TRUE(cache.contains(QStringLiteral("c.png"), 0));
    EXPECT_EQ(cache.keys().size(), 2);
}

// ── clear ─────────────────────────────────────────────────────────

TEST_F(ThumbnailCacheTest, Clear_PopulatedCache_RemovesAllEntries)
{
    // Arrange
    ThumbnailCache cache;
    const QImage frame(2, 2, QImage::Format_RGB32);
    cache.add(QStringLiteral("a.png"), 0, frame);
    cache.add(QStringLiteral("a.png"), 1, frame);
    cache.add(QStringLiteral("b.png"), 0, frame);

    // Act
    cache.clear();

    // Assert
    EXPECT_TRUE(cache.keys().isEmpty());
    EXPECT_FALSE(cache.contains(QStringLiteral("any.png"), 0));
    EXPECT_EQ(cache.keys().size(), 0);
}

TEST_F(ThumbnailCacheTest, Clear_EmptyCache_IsSafeNoOp)
{
    // Arrange
    ThumbnailCache cache;

    // Act：空缓存上 clear（边界：无条目）
    cache.clear();

    // Assert
    EXPECT_TRUE(cache.keys().isEmpty());
    EXPECT_FALSE(cache.contains(QStringLiteral("any.png"), 0));
    EXPECT_EQ(cache.keys().size(), 0);
}

// ── keys ──────────────────────────────────────────────────────────

TEST_F(ThumbnailCacheTest, Keys_MultiPathAndFrame_ListsAllEntries)
{
    // Arrange
    ThumbnailCache cache;
    const QImage frame(2, 2, QImage::Format_RGB32);
    cache.add(QStringLiteral("a.png"), 0, frame);
    cache.add(QStringLiteral("a.png"), 1, frame);
    cache.add(QStringLiteral("b.png"), 0, frame);

    // Act
    const QList<ThumbnailCache::Key> keys = cache.keys();

    // Assert
    EXPECT_EQ(keys.size(), 3);
    EXPECT_TRUE(keys.contains(qMakePair(QStringLiteral("a.png"), 0)));
    EXPECT_TRUE(keys.contains(qMakePair(QStringLiteral("a.png"), 1)));
    EXPECT_TRUE(keys.contains(qMakePair(QStringLiteral("b.png"), 0)));
}

// ── setMaxCost ────────────────────────────────────────────────────

TEST_F(ThumbnailCacheTest, SetMaxCost_ReducedBelowUsage_EvictsOldest)
{
    // Arrange
    ThumbnailCache cache;
    const QImage frame(2, 2, QImage::Format_RGB32);
    cache.add(QStringLiteral("old.png"), 0, frame);
    cache.add(QStringLiteral("mid.png"), 0, frame);
    cache.add(QStringLiteral("new.png"), 0, frame);

    // Act：容量收缩到 1（低于当前条目数）
    cache.setMaxCost(1);

    // Assert：仅保留最新条目
    EXPECT_FALSE(cache.contains(QStringLiteral("old.png"), 0));
    EXPECT_FALSE(cache.contains(QStringLiteral("mid.png"), 0));
    EXPECT_TRUE(cache.contains(QStringLiteral("new.png"), 0));
    EXPECT_EQ(cache.keys().size(), 1);
}

TEST_F(ThumbnailCacheTest, SetMaxCost_Zero_DropsEveryNewEntry)
{
    // Arrange
    ThumbnailCache cache;
    cache.setMaxCost(0);
    const QImage frame(2, 2, QImage::Format_RGB32);

    // Act：maxCost=0 时插入 cost=1 的条目（边界：容量为零）
    cache.add(QStringLiteral("z.png"), 0, frame);

    // Assert：条目立即被丢弃
    EXPECT_FALSE(cache.contains(QStringLiteral("z.png"), 0));
    EXPECT_TRUE(cache.keys().isEmpty());
    EXPECT_EQ(cache.keys().size(), 0);
}

// ── toFindKey ─────────────────────────────────────────────────────

TEST_F(ThumbnailCacheTest, ToFindKey_PathAndFrame_CombinedIntoPair)
{
    // Arrange
    ThumbnailCache cache;

    // Act
    const ThumbnailCache::Key key = cache.toFindKey(QStringLiteral("a.png"), 3);

    // Assert
    EXPECT_EQ(key, qMakePair(QStringLiteral("a.png"), 3));
    EXPECT_EQ(key.first, QStringLiteral("a.png"));
}

TEST_F(ThumbnailCacheTest, ToFindKey_DistinctInputs_ProduceDistinctKeys)
{
    // Arrange
    ThumbnailCache cache;

    // Act
    const ThumbnailCache::Key byFrame = cache.toFindKey(QStringLiteral("a.png"), 3);
    const ThumbnailCache::Key byPath = cache.toFindKey(QStringLiteral("b.png"), 3);

    // Assert：帧号或路径任一不同即不同 key
    EXPECT_NE(byFrame, cache.toFindKey(QStringLiteral("a.png"), 4));
    EXPECT_NE(byPath, byFrame);
    EXPECT_EQ(byFrame, cache.toFindKey(QStringLiteral("a.png"), 3));
}
