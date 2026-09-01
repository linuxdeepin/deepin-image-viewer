// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | PathViewProxyModel(srcModel,parent) | low | - | 1 | 1 |
// | ~PathViewProxyModel | low | destructor | 1 | 1 |
// | roleNames() | low | - | 1 | 1 |
// | data(index,role) | mid | complexity:5,in_degree:5 | 2 | 8 |
// | rowCount(parent) | low | - | 1 | 1 |
// | currentIndex() | mid | in_degree:5 | 2 | 2 |
// | setCurrentIndex(index) | low | - | 1 | 2 |
// | setQueueCount(count) | low | - | 1 | 3 |
// | dumpInfo() | low | - | 1 | 1 |
// | setCurrentSourceIndex(srcIdx,frameIdx) | low | - | 1 | 6 |
// | movePrevoius() | low | - | 1 | 1 |
// | moveNext() | low | - | 1 | 2 |
// | resetModel(srcIdx,frameIdx) | mid | alloc_in_loop:1 | 3 | 3 |
// | deleteCurrent() | mid | lines:62 | 2 | 5 |
// | syncState() | low | - | 1 | 2 |
// | jumpToIndex(srcIdx,frameIdx) | low | - | 1 | 3 |
// | refreshBothSideData() | low | - | 1 | 2 |
// | sourcePath(sourceIndex) | low | - | 1 | 4 |
// | distance(srcIdx,frameIdx) | high | complexity:10 | 3 | 10 |
// | previousPorxyIdx(base) | low | - | 1 | 3 |
// | nextProxyIdx(base) | low | - | 1 | 3 |
// | infoFromIndex(srcIdx,frameIdx) | low | - | 1 | 3 |
// | asyncUpdateLoadInfo(url,srcIdx,frameIdx) | high | complexity:8 | 3 | 6 |
// | createPreviousIndexInfo(baseInfo) | low | - | 1 | 4 |
// | createNextIndexInfo(baseInfo) | low | - | 1 | 4 |
// | updateIndexInfo(proxyIndex,info) | low | - | 1 | 1 |
// | setData(index,value,role) | high | complexity:11 | 3 | 7 |
// | IndexInfo(other) | low | - | 1 | 2 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x] （28 个 testable 方法全部有以方法名开头的用例）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x] （源索引合法/越界/负数、frameIndex 0/非 0/越界、角色支持/不支持、队列空/非空/槽位空）
// 3. 每个等价类的边界值显式覆盖: [x] （源索引 -1/0/N-1/N、代理索引 0/size-1 环绕、frameIndex=2 越界、图片数 0/2/5）
// 4. 同质 ≥ 3 组用 TEST_P: [x] （data×5、distance×7、sourcePath×4、next/previousPorxyIdx×3、setQueueCount×3）
// 5. 分支清单 → 用例映射已列出: [x] （见下方分支清单）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x] （源码无 throw，其余分支均有映射）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x] （源码无 throw 分支，不适用）
// 8. 负面场景有专门用例: [x] （无效索引/越界索引/空队列/空槽位/非 0 帧索引/越界帧）
// 9. 负面用例验证强异常安全: [x] （早退分支均断言队列与 currentIndex 不变）
// 10. stub_ext vs gMock 选择正确: [x] （Qt 类/项目内无虚依赖一律 stub_ext，无 gMock）
//
// 环境隔离说明（SetUp 全局生效）：
// - stub ImageInfoCache::load → 阻止真实图片解码与线程池任务，测试不触盘
// - 源数据 URL 使用无 scheme 的相对路径（ut-pathviewproxy-src-N.png）：
//   QUrl::toLocalFile() 为空 → ImageInfo::refreshDataFromCache 走 Error 分支，
//   infoFromIndex 不会级联 asyncUpdateLoadInfo，frameCount 固定为 1，行为完全确定
// - file:/// 前缀的 URL 仅用于需要 Loading 状态（异步补偿链）的用例，load 已被 stub
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:48-65 PathViewProxyModel::data）
// B1: if (!checkIndex(...)) → return {} 非法索引早退
// B2: case Types::ImageUrlRole → 进入 URL 角色分支
// B3: 三元 infoPtr 非空 → return infoPtr->url
// B4: 三元 infoPtr 为空 → return QUrl()
// B5: case Types::FrameIndexRole → 进入帧索引角色分支
// B6: 三元 infoPtr 非空 → return infoPtr->frameIndex
// B7: 三元 infoPtr 为空 → return 0
// B8: default → break
// B9: 末尾 return {} 未匹配角色
// 用例映射：
// - Data_ByRowAndRole_ReturnsExpectedVariant（TEST_P×5）          → B2/B3/B5/B6/B8/B9
// - Data_InvalidModelIndex_ReturnsEmptyVariant                    → B1
// - Data_RowOutOfRange_ReturnsEmptyVariant                        → B1
// - Data_NullInfoSlot_ReturnsUrlAndFrameDefaults                  → B4/B7
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:67-110 PathViewProxyModel::setData）
// B1: if (!checkIndex(...)) → return false 非法索引早退
// B2: case Types::ImageUrlRole → 进入 URL 设置分支
// B3: if (infoPtr) 非空 → 执行更新
// B4: Q_EMIT dataChanged(idx, idx) 自身槽位更新
// B5: for previous 回扫（idx.row()-1 → 0）
// B6: if (preInfo) 且 url == oldUrl → 更新 + dataChanged
// B7: for next 前扫（idx.row()+1 → size-1）
// B8: if (nextInfo) 且 url == oldUrl → 更新 + dataChanged
// B9: return true（ImageUrlRole 分支成功）
// B10: default → break
// B11: 末尾 return false 不支持角色
// B12: infoPtr 为空 → 跳过更新直接 return true
// 用例映射：
// - SetData_InvalidModelIndex_ReturnsFalse                        → B1
// - SetData_RowOutOfRange_ReturnsFalse                            → B1
// - SetData_UnsupportedRole_ReturnsFalse                          → B2/B10/B11
// - SetData_ImageUrlRole_UpdatesUrlAndEmitsChanged                → B2/B3/B4/B9
// - SetData_ImageUrlRole_PropagatesForwardToDuplicateUrl          → B7/B8
// - SetData_ImageUrlRole_PropagatesBackwardToDuplicateUrl         → B5/B6
// - SetData_NullInfoSlot_ReturnsTrueWithoutEmit                   → B12
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:464-513 PathViewProxyModel::distance）
// B1: if (sourceIndex < 0 || >= sourceModel->rowCount()) → Invalid 早退 #1
// B2: if (indexQueue.isEmpty()) → Invalid 早退 #2
// B3: if (!current) → Invalid 早退 #3
// B4: case Current → range = frameIndex - current->frameIndex
// B5: case Previous → 取 previousPorxyIdx 槽位
// B6: if (previous) 非空 → range = -(frameCount - frameIndex) - current->frameIndex
// B7: case Next → range = current->frameCount - current->frameIndex + frameIndex
// B8: default → range 保持 OutOfRange
// B9: if (Previous <= range && range <= Next) → 界内
// B10: 界内 → return static_cast<DistanceType>(range)
// B11: 界外 → 末尾 return OutOfRange
// B12: return Invalid（B1 早退语句）
// B13: return Invalid（B2 早退语句）
// 用例映射：
// - Distance_BySourceAndFrameIndex_ReturnsExpectedType（TEST_P×7） → B1/B4/B5/B6/B7/B8/B9/B10/B11
// - Distance_EmptyQueue_ReturnsInvalid                            → B2/B13
// - Distance_NullCurrentInfo_ReturnsInvalid                       → B3
// - Distance_NullPreviousInfo_ReturnsOutOfRange                   → B5（previous 为空 → B11）
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:284-345 PathViewProxyModel::deleteCurrent）
// B1: if (indexQueue.isEmpty()) → 早退 return #1
// B2: if (0 == sourceModel->rowCount()) → 重置模型清空队列 + 早退 return #2
// B3: if (!current) → 早退 return #3
// B4: if (atEnd) → 更新前一张 + setCurrentIndex(previousIdx) + refreshBothSideData
// B5: else（非末尾）→ 更新后一张 + setCurrentIndex(nextIdx) + refreshBothSideData
// B6: B1 分支的早退 return 语句
// 用例映射：
// - DeleteCurrent_EmptyQueue_EarlyReturnKeepsState               → B1/B6
// - DeleteCurrent_EmptySourceModel_ResetsQueueToEmpty            → B2
// - DeleteCurrent_NullCurrentInfo_EarlyReturnKeepsQueue          → B3
// - DeleteCurrent_LastImage_MovesToPreviousImage                 → B4
// - DeleteCurrent_MiddleImage_MovesToNextImage                   → B5
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:369-385 PathViewProxyModel::dumpInfo）
// B1: for 遍历 indexQueue
// B2: if (info) 非空 → 打印详情 / else 打印空槽位
// 用例映射：
// - DumpInfo_MixedEntries_KeepsStateIntact                       → B1/B2
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:557-581 PathViewProxyModel::infoFromIndex）
// B1: if (url.isEmpty()) → return {} 空指针
// B2: if (ImageInfo::Loading == imageInfo.status()) → asyncUpdateLoadInfo 异步补偿
// B3: B1 分支的早退 return 语句
// 用例映射：
// - InfoFromIndex_ValidSourceIndex_ReturnsFilledInfo              → B2 未触发（Error 态）+ 正常构造
// - InfoFromIndex_OutOfRangeIndex_ReturnsNullInfo                 → B1/B3
// - InfoFromIndex_LoadingFileUrl_SchedulesAsyncLoadInfo           → B2 触发
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:587-650 PathViewProxyModel::asyncUpdateLoadInfo）
// B1: if (0 == frameIndex) → 创建 delayInfo 挂异步链
// B2: if (ImageInfo::Loading == delayInfo->status()) → return 等待终态
// B3: if (ImageInfo::Ready == status && Types::MultiImage == type) → 进入更新分支
// B4: if (!current) → deleteLater + return
// B5: if (sourceIndex == current->index) → 更新 frameCount + refreshBothSideData
// B6: else if (sourceIndex < current->index) → 进入前侧回填
// B7: for (i < radius) 前侧循环 updateIndexInfo
// B8: else 后侧回填的 for (i < radius) 循环
// B9: B2/B4 早退 return 语句
// 用例映射：
// - AsyncUpdateLoadInfo_NonZeroFrameIndex_SkipsAsyncLoad          → B1 不成立
// - AsyncUpdateLoadInfo_NullCurrentInfo_IgnoresUpdate             → B4/B9
// - AsyncUpdateLoadInfo_SourceEqualsCurrent_UpdatesFrameAndSides  → B5
// - AsyncUpdateLoadInfo_SourceBeforeCurrent_RefreshesPreviousSide → B6/B7
// - AsyncUpdateLoadInfo_SourceAfterCurrent_RefreshesNextSide      → B8
// - AsyncUpdateLoadInfo_NotReadyMultiImage_IgnoresUpdate          → B3 不成立
// - AsyncUpdateLoadInfo_LoadingStatus_WaitsForTerminalState       → B2
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:241-279 PathViewProxyModel::resetModel）
// B1: while ((indexQueue.size() + prependQueue.size()) < maxCount) 循环填充
// B2: if (prependQueue.isEmpty()) → 首轮前驱取自 indexQueue.first()
// B3: else → 前驱取自 prependQueue.first()
// B4: 区间右侧 append createNextIndexInfo（越界时为空槽位）
// 用例映射：
// - ResetModel_MoreImagesThanQueue_WrapsSourceIndices             → B1/B2/B3/B4（N 次循环）
// - ResetModel_FewerImagesThanQueue_FillsTailWithNull             → B4 越界边界（alloc_in_loop 循环边界）
// - ResetModel_EmptySource_AllSlotsNull                           → B1 0 张图片边界
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:161-198 PathViewProxyModel::setCurrentSourceIndex）
// B1: if (indexQueue.isEmpty()) → return 早退
// B2: if (Current != type && Current != jumpFlag) → refreshBothSideData 打断动画
// B3: case Previous → movePrevoius()
// B4: case Next → moveNext()
// B5: case OutOfRange → jumpToIndex()
// B6: default（Current/Invalid）→ 无动作
// 用例映射：
// - SetCurrentSourceIndex_EmptyQueue_EarlyReturn                  → B1
// - SetCurrentSourceIndex_CurrentDistance_KeepsPosition           → B6
// - SetCurrentSourceIndex_NextDistance_MovesNext                  → B4
// - SetCurrentSourceIndex_PreviousDistance_MovesPrevious          → B3
// - SetCurrentSourceIndex_OutOfRangeDistance_JumpsToTarget        → B5
// - SetCurrentSourceIndex_JumpInProgress_RefreshesSidesThenMoves  → B2/B3
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:683-700 PathViewProxyModel::createNextIndexInfo）
// B1: if (!baseInfo) → return {} 空指针
// B2: if (frameCount - 1 > frameIndex) → 复制本图并 frameIndex++
// B3: 跨图 → infoFromIndex(index + 1)
// B4: B1 分支的早退 return 语句
// 用例映射：
// - CreateNextIndexInfo_NullBase_ReturnsNullInfo                  → B1/B4
// - CreateNextIndexInfo_MiddleFrame_ReturnsNextFrameInfo          → B2
// - CreateNextIndexInfo_LastFrame_ReturnsNextImageInfo            → B3
// - CreateNextIndexInfo_LastImageInSource_ReturnsNullInfo         → B3 越界
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:655-678 PathViewProxyModel::createPreviousIndexInfo）
// B1: if (!baseInfo) → return {} 空指针
// B2: if (frameIndex > 0) → 复制本图并 frameIndex--
// B3: 跨图 → previous = infoFromIndex(index - 1)
// B4: if (previous) → frameIndex = frameCount - 1
// B5: B1 分支的早退 return 语句
// 用例映射：
// - CreatePreviousIndexInfo_NullBase_ReturnsNullInfo              → B1/B5
// - CreatePreviousIndexInfo_MiddleFrame_ReturnsPreviousFrameInfo  → B2
// - CreatePreviousIndexInfo_FirstFrame_ReturnsPreviousImageLastFrame → B3/B4
// - CreatePreviousIndexInfo_FirstImageInSource_ReturnsNullInfo    → B3 越界
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:390-429 PathViewProxyModel::jumpToIndex）
// B1: if (!current) → dumpInfo() + return 早退
// B2: if (sourceIndex >= current->index && frameIndex >= current->frameIndex) → Next 方向 / else → Previous 方向
// 用例映射：
// - JumpToIndex_ForwardOrEqualFrame_SetsNextFlag                  → B2 成立
// - JumpToIndex_BackwardTarget_SetsPreviousFlag                   → B2 不成立
// - JumpToIndex_NullCurrentInfo_EarlyReturnsSafely                → B1
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:434-459 PathViewProxyModel::refreshBothSideData）
// B1: for (i < radius) 中心向两侧增长并 updateIndexInfo
// 用例映射：
// - RefreshBothSideData_ConsistentQueue_RebuildsWithFourEmits     → B1
// - RefreshBothSideData_AfterJump_ResetsJumpFlagToCurrent         → B1 + jumpFlag 复位
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:518-529 PathViewProxyModel::sourcePath）
// B1: if (0 <= sourceIndex && < sourceModel->rowCount()) → 返回源模型 URL
// B2: 越界 → 返回空 QUrl（含早退 return 语句）
// 用例映射：
// - SourcePath_BySourceIndex_ReturnsUrlOrEmpty（TEST_P×4）        → B1/B2（-1/0/N-1/N 边界）
//
// 分支清单（来源：get_code_snippet pathviewproxymodel.cpp:41-110 析构~PathViewProxyModel：
//           该 qn 的 snippet 区段为空实现析构 + 随后的 roleNames/data/setData 函数体）
// B1: ~PathViewProxyModel() { } → 空实现，无自身分支
// B2: roleNames return {ImageUrlRole, FrameIndexRole}（无分支，角色注册）
// B3: data if (!checkIndex(...)) → return {} 非法索引早退
// B4: data case Types::ImageUrlRole → 进入 URL 角色分支
// B5: data 三元 infoPtr 非空 → return infoPtr->url
// B6: data 三元 infoPtr 为空 → return QUrl()
// B7: data case Types::FrameIndexRole → 进入帧索引角色分支
// B8: data 三元 infoPtr 非空 → return infoPtr->frameIndex / 为空 → return 0
// B9: data default → break → 末尾 return {} 未匹配角色
// B10: setData if (!checkIndex(...)) → return false 非法索引早退
// B11: setData case Types::ImageUrlRole → 进入 URL 设置分支
// B12: setData if (infoPtr) 非空 → 执行更新并 Q_EMIT dataChanged(idx, idx)
// B13: setData for previous 回扫（idx.row()-1 → 0）
// B14: setData if (auto preInfo) → 进入前驱判空
// B15: setData if (preInfo->url == oldUrl) → 更新 + dataChanged
// B16: setData for next 前扫（idx.row()+1 → size-1）
// B17: setData if (auto nextInfo) → 进入后继判空
// B18: setData if (nextInfo->url == oldUrl) → 更新 + dataChanged
// 用例映射：
// - Destructor_DeleteLoadedModel_SourceModelStaysUsable              → B1/B2（析构本身无分支）
// - Data_ByRowAndRole_ReturnsExpectedVariant（TEST_P×5）              → B4/B5/B7/B8/B9
// - Data_InvalidModelIndex_ReturnsEmptyVariant                        → B3
// - Data_RowOutOfRange_ReturnsEmptyVariant                            → B3
// - Data_NullInfoSlot_ReturnsUrlAndFrameDefaults                      → B6/B8
// - SetData_InvalidModelIndex_ReturnsFalse                            → B10
// - SetData_RowOutOfRange_ReturnsFalse                                → B10
// - SetData_UnsupportedRole_ReturnsFalse                              → B11（default）/B12 不成立
// - SetData_ImageUrlRole_UpdatesUrlAndEmitsChanged                    → B11/B12
// - SetData_ImageUrlRole_PropagatesForwardToDuplicateUrl              → B16/B17/B18
// - SetData_ImageUrlRole_PropagatesBackwardToDuplicateUrl             → B13/B14/B15
// - SetData_NullInfoSlot_ReturnsTrueWithoutEmit                       → B12 不成立

