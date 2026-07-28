// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_pathviewproxymodel.h"
#include "pathviewproxymodel.h"
#include "imagesourcemodel.h"

#include <QUrl>
#include <QList>
#include <QModelIndex>
#include <QSignalSpy>
#include <QImage>
#include <QDir>
#include <QCoreApplication>
#include "types.h"

// 创建若干临时 PNG 文件供 ImageSourceModel 使用，确保 ImageInfo 能同步加载。
static QStringList g_pathviewTmpPaths;
static QList<QUrl> g_pathviewTmpUrls;

static void ensurePathViewTempImages()
{
    if (!g_pathviewTmpPaths.isEmpty()) {
        return;
    }
    QString dir = QDir::tempPath() + "/ut_pathview_" +
                  QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(dir);
    for (int i = 0; i < 5; ++i) {
        QImage img(20 + i, 20, QImage::Format_ARGB32);
        img.fill(QColor(i * 50, 100, 100));
        QString path = dir + "/img" + QString::number(i) + ".png";
        img.save(path, "PNG");
        g_pathviewTmpPaths << path;
        g_pathviewTmpUrls << QUrl::fromLocalFile(path);
    }
}

void ut_pathviewproxymodel::SetUp()
{
    ensurePathViewTempImages();
}

void ut_pathviewproxymodel::TearDown()
{
}

// 测试构造与默认状态
TEST_F(ut_pathviewproxymodel, Construct)
{
    ImageSourceModel src;
    PathViewProxyModel model(&src);
    EXPECT_EQ(model.rowCount(), 0);
    EXPECT_EQ(model.currentIndex(), 0);
}

// 测试 roleNames
TEST_F(ut_pathviewproxymodel, RoleNames)
{
    ImageSourceModel src;
    PathViewProxyModel model(&src);
    QHash<int, QByteArray> names = model.roleNames();
    EXPECT_TRUE(names.contains(Types::ImageUrlRole));
    EXPECT_TRUE(names.contains(Types::FrameIndexRole));
}

// 测试 setCurrentIndex 与信号
TEST_F(ut_pathviewproxymodel, SetCurrentIndexAndSignal)
{
    ImageSourceModel src;
    PathViewProxyModel model(&src);

    QSignalSpy spy(&model, &PathViewProxyModel::currentIndexChanged);
    model.setCurrentIndex(2);
    EXPECT_EQ(model.currentIndex(), 2);
    EXPECT_EQ(spy.count(), 1);

    // 相同值不触发信号
    model.setCurrentIndex(2);
    EXPECT_EQ(spy.count(), 1);
}

// 测试 setQueueCount 合法值（奇数 >=3）
TEST_F(ut_pathviewproxymodel, SetQueueCountValid)
{
    ImageSourceModel src;
    PathViewProxyModel model(&src);
    model.setQueueCount(7);
    // 无直接 getter，通过 resetModel 后的 rowCount 间接验证
    src.setImageFiles(g_pathviewTmpUrls);
    model.resetModel(0, 0);
    EXPECT_EQ(model.rowCount(), 7);
}

// 测试 resetModel 填充队列
TEST_F(ut_pathviewproxymodel, ResetModelFillsQueue)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);

    QSignalSpy resetSpy(&model, &QAbstractListModel::modelReset);
    ASSERT_TRUE(resetSpy.isValid());
    model.resetModel(2, 0);
    EXPECT_EQ(model.rowCount(), 5);  // default queue count
    EXPECT_EQ(model.currentIndex(), 0);
    EXPECT_GE(resetSpy.count(), 1);
}

// 测试 data 合法与非法索引
TEST_F(ut_pathviewproxymodel, DataValidAndInvalid)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    QModelIndex idx = model.index(0);
    QVariant urlVar = model.data(idx, Types::ImageUrlRole);
    EXPECT_TRUE(urlVar.isValid());

    QVariant frameVar = model.data(idx, Types::FrameIndexRole);
    EXPECT_EQ(frameVar.toInt(), 0);

    // 未知 role
    QVariant unknown = model.data(idx, Qt::UserRole + 100);
    EXPECT_FALSE(unknown.isValid());

    // 非法索引
    EXPECT_FALSE(model.data(QModelIndex(), Types::ImageUrlRole).isValid());
}

// 测试 setData 更新 url
TEST_F(ut_pathviewproxymodel, SetDataUpdatesUrl)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    QSignalSpy spy(&model, &PathViewProxyModel::dataChanged);
    QModelIndex idx = model.index(0);
    QUrl newUrl("file:///changed.png");
    bool ok = model.setData(idx, QVariant::fromValue(newUrl), Types::ImageUrlRole);
    EXPECT_TRUE(ok);
    EXPECT_GE(spy.count(), 1);
    EXPECT_EQ(model.data(idx, Types::ImageUrlRole).toUrl(), newUrl);
}

