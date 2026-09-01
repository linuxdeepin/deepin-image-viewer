// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | ImageSourceModel(parent) | low | - | 1 | 1 |
// | ~ImageSourceModel | low | - | 1 | 1 |
// | data(index,role) | mid | - | 2 | 5 |
// | indexForImagePath(file) | low | - | 1 | 4 |
// | insertImage(file) | low | - | 1 | 5 |
// | removeImage(fileName) | mid | - | 2 | 3 |
// | roleNames() | low | - | 1 | 1 |
// | rowCount(parent) | low | - | 1 | 2 |
// | setData(index,value,role) | low | - | 1 | 3 |
// | setImageFiles(files) | mid | - | 2 | 3 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（10 个方法全覆盖，含构造/析构）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（空/单/多元素列表；空/重复/缺失 URL；
//    合法/未知/负数 role；空模型/非空模型）
// 3. 每个等价类的边界值显式覆盖: [x]（首行 0/末行 size-1、空 URL、空列表、
//    数值序 img2 < img10 的 QCollator numericMode 边界）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（Data_RoleLookup_ParamSet 4 组、
//    IndexForImagePath_Lookup_ParamSet 4 组）
// 5. 分支清单 → 用例映射已列出: [x]（见下方 data/insertImage/setData 分支清单块，
//    其余方法分支数 <3 且 complexity<10，按 §4.1 允许省略）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（见各分支清单映射）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无显式 throw；错误路径以
//    无效 QVariant/false 返回值断言覆盖）
// 8. 负面场景有专门用例: [x]（Empty/Missing/Invalid/Unknown 系列）
// 9. 负面用例验证强异常安全: [x]（setData/insertImage 失败路径后模型行数与
//    既有数据保持原值断言）
// 10. stub_ext vs gMock 选择正确: [x]（纯内存 QAbstractListModel，无外部依赖，
//     无需 stub；副作用经 QSignalSpy 断言）
//
// 说明：
// - insertImage/setImageFiles 仅做 QFileInfo::baseName 字符串运算，不访问文件系统，
//   URL 一律由 QTemporaryDir 路径构造，避免硬编码绝对路径。
// - data()/setData() 依赖 checkIndex(ParentIsInvalid|IndexIsValid)，用例只构造
//   界内索引与默认构造 QModelIndex()（invalid），不构造越界索引（QList::at 越界为 UB）。
//
// ─────────────────────────────────────────────────────────────
// 分支清单（来源：ImageSourceModel::data(const QModelIndex&,int)）
// ─────────────────────────────────────────────────────────────
// B1: !checkIndex(ParentIsInvalid|IndexIsValid) → 提前 return {}
// B2: role == Types::ImageUrlRole → return imageUrlList.at(row)
// B3: default（未知 role）→ break 落到末尾
// B4: 无效索引早退 return {}（同 B1 路径的返回）
// B5: 末尾兜底 return {}（未知 role 的返回）
// 映射： Data_RoleLookup_ParamSet（ImageUrlRole 实例）→ B2
//        Data_RoleLookup_ParamSet（role=0/-1/Qt::DecorationRole 实例）→ B3+B5
//        Data_InvalidIndex_ReturnsInvalidVariant → B1+B4（对照有效索引 → B2）
//
// 分支清单（来源：ImageSourceModel::insertImage(QUrl)）
// ─────────────────────────────────────────────────────────────
// B1: file.isEmpty() → return -1
// B2: existingIndex != -1（已存在）→ return existingIndex
// B3: for 遍历既有条目（空/单/多元素循环边界）
// B4: sortCollator.compare(baseName, current) < 0 → insertIndex=i; break（前插）
// B5: 循环未命中 → insertIndex=size（尾部追加）
// B6: return insertIndex（beginInsertRows/endInsertRows 后）
// 映射： InsertImage_EmptyUrl_ReturnsMinusOneWithoutRowChange → B1
//        InsertImage_DuplicateUrl_ReturnsExistingIndexWithoutInsert → B2
//        InsertImage_EmptyModel_InsertsAtFirstPosition → B3（0 次迭代）+B5+B6
//        InsertImage_LowerBaseName_InsertedInFront → B3+B4+B6
//        InsertImage_NumericBaseNames_SortedNumerically → B3+B4+B5+B6
//
// 分支清单（来源：ImageSourceModel::setData(QModelIndex,QVariant,int)）
// ─────────────────────────────────────────────────────────────
// B1: !checkIndex(ParentIsInvalid|IndexIsValid) → 提前 return false
// B2: case Types::ImageUrlRole → replace + Q_EMIT dataChanged
// B3: default（未知 role）→ break 落到末尾
// B4: ImageUrlRole 成功路径 return true（早退返回）
// B5: 末尾兜底 return false
// 映射： SetData_ValidIndexAndImageUrlRole_UpdatesValueAndEmitsDataChanged → B2+B4
//        SetData_InvalidIndex_ReturnsFalseWithoutSignal → B1
//        SetData_UnknownRole_ReturnsFalseAndKeepsValue → B3+B5