#include <gtest/gtest.h>

#include <QAbstractListModel>
#include <QList>
#include <QModelIndex>
#include <QSignalSpy>
#include <QUrl>
#include <QVariant>

#include "stub_ext/stubext.h"
#include "pathviewproxymodel.h"
#include "imageinfo.h"
#include "imagesourcemodel.h"
#include "imageinfo.h"
#include "types.h"

// ImageInfoCache 定义在 imageinfo.cpp 内部（无公共头文件声明），
// 此处按源码签名镜像声明，仅用于取成员指针做 stub（stub-ext 按函数入口改写，跨 TU 生效）
class ImageInfoCache : public QObject
{
public:
    void load(const QString &path, int frameIndex, bool reload = false);
};

namespace {

constexpr int kDefaultQueueCount = 5;   // 构造函数内 setQueueCount(5)
constexpr int kDefaultRadius = 2;       // qFloor(5 / 2)

// 无 scheme 相对路径：toLocalFile() 为空 → ImageInfo 走 Error 分支，不触发异步加载
QList<QUrl> makeSourceUrls(int count)
{
    QList<QUrl> urls;
    for (int i = 0; i < count; ++i)
        urls << QUrl(QStringLiteral("ut-pathviewproxy-src-%1.png").arg(i));
    return urls;
}

// 手工构造 IndexInfo，绕开 ImageInfo 加载链路
PathViewProxyModel::IndexInfoPtr makeInfo(const QUrl &url, int index, int frameCount, int frameIndex)
{
    auto info = PathViewProxyModel::IndexInfoPtr::create();
    info->url = url;
    info->index = index;
    info->frameCount = frameCount;
    info->frameIndex = frameIndex;
    return info;
}

// data() 参数化用例：行 × 角色 → 期望 QVariant
struct DataCase {
    int row;
    int role;
    QVariant expected;
};

// distance() 参数化用例：源索引 + 帧索引 → 期望距离类型
struct DistanceCase {
    int sourceIndex;
    int frameIndex;
    PathViewProxyModel::DistanceType expected;
};

// sourcePath() 参数化用例：源索引 → 期望 URL
struct SourcePathCase {
    int sourceIndex;
    QUrl expected;
};

// nextProxyIdx()/previousPorxyIdx() 参数化用例：基点 → 期望代理索引
struct ProxyIdxCase {
    int base;
    int expected;
};

// setQueueCount() 参数化用例：奇数队列长 → 期望半径
struct QueueCountCase {
    int count;
    int expectedRadius;
};

}  // namespace