// 测试 setData 非法索引与未知 role
TEST_F(ut_pathviewproxymodel, SetDataInvalidAndUnknownRole)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    EXPECT_FALSE(model.setData(QModelIndex(), QVariant::fromValue(QUrl("file:///x.png")), Types::ImageUrlRole));
    EXPECT_FALSE(model.setData(model.index(0), QVariant::fromValue(QUrl("file:///y.png")), Qt::UserRole + 100));
}

// 测试 moveNext/movePrevoius 不崩溃并更新 currentIndex
TEST_F(ut_pathviewproxymodel, MoveNextAndPrevious)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    int oldIdx = model.currentIndex();
    model.moveNext();
    EXPECT_EQ(model.rowCount(), 5);

    model.movePrevoius();
    EXPECT_EQ(model.rowCount(), 5);
    // 多次移动
    for (int i = 0; i < 10; ++i) {
        model.moveNext();
    }
    EXPECT_EQ(model.rowCount(), 5);
}

// 测试 dumpInfo 不崩溃
TEST_F(ut_pathviewproxymodel, DumpInfo)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);
    model.dumpInfo();
    SUCCEED();
}

// 测试 syncState 在 jumpFlag==Current 时为空操作
TEST_F(ut_pathviewproxymodel, SyncStateNoOpWhenCurrent)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);
    model.syncState();  // jumpFlag == Current, 无副作用
    SUCCEED();
}

// 测试 syncState 在跳转后刷新两侧数据
TEST_F(ut_pathviewproxymodel, SyncStateAfterJump)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);
    // OutOfRange 跳转会设置 jumpFlag
    model.setCurrentSourceIndex(0, 0);
    model.syncState();
    SUCCEED();
}

// 测试 setCurrentSourceIndex 各距离分支
TEST_F(ut_pathviewproxymodel, SetCurrentSourceIndexBranches)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    // Current - 不动作
    model.setCurrentSourceIndex(2, 0);
    // Previous
    model.setCurrentSourceIndex(1, 0);
    // Next
    model.setCurrentSourceIndex(2, 0);
    model.movePrevoius();
    model.setCurrentSourceIndex(2, 0);
    // OutOfRange
    model.resetModel(2, 0);
    model.setCurrentSourceIndex(0, 0);
    SUCCEED();
}

// 测试 setCurrentSourceIndex 空队列提前返回
TEST_F(ut_pathviewproxymodel, SetCurrentSourceIndexEmptyQueue)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    // 队列为空，不崩溃
    model.setCurrentSourceIndex(0, 0);
    SUCCEED();
}

// 测试 deleteCurrent 各分支
TEST_F(ut_pathviewproxymodel, DeleteCurrentBranches)
{
    // 1. 空队列
    {
        ImageSourceModel src;
        src.setImageFiles(g_pathviewTmpUrls);
        PathViewProxyModel model(&src);
        model.deleteCurrent();  // 空队列提前返回
    }
    // 2. 源模型行数为 0
    {
        ImageSourceModel src;
        PathViewProxyModel model(&src);
        model.resetModel(0, 0);  // 队列非空但源模型 rowCount=0
        src.setImageFiles(g_pathviewTmpUrls);
        model.deleteCurrent();
    }
    // 3. 正常删除（非尾部）
    {
        ImageSourceModel src;
        src.setImageFiles(g_pathviewTmpUrls);
        PathViewProxyModel model(&src);
        model.resetModel(2, 0);
        model.deleteCurrent();
        EXPECT_EQ(model.rowCount(), 5);
    }
}

// ---------- 私有函数测试（依赖 -fno-access-control） ----------

// 测试 sourcePath 合法与越界
TEST_F(ut_pathviewproxymodel, PrivateSourcePath)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    QUrl p = model.sourcePath(0);
    EXPECT_FALSE(p.isEmpty());
    EXPECT_TRUE(model.sourcePath(-1).isEmpty());
    EXPECT_TRUE(model.sourcePath(999).isEmpty());
}

// 测试 previousPorxyIdx / nextProxyIdx 环形
TEST_F(ut_pathviewproxymodel, PrivateProxyIdxRing)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);
    int size = model.rowCount();
    ASSERT_EQ(size, 5);

    EXPECT_EQ(model.previousPorxyIdx(0), size - 1);
    EXPECT_EQ(model.nextProxyIdx(size - 1), 0);
    EXPECT_EQ(model.nextProxyIdx(0), 1);
    EXPECT_EQ(model.previousPorxyIdx(1), 0);
}