#include <gtest/gtest.h>

#include <QAbstractItemModel>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>

#include "imagesourcemodel.h"
#include "stub_ext/stubext.h"
#include "types.h"

namespace {
// 由临时目录构造本地文件 URL（仅作字符串来源，不落盘）
QUrl localUrl(const QTemporaryDir &dir, const QString &name)
{
    return QUrl::fromLocalFile(dir.filePath(name));
}
}  // namespace

class ImageSourceModelTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        model = new ImageSourceModel();
    }

    void TearDown() override
    {
        delete model;
        model = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    QTemporaryDir tmpDir;
    ImageSourceModel *model = nullptr;
};

// ── 构造 / 析构 ───────────────────────────────────────────────────

TEST_F(ImageSourceModelTest, ImageSourceModel_Constructor_StartsEmptyWithZeroRows)
{
    // Arrange：SetUp 已构造空模型
    const QUrl probe = localUrl(tmpDir, QStringLiteral("a.png"));

    // Act
    const int rows = model->rowCount();
    const int found = model->indexForImagePath(probe);

    // Assert
    EXPECT_EQ(rows, 0);
    EXPECT_EQ(found, -1);
}

TEST_F(ImageSourceModelTest, ImageSourceModel_Destructor_ModelWithRowsEndsCleanly)
{
    // Arrange
    ImageSourceModel *heap = new ImageSourceModel();
    heap->setImageFiles({localUrl(tmpDir, QStringLiteral("a.png")),
                         localUrl(tmpDir, QStringLiteral("b.png"))});

    // Act：delete 触发析构
    EXPECT_NO_THROW(delete heap);

    // Assert：析构不影响其它模型实例
    EXPECT_EQ(model->rowCount(), 0);
    EXPECT_EQ(model->indexForImagePath(localUrl(tmpDir, QStringLiteral("a.png"))), -1);
}

// ── setImageFiles ─────────────────────────────────────────────────

TEST_F(ImageSourceModelTest, SetImageFiles_NonEmptyList_ReplacesRowsAndResetsModel)
{
    // Arrange
    QSignalSpy resetSpy(model, &QAbstractItemModel::modelReset);
    const QList<QUrl> files{localUrl(tmpDir, QStringLiteral("a.png")),
                            localUrl(tmpDir, QStringLiteral("b.png")),
                            localUrl(tmpDir, QStringLiteral("c.png"))};

    // Act
    model->setImageFiles(files);

    // Assert
    EXPECT_EQ(model->rowCount(), 3);
    EXPECT_EQ(model->data(model->index(1, 0), Types::ImageUrlRole).toUrl(), files.at(1));
    EXPECT_EQ(resetSpy.count(), 1);
}

TEST_F(ImageSourceModelTest, SetImageFiles_EmptyList_ClearsAllRows)
{
    // Arrange
    model->setImageFiles({localUrl(tmpDir, QStringLiteral("a.png")),
                          localUrl(tmpDir, QStringLiteral("b.png"))});
    QSignalSpy resetSpy(model, &QAbstractItemModel::modelReset);
    ASSERT_EQ(resetSpy.count(), 0);

    // Act：空列表（边界）
    model->setImageFiles({});

    // Assert
    EXPECT_EQ(model->rowCount(), 0);
    EXPECT_EQ(resetSpy.count(), 1);
}

TEST_F(ImageSourceModelTest, SetImageFiles_SecondCall_OverwritesPreviousEntries)
{
    // Arrange
    model->setImageFiles({localUrl(tmpDir, QStringLiteral("a.png")),
                          localUrl(tmpDir, QStringLiteral("b.png"))});
    const QUrl replacement = localUrl(tmpDir, QStringLiteral("c.png"));

    // Act
    model->setImageFiles({replacement});

    // Assert：旧条目整体被替换
    EXPECT_EQ(model->rowCount(), 1);
    EXPECT_EQ(model->indexForImagePath(localUrl(tmpDir, QStringLiteral("a.png"))), -1);
    EXPECT_EQ(model->indexForImagePath(replacement), 0);
}

// ── insertImage ───────────────────────────────────────────────────