// 公共环境：源模型 + 被测代理模型 + ImageInfoCache::load 隔离
class PathViewProxyModelTestEnv {
protected:
    void initEnv()
    {
        stub.clear();
        sourceModel = new ImageSourceModel();
        obj = new PathViewProxyModel(sourceModel);
        // 隔离：拦截缓存加载，阻止真实图片解码与线程池任务
        stub.set_lamda(VADDR(ImageInfoCache, load),
                       [](ImageInfoCache *self, const QString &path, int frameIndex, bool reload) {
                           Q_UNUSED(self)
                           Q_UNUSED(path)
                           Q_UNUSED(frameIndex)
                           Q_UNUSED(reload)
                       });
    }

    void destroyEnv()
    {
        delete obj;
        delete sourceModel;
        stub.clear();
    }

    // 5 张图片 + resetModel(2, 0)，队列布局 [u2,u3,u4,u0,u1]
    void resetWithFiveUrls()
    {
        sourceModel->setImageFiles(makeSourceUrls(5));
        obj->resetModel(2, 0);
    }

    stub_ext::StubExt stub;
    ImageSourceModel *sourceModel = nullptr;
    PathViewProxyModel *obj = nullptr;
};

class PathViewProxyModelTest : public ::testing::Test, protected PathViewProxyModelTestEnv {
protected:
    void SetUp() override { initEnv(); }
    void TearDown() override
    {
        destroyEnv();
        stub.clear();   // 兜底：确保桩在 TearDown 中显式清理
    }
};

// ═══════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F/TEST_P 包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════

TEST_F(PathViewProxyModelTest, PathViewProxyModel_ConstructWithSourceModel_SetsDefaultQueueState)
{
    // Arrange: SetUp 已用非空源模型构造
    QSignalSpy spy(obj, &PathViewProxyModel::currentIndexChanged);

    // Act
    const int rows = obj->rowCount();
    const int curIdx = obj->currentIndex();

    // Assert  // 构造内 setQueueCount(5)：maxCount=5, radius=2，队列尚未填充
    EXPECT_EQ(obj->maxCount, kDefaultQueueCount);
    EXPECT_EQ(obj->radius, kDefaultRadius);
    EXPECT_EQ(rows, 0);
    EXPECT_EQ(curIdx, 0);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->jumpFlag, PathViewProxyModel::Current);
}

TEST_F(PathViewProxyModelTest, Destructor_DeleteLoadedModel_SourceModelStaysUsable)
{
    // Arrange
    sourceModel->setImageFiles(makeSourceUrls(3));
    obj->resetModel(0, 0);

    // Act
    delete obj;
    obj = nullptr;

    // Assert: 析构后源模型完好，可再次挂接新代理模型
    EXPECT_EQ(sourceModel->rowCount(), 3);
    PathViewProxyModel *rebuilt = new PathViewProxyModel(sourceModel);
    EXPECT_EQ(rebuilt->rowCount(), 0);
    delete rebuilt;
}

TEST_F(PathViewProxyModelTest, RoleNames_DefaultRoles_ContainImageUrlAndFrameIndex)
{
    // Arrange
    const int imageUrlRole = Types::ImageUrlRole;
    const int frameIndexRole = Types::FrameIndexRole;
    QHash<int, QByteArray> roles;

    // Act
    roles = obj->roleNames();

    // Assert
    EXPECT_EQ(roles.size(), 2);
    EXPECT_EQ(roles.value(imageUrlRole), QByteArray("imageUrl"));
    EXPECT_EQ(roles.value(frameIndexRole), QByteArray("frameIndex"));
}

TEST_F(PathViewProxyModelTest, RowCount_BeforeAndAfterReset_TracksQueueSize)
{
    // Arrange
    const QModelIndex noParent;

    // Act
    const int before = obj->rowCount(noParent);
    resetWithFiveUrls();
    const int after = obj->rowCount(QModelIndex());

    // Assert: 队列固定为 maxCount 长度（即便源数据不足也保持）
    EXPECT_EQ(before, 0);
    EXPECT_EQ(after, kDefaultQueueCount);
}

TEST_F(PathViewProxyModelTest, CurrentIndex_FreshAndAfterReset_StaysAtZero)
{
    // Arrange
    const int initialIdx = obj->currentIndex();

    // Act
    resetWithFiveUrls();

    // Assert: resetModel 将探针归零，保证首张图片指向代理索引 0
    EXPECT_EQ(initialIdx, 0);
    EXPECT_EQ(obj->currentIndex(), 0);
}

TEST_F(PathViewProxyModelTest, CurrentIndex_AfterSetCurrentIndex_ReturnsNewValue)
{
    // Arrange
    resetWithFiveUrls();

    // Act
    obj->setCurrentIndex(3);

    // Assert
    EXPECT_EQ(obj->currentIndex(), 3);
    EXPECT_EQ(obj->currentProxyIdx, 3);
}

TEST_F(PathViewProxyModelTest, SetCurrentIndex_DifferentValue_UpdatesAndEmitsChanged)
{
    // Arrange
    resetWithFiveUrls();
    QSignalSpy spy(obj, &PathViewProxyModel::currentIndexChanged);

    // Act
    obj->setCurrentIndex(2);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 2);
    EXPECT_EQ(obj->currentIndex(), 2);
}

TEST_F(PathViewProxyModelTest, SetCurrentIndex_SameValue_DoesNotEmitChanged)
{
    // Arrange
    resetWithFiveUrls();
    QSignalSpy spy(obj, &PathViewProxyModel::currentIndexChanged);

    // Act
    obj->setCurrentIndex(0);   // 与初始探针相同

    // Assert: 同值设置无副作用（强异常安全）
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->currentIndex(), 0);
}

TEST_F(PathViewProxyModelTest, DumpInfo_MixedEntries_KeepsStateIntact)
{
    // Arrange: 2 张图片 → 队列含非空与空槽位
    sourceModel->setImageFiles(makeSourceUrls(2));
    obj->resetModel(0, 0);

    // Act
    obj->dumpInfo();

    // Assert: dumpInfo 仅输出日志，模型状态不变
    EXPECT_EQ(obj->rowCount(), kDefaultQueueCount);
    EXPECT_EQ(obj->currentIndex(), 0);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(2).at(0));
}

// ─── setQueueCount / sourcePath / nextProxyIdx / previousPorxyIdx 参数化 ───

class PathViewProxyModelQueueCountTest :
    public ::testing::TestWithParam<QueueCountCase>, protected PathViewProxyModelTestEnv {
protected:
    void SetUp() override { initEnv(); }
    void TearDown() override { destroyEnv(); }
};

TEST_P(PathViewProxyModelQueueCountTest, SetQueueCount_OddCountAtLeastThree_SetsMaxCountAndRadius)
{
    const auto &c = GetParam();
    // Arrange: SetUp 已按默认 5 构造
    const int defaultMaxCount = obj->maxCount;

    // Act
    obj->setQueueCount(c.count);

    // Assert  // radius = qFloor(count / 2)
    EXPECT_EQ(defaultMaxCount, kDefaultQueueCount);
    EXPECT_EQ(obj->maxCount, c.count);
    EXPECT_EQ(obj->radius, c.expectedRadius);
}

INSTANTIATE_TEST_SUITE_P(
    QueueCountCases,
    PathViewProxyModelQueueCountTest,
    ::testing::Values(
        QueueCountCase{3, 1},   // 下边界：最小合法奇数
        QueueCountCase{5, 2},   // 默认值
        QueueCountCase{9, 4}    // 较大奇数
        ));

class PathViewProxyModelSourcePathTest :
    public ::testing::TestWithParam<SourcePathCase>, protected PathViewProxyModelTestEnv {
protected:
    void SetUp() override
    {
        initEnv();
        sourceModel->setImageFiles(makeSourceUrls(5));
    }
    void TearDown() override { destroyEnv(); }
};

TEST_P(PathViewProxyModelSourcePathTest, SourcePath_BySourceIndex_ReturnsUrlOrEmpty)
{
    const auto &c = GetParam();
    // Arrange: 源模型固定 5 张图片
    const int sourceRowCount = sourceModel->rowCount();

    // Act
    const QUrl actual = obj->sourcePath(c.sourceIndex);

    // Assert  // 越界（-1 与 5）返回空 QUrl
    EXPECT_EQ(sourceRowCount, 5);
    EXPECT_EQ(actual, c.expected);
}