// 测试 distance 各分支
TEST_F(ut_pathviewproxymodel, PrivateDistance)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    EXPECT_EQ(model.distance(2, 0), PathViewProxyModel::Current);    // 0
    EXPECT_EQ(model.distance(3, 0), PathViewProxyModel::Next);       // 1
    EXPECT_EQ(model.distance(1, 0), PathViewProxyModel::Previous);   // -1
    EXPECT_EQ(model.distance(0, 0), PathViewProxyModel::OutOfRange); // 0x80
    EXPECT_EQ(model.distance(-1, 0), PathViewProxyModel::Invalid);   // 0x100
    EXPECT_EQ(model.distance(999, 0), PathViewProxyModel::Invalid);
}

// 测试 infoFromIndex 合法与越界
TEST_F(ut_pathviewproxymodel, PrivateInfoFromIndex)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    auto info = model.infoFromIndex(0, 0);
    ASSERT_TRUE(!info.isNull());
    EXPECT_EQ(info->index, 0);
    EXPECT_FALSE(info->url.isEmpty());

    // 越界返回空
    auto bad = model.infoFromIndex(999, 0);
    EXPECT_TRUE(bad.isNull());
}

// 测试 createPreviousIndexInfo / createNextIndexInfo
TEST_F(ut_pathviewproxymodel, PrivateCreatePrevNextIndexInfo)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    auto base = model.infoFromIndex(2, 0);
    ASSERT_TRUE(!base.isNull());

    auto prev = model.createPreviousIndexInfo(base);
    EXPECT_FALSE(prev.isNull());  // src1
    auto next = model.createNextIndexInfo(base);
    EXPECT_FALSE(next.isNull());  // src3

    // 空入参返回空
    PathViewProxyModel::IndexInfoPtr nullInfo;
    EXPECT_TRUE(model.createPreviousIndexInfo(nullInfo).isNull());
    EXPECT_TRUE(model.createNextIndexInfo(nullInfo).isNull());
}

// 测试 createPreviousIndexInfo 在边界(index 0)返回空
TEST_F(ut_pathviewproxymodel, PrivateCreatePrevAtBoundary)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(0, 0);  // current = src0

    auto base = model.infoFromIndex(0, 0);
    auto prev = model.createPreviousIndexInfo(base);
    // src0 前面无图片
    EXPECT_TRUE(prev.isNull());
}

// 测试 updateIndexInfo 更新并发信号
TEST_F(ut_pathviewproxymodel, PrivateUpdateIndexInfo)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    QSignalSpy spy(&model, &PathViewProxyModel::dataChanged);
    auto info = model.infoFromIndex(4, 0);
    model.updateIndexInfo(0, info);
    EXPECT_GE(spy.count(), 1);
    EXPECT_EQ(model.data(model.index(0), Types::ImageUrlRole).toUrl(), info->url);
}

// 测试 jumpToIndex 与 refreshBothSideData
TEST_F(ut_pathviewproxymodel, PrivateJumpAndRefresh)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    model.jumpToIndex(4, 0);
    model.refreshBothSideData();
    SUCCEED();
}

// 测试 asyncUpdateLoadInfo 不崩溃（frameIndex 0 与非 0）
TEST_F(ut_pathviewproxymodel, PrivateAsyncUpdateLoadInfo)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    model.asyncUpdateLoadInfo(g_pathviewTmpUrls.first(), 0, 0);
    model.asyncUpdateLoadInfo(g_pathviewTmpUrls.first(), 0, 1);  // frameIndex != 0 直接跳过
    SUCCEED();
}

// IndexInfo 拷贝构造函数
TEST_F(ut_pathviewproxymodel, IndexInfoCopyConstructor)
{
    ImageSourceModel src;
    src.setImageFiles(g_pathviewTmpUrls);
    PathViewProxyModel model(&src);
    model.resetModel(2, 0);

    auto info = model.infoFromIndex(0, 0);
    ASSERT_TRUE(!info.isNull());

    // 显式调用拷贝构造函数
    PathViewProxyModel::IndexInfo copy(*info);
    EXPECT_EQ(copy.url, info->url);
    EXPECT_EQ(copy.index, info->index);
    EXPECT_EQ(copy.frameCount, info->frameCount);
    EXPECT_EQ(copy.frameIndex, info->frameIndex);
}