TEST_F(ImageSourceModelTest, InsertImage_EmptyUrl_ReturnsMinusOneWithoutRowChange)
{
    // Arrange
    QSignalSpy rowsSpy(model, &QAbstractItemModel::rowsInserted);

    // Act：空 URL（边界）
    const int index = model->insertImage(QUrl());

    // Assert
    EXPECT_EQ(index, -1);
    EXPECT_EQ(model->rowCount(), 0);
    EXPECT_EQ(rowsSpy.count(), 0);
}

TEST_F(ImageSourceModelTest, InsertImage_EmptyModel_InsertsAtFirstPosition)
{
    // Arrange
    QSignalSpy rowsSpy(model, &QAbstractItemModel::rowsInserted);
    const QUrl first = localUrl(tmpDir, QStringLiteral("a.png"));

    // Act
    const int index = model->insertImage(first);

    // Assert
    EXPECT_EQ(index, 0);
    EXPECT_EQ(model->rowCount(), 1);
    EXPECT_EQ(rowsSpy.count(), 1);
    EXPECT_EQ(rowsSpy.at(0).at(1).toInt(), 0);  // 首参 row = 0
}

TEST_F(ImageSourceModelTest, InsertImage_DuplicateUrl_ReturnsExistingIndexWithoutInsert)
{
    // Arrange
    const QUrl a = localUrl(tmpDir, QStringLiteral("a.png"));
    const QUrl b = localUrl(tmpDir, QStringLiteral("b.png"));
    ASSERT_EQ(model->insertImage(a), 0);
    ASSERT_EQ(model->insertImage(b), 1);
    QSignalSpy rowsSpy(model, &QAbstractItemModel::rowsInserted);

    // Act：重复插入已有 URL
    const int index = model->insertImage(a);

    // Assert：返回既有下标，不新增行（强异常安全）
    EXPECT_EQ(index, 0);
    EXPECT_EQ(model->rowCount(), 2);
    EXPECT_EQ(rowsSpy.count(), 0);
}

TEST_F(ImageSourceModelTest, InsertImage_LowerBaseName_InsertedInFront)
{
    // Arrange：字典序 c > b
    ASSERT_EQ(model->insertImage(localUrl(tmpDir, QStringLiteral("b.png"))), 0);
    ASSERT_EQ(model->insertImage(localUrl(tmpDir, QStringLiteral("c.png"))), 1);
    const QUrl small = localUrl(tmpDir, QStringLiteral("a.png"));

    // Act
    const int index = model->insertImage(small);

    // Assert：插入到最前，原有顺序后移
    EXPECT_EQ(index, 0);
    EXPECT_EQ(model->data(model->index(0, 0), Types::ImageUrlRole).toUrl(), small);
    EXPECT_EQ(model->data(model->index(2, 0), Types::ImageUrlRole).toUrl().fileName(),
              QStringLiteral("c.png"));
}

TEST_F(ImageSourceModelTest, InsertImage_NumericBaseNames_SortedNumerically)
{
    // Arrange：QCollator numericMode 使 img2 < img10（数字序而非字典序）
    ASSERT_EQ(model->insertImage(localUrl(tmpDir, QStringLiteral("img10.png"))), 0);

    // Act
    const int img2Index = model->insertImage(localUrl(tmpDir, QStringLiteral("img2.png")));
    const int img5Index = model->insertImage(localUrl(tmpDir, QStringLiteral("img5.png")));

    // Assert：img2 排在 img10 前，img5 落在两者之间
    EXPECT_EQ(img2Index, 0);
    EXPECT_EQ(img5Index, 1);
    EXPECT_EQ(model->rowCount(), 3);
}

// ── removeImage ───────────────────────────────────────────────────

TEST_F(ImageSourceModelTest, RemoveImage_ExistingUrl_RemovesRowAndEmitsRowsRemoved)
{
    // Arrange
    const QList<QUrl> files{localUrl(tmpDir, QStringLiteral("a.png")),
                            localUrl(tmpDir, QStringLiteral("b.png")),
                            localUrl(tmpDir, QStringLiteral("c.png"))};
    model->setImageFiles(files);
    QSignalSpy removedSpy(model, &QAbstractItemModel::rowsRemoved);

    // Act
    model->removeImage(files.at(1));

    // Assert
    EXPECT_EQ(model->rowCount(), 2);
    EXPECT_EQ(removedSpy.count(), 1);
    EXPECT_EQ(model->indexForImagePath(files.at(1)), -1);
    EXPECT_EQ(model->data(model->index(1, 0), Types::ImageUrlRole).toUrl(), files.at(2));
}