INSTANTIATE_TEST_SUITE_P(
    SourcePathCases,
    PathViewProxyModelSourcePathTest,
    ::testing::Values(
        SourcePathCase{-1, QUrl()},                                        // 负数越界
        SourcePathCase{0, QUrl(QStringLiteral("ut-pathviewproxy-src-0.png"))},   // 首张边界
        SourcePathCase{4, QUrl(QStringLiteral("ut-pathviewproxy-src-4.png"))},   // 末张边界
        SourcePathCase{5, QUrl()}                                          // 越上界
        ));

class PathViewProxyModelNextIdxTest :
    public ::testing::TestWithParam<ProxyIdxCase>, protected PathViewProxyModelTestEnv {
protected:
    void SetUp() override
    {
        initEnv();
        resetWithFiveUrls();
    }
    void TearDown() override { destroyEnv(); }
};

TEST_P(PathViewProxyModelNextIdxTest, NextProxyIdx_WrapsAroundQueueEnd_ReturnsNextRow)
{
    const auto &c = GetParam();
    // Arrange: 队列长度固定 5（环形缓冲）
    const int queueSize = obj->rowCount();

    // Act
    const int next = obj->nextProxyIdx(c.base);

    // Assert  // (base + 1) % size，末尾环绕到 0
    EXPECT_EQ(queueSize, kDefaultQueueCount);
    EXPECT_EQ(next, c.expected);
}

INSTANTIATE_TEST_SUITE_P(
    NextIdxCases,
    PathViewProxyModelNextIdxTest,
    ::testing::Values(
        ProxyIdxCase{0, 1},
        ProxyIdxCase{2, 3},
        ProxyIdxCase{4, 0}    // 环绕边界
        ));

class PathViewProxyModelPrevIdxTest :
    public ::testing::TestWithParam<ProxyIdxCase>, protected PathViewProxyModelTestEnv {
protected:
    void SetUp() override
    {
        initEnv();
        resetWithFiveUrls();
    }
    void TearDown() override { destroyEnv(); }
};

TEST_P(PathViewProxyModelPrevIdxTest, PreviousPorxyIdx_WrapsAroundQueueStart_ReturnsPreviousRow)
{
    const auto &c = GetParam();
    // Arrange: 队列长度固定 5（环形缓冲）
    const int queueSize = obj->rowCount();

    // Act
    const int previous = obj->previousPorxyIdx(c.base);

    // Assert  // (base - 1 + size) % size，头部环绕到 size-1
    EXPECT_EQ(queueSize, kDefaultQueueCount);
    EXPECT_EQ(previous, c.expected);
}

INSTANTIATE_TEST_SUITE_P(
    PrevIdxCases,
    PathViewProxyModelPrevIdxTest,
    ::testing::Values(
        ProxyIdxCase{0, 4},   // 环绕边界
        ProxyIdxCase{2, 1},
        ProxyIdxCase{4, 3}
        ));

// ─── infoFromIndex ───

TEST_F(PathViewProxyModelTest, InfoFromIndex_ValidSourceIndex_ReturnsFilledInfo)
{
    // Arrange
    const QList<QUrl> urls = makeSourceUrls(5);
    sourceModel->setImageFiles(urls);

    // Act
    const auto info = obj->infoFromIndex(1);

    // Assert: 相对路径 → Error 态，无异步补偿；单帧图 frameCount 兜底为 1
    ASSERT_TRUE(!info.isNull());
    EXPECT_EQ(info->url, urls.at(1));
    EXPECT_EQ(info->index, 1);
    EXPECT_EQ(info->frameCount, 1);
    EXPECT_EQ(info->frameIndex, 0);
}

TEST_F(PathViewProxyModelTest, InfoFromIndex_OutOfRangeIndex_ReturnsNullInfo)
{
    // Arrange
    sourceModel->setImageFiles(makeSourceUrls(5));

    // Act
    const auto aboveRange = obj->infoFromIndex(5);
    const auto belowRange = obj->infoFromIndex(-1);

    // Assert: 越界源索引（sourcePath 为空）返回空指针
    EXPECT_EQ(aboveRange.data(), nullptr);
    EXPECT_EQ(belowRange.data(), nullptr);
}

TEST_F(PathViewProxyModelTest, InfoFromIndex_LoadingFileUrl_SchedulesAsyncLoadInfo)
{
    // Arrange: file URL 未命中缓存 → ImageInfo 处于 Loading，触发异步补偿
    const QUrl loadingUrl = QUrl::fromLocalFile(QStringLiteral("ut-pathviewproxy-loading.png"));
    sourceModel->setImageFiles(QList<QUrl>() << loadingUrl);
    const int childrenBefore = obj->children().count();

    // Act
    const auto info = obj->infoFromIndex(0);

    // Assert: asyncUpdateLoadInfo 创建 delayInfo 并挂到模型（子对象 +1）
    ASSERT_TRUE(!info.isNull());
    EXPECT_EQ(obj->children().count(), childrenBefore + 1);
    EXPECT_EQ(info->frameCount, 1);   // 加载未完成时单帧兜底
}

// ─── createNextIndexInfo / createPreviousIndexInfo ───

TEST_F(PathViewProxyModelTest, CreateNextIndexInfo_NullBase_ReturnsNullInfo)
{
    // Arrange
    const PathViewProxyModel::IndexInfoPtr nullBase;

    // Act
    const auto next = obj->createNextIndexInfo(nullBase);

    // Assert
    EXPECT_EQ(next.data(), nullptr);
    EXPECT_EQ(obj->rowCount(), 0);   // 模型状态不受影响
}

TEST_F(PathViewProxyModelTest, CreateNextIndexInfo_MiddleFrame_ReturnsNextFrameInfo)
{
    // Arrange: 3 帧图的第 2 帧
    const QUrl url(QStringLiteral("ut-pathviewproxy-multi.png"));
    const auto base = makeInfo(url, 2, 3, 1);

    // Act
    const auto next = obj->createNextIndexInfo(base);

    // Assert: 同图内推进帧索引，其余字段复制
    ASSERT_TRUE(!next.isNull());
    EXPECT_EQ(next->frameIndex, 2);
    EXPECT_EQ(next->index, 2);
    EXPECT_EQ(next->frameCount, 3);
    EXPECT_EQ(next->url, url);
}

TEST_F(PathViewProxyModelTest, CreateNextIndexInfo_LastFrame_ReturnsNextImageInfo)
{
    // Arrange: 单帧图（无下一帧）→ 跨图取源索引 +1
    const QList<QUrl> urls = makeSourceUrls(5);
    sourceModel->setImageFiles(urls);
    const auto base = makeInfo(urls.at(2), 2, 1, 0);

    // Act
    const auto next = obj->createNextIndexInfo(base);

    // Assert
    ASSERT_TRUE(!next.isNull());
    EXPECT_EQ(next->url, urls.at(3));
    EXPECT_EQ(next->index, 3);
    EXPECT_EQ(next->frameIndex, 0);
}

TEST_F(PathViewProxyModelTest, CreateNextIndexInfo_LastImageInSource_ReturnsNullInfo)
{
    // Arrange: 源数据末张（index+1 越界）
    const QList<QUrl> urls = makeSourceUrls(5);
    sourceModel->setImageFiles(urls);
    const auto base = makeInfo(urls.at(4), 4, 1, 0);

    // Act
    const auto next = obj->createNextIndexInfo(base);

    // Assert
    EXPECT_EQ(next.data(), nullptr);
    EXPECT_EQ(sourceModel->rowCount(), 5);
}

TEST_F(PathViewProxyModelTest, CreatePreviousIndexInfo_NullBase_ReturnsNullInfo)
{
    // Arrange
    const PathViewProxyModel::IndexInfoPtr nullBase;

    // Act
    const auto previous = obj->createPreviousIndexInfo(nullBase);

    // Assert
    EXPECT_EQ(previous.data(), nullptr);
    EXPECT_EQ(obj->rowCount(), 0);
}

TEST_F(PathViewProxyModelTest, CreatePreviousIndexInfo_MiddleFrame_ReturnsPreviousFrameInfo)
{
    // Arrange: 3 帧图的第 3 帧
    const QUrl url(QStringLiteral("ut-pathviewproxy-multi.png"));
    const auto base = makeInfo(url, 1, 3, 2);

    // Act
    const auto previous = obj->createPreviousIndexInfo(base);

    // Assert: 同图内回退帧索引
    ASSERT_TRUE(!previous.isNull());
    EXPECT_EQ(previous->frameIndex, 1);
    EXPECT_EQ(previous->index, 1);
    EXPECT_EQ(previous->frameCount, 3);
    EXPECT_EQ(previous->url, url);
}

TEST_F(PathViewProxyModelTest, CreatePreviousIndexInfo_FirstFrame_ReturnsPreviousImageLastFrame)
{
    // Arrange: 单帧图首帧 → 跨图取源索引 -1 的末帧
    const QList<QUrl> urls = makeSourceUrls(5);
    sourceModel->setImageFiles(urls);
    const auto base = makeInfo(urls.at(2), 2, 1, 0);

    // Act
    const auto previous = obj->createPreviousIndexInfo(base);

    // Assert: previous 非空 → frameIndex = frameCount - 1（单帧即 0）
    ASSERT_TRUE(!previous.isNull());
    EXPECT_EQ(previous->url, urls.at(1));
    EXPECT_EQ(previous->index, 1);
    EXPECT_EQ(previous->frameIndex, 0);
}

TEST_F(PathViewProxyModelTest, CreatePreviousIndexInfo_FirstImageInSource_ReturnsNullInfo)
{
    // Arrange: 源数据首张（index-1 越界）
    const QList<QUrl> urls = makeSourceUrls(5);
    sourceModel->setImageFiles(urls);
    const auto base = makeInfo(urls.at(0), 0, 1, 0);

    // Act
    const auto previous = obj->createPreviousIndexInfo(base);

    // Assert
    EXPECT_EQ(previous.data(), nullptr);
    EXPECT_EQ(sourceModel->rowCount(), 5);
}

