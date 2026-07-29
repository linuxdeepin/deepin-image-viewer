// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_thumbnailcache.h"
#include "thumbnailcache.h"

#include <QImage>
#include <QList>

void ut_thumbnailcache::SetUp()
{
}

void ut_thumbnailcache::TearDown()
{
}

// 测试默认构造与析构
TEST_F(ut_thumbnailcache, DefaultConstruct)
{
    ThumbnailCache cache;
    EXPECT_TRUE(cache.keys().isEmpty());
}

// 测试 instance() 返回同一单例
TEST_F(ut_thumbnailcache, InstanceReturnsSameSingleton)
{
    ThumbnailCache *p1 = ThumbnailCache::instance();
    ThumbnailCache *p2 = ThumbnailCache::instance();
    EXPECT_NE(p1, nullptr);
    EXPECT_EQ(p1, p2);
}

// 测试 add/contains/get 协同
TEST_F(ut_thumbnailcache, AddContainsAndGet)
{
    ThumbnailCache cache;
    QImage img(10, 10, QImage::Format_ARGB32);
    img.fill(Qt::red);

    EXPECT_FALSE(cache.contains("path1"));
    cache.add("path1", 0, img);
    EXPECT_TRUE(cache.contains("path1", 0));

    QImage got = cache.get("path1", 0);
    EXPECT_FALSE(got.isNull());
    EXPECT_EQ(got.size(), QSize(10, 10));
}

// 测试 get 未命中返回空图像
TEST_F(ut_thumbnailcache, GetNonexistentReturnsNull)
{
    ThumbnailCache cache;
    QImage got = cache.get("nonexistent", 0);
    EXPECT_TRUE(got.isNull());
}

// 测试不同帧索引独立缓存
TEST_F(ut_thumbnailcache, FrameIndexIndependence)
{
    ThumbnailCache cache;
    QImage img1(2, 2, QImage::Format_ARGB32);
    img1.fill(Qt::blue);
    QImage img2(4, 4, QImage::Format_ARGB32);
    img2.fill(Qt::green);

    cache.add("file", 0, img1);
    cache.add("file", 1, img2);

    EXPECT_TRUE(cache.contains("file", 0));
    EXPECT_TRUE(cache.contains("file", 1));
    EXPECT_EQ(cache.get("file", 0).size(), QSize(2, 2));
    EXPECT_EQ(cache.get("file", 1).size(), QSize(4, 4));
}

// 测试 take 取出后从缓存移除
TEST_F(ut_thumbnailcache, TakeRemovesEntry)
{
    ThumbnailCache cache;
    QImage img(8, 8, QImage::Format_ARGB32);
    img.fill(Qt::black);

    cache.add("takepath", 3, img);
    EXPECT_TRUE(cache.contains("takepath", 3));

    QImage taken = cache.take("takepath", 3);
    EXPECT_FALSE(taken.isNull());
    EXPECT_FALSE(cache.contains("takepath", 3));
}

// 测试 take 未命中返回空图像
TEST_F(ut_thumbnailcache, TakeNonexistentReturnsNull)
{
    ThumbnailCache cache;
    QImage taken = cache.take("nope", 0);
    EXPECT_TRUE(taken.isNull());
}

// 测试 remove
TEST_F(ut_thumbnailcache, RemoveEntry)
{
    ThumbnailCache cache;
    QImage img(1, 1, QImage::Format_ARGB32);
    cache.add("rm", 0, img);
    EXPECT_TRUE(cache.contains("rm", 0));

    cache.remove("rm", 0);
    EXPECT_FALSE(cache.contains("rm", 0));

    // remove 不存在的项不应崩溃
    cache.remove("rm", 0);
}

// 测试 setMaxCost 驱逐策略
TEST_F(ut_thumbnailcache, SetMaxCostEvictsOldEntries)
{
    ThumbnailCache cache;
    cache.setMaxCost(1);

    QImage img1(3, 3, QImage::Format_ARGB32);
    QImage img2(5, 5, QImage::Format_ARGB32);

    cache.add("a", 0, img1);
    EXPECT_TRUE(cache.contains("a", 0));

    // 新插入项(cost=1)超过总容量 1，应驱逐旧项
    cache.add("b", 0, img2);
    EXPECT_FALSE(cache.contains("a", 0));
    EXPECT_TRUE(cache.contains("b", 0));
}

// 测试 clear 清空全部
TEST_F(ut_thumbnailcache, ClearAll)
{
    ThumbnailCache cache;
    QImage img(2, 2, QImage::Format_ARGB32);
    cache.add("c1", 0, img);
    cache.add("c2", 0, img);
    EXPECT_EQ(cache.keys().size(), 2);

    cache.clear();
    EXPECT_TRUE(cache.keys().isEmpty());
    EXPECT_FALSE(cache.contains("c1", 0));
}

// 测试 keys 返回所有键
TEST_F(ut_thumbnailcache, KeysReturnsAllEntries)
{
    ThumbnailCache cache;
    QImage img(1, 1, QImage::Format_ARGB32);
    cache.add("k1", 0, img);
    cache.add("k2", 2, img);

    QList<ThumbnailCache::Key> ks = cache.keys();
    EXPECT_EQ(ks.size(), 2);
}

// 测试静态 toFindKey 组合路径与帧号
TEST_F(ut_thumbnailcache, ToFindKeyCombinesPathAndFrame)
{
    ThumbnailCache::Key k = ThumbnailCache::toFindKey("abc", 5);
    EXPECT_EQ(k.first, QString("abc"));
    EXPECT_EQ(k.second, 5);

    // 默认帧号
    ThumbnailCache::Key k2 = ThumbnailCache::toFindKey("xyz");
    EXPECT_EQ(k2.second, 0);
}