TEST_F(ImageSourceModelTest, RemoveImage_MissingUrl_KeepsModelUnchanged)
{
    // Arrange
    const QUrl kept = localUrl(tmpDir, QStringLiteral("a.png"));
    model->setImageFiles({kept});
    QSignalSpy removedSpy(model, &QAbstractItemModel::rowsRemoved);

    // Act：移除不存在的 URL（负面）
    model->removeImage(localUrl(tmpDir, QStringLiteral("ghost.png")));

    // Assert：模型保持原状（强异常安全）
    EXPECT_EQ(model->rowCount(), 1);
    EXPECT_EQ(removedSpy.count(), 0);
    EXPECT_EQ(model->indexForImagePath(kept), 0);
}

TEST_F(ImageSourceModelTest, RemoveImage_RepeatedUntilEmpty_ModelClears)
{
    // Arrange
    const QList<QUrl> files{localUrl(tmpDir, QStringLiteral("a.png")),
                            localUrl(tmpDir, QStringLiteral("b.png"))};
    model->setImageFiles(files);

    // Act：逐行移除至空（循环边界：末行）
    for (const QUrl &url : files)
        model->removeImage(url);

    // Assert
    EXPECT_EQ(model->rowCount(), 0);
    EXPECT_EQ(model->indexForImagePath(files.at(0)), -1);
}

// ── data（参数化：role 等价类） ───────────────────────────────────

namespace {
struct DataRoleCase {
    int role;
    bool expectedValid;
};
}  // namespace

class ImageSourceModelDataParamTest : public ImageSourceModelTest,
                                      public ::testing::WithParamInterface<DataRoleCase> {};

TEST_P(ImageSourceModelDataParamTest, Data_RoleLookup_ParamSet)
{
    // Arrange
    const QUrl urlA = localUrl(tmpDir, QStringLiteral("a.png"));
    model->setImageFiles({urlA});
    const QModelIndex index = model->index(0, 0);
    const DataRoleCase c = GetParam();

    // Act
    const QVariant result = model->data(index, c.role);

    // Assert
    EXPECT_EQ(result.isValid(), c.expectedValid);
    EXPECT_EQ(result.toUrl(), c.expectedValid ? urlA : QUrl());
}

INSTANTIATE_TEST_SUITE_P(
    ModelRoles, ImageSourceModelDataParamTest,
    ::testing::Values(
        DataRoleCase{Types::ImageUrlRole, true},   // 唯一支持的角色
        DataRoleCase{0, false},                    // 边界：role 0
        DataRoleCase{-1, false},                   // 边界：负数 role
        DataRoleCase{Qt::DecorationRole, false})); // 未知 Qt 角色

TEST_F(ImageSourceModelTest, Data_InvalidIndex_ReturnsInvalidVariant)
{
    // Arrange
    const QUrl urlA = localUrl(tmpDir, QStringLiteral("a.png"));
    model->setImageFiles({urlA});

    // Act：默认构造 QModelIndex()（invalid）
    const QVariant result = model->data(QModelIndex(), Types::ImageUrlRole);

    // Assert：对照有效索引返回正常值
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(model->data(model->index(0, 0), Types::ImageUrlRole).toUrl(), urlA);
}

// ── setData ───────────────────────────────────────────────────────

TEST_F(ImageSourceModelTest, SetData_ValidIndexAndImageUrlRole_UpdatesValueAndEmitsDataChanged)
{
    // Arrange
    const QUrl original = localUrl(tmpDir, QStringLiteral("a.png"));
    model->setImageFiles({original});
    QSignalSpy changedSpy(model, &QAbstractItemModel::dataChanged);
    const QUrl updated = localUrl(tmpDir, QStringLiteral("z.png"));

    // Act
    const bool ok = model->setData(model->index(0, 0), QVariant::fromValue(updated),
                                   Types::ImageUrlRole);

    // Assert
    EXPECT_TRUE(ok);  // branch: case ImageUrlRole → return true
    EXPECT_EQ(model->data(model->index(0, 0), Types::ImageUrlRole).toUrl(), updated);
    EXPECT_EQ(changedSpy.count(), 1);
}

TEST_F(ImageSourceModelTest, SetData_InvalidIndex_ReturnsFalseWithoutSignal)
{
    // Arrange
    const QUrl original = localUrl(tmpDir, QStringLiteral("a.png"));
    model->setImageFiles({original});
    QSignalSpy changedSpy(model, &QAbstractItemModel::dataChanged);

    // Act：invalid 索引（负面）
    const bool ok = model->setData(QModelIndex(), QVariant::fromValue(original),
                                   Types::ImageUrlRole);

    // Assert
    EXPECT_FALSE(ok);  // branch: !checkIndex → return false
    EXPECT_EQ(changedSpy.count(), 0);
    EXPECT_EQ(model->data(model->index(0, 0), Types::ImageUrlRole).toUrl(), original);
}