// ─── updateIndexInfo ───

TEST_F(PathViewProxyModelTest, UpdateIndexInfo_ValidProxyIndex_ReplacesInfoAndEmits)
{
    // Arrange
    resetWithFiveUrls();
    const QUrl newUrl(QStringLiteral("ut-pathviewproxy-new.png"));
    const auto replacement = makeInfo(newUrl, 9, 1, 0);
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    obj->updateIndexInfo(1, replacement);

    // Assert: 槽位被替换并广播 ImageUrl/FrameIndex 两个角色
    EXPECT_EQ(obj->indexQueue.at(1), replacement);
    EXPECT_EQ(obj->data(obj->index(1, 0), Types::ImageUrlRole).toUrl(), newUrl);
    ASSERT_EQ(spy.count(), 1);
    const QList<int> roles = spy.at(0).at(2).value<QList<int>>();
    EXPECT_TRUE(roles.contains(Types::ImageUrlRole));
    EXPECT_TRUE(roles.contains(Types::FrameIndexRole));
}

// ─── jumpToIndex / refreshBothSideData / syncState ───

TEST_F(PathViewProxyModelTest, JumpToIndex_ForwardOrEqualFrame_SetsNextFlag)
{
    // Arrange: 当前源索引 2；目标相同源索引且帧不小于当前帧 → Next 方向
    resetWithFiveUrls();

    // Act
    obj->jumpToIndex(2, 0);

    // Assert: 跳转槽位 nextProxyIdx(0)=1，jumpFlag = Next 触发动画
    EXPECT_EQ(obj->currentIndex(), 1);
    EXPECT_EQ(obj->data(obj->index(1, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(2));
    EXPECT_EQ(obj->jumpFlag, PathViewProxyModel::Next);
}

TEST_F(PathViewProxyModelTest, JumpToIndex_BackwardTarget_SetsPreviousFlag)
{
    // Arrange: 当前源索引 2，目标源索引更小
    resetWithFiveUrls();

    // Act
    obj->jumpToIndex(1, 0);

    // Assert: 跳转槽位 previousPorxyIdx(0)=4，jumpFlag = Previous
    EXPECT_EQ(obj->currentIndex(), 4);
    EXPECT_EQ(obj->data(obj->index(4, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(1));
    EXPECT_EQ(obj->jumpFlag, PathViewProxyModel::Previous);
}

TEST_F(PathViewProxyModelTest, JumpToIndex_NullCurrentInfo_EarlyReturnsSafely)
{
    // Arrange: 空源数据 → 队列 5 个空槽位，当前信息为空
    sourceModel->setImageFiles(QList<QUrl>());
    obj->resetModel(0, 0);
    QSignalSpy spy(obj, &PathViewProxyModel::currentIndexChanged);

    // Act
    obj->jumpToIndex(0, 0);

    // Assert: 仅 dumpInfo 输出，探针与队列不变（强异常安全）
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->rowCount(), kDefaultQueueCount);
    EXPECT_EQ(obj->currentIndex(), 0);
}

TEST_F(PathViewProxyModelTest, RefreshBothSideData_ConsistentQueue_RebuildsWithFourEmits)
{
    // Arrange: 队列 [u2,u3,u4,u0,u1]，数据自洽 → 重建后幂等
    resetWithFiveUrls();
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    obj->refreshBothSideData();

    // Assert: radius=2 → 两侧各 2 次 updateIndexInfo = 4 次 dataChanged
    EXPECT_EQ(spy.count(), 2 * kDefaultRadius);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(2));
    EXPECT_EQ(obj->data(obj->index(3, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(0));
    EXPECT_EQ(obj->data(obj->index(4, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(1));
}

TEST_F(PathViewProxyModelTest, RefreshBothSideData_AfterJump_ResetsJumpFlagToCurrent)
{
    // Arrange: 先跳转让 jumpFlag = Next
    resetWithFiveUrls();
    obj->jumpToIndex(3, 0);
    EXPECT_EQ(obj->jumpFlag, PathViewProxyModel::Next);

    // Act
    obj->refreshBothSideData();

    // Assert: 刷新两侧后取消跳转状态；当前槽位仍指向源索引 3
    EXPECT_EQ(obj->jumpFlag, PathViewProxyModel::Current);
    EXPECT_EQ(obj->currentIndex(), 1);
    EXPECT_EQ(obj->data(obj->index(1, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(3));
}

TEST_F(PathViewProxyModelTest, SyncState_IdleFlag_SkipsRefresh)
{
    // Arrange: resetModel 后 jumpFlag == Current
    resetWithFiveUrls();
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    obj->syncState();

    // Assert: 无跳转挂起时不刷新（无 dataChanged）
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(2));
}

TEST_F(PathViewProxyModelTest, SyncState_ActiveJumpFlag_RefreshesAndResetsFlag)
{
    // Arrange: 跳转后 jumpFlag = Next，两侧数据待同步
    resetWithFiveUrls();
    obj->jumpToIndex(3, 0);
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    obj->syncState();

    // Assert: 触发 refreshBothSideData（4 次更新）并复位 jumpFlag
    EXPECT_EQ(spy.count(), 2 * kDefaultRadius);
    EXPECT_EQ(obj->jumpFlag, PathViewProxyModel::Current);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(2));
}

// ─── moveNext / movePrevoius ───

TEST_F(PathViewProxyModelTest, MoveNext_AdvancesCurrent_AndSlidesWindowRight)
{
    // Arrange: 队列 [u2,u3,u4,u0,u1]
    resetWithFiveUrls();
    QSignalSpy spy(obj, &PathViewProxyModel::currentIndexChanged);

    // Act
    obj->moveNext();

    // Assert: 探针 0→1；右侧新槽位 (1+radius)%5=3 被填充为下下张（源索引 5 越界 → 空）
    EXPECT_EQ(obj->currentIndex(), 1);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(obj->data(obj->index(1, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(3));
    EXPECT_TRUE(obj->data(obj->index(3, 0), Types::ImageUrlRole).toUrl().isEmpty());
}

TEST_F(PathViewProxyModelTest, MoveNext_NullSideInfo_KeepsSlotEmpty)
{
    // Arrange: 2 张图片 → 队列 [u0,u1,null,null,null]
    sourceModel->setImageFiles(makeSourceUrls(2));
    obj->resetModel(0, 0);

    // Act
    obj->moveNext();

    // Assert: 基点为空槽位 → 新槽位仍为空，但探针正常推进
    EXPECT_EQ(obj->currentIndex(), 1);
    EXPECT_EQ(obj->data(obj->index(1, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(2).at(1));
    EXPECT_TRUE(obj->data(obj->index(3, 0), Types::ImageUrlRole).toUrl().isEmpty());
}

TEST_F(PathViewProxyModelTest, MovePrevoius_RetreatsCurrent_AndSlidesWindowLeft)
{
    // Arrange: 队列 [u2,u3,u4,u0,u1]
    resetWithFiveUrls();
    QSignalSpy spy(obj, &PathViewProxyModel::currentIndexChanged);

    // Act
    obj->movePrevoius();

    // Assert: 探针 0→4（环绕）；左侧新槽位 (4+radius+1)%5=2 被填充为源索引 1 的前驱（-1 越界 → 空）
    EXPECT_EQ(obj->currentIndex(), 4);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(obj->data(obj->index(4, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(1));
    EXPECT_TRUE(obj->data(obj->index(2, 0), Types::ImageUrlRole).toUrl().isEmpty());
}

// ─── resetModel ───

TEST_F(PathViewProxyModelTest, ResetModel_MoreImagesThanQueue_WrapsSourceIndices)
{
    // Arrange
    const QList<QUrl> urls = makeSourceUrls(5);
    sourceModel->setImageFiles(urls);
    QSignalSpy spy(obj, &QAbstractItemModel::modelReset);

    // Act
    obj->resetModel(2, 0);

    // Assert: 首张图片在代理索引 0，右侧追加 2 张、左侧环绕回填 2 张
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(obj->rowCount(), kDefaultQueueCount);
    EXPECT_EQ(obj->currentIndex(), 0);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), urls.at(2));
    EXPECT_EQ(obj->data(obj->index(1, 0), Types::ImageUrlRole).toUrl(), urls.at(3));
    EXPECT_EQ(obj->data(obj->index(2, 0), Types::ImageUrlRole).toUrl(), urls.at(4));
    EXPECT_EQ(obj->data(obj->index(3, 0), Types::ImageUrlRole).toUrl(), urls.at(0));
    EXPECT_EQ(obj->data(obj->index(4, 0), Types::ImageUrlRole).toUrl(), urls.at(1));
}

TEST_F(PathViewProxyModelTest, ResetModel_FewerImagesThanQueue_FillsTailWithNull)
{
    // Arrange: 2 张图片（alloc_in_loop 循环边界：源数据不足时继续分配空槽位）
    const QList<QUrl> urls = makeSourceUrls(2);
    sourceModel->setImageFiles(urls);

    // Act
    obj->resetModel(0, 0);

    // Assert: 队列仍为 maxCount 长，越界槽位为空
    EXPECT_EQ(obj->rowCount(), kDefaultQueueCount);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), urls.at(0));
    EXPECT_EQ(obj->data(obj->index(1, 0), Types::ImageUrlRole).toUrl(), urls.at(1));
    EXPECT_TRUE(obj->data(obj->index(2, 0), Types::ImageUrlRole).toUrl().isEmpty());
    EXPECT_TRUE(obj->data(obj->index(4, 0), Types::ImageUrlRole).toUrl().isEmpty());
}

TEST_F(PathViewProxyModelTest, ResetModel_EmptySource_AllSlotsNull)
{
    // Arrange: 源数据为空（循环 0 张边界）
    sourceModel->setImageFiles(QList<QUrl>());

    // Act
    obj->resetModel(0, 0);

    // Assert: 队列固定长度 5，全部为空槽位
    EXPECT_EQ(obj->rowCount(), kDefaultQueueCount);
    EXPECT_TRUE(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl().isEmpty());
    EXPECT_TRUE(obj->data(obj->index(4, 0), Types::ImageUrlRole).toUrl().isEmpty());
    EXPECT_EQ(obj->currentIndex(), 0);
}

// ─── deleteCurrent ───

TEST_F(PathViewProxyModelTest, DeleteCurrent_EmptyQueue_EarlyReturnKeepsState)
{
    // Arrange: 未 resetModel → 队列为空
    QSignalSpy resetSpy(obj, &QAbstractItemModel::modelReset);

    // Act
    obj->deleteCurrent();

    // Assert: 直接返回，不触发重置（强异常安全）
    EXPECT_EQ(resetSpy.count(), 0);
    EXPECT_EQ(obj->rowCount(), 0);
    EXPECT_EQ(obj->currentIndex(), 0);
}

TEST_F(PathViewProxyModelTest, DeleteCurrent_EmptySourceModel_ResetsQueueToEmpty)
{
    // Arrange: 源数据无文件（队列全空槽位）
    sourceModel->setImageFiles(QList<QUrl>());
    obj->resetModel(0, 0);
    QSignalSpy resetSpy(obj, &QAbstractItemModel::modelReset);

    // Act
    obj->deleteCurrent();

    // Assert: 特殊分支 → beginResetModel/clear/endResetModel
    EXPECT_EQ(resetSpy.count(), 1);
    EXPECT_EQ(obj->rowCount(), 0);
    EXPECT_EQ(obj->currentIndex(), 0);
}

TEST_F(PathViewProxyModelTest, DeleteCurrent_NullCurrentInfo_EarlyReturnKeepsQueue)
{
    // Arrange: 队列全空槽位，但源模型随后补充了数据
    sourceModel->setImageFiles(QList<QUrl>());
    obj->resetModel(0, 0);
    sourceModel->setImageFiles(makeSourceUrls(5));
    QSignalSpy resetSpy(obj, &QAbstractItemModel::modelReset);

    // Act
    obj->deleteCurrent();

    // Assert: 当前信息为空 → 早退，队列保持 5 个空槽位
    EXPECT_EQ(resetSpy.count(), 0);
    EXPECT_EQ(obj->rowCount(), kDefaultQueueCount);
    EXPECT_EQ(obj->currentIndex(), 0);
}

TEST_F(PathViewProxyModelTest, DeleteCurrent_LastImage_MovesToPreviousImage)
{
    // Arrange: 当前指向源数据末张（索引 4），删除后源模型只剩 4 张
    const QList<QUrl> urls = makeSourceUrls(5);
    sourceModel->setImageFiles(urls);
    obj->resetModel(4, 0);
    QList<QUrl> afterDelete = urls;
    afterDelete.removeLast();
    sourceModel->setImageFiles(afterDelete);

    // Act
    obj->deleteCurrent();

    // Assert: atEnd → 移到前一张（源索引 3），两侧围绕其重建（源索引 4 越界 → 空槽）
    EXPECT_EQ(obj->currentIndex(), 4);
    EXPECT_EQ(obj->data(obj->index(4, 0), Types::ImageUrlRole).toUrl(), urls.at(3));
    EXPECT_EQ(obj->data(obj->index(3, 0), Types::ImageUrlRole).toUrl(), urls.at(2));
    EXPECT_EQ(obj->data(obj->index(2, 0), Types::ImageUrlRole).toUrl(), urls.at(1));
    EXPECT_TRUE(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl().isEmpty());
}

TEST_F(PathViewProxyModelTest, DeleteCurrent_MiddleImage_MovesToNextImage)
{
    // Arrange: 当前指向源索引 2，删除后源模型 4 张（u0,u1,u3,u4）
    const QList<QUrl> urls = makeSourceUrls(5);
    sourceModel->setImageFiles(urls);
    obj->resetModel(2, 0);
    QList<QUrl> afterDelete = urls;
    afterDelete.removeAt(2);
    sourceModel->setImageFiles(afterDelete);

    // Act
    obj->deleteCurrent();

    // Assert: 非 atEnd → 移到后一张（原 u3 现源索引 2），两侧重建
    EXPECT_EQ(obj->currentIndex(), 1);
    EXPECT_EQ(obj->data(obj->index(1, 0), Types::ImageUrlRole).toUrl(), urls.at(3));
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), urls.at(1));
    EXPECT_EQ(obj->data(obj->index(2, 0), Types::ImageUrlRole).toUrl(), urls.at(4));
    EXPECT_TRUE(obj->data(obj->index(3, 0), Types::ImageUrlRole).toUrl().isEmpty());
}

// ─── data（含参数化） ───

TEST_F(PathViewProxyModelTest, Data_InvalidModelIndex_ReturnsEmptyVariant)
{
    // Arrange
    resetWithFiveUrls();

    // Act
    const QVariant urlRole = obj->data(QModelIndex(), Types::ImageUrlRole);
    const QVariant frameRole = obj->data(QModelIndex(), Types::FrameIndexRole);

    // Assert: checkIndex 拒绝非法索引
    EXPECT_EQ(urlRole, QVariant());
    EXPECT_EQ(frameRole, QVariant());
}

TEST_F(PathViewProxyModelTest, Data_RowOutOfRange_ReturnsEmptyVariant)
{
    // Arrange
    resetWithFiveUrls();
    const QModelIndex outOfRange = obj->index(99, 0);

    // Act
    const QVariant urlRole = obj->data(outOfRange, Types::ImageUrlRole);

    // Assert
    EXPECT_EQ(urlRole, QVariant());
    EXPECT_EQ(obj->rowCount(), kDefaultQueueCount);
}

TEST_F(PathViewProxyModelTest, Data_NullInfoSlot_ReturnsUrlAndFrameDefaults)
{
    // Arrange: 2 张图片 → 队列 [u0,u1,null,null,null]
    sourceModel->setImageFiles(makeSourceUrls(2));
    obj->resetModel(0, 0);

    // Act
    const QVariant nullUrl = obj->data(obj->index(2, 0), Types::ImageUrlRole);
    const QVariant nullFrame = obj->data(obj->index(4, 0), Types::FrameIndexRole);

    // Assert: 空槽位返回默认构造值 QUrl() / 0
    EXPECT_EQ(nullUrl.toUrl(), QUrl());
    EXPECT_EQ(nullFrame.toInt(), 0);
}

class PathViewProxyModelDataTest :
    public ::testing::TestWithParam<DataCase>, protected PathViewProxyModelTestEnv {
protected:
    void SetUp() override
    {
        initEnv();
        resetWithFiveUrls();
    }
    void TearDown() override { destroyEnv(); }
};

TEST_P(PathViewProxyModelDataTest, Data_ByRowAndRole_ReturnsExpectedVariant)
{
    const auto &c = GetParam();
    // Arrange: 队列 [u2,u3,u4,u0,u1]
    const QModelIndex idx = obj->index(c.row, 0);

    // Act
    const QVariant actual = obj->data(idx, c.role);

    // Assert
    EXPECT_EQ(actual, c.expected);
    EXPECT_EQ(obj->rowCount(), kDefaultQueueCount);
}

INSTANTIATE_TEST_SUITE_P(
    RowRoleCases,
    PathViewProxyModelDataTest,
    ::testing::Values(
        DataCase{0, Types::ImageUrlRole, QVariant(QUrl(QStringLiteral("ut-pathviewproxy-src-2.png")))},
        DataCase{2, Types::ImageUrlRole, QVariant(QUrl(QStringLiteral("ut-pathviewproxy-src-4.png")))},
        DataCase{4, Types::ImageUrlRole, QVariant(QUrl(QStringLiteral("ut-pathviewproxy-src-1.png")))},
        DataCase{0, Types::FrameIndexRole, QVariant(0)},
        DataCase{1, Qt::DisplayRole, QVariant()}   // 未注册角色 → 空 QVariant
        ));

// ─── distance（含参数化） ───

class PathViewProxyModelDistanceTest :
    public ::testing::TestWithParam<DistanceCase>, protected PathViewProxyModelTestEnv {
protected:
    void SetUp() override
    {
        initEnv();
        resetWithFiveUrls();
    }
    void TearDown() override { destroyEnv(); }
};

TEST_P(PathViewProxyModelDistanceTest, Distance_BySourceAndFrameIndex_ReturnsExpectedType)
{
    const auto &c = GetParam();
    // Arrange: 队列 [u2,u3,u4,u0,u1]，当前源索引 2（单帧图 frameIndex=0）
    const auto current = obj->indexQueue.at(obj->currentProxyIdx);

    // Act
    const PathViewProxyModel::DistanceType actual = obj->distance(c.sourceIndex, c.frameIndex);

    // Assert
    ASSERT_TRUE(!current.isNull());
    EXPECT_EQ(current->index, 2);
    EXPECT_EQ(actual, c.expected);
}

INSTANTIATE_TEST_SUITE_P(
    DistanceCases,
    PathViewProxyModelDistanceTest,
    ::testing::Values(
        DistanceCase{2, 0, PathViewProxyModel::Current},      // 同图同帧
        DistanceCase{1, 0, PathViewProxyModel::Previous},     // 前一张
        DistanceCase{3, 0, PathViewProxyModel::Next},         // 后一张
        DistanceCase{4, 0, PathViewProxyModel::OutOfRange},   // 隔一张 → 超界
        DistanceCase{2, 2, PathViewProxyModel::OutOfRange},   // 同图帧差 +2 → 超界
        DistanceCase{-1, 0, PathViewProxyModel::Invalid},     // 源索引负越界
        DistanceCase{5, 0, PathViewProxyModel::Invalid}       // 源索引上越界
        ));

TEST_F(PathViewProxyModelTest, Distance_EmptyQueue_ReturnsInvalid)
{
    // Arrange: 未 resetModel → 队列为空
    sourceModel->setImageFiles(makeSourceUrls(5));

    // Act
    const PathViewProxyModel::DistanceType actual = obj->distance(0, 0);

    // Assert
    EXPECT_EQ(actual, PathViewProxyModel::Invalid);
    EXPECT_EQ(obj->rowCount(), 0);
}

TEST_F(PathViewProxyModelTest, Distance_NullCurrentInfo_ReturnsInvalid)
{
    // Arrange: 空源数据 → 队列全空槽位
    sourceModel->setImageFiles(QList<QUrl>());
    obj->resetModel(0, 0);

    // Act
    const PathViewProxyModel::DistanceType actual = obj->distance(0, 0);

    // Assert
    EXPECT_EQ(actual, PathViewProxyModel::Invalid);
    EXPECT_EQ(obj->rowCount(), kDefaultQueueCount);
}

TEST_F(PathViewProxyModelTest, Distance_NullPreviousInfo_ReturnsOutOfRange)
{
    // Arrange: 队列 [u1,u2,u3,u4,u0]，将 previous 槽位（索引 4）置空
    sourceModel->setImageFiles(makeSourceUrls(5));
    obj->resetModel(1, 0);
    obj->updateIndexInfo(4, PathViewProxyModel::IndexInfoPtr());

    // Act
    const PathViewProxyModel::DistanceType actual = obj->distance(0, 0);

    // Assert: previous 为空 → range 保持 OutOfRange
    EXPECT_EQ(actual, PathViewProxyModel::OutOfRange);
    EXPECT_EQ(obj->indexQueue.at(4).data(), nullptr);
}

// ─── setData ───

TEST_F(PathViewProxyModelTest, SetData_InvalidModelIndex_ReturnsFalse)
{
    // Arrange
    resetWithFiveUrls();
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    const bool changed = obj->setData(QModelIndex(), QVariant(QUrl(QStringLiteral("ut-x.png"))), Types::ImageUrlRole);

    // Assert
    EXPECT_EQ(changed, false);   // branch: checkIndex 拒绝
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(PathViewProxyModelTest, SetData_RowOutOfRange_ReturnsFalse)
{
    // Arrange
    resetWithFiveUrls();
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    const bool changed = obj->setData(obj->index(99, 0), QVariant(QUrl(QStringLiteral("ut-x.png"))), Types::ImageUrlRole);

    // Assert
    EXPECT_EQ(changed, false);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(PathViewProxyModelTest, SetData_UnsupportedRole_ReturnsFalse)
{
    // Arrange
    resetWithFiveUrls();
    const QUrl before = obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl();
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    const bool changed = obj->setData(obj->index(0, 0), QVariant(3), Types::FrameIndexRole);

    // Assert: switch 仅处理 ImageUrlRole，其余角色拒绝且状态不变（强异常安全）
    EXPECT_EQ(changed, false);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), before);
}

TEST_F(PathViewProxyModelTest, SetData_ImageUrlRole_UpdatesUrlAndEmitsChanged)
{
    // Arrange
    resetWithFiveUrls();
    const QUrl newUrl(QStringLiteral("ut-pathviewproxy-renamed.png"));
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    const bool changed = obj->setData(obj->index(0, 0), QVariant(newUrl), Types::ImageUrlRole);

    // Assert: 无相邻重复 URL → 仅自身 1 次广播
    EXPECT_EQ(changed, true);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), newUrl);
}

TEST_F(PathViewProxyModelTest, SetData_ImageUrlRole_PropagatesForwardToDuplicateUrl)
{
    // Arrange: row2 与 row0 持相同 URL（模拟改名联动）
    resetWithFiveUrls();
    const QList<QUrl> urls = makeSourceUrls(5);
    obj->indexQueue[2]->url = urls.at(2);   // row0 即 u2，制造重复
    const QUrl newUrl(QStringLiteral("ut-pathviewproxy-renamed.png"));
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    const bool changed = obj->setData(obj->index(0, 0), QVariant(newUrl), Types::ImageUrlRole);

    // Assert: 前扫命中 row2 → 自身 + row2 共 2 次广播
    EXPECT_EQ(changed, true);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(obj->data(obj->index(2, 0), Types::ImageUrlRole).toUrl(), newUrl);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), newUrl);
}

TEST_F(PathViewProxyModelTest, SetData_ImageUrlRole_PropagatesBackwardToDuplicateUrl)
{
    // Arrange: row2 与 row0 持相同 URL，从 row2 发起改名
    resetWithFiveUrls();
    const QList<QUrl> urls = makeSourceUrls(5);
    obj->indexQueue[2]->url = urls.at(2);
    const QUrl newUrl(QStringLiteral("ut-pathviewproxy-renamed.png"));
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    const bool changed = obj->setData(obj->index(2, 0), QVariant(newUrl), Types::ImageUrlRole);

    // Assert: 回扫命中 row0 → 自身 + row0 共 2 次广播
    EXPECT_EQ(changed, true);
    EXPECT_EQ(spy.count(), 2);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), newUrl);
    EXPECT_EQ(obj->data(obj->index(2, 0), Types::ImageUrlRole).toUrl(), newUrl);
}

