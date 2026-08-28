// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_imagesourcemodel.h"
#include "imagesourcemodel.h"

#include <QUrl>
#include <QList>
#include <QModelIndex>
#include <QSignalSpy>
#include "types.h"

void ut_imagesourcemodel::SetUp()
{
}

void ut_imagesourcemodel::TearDown()
{
}

// 测试默认构造与空模型
TEST_F(ut_imagesourcemodel, DefaultConstruct)
{
    ImageSourceModel model;
    EXPECT_EQ(model.rowCount(), 0);
}

// 测试 roleNames 包含 imageUrl
TEST_F(ut_imagesourcemodel, RoleNames)
{
    ImageSourceModel model;
    QHash<int, QByteArray> names = model.roleNames();
    EXPECT_TRUE(names.contains(Types::ImageUrlRole));
    EXPECT_EQ(names.value(Types::ImageUrlRole), QByteArray("imageUrl"));
}

// 测试 setImageFiles 后 rowCount 与 data
TEST_F(ut_imagesourcemodel, SetImageFilesAndUpdateRowCount)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///a.jpg") << QUrl("file:///b.jpg") << QUrl("file:///c.jpg");

    model.setImageFiles(files);
    EXPECT_EQ(model.rowCount(), 3);

    QModelIndex idx = model.index(1);
    EXPECT_EQ(model.data(idx, Types::ImageUrlRole).toUrl(), QUrl("file:///b.jpg"));
}

// 测试 data 传入非法索引返回空
TEST_F(ut_imagesourcemodel, DataInvalidIndexReturnsEmpty)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///a.jpg");
    model.setImageFiles(files);

    QVariant v = model.data(QModelIndex(), Types::ImageUrlRole);
    EXPECT_FALSE(v.isValid());

    QModelIndex bad = model.index(99);
    QVariant v2 = model.data(bad, Types::ImageUrlRole);
    EXPECT_FALSE(v2.isValid());
}

// 测试 data 传入未知 role 返回空
TEST_F(ut_imagesourcemodel, DataUnknownRoleReturnsEmpty)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///a.jpg");
    model.setImageFiles(files);

    QVariant v = model.data(model.index(0), Qt::UserRole + 100);
    EXPECT_FALSE(v.isValid());
}

// 测试 setData 成功并发信号
TEST_F(ut_imagesourcemodel, SetDataValidEmitsSignal)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///a.jpg") << QUrl("file:///b.jpg");
    model.setImageFiles(files);

    QSignalSpy spy(&model, &ImageSourceModel::dataChanged);
    QModelIndex idx = model.index(0);
    bool ok = model.setData(idx, QVariant::fromValue(QUrl("file:///changed.jpg")), Types::ImageUrlRole);
    EXPECT_TRUE(ok);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(model.data(idx, Types::ImageUrlRole).toUrl(), QUrl("file:///changed.jpg"));
}

// 测试 setData 非法索引返回 false
TEST_F(ut_imagesourcemodel, SetDataInvalidIndexReturnsFalse)
{
    ImageSourceModel model;
    bool ok = model.setData(QModelIndex(), QVariant::fromValue(QUrl("file:///x.jpg")), Types::ImageUrlRole);
    EXPECT_FALSE(ok);
}

// 测试 setData 未知 role 返回 false
TEST_F(ut_imagesourcemodel, SetDataUnknownRoleReturnsFalse)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///a.jpg");
    model.setImageFiles(files);

    bool ok = model.setData(model.index(0), QVariant::fromValue(QUrl("file:///y.jpg")), Qt::UserRole + 100);
    EXPECT_FALSE(ok);
}

// 测试 indexForImagePath 命中/未命中/空
TEST_F(ut_imagesourcemodel, IndexForImagePath)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///a.jpg") << QUrl("file:///b.jpg");
    model.setImageFiles(files);

    EXPECT_EQ(model.indexForImagePath(QUrl("file:///b.jpg")), 1);
    EXPECT_EQ(model.indexForImagePath(QUrl("file:///notexist.jpg")), -1);
    EXPECT_EQ(model.indexForImagePath(QUrl()), -1);
}

// 测试 removeImage 移除存在的项
TEST_F(ut_imagesourcemodel, RemoveExistingImage)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///a.jpg") << QUrl("file:///b.jpg") << QUrl("file:///c.jpg");
    model.setImageFiles(files);

    model.removeImage(QUrl("file:///b.jpg"));
    EXPECT_EQ(model.rowCount(), 2);
    EXPECT_EQ(model.data(model.index(1), Types::ImageUrlRole).toUrl(), QUrl("file:///c.jpg"));
}

// 测试 removeImage 移除不存在的项不报错
TEST_F(ut_imagesourcemodel, RemoveNonexistingImage)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///a.jpg");
    model.setImageFiles(files);

    model.removeImage(QUrl("file:///nope.jpg"));
    EXPECT_EQ(model.rowCount(), 1);
}

// 测试 rowCount 带父索引参数
TEST_F(ut_imagesourcemodel, RowCountWithParent)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///a.jpg") << QUrl("file:///b.jpg");
    model.setImageFiles(files);
    EXPECT_EQ(model.rowCount(QModelIndex()), 2);
}

// 测试 insertImage: 插入新图片(排序在已有图片之前)触发 L137-138
TEST_F(ut_imagesourcemodel, InsertImage_NewImageSortsBeforeExisting)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///c.jpg") << QUrl("file:///d.jpg");
    model.setImageFiles(files);

    // 插入 b.jpg，其 baseName 排在 c.jpg 之前
    int idx = model.insertImage(QUrl("file:///b.jpg"));
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(model.rowCount(), 3);
    EXPECT_EQ(model.data(model.index(0), Types::ImageUrlRole).toUrl(), QUrl("file:///b.jpg"));
}

// 测试 insertImage: 插入已存在的图片返回已有索引
TEST_F(ut_imagesourcemodel, InsertImage_ExistingImageReturnsIndex)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///a.jpg") << QUrl("file:///b.jpg");
    model.setImageFiles(files);

    int idx = model.insertImage(QUrl("file:///b.jpg"));
    EXPECT_EQ(idx, 1);
    EXPECT_EQ(model.rowCount(), 2);
}

// 测试 insertImage: 空路径返回 -1
TEST_F(ut_imagesourcemodel, InsertImage_EmptyPathReturnsNegative)
{
    ImageSourceModel model;
    int idx = model.insertImage(QUrl());
    EXPECT_EQ(idx, -1);
}

// 测试 insertImage: 插入到末尾
TEST_F(ut_imagesourcemodel, InsertImage_AppendToEnd)
{
    ImageSourceModel model;
    QList<QUrl> files;
    files << QUrl("file:///a.jpg") << QUrl("file:///b.jpg");
    model.setImageFiles(files);

    int idx = model.insertImage(QUrl("file:///z.jpg"));
    EXPECT_EQ(idx, 2);
    EXPECT_EQ(model.rowCount(), 3);
}