TEST_F(ImageSourceModelTest, SetData_UnknownRole_ReturnsFalseAndKeepsValue)
{
    // Arrange
    const QUrl original = localUrl(tmpDir, QStringLiteral("a.png"));
    model->setImageFiles({original});
    QSignalSpy changedSpy(model, &QAbstractItemModel::dataChanged);
    const QUrl other = localUrl(tmpDir, QStringLiteral("z.png"));

    // Act：未支持的 role（负面）
    const bool ok = model->setData(model->index(0, 0), QVariant::fromValue(other),
                                   Qt::DecorationRole);

    // Assert：值未被改动（强异常安全）
    EXPECT_FALSE(ok);  // branch: default → 末尾 return false
    EXPECT_EQ(changedSpy.count(), 0);
    EXPECT_EQ(model->data(model->index(0, 0), Types::ImageUrlRole).toUrl(), original);
}

// ── roleNames ─────────────────────────────────────────────────────

TEST_F(ImageSourceModelTest, RoleNames_ImageUrlRole_MapsToQmlName)
{
    // Arrange：先放入一条数据，排除空模型干扰
    model->setImageFiles({localUrl(tmpDir, QStringLiteral("a.png"))});

    // Act
    const QHash<int, QByteArray> names = model->roleNames();

    // Assert
    EXPECT_EQ(names.size(), 1);
    EXPECT_EQ(names.value(Types::ImageUrlRole), QByteArray("imageUrl"));
}

// ── rowCount ──────────────────────────────────────────────────────

TEST_F(ImageSourceModelTest, RowCount_EmptyModel_ReturnsZero)
{
    // Arrange：空模型（SetUp 构造后未填充数据），parent 取默认无效索引
    const QModelIndex noParent;

    // Act
    const int rows = model->rowCount();
    const int rowsWithParent = model->rowCount(noParent);

    // Assert
    EXPECT_EQ(rows, 0);
    EXPECT_EQ(rowsWithParent, 0);
}

TEST_F(ImageSourceModelTest, RowCount_WithParentIndexArgument_ReturnsTotalCount)
{
    // Arrange：3 行数据；parent 参数被 Q_UNUSED
    model->setImageFiles({localUrl(tmpDir, QStringLiteral("a.png")),
                          localUrl(tmpDir, QStringLiteral("b.png")),
                          localUrl(tmpDir, QStringLiteral("c.png"))});

    // Act：传入默认 parent 与一个有效索引作 parent
    const int withDefaultParent = model->rowCount(QModelIndex());
    const int withIndexParent = model->rowCount(model->index(0, 0));

    // Assert：均为总行数
    EXPECT_EQ(withDefaultParent, 3);
    EXPECT_EQ(withIndexParent, 3);
}

// ── indexForImagePath（参数化：命中/未命中/空） ────────────────────

namespace {
struct PathLookupCase {
    QString name;          // 文件名；空串代表空 URL 输入
    int expectedIndex;     // 期望返回下标，-1 表示未命中
};
}  // namespace

class ImageSourceModelPathParamTest : public ImageSourceModelTest,
                                      public ::testing::WithParamInterface<PathLookupCase> {};

TEST_P(ImageSourceModelPathParamTest, IndexForImagePath_Lookup_ParamSet)
{
    // Arrange
    model->setImageFiles({localUrl(tmpDir, QStringLiteral("a.png")),
                          localUrl(tmpDir, QStringLiteral("b.png"))});
    const PathLookupCase c = GetParam();
    const QUrl url = c.name.isEmpty()
                             ? QUrl()
                             : localUrl(tmpDir, c.name);

    // Act
    const int index = model->indexForImagePath(url);

    // Assert
    EXPECT_EQ(index, c.expectedIndex);
    if (index >= 0)
        EXPECT_EQ(model->data(model->index(index, 0), Types::ImageUrlRole).toUrl(), url);
    else
        EXPECT_EQ(model->indexForImagePath(url), -1);  // 幂等：再次查找仍未命中
}

INSTANTIATE_TEST_SUITE_P(
    PathLookups, ImageSourceModelPathParamTest,
    ::testing::Values(
        PathLookupCase{QStringLiteral("a.png"), 0},      // 命中首行
        PathLookupCase{QStringLiteral("b.png"), 1},      // 命中末行
        PathLookupCase{QStringLiteral("zz.png"), -1},    // 未命中
        PathLookupCase{QStringLiteral(""), -1}));        // 边界：空 URL