TEST_F(PathViewProxyModelTest, SetData_NullInfoSlot_ReturnsTrueWithoutEmit)
{
    // Arrange: 2 张图片 → row2 为空槽位
    sourceModel->setImageFiles(makeSourceUrls(2));
    obj->resetModel(0, 0);
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    const bool changed = obj->setData(obj->index(2, 0), QVariant(QUrl(QStringLiteral("ut-x.png"))), Types::ImageUrlRole);

    // Assert: infoPtr 为空 → 跳过更新仍返回 true，槽位保持空
    EXPECT_EQ(changed, true);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_TRUE(obj->data(obj->index(2, 0), Types::ImageUrlRole).toUrl().isEmpty());
}

// ─── setCurrentSourceIndex ───

TEST_F(PathViewProxyModelTest, SetCurrentSourceIndex_EmptyQueue_EarlyReturn)
{
    // Arrange: 未 resetModel
    sourceModel->setImageFiles(makeSourceUrls(5));
    QSignalSpy spy(obj, &PathViewProxyModel::currentIndexChanged);

    // Act
    obj->setCurrentSourceIndex(0, 0);

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->rowCount(), 0);
    EXPECT_EQ(obj->currentIndex(), 0);
}

TEST_F(PathViewProxyModelTest, SetCurrentSourceIndex_CurrentDistance_KeepsPosition)
{
    // Arrange: 当前源索引 2
    resetWithFiveUrls();

    // Act
    obj->setCurrentSourceIndex(2, 0);

    // Assert: Current/Invalid → 无动作
    EXPECT_EQ(obj->currentIndex(), 0);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(2));
}

TEST_F(PathViewProxyModelTest, SetCurrentSourceIndex_NextDistance_MovesNext)
{
    // Arrange
    resetWithFiveUrls();

    // Act
    obj->setCurrentSourceIndex(3, 0);

    // Assert: 走 moveNext，右侧滑窗槽位被清空（源索引 5 越界）
    EXPECT_EQ(obj->currentIndex(), 1);
    EXPECT_EQ(obj->data(obj->index(1, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(3));
    EXPECT_TRUE(obj->data(obj->index(3, 0), Types::ImageUrlRole).toUrl().isEmpty());
}

TEST_F(PathViewProxyModelTest, SetCurrentSourceIndex_PreviousDistance_MovesPrevious)
{
    // Arrange
    resetWithFiveUrls();

    // Act
    obj->setCurrentSourceIndex(1, 0);

    // Assert: 走 movePrevoius
    EXPECT_EQ(obj->currentIndex(), 4);
    EXPECT_EQ(obj->data(obj->index(4, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(1));
    EXPECT_TRUE(obj->data(obj->index(2, 0), Types::ImageUrlRole).toUrl().isEmpty());
}

TEST_F(PathViewProxyModelTest, SetCurrentSourceIndex_OutOfRangeDistance_JumpsToTarget)
{
    // Arrange
    resetWithFiveUrls();

    // Act
    obj->setCurrentSourceIndex(4, 0);

    // Assert: 走 jumpToIndex（源索引 4 距当前 2 隔一张），跳转方向 Next
    EXPECT_EQ(obj->currentIndex(), 1);
    EXPECT_EQ(obj->data(obj->index(1, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(4));
    EXPECT_EQ(obj->jumpFlag, PathViewProxyModel::Next);
}

TEST_F(PathViewProxyModelTest, SetCurrentSourceIndex_JumpInProgress_RefreshesSidesThenMoves)
{
    // Arrange: 先跳转到源索引 3（jumpFlag=Next，动画未完成）
    resetWithFiveUrls();
    obj->jumpToIndex(3, 0);
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act: 回跳源索引 2（Previous 距离）→ 打断动画：先 refreshBothSideData 再 movePrevoius
    obj->setCurrentSourceIndex(2, 0);

    // Assert: 两侧围绕源索引 3 重建（4 次更新）后回移，队列恢复 [u2,u3,u4,u0,u1]
    EXPECT_GE(spy.count(), 2 * kDefaultRadius);
    EXPECT_EQ(obj->currentIndex(), 0);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(2));
    EXPECT_EQ(obj->data(obj->index(3, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(0));
    EXPECT_EQ(obj->jumpFlag, PathViewProxyModel::Current);
}

// ─── asyncUpdateLoadInfo ───
//
// 手动触发链：asyncUpdateLoadInfo 内部 new ImageInfo(url, this) 后才 connect
// statusChanged，构造期发射不会进入回调；测试通过 children() 拿到 delayInfo
// 并 Q_EMIT statusChanged() 模拟异步加载状态翻转，驱动 lambda 各分支。

TEST_F(PathViewProxyModelTest, AsyncUpdateLoadInfo_NonZeroFrameIndex_SkipsAsyncLoad)
{
    // Arrange
    resetWithFiveUrls();
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);
    const int childrenBefore = obj->children().count();

    // Act: 仅 frameIndex == 0 的首页需要异步补偿
    obj->asyncUpdateLoadInfo(QUrl::fromLocalFile(QStringLiteral("ut-pathviewproxy-async.png")), 0, 1);

    // Assert: 不创建 delayInfo、不更新数据
    EXPECT_EQ(obj->children().count(), childrenBefore);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->currentIndex(), 0);
}

TEST_F(PathViewProxyModelTest, AsyncUpdateLoadInfo_NullCurrentInfo_IgnoresUpdate)
{
    // Arrange: 空源数据 → 队列全空槽位；就绪多帧使回调进入更新分支
    sourceModel->setImageFiles(QList<QUrl>());
    obj->resetModel(0, 0);
    stub.set_lamda(VADDR(ImageInfo, status), [](ImageInfo *self) {
        Q_UNUSED(self)
        return ImageInfo::Ready;
    });
    stub.set_lamda(VADDR(ImageInfo, type), [](ImageInfo *self) {
        Q_UNUSED(self)
        return Types::MultiImage;
    });
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    obj->asyncUpdateLoadInfo(QUrl::fromLocalFile(QStringLiteral("ut-pathviewproxy-nullcur.png")), 0, 0);
    ASSERT_EQ(obj->children().count(), 1);
    auto *delayInfo = qobject_cast<ImageInfo *>(obj->children().constFirst());
    ASSERT_NE(delayInfo, nullptr);
    Q_EMIT delayInfo->statusChanged();

    // Assert: current 为空 → 记录警告并回收，不更新任何槽位
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->rowCount(), kDefaultQueueCount);
    EXPECT_EQ(obj->currentIndex(), 0);
}

TEST_F(PathViewProxyModelTest, AsyncUpdateLoadInfo_SourceEqualsCurrent_UpdatesFrameAndSides)
{
    // Arrange: 当前源索引 2；就绪多帧图
    resetWithFiveUrls();
    stub.set_lamda(VADDR(ImageInfo, status), [](ImageInfo *self) {
        Q_UNUSED(self)
        return ImageInfo::Ready;
    });
    stub.set_lamda(VADDR(ImageInfo, type), [](ImageInfo *self) {
        Q_UNUSED(self)
        return Types::MultiImage;
    });
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    obj->asyncUpdateLoadInfo(QUrl::fromLocalFile(QStringLiteral("ut-pathviewproxy-equal.png")), 2, 0);
    ASSERT_EQ(obj->children().count(), 1);
    auto *delayInfo = qobject_cast<ImageInfo *>(obj->children().constFirst());
    ASSERT_NE(delayInfo, nullptr);
    Q_EMIT delayInfo->statusChanged();

    // Assert: 仅更新当前帧总数并刷新两侧（2*radius 次广播），队列布局幂等
    EXPECT_EQ(spy.count(), 2 * kDefaultRadius);
    EXPECT_EQ(obj->currentIndex(), 0);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(2));
    EXPECT_EQ(obj->indexQueue.at(0)->frameCount, 1);   // delayInfo 无数据 → frameCount() 兜底 1
}

TEST_F(PathViewProxyModelTest, AsyncUpdateLoadInfo_SourceBeforeCurrent_RefreshesPreviousSide)
{
    // Arrange: 当前源索引 2，异步完成的是源索引 0
    resetWithFiveUrls();
    stub.set_lamda(VADDR(ImageInfo, status), [](ImageInfo *self) {
        Q_UNUSED(self)
        return ImageInfo::Ready;
    });
    stub.set_lamda(VADDR(ImageInfo, type), [](ImageInfo *self) {
        Q_UNUSED(self)
        return Types::MultiImage;
    });
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    obj->asyncUpdateLoadInfo(QUrl::fromLocalFile(QStringLiteral("ut-pathviewproxy-before.png")), 0, 0);
    ASSERT_EQ(obj->children().count(), 1);
    auto *delayInfo = qobject_cast<ImageInfo *>(obj->children().constFirst());
    ASSERT_NE(delayInfo, nullptr);
    Q_EMIT delayInfo->statusChanged();

    // Assert: 仅回填前侧 radius 个槽位（2 次广播），数据内容幂等
    EXPECT_EQ(spy.count(), kDefaultRadius);
    EXPECT_EQ(obj->currentIndex(), 0);
    EXPECT_EQ(obj->data(obj->index(3, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(0));
    EXPECT_EQ(obj->data(obj->index(4, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(1));
}

TEST_F(PathViewProxyModelTest, AsyncUpdateLoadInfo_SourceAfterCurrent_RefreshesNextSide)
{
    // Arrange: 当前源索引 2，异步完成的是源索引 4
    resetWithFiveUrls();
    stub.set_lamda(VADDR(ImageInfo, status), [](ImageInfo *self) {
        Q_UNUSED(self)
        return ImageInfo::Ready;
    });
    stub.set_lamda(VADDR(ImageInfo, type), [](ImageInfo *self) {
        Q_UNUSED(self)
        return Types::MultiImage;
    });
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    obj->asyncUpdateLoadInfo(QUrl::fromLocalFile(QStringLiteral("ut-pathviewproxy-after.png")), 4, 0);
    ASSERT_EQ(obj->children().count(), 1);
    auto *delayInfo = qobject_cast<ImageInfo *>(obj->children().constFirst());
    ASSERT_NE(delayInfo, nullptr);
    Q_EMIT delayInfo->statusChanged();

    // Assert: 仅回填后侧 radius 个槽位（2 次广播）
    EXPECT_EQ(spy.count(), kDefaultRadius);
    EXPECT_EQ(obj->currentIndex(), 0);
    EXPECT_EQ(obj->data(obj->index(1, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(3));
    EXPECT_EQ(obj->data(obj->index(2, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(4));
}

TEST_F(PathViewProxyModelTest, AsyncUpdateLoadInfo_NotReadyMultiImage_IgnoresUpdate)
{
    // Arrange: 状态 Ready 但类型非 MultiImage（普通静态图无需帧补偿）
    resetWithFiveUrls();
    stub.set_lamda(VADDR(ImageInfo, status), [](ImageInfo *self) {
        Q_UNUSED(self)
        return ImageInfo::Ready;
    });
    stub.set_lamda(VADDR(ImageInfo, type), [](ImageInfo *self) {
        Q_UNUSED(self)
        return Types::NormalImage;
    });
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);
    const QUrl urlBefore = obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl();

    // Act
    obj->asyncUpdateLoadInfo(QUrl::fromLocalFile(QStringLiteral("ut-pathviewproxy-static.png")), 2, 0);
    ASSERT_EQ(obj->children().count(), 1);
    auto *delayInfo = qobject_cast<ImageInfo *>(obj->children().constFirst());
    ASSERT_NE(delayInfo, nullptr);
    Q_EMIT delayInfo->statusChanged();

    // Assert: else 分支仅回收 delayInfo，队列与信号均不变（强异常安全）
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->rowCount(), kDefaultQueueCount);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), urlBefore);
}

TEST_F(PathViewProxyModelTest, AsyncUpdateLoadInfo_LoadingStatus_WaitsForTerminalState)
{
    // Arrange: Loading 为瞬态，回调应直接返回等待 Ready/Error
    resetWithFiveUrls();
    stub.set_lamda(VADDR(ImageInfo, status), [](ImageInfo *self) {
        Q_UNUSED(self)
        return ImageInfo::Loading;
    });
    QSignalSpy spy(obj, &QAbstractItemModel::dataChanged);

    // Act
    obj->asyncUpdateLoadInfo(QUrl::fromLocalFile(QStringLiteral("ut-pathviewproxy-loading.png")), 2, 0);
    ASSERT_EQ(obj->children().count(), 1);
    auto *delayInfo = qobject_cast<ImageInfo *>(obj->children().constFirst());
    ASSERT_NE(delayInfo, nullptr);
    Q_EMIT delayInfo->statusChanged();

    // Assert: 早退不触碰队列（后续终态才会真正更新）
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(obj->currentIndex(), 0);
    EXPECT_EQ(obj->data(obj->index(0, 0), Types::ImageUrlRole).toUrl(), makeSourceUrls(5).at(2));
}

// ─── IndexInfo（嵌套结构体：拷贝构造 + 默认构造） ───

TEST_F(PathViewProxyModelTest, IndexInfo_CopyConstructor_CopiesAllFields)
{
    // Arrange
    const QUrl url(QStringLiteral("ut-pathviewproxy-copy.png"));
    PathViewProxyModel::IndexInfo original;
    original.url = url;
    original.index = 7;
    original.frameCount = 4;
    original.frameIndex = 2;

    // Act
    const PathViewProxyModel::IndexInfo copied(original);

    // Assert: url/index/frameCount/frameIndex 全量深拷贝
    EXPECT_EQ(copied.url, url);
    EXPECT_EQ(copied.index, 7);
    EXPECT_EQ(copied.frameCount, 4);
    EXPECT_EQ(copied.frameIndex, 2);
}

TEST_F(PathViewProxyModelTest, IndexInfo_DefaultConstruction_HasSafeDefaults)
{
    // Arrange
    PathViewProxyModel::IndexInfo info;

    // Act
    const int defaultIndex = info.index;
    const int defaultFrameCount = info.frameCount;
    const int defaultFrameIndex = info.frameIndex;

    // Assert: 默认帧总数 1、帧索引 0，避免除零/越界
    EXPECT_TRUE(info.url.isEmpty());
    EXPECT_EQ(defaultIndex, 0);
    EXPECT_EQ(defaultFrameCount, 1);
    EXPECT_EQ(defaultFrameIndex, 0);
}
