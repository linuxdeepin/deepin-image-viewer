// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | GlobalControl | low | - | 1 | 3 |
// | ~GlobalControl | low | - | 1 | 1 |
// | addImageAndSetCurrentSource | low | - | 1 | 7 |
// | checkSwitchEnable | mid | complexity:5 | 2 | 6 |
// | currentFrameIndex | low | - | 1 | 1 |
// | currentIndex | mid | in_degree:4 | 2 | 2 |
// | currentRotation | mid | in_degree:4 | 2 | 2 |
// | currentSource | mid | in_degree:3 | 2 | 2 |
// | enableMultiThread | mid | in_degree:3 | 2 | 4 |
// | firstImage | low | - | 1 | 2 |
// | forceExit | low | - | 1 | 1 |
// | globalModel | low | - | 1 | 1 |
// | hasNextImage | mid | in_degree:3 | 2 | 3 |
// | hasPreviousImage | mid | in_degree:3 | 2 | 3 |
// | imageCount | mid | in_degree:8 | 2 | 2 |
// | lastImage | low | - | 1 | 3 |
// | nextImage | mid | in_degree:7 | 2 | 5 |
// | previousImage | mid | in_degree:7 | 2 | 5 |
// | removeImage | high | lines:54 | 3 | 5 |
// | renameImage | low | - | 1 | 2 |
// | setCurrentFrameIndex | low | - | 1 | 2 |
// | setCurrentIndex | low | - | 1 | 2 |
// | setCurrentRotation | mid | in_degree:3 | 2 | 4 |
// | setCurrentSource | low | - | 1 | 3 |
// | setImageFiles | low | - | 1 | 4 |
// | setIndexAndFrameIndex | mid | in_degree:3 | 2 | 4 |
// | submitImageChangeImmediately | mid | in_degree:5 | 2 | 3 |
// | timerEvent | low | - | 1 | 4 |
// | viewModel | mid | in_degree:3 | 2 | 2 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]（enableMultiThread 线程数 1/2/3/16 覆盖 >2 边界；
//    索引 -1/0/N-1/N 越界钳制见 SetCurrentIndex/SetIndexAndFrameIndex/CurrentFrameIndex 用例）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（addImageAndSetCurrentSource 3 组非法输入；enableMultiThread 4 组线程数）
// 5. 分支清单 → 用例映射已列出: [x]（见下方 9 个分支清单段落）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无显式 throw，异常路径以错误返回值/负面前置条件覆盖）
// 8. 负面场景有专门用例: [x]（空 URL/不存在文件/目录路径/未知路径/空列表/越界索引）
// 9. 负面用例验证强异常安全: [x]（RemoveImage_UnknownImage / SetCurrentSource_UnknownImage 断言状态不变）
// 10. stub_ext vs gMock 选择正确: [x]（依赖均为 Qt 类/项目内非虚类，全用 stub_ext）

#include <gtest/gtest.h>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QThread>
#include <QTimerEvent>
#include <QUrl>

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

// gcov 计数转储（GCC 专属）。
// 注意 1：必须 extern "C" 强引用——gcov.h 的声明无 extern "C" 包裹，C++ 直用会被
// 名字修饰成 __gcov_dump() 未定义；弱引用又不会把 libgcov.a 的 gcov-interface.o
// 拉进链接（__gcov_dump 恒为空指针，fork 子进程覆盖数据静默丢失，实测 FNDA 恒 0）。
// 注意 2：tests/CMakeLists.txt 固定注入 -fprofile-arcs -ftest-coverage，本目标必然
// 处于 coverage 构建形态，强引用安全。
extern "C" void __gcov_dump(void);
#define UT_GCOV_DUMP() __gcov_dump()

#include "stub_ext/stubext.h"

#include "globalcontrol.h"
#include "imagesourcemodel.h"
#include "pathviewproxymodel.h"
#include "rotateimagehelper.h"

namespace {

// enableMultiThread 参数化用例：sc_MaxThreadCountLimit = 2，判定 idealThreadCount() > 2
struct ThreadCountCase {
    int idealThreads;
    bool expected;
};

// addImageAndSetCurrentSource 参数化用例：三类非法输入（B1 的三个短路条件）
enum class InvalidImageKind {
    EmptyUrl,        // image.isEmpty()
    NonexistentFile, // QFileInfo(toLocalFile()).isFile() 为 false（文件不存在）
    DirectoryPath    // QFileInfo(toLocalFile()).isFile() 为 false（路径是目录）
};

} // namespace

// ═══════════════ 复杂方法分支清单（来源：get_code_snippet 真实源码）═══════════════

// 分支清单（来源：GlobalControl::addImageAndSetCurrentSource，共 7 分支）
// B1: !sourceModel || image.isEmpty() || !QFileInfo(image.toLocalFile()).isFile() → return false
// B2: -1 != existingIndex（图片已在模型中）→ setIndexAndFrameIndex(existingIndex, 0) 后提前返回
// B3: -1 == index（sourceModel->insertImage 拒绝插入）→ return false
// B4: 新图插入成功主路径 → setSource / curIndex / curFrameIndex 归零并 emit currentSourceChanged + currentIndexChanged
// B5: frameIndexChanged（插入前 curFrameIndex != 0）→ emit currentFrameIndexChanged
// B6: B2 分支提前 return true
// B7: B3 分支提前 return false
// 用例映射：
// - AddImageAndSetCurrentSource_InvalidInput_ReturnsFalse（TEST_P ×3）→ B1
// - AddImageAndSetCurrentSource_ExistingImage_ReusesIndexWithoutInsert → B1+B2+B6
// - AddImageAndSetCurrentSource_ValidNewImage_UpdatesCurrentAndEmitsSignals → B4（B5 条件为假）
// - AddImageAndSetCurrentSource_InsertRejectedByModel_ReturnsFalse → B3+B7
// - AddImageAndSetCurrentSource_FrameIndexNonZero_EmitsFrameIndexChanged → B4+B5
//
// 分支清单（来源：GlobalControl::checkSwitchEnable，共 5 分支）
// B1: !sourceModel || sourceModel->rowCount() <= 0 → 复位 hasPrevious/hasNext 后 return
// B2: 空模型复位路径中 hasPrevious == true → emit hasPreviousImageChanged
// B3: 空模型复位路径中 hasNext == true → emit hasNextImageChanged
// B4: previous != hasPrevious（previous = curIndex > 0 || curFrameIndex > 0）→ 更新并 emit hasPreviousImageChanged
// B5: next != hasNext（next = curIndex < rowCount-1 || (frameCount > 1 && curFrameIndex < frameCount-1)) → 更新并 emit hasNextImageChanged
// 用例映射：
// - CheckSwitchEnable_EmptyModel_ResetsBothFlags → B1+B2+B3
// - CheckSwitchEnable_MiddleIndex_EnablesBothDirections → B4+B5
// - CheckSwitchEnable_FirstIndex_HasNoPrevious → B4
// - CheckSwitchEnable_LastIndex_HasNoNext → B5
// - CheckSwitchEnable_UnchangedFlags_EmitNoSignals → B4/B5 条件均为假
// - CheckSwitchEnable_MultiImageMiddleFrame_EnablesNext → B4+B5（frameCount>1 短路右真）
//
// 分支清单（来源：GlobalControl::lastImage，共 3 分支）
// B1: count == 0（空模型）→ return false
// B2: Types::MultiImage == currentImage.type() → frameIndex = frameCount() - 1
// B3: count != 0 主路径 → setIndexAndFrameIndex(count-1, frameIndex) 后提前 return true
// 用例映射：
// - LastImage_EmptyModel_ReturnsFalse → B1
// - LastImage_NormalImage_MovesToLastIndexWithZeroFrame → B3（B2 为假）
// - LastImage_MultiImage_SetsFrameIndexToLastFrame → B2+B3
//
// 分支清单（来源：GlobalControl::nextImage，共 6 分支）
// B1: hasNextImage() == false → 落到末尾 return false
// B2: Types::MultiImage == currentImage.type() → 允许帧内前进
// B3: B2 成立且 curFrameIndex < frameCount()-1 → setIndexAndFrameIndex(curIndex, curFrameIndex+1) 后提前 return true
// B4: curIndex < sourceModel->rowCount()-1 → setIndexAndFrameIndex(curIndex+1, 0) 后提前 return true
// B5: B3 帧路径提前 return true
// B6: B4 索引路径提前 return true
// 用例映射：
// - NextImage_SingleImage_ReturnsFalse → B1
// - NextImage_MiddleImage_AdvancesToNextIndex → B1+B2(假)+B4+B6
// - NextImage_LastIndex_ReturnsFalse → B1
// - NextImage_MultiImage_AdvancesFrameBeforeIndex → B2+B3+B5
// - NextImage_MultiImageLastFrame_MovesToNextImageFrameZero → B2+B3(假)+B4+B6
//
// 分支清单（来源：GlobalControl::previousImage，共 6 分支）
// B1: hasPreviousImage() == false → 落到末尾 return false
// B2: Types::MultiImage == currentImage.type() → 允许帧内后退
// B3: B2 成立且 curFrameIndex > 0 → setIndexAndFrameIndex(curIndex, curFrameIndex-1) 后提前 return true
// B4: curIndex > 0 → setIndexAndFrameIndex(curIndex-1, INT_MAX) 后提前 return true
// B5: B3 帧路径提前 return true
// B6: B4 索引路径提前 return true
// 用例映射：
// - PreviousImage_FirstImage_ReturnsFalse → B1
// - PreviousImage_MiddleImage_MovesToPreviousIndex → B4+B6
// - PreviousImage_EmptyModel_ReturnsFalse → B1
// - PreviousImage_MultiImage_RetreatsFrameBeforeIndex → B2+B3+B5
// - PreviousImage_MultiImageFirstFrame_MovesToPreviousImage → B2+B3(假)+B4+B6
//
// 分支清单（来源：GlobalControl::removeImage，共 4 分支）
// B1: 0 != currentRotation() → setCurrentRotation(0) + submitTimer.stop()
// B2: removeImage == currentImage.source() → viewModel()->deleteCurrent()
// B3: !atEnd（当前图不在尾部，curIndex < rowCount-1）→ 读取 index(curIndex) 更新当前图并 emit 双信号
// B4: atEnd 且 sourceModel->rowCount() != 0 → 读取 index(curIndex-1) 且 setIndexAndFrameIndex(curIndex-1, INT_MAX)
//     （else：rowCount == 0 无剩余图片，清空 currentImage/curIndex 并 emit 双信号，随后复位导航标志）
// 用例映射：
// - RemoveImage_CurrentMiddleImage_ShiftsToFollowingImage → B3（B1/B2 命中）
// - RemoveImage_CurrentLastImage_MovesToPreviousImage → B4（B2 命中）
// - RemoveImage_LastRemainingImage_ClearsCurrentState → B4(假) 落 else
// - RemoveImage_WithPendingRotation_SubmitsRotationBeforeRemoval → B1
// - RemoveImage_UnknownImage_KeepsCurrentStateIntact → B3（状态不变断言）
//
// 分支清单（来源：GlobalControl::setCurrentRotation，共 3 分支）
// B1: imageRotation != angle → 进入更新（else：角度未变，跳过全部处理）
// B2: 0 != angle % 90 → qCWarning（非 90 倍数仅告警不拒绝）
// B3: needSwap（angle 非 0 且 (angle - imageRotation) % 180 非 0）→ currentImage.swapWidthAndHeight()
// 用例映射：
// - SetCurrentRotation_NinetyDegrees_UpdatesStateEmitsAndSwapsSize → B1+B3（B2 为假）
// - SetCurrentRotation_SameAngle_SkipsUpdate → B1 为假
// - SetCurrentRotation_ResetToZero_EmitsWithoutSwap → B1+B3(假，angle 为 0 特殊处理)
// - SetCurrentRotation_AngleNotMultipleOfNinety_StillApplied → B1+B2+B3
//
// 分支清单（来源：GlobalControl::setImageFiles，共 5 分支）
// B1: filePaths.isEmpty() → return false
// B2: filePaths.indexOf(openFile) == -1（打开文件不在列表）→ return false
// B3: currentImage.source() != currentSource → currentImage.setSource(currentSource)
// B4: B1 分支提前 return false
// B5: B2 分支提前 return false
// 用例映射：
// - SetImageFiles_EmptyList_ReturnsFalse → B1+B4
// - SetImageFiles_OpenFileMissingFromList_ReturnsFalse → B2+B5
// - SetImageFiles_ValidList_ReturnsTrueAndOpensTarget → B3
// - SetImageFiles_RepeatedCall_KeepsStateConsistent → B3 条件为假
//
// 分支清单（来源：GlobalControl::timerEvent，共 3 分支）
// B1: submitTimer.timerId() == event->timerId() → submitTimer.stop() + submitImageChangeImmediately()
// B2: switchCheckTimer.timerId() == event->timerId() → switchCheckTimer.stop() + checkSwitchEnable()
// B3: viewModelSyncTimer.timerId() == event->timerId() → viewModelSyncTimer.stop() + viewSourceModel->setCurrentSourceIndex()
//     （三个条件均不成立：事件被忽略）
// 用例映射：
// - TimerEvent_SubmitTimerExpired_SubmitsPendingRotation → B1
// - TimerEvent_SwitchCheckTimerExpired_RechecksNavigationFlags → B2
// - TimerEvent_ViewModelSyncTimerExpired_SyncsViewModel → B3
// - TimerEvent_UnknownTimerId_IgnoresEventSafely → B1/B2/B3 均为假

// ═══════════════════════════════════════════════════════════════════

class GlobalControlTest : public ::testing::Test {
protected:
    void SetUp() override {
        stub.clear();
        rotateFileCalls = 0;
        swapCalls = 0;
        reloadCalls = 0;

        ctrl = new GlobalControl(nullptr);

        // 基础 stub：阻断 ImageInfo 缓存异步加载（避免线程/磁盘不确定性），
        // data 保持为空 → frameCount()==1 / type()==NullImage，与"普通单帧图"行为一致
        stub.set_lamda(VADDR(ImageInfo, refreshDataFromCache), [](ImageInfo *, bool) {});

        // 基础 stub：RotateImageHelper 为真实单例，rotateImageFile 会排队异步写文件，必须隔离
        stub.set_lamda(VADDR(RotateImageHelper, rotateImageFile),
                       [this](RotateImageHelper *, const QString &, int) { ++rotateFileCalls; });
    }

    void TearDown() override {
        delete ctrl;  // stub 仍生效，析构中的 submitImageChangeImmediately 不会写真实文件
        ctrl = nullptr;
        stub.clear();
    }

    // 在临时目录创建真实文件（满足 addImageAndSetCurrentSource 的 QFileInfo::isFile 检查）
    QUrl makeImageFile(const QString &name)
    {
        const QString path = tempDir.filePath(name);
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write("dummy-image-content");
            file.close();
        }
        return QUrl::fromLocalFile(path);
    }

    // 构造 a.png/b.png/c.png 三张图并以 b.png（index 1）打开
    QList<QUrl> loadThreeImages()
    {
        const QList<QUrl> urls = {makeImageFile(QStringLiteral("a.png")),
                                  makeImageFile(QStringLiteral("b.png")),
                                  makeImageFile(QStringLiteral("c.png"))};
        QStringList paths;
        for (const QUrl &u : urls)
            paths << u.toString();  // setImageFiles 内部 QUrl::fromStringList 需 file:// 形态，纯路径会变 scheme-less URL
        ctrl->setImageFiles(paths, paths.at(1));
        return urls;
    }

    stub_ext::StubExt stub;
    GlobalControl *ctrl = nullptr;
    QTemporaryDir tempDir;
    int rotateFileCalls = 0;
    int swapCalls = 0;
    int reloadCalls = 0;
};

// TEST_P 参数化子 Fixture（继承主 Fixture，补充 WithParamInterface）
struct GlobalControlInvalidKindTest : public GlobalControlTest,
                                      public ::testing::WithParamInterface<InvalidImageKind> {
};
struct GlobalControlThreadCountTest : public GlobalControlTest,
                                      public ::testing::WithParamInterface<ThreadCountCase> {
};

// ═══════════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F/TEST_P 均包含 // Arrange / // Act / // Assert 三段注释

// ── 构造 / 析构 ──

TEST_F(GlobalControlTest, GlobalControl_Construction_InitializesModelsAndDefaults)
{
    // Arrange（SetUp 已构造 ctrl）
    ASSERT_NE(ctrl, nullptr);

    // Act：取构造产物引用，供断言使用
    ImageSourceModel *model = ctrl->globalModel();
    PathViewProxyModel *proxy = ctrl->viewModel();

    // Assert
    EXPECT_NE(model, nullptr);
    EXPECT_NE(proxy, nullptr);
    EXPECT_EQ(ctrl->imageCount(), 0);
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(ctrl->currentFrameIndex(), 0);
    EXPECT_EQ(ctrl->currentRotation(), 0);
    EXPECT_TRUE(ctrl->currentSource().isEmpty());
}

TEST_F(GlobalControlTest, GlobalControl_DestructionWithPendingRotation_SubmitsChange)
{
    // Arrange
    loadThreeImages();
    ctrl->setCurrentRotation(90);  // 挂起 90 度未提交旋转
    QSignalSpy spyRotate(ctrl, &GlobalControl::requestRotateImage);

    // Act
    delete ctrl;
    ctrl = nullptr;

    // Assert：析构函数触发 submitImageChangeImmediately，发出旋转请求并复位角度
    EXPECT_EQ(spyRotate.count(), 1);
    EXPECT_EQ(spyRotate.at(0).at(1).toInt(), 90);
}

// ── addImageAndSetCurrentSource ──

TEST_P(GlobalControlInvalidKindTest, AddImageAndSetCurrentSource_InvalidInput_ReturnsFalse)
{
    // Arrange
    QUrl url;
    switch (GetParam()) {
    case InvalidImageKind::EmptyUrl:
        url = QUrl();
        break;
    case InvalidImageKind::NonexistentFile:
        url = QUrl::fromLocalFile(tempDir.filePath(QStringLiteral("missing.png")));
        break;
    case InvalidImageKind::DirectoryPath:
        url = QUrl::fromLocalFile(tempDir.path());
        break;
    }

    // Act
    const bool ret = ctrl->addImageAndSetCurrentSource(url);

    // Assert：非法输入被拒绝且无任何副作用（强异常安全）
    EXPECT_FALSE(ret);  // branch: !sourceModel || image.isEmpty() || !isFile
    EXPECT_EQ(ctrl->imageCount(), 0);
    EXPECT_TRUE(ctrl->currentSource().isEmpty());
}

INSTANTIATE_TEST_SUITE_P(InvalidImageInputs, GlobalControlInvalidKindTest,
                         ::testing::Values(InvalidImageKind::EmptyUrl,
                                           InvalidImageKind::NonexistentFile,
                                           InvalidImageKind::DirectoryPath));

TEST_F(GlobalControlTest, AddImageAndSetCurrentSource_ExistingImage_ReusesIndexWithoutInsert)
{
    // Arrange
    const QList<QUrl> urls = loadThreeImages();  // 当前 b.png(index 1)
    QSignalSpy spyCount(ctrl, &GlobalControl::imageCountChanged);

    // Act：再次添加已存在的 b.png
    const bool ret = ctrl->addImageAndSetCurrentSource(urls.at(1));

    // Assert：命中已有索引，不新增数据、不重复插入
    EXPECT_TRUE(ret);  // branch: -1 != existingIndex
    EXPECT_EQ(ctrl->imageCount(), 3);
    EXPECT_EQ(ctrl->currentIndex(), 1);
    EXPECT_EQ(ctrl->currentSource(), urls.at(1));
    EXPECT_EQ(spyCount.count(), 0);
}

TEST_F(GlobalControlTest, AddImageAndSetCurrentSource_ValidNewImage_UpdatesCurrentAndEmitsSignals)
{
    // Arrange
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);
    QSignalSpy spyIndex(ctrl, &GlobalControl::currentIndexChanged);
    QSignalSpy spyFrame(ctrl, &GlobalControl::currentFrameIndexChanged);
    QSignalSpy spyCount(ctrl, &GlobalControl::imageCountChanged);

    // Act：向空模型添加真实存在的文件
    const bool ret = ctrl->addImageAndSetCurrentSource(urlA);

    // Assert：插入成功，当前源/索引/数量全部更新
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->imageCount(), 1);
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(ctrl->currentSource(), urlA);
    EXPECT_EQ(spySource.count(), 1);
    EXPECT_EQ(spyIndex.count(), 1);
    EXPECT_EQ(spyFrame.count(), 0);  // 初始帧为 0，frameIndexChanged 为假
    EXPECT_EQ(spyCount.count(), 1);
}

TEST_F(GlobalControlTest, AddImageAndSetCurrentSource_InsertRejectedByModel_ReturnsFalse)
{
    // Arrange：让 sourceModel->insertImage 返回 -1（B3 防御分支）
    stub.set_lamda(VADDR(ImageSourceModel, insertImage),
                   [](ImageSourceModel *, const QUrl &) -> int { return -1; });
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));

    // Act
    const bool ret = ctrl->addImageAndSetCurrentSource(urlA);

    // Assert
    EXPECT_FALSE(ret);  // branch: -1 == index
    EXPECT_EQ(ctrl->imageCount(), 0);
    EXPECT_TRUE(ctrl->currentSource().isEmpty());
}

TEST_F(GlobalControlTest, AddImageAndSetCurrentSource_FrameIndexNonZero_EmitsFrameIndexChanged)
{
    // Arrange：构造多帧图场景（frameCount=3），先把帧翻到 1 再添加新图
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    ASSERT_TRUE(ctrl->addImageAndSetCurrentSource(urlA));
    ctrl->setCurrentFrameIndex(1);
    ASSERT_EQ(ctrl->currentFrameIndex(), 1);
    const QUrl urlB = makeImageFile(QStringLiteral("b.png"));
    QSignalSpy spyFrame(ctrl, &GlobalControl::currentFrameIndexChanged);

    // Act
    const bool ret = ctrl->addImageAndSetCurrentSource(urlB);

    // Assert：插入前帧非 0 → 发出帧变更信号并复位为 0
    EXPECT_TRUE(ret);
    EXPECT_EQ(spyFrame.count(), 1);  // branch: frameIndexChanged
    EXPECT_EQ(ctrl->currentFrameIndex(), 0);
    EXPECT_EQ(ctrl->currentIndex(), 1);
    EXPECT_EQ(ctrl->currentSource(), urlB);
}

// ── checkSwitchEnable ──

TEST_F(GlobalControlTest, CheckSwitchEnable_EmptyModel_ResetsBothFlags)
{
    // Arrange：空模型 + 人为置位的导航标志（private 成员，-fno-access-control 直接访问）
    ctrl->hasPrevious = true;
    ctrl->hasNext = true;
    QSignalSpy spyPrev(ctrl, &GlobalControl::hasPreviousImageChanged);
    QSignalSpy spyNext(ctrl, &GlobalControl::hasNextImageChanged);

    // Act
    ctrl->checkSwitchEnable();

    // Assert：空模型下两个方向都不可用，且各自只发一次变更信号
    EXPECT_FALSE(ctrl->hasPreviousImage());
    EXPECT_FALSE(ctrl->hasNextImage());
    EXPECT_EQ(spyPrev.count(), 1);
    EXPECT_EQ(spyNext.count(), 1);
}

TEST_F(GlobalControlTest, CheckSwitchEnable_MiddleIndex_EnablesBothDirections)
{
    // Arrange：三张图打开中间张（index 1）
    loadThreeImages();
    QSignalSpy spyPrev(ctrl, &GlobalControl::hasPreviousImageChanged);
    QSignalSpy spyNext(ctrl, &GlobalControl::hasNextImageChanged);

    // Act
    ctrl->checkSwitchEnable();

    // Assert：中间位置前后均可切换，且无状态翻转（setup 时已翻转）→ 不再发信号
    EXPECT_TRUE(ctrl->hasPreviousImage());
    EXPECT_TRUE(ctrl->hasNextImage());
    EXPECT_EQ(spyPrev.count(), 0);
    EXPECT_EQ(spyNext.count(), 0);
}

TEST_F(GlobalControlTest, CheckSwitchEnable_FirstIndex_HasNoPrevious)
{
    // Arrange
    const QList<QUrl> urls = loadThreeImages();
    ctrl->curIndex = 0;  // 移到首张
    QSignalSpy spyPrev(ctrl, &GlobalControl::hasPreviousImageChanged);
    QSignalSpy spyNext(ctrl, &GlobalControl::hasNextImageChanged);

    // Act
    ctrl->checkSwitchEnable();

    // Assert
    EXPECT_FALSE(ctrl->hasPreviousImage());  // branch: previous != hasPrevious → false
    EXPECT_EQ(spyPrev.count(), 1);
    EXPECT_EQ(spyNext.count(), 0);
    EXPECT_EQ(ctrl->currentIndex(), 0);
    // checkSwitchEnable 只重估导航标志，不得改动当前源（仍为 b.png）
    EXPECT_EQ(ctrl->currentSource(), urls.at(1));
}

TEST_F(GlobalControlTest, CheckSwitchEnable_LastIndex_HasNoNext)
{
    // Arrange
    loadThreeImages();
    ctrl->curIndex = 2;  // 移到末张
    QSignalSpy spyNext(ctrl, &GlobalControl::hasNextImageChanged);
    QSignalSpy spyPrev(ctrl, &GlobalControl::hasPreviousImageChanged);

    // Act
    ctrl->checkSwitchEnable();

    // Assert
    EXPECT_FALSE(ctrl->hasNextImage());  // branch: next != hasNext → false
    EXPECT_EQ(spyNext.count(), 1);
    EXPECT_EQ(spyPrev.count(), 0);
}

TEST_F(GlobalControlTest, CheckSwitchEnable_UnchangedFlags_EmitNoSignals)
{
    // Arrange：三张图打开中间张（index 1），标志已稳定为 true/true
    loadThreeImages();
    QSignalSpy spyPrev(ctrl, &GlobalControl::hasPreviousImageChanged);
    QSignalSpy spyNext(ctrl, &GlobalControl::hasNextImageChanged);

    // Act：重复检查
    ctrl->checkSwitchEnable();
    ctrl->checkSwitchEnable();

    // Assert：状态无变化时不发信号
    EXPECT_EQ(spyPrev.count(), 0);
    EXPECT_EQ(spyNext.count(), 0);
    EXPECT_TRUE(ctrl->hasPreviousImage());
    EXPECT_TRUE(ctrl->hasNextImage());
}

TEST_F(GlobalControlTest, CheckSwitchEnable_MultiImageMiddleFrame_EnablesNext)
{
    // Arrange：单张图初始 flags=false/false（真实 frameCount=1），随后才注入多帧 frameCount=3
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    QStringList paths{urlA.toString()};  // file:// 形态与模型存储一致
    ctrl->setImageFiles(paths, paths.at(0));
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    ctrl->curFrameIndex = 1;  // 直接置位中间帧，避免提前触发 checkSwitchEnable
    QSignalSpy spyPrev(ctrl, &GlobalControl::hasPreviousImageChanged);
    QSignalSpy spyNext(ctrl, &GlobalControl::hasNextImageChanged);

    // Act
    ctrl->checkSwitchEnable();

    // Assert：frameCount > 1 && curFrameIndex < frameCount-1 短路右真 → next=true
    EXPECT_TRUE(ctrl->hasNextImage());
    EXPECT_TRUE(ctrl->hasPreviousImage());  // curFrameIndex > 0 也使 previous=true
    EXPECT_EQ(spyNext.count(), 1);
    EXPECT_EQ(spyPrev.count(), 1);
}

// ── 简单 getter：currentFrameIndex / currentIndex / currentRotation / currentSource ──

TEST_F(GlobalControlTest, CurrentFrameIndex_InitialAndUpdatedValues_ReflectState)
{
    // Arrange
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    loadThreeImages();
    EXPECT_EQ(ctrl->currentFrameIndex(), 0);  // 初始为 0

    // Act
    ctrl->setCurrentFrameIndex(2);

    // Assert
    EXPECT_EQ(ctrl->currentFrameIndex(), 2);
    EXPECT_EQ(ctrl->currentIndex(), 1);
}

TEST_F(GlobalControlTest, CurrentIndex_InitialState_ReturnsZero)
{
    // Arrange（SetUp 已构造）
    ASSERT_NE(ctrl, nullptr);

    // Act
    const int index = ctrl->currentIndex();

    // Assert
    EXPECT_EQ(index, 0);
    EXPECT_EQ(ctrl->imageCount(), 0);
}

TEST_F(GlobalControlTest, CurrentIndex_AfterOpenFile_ReturnsOpenFileIndex)
{
    // Arrange
    const QList<QUrl> urls = loadThreeImages();  // 打开 b.png

    // Act
    const int index = ctrl->currentIndex();

    // Assert
    EXPECT_EQ(index, 1);
    EXPECT_EQ(ctrl->currentSource(), urls.at(1));
}

TEST_F(GlobalControlTest, CurrentRotation_InitialState_ReturnsZero)
{
    // Arrange（SetUp 已构造）
    ASSERT_NE(ctrl, nullptr);

    // Act
    const int rotation = ctrl->currentRotation();

    // Assert
    EXPECT_EQ(rotation, 0);
    EXPECT_EQ(rotateFileCalls, 0);  // 未设置过角度，不触发旋转请求
}

TEST_F(GlobalControlTest, CurrentRotation_AfterRotationChange_ReturnsAngleAndResets)
{
    // Arrange
    loadThreeImages();

    // Act
    ctrl->setCurrentRotation(90);
    const int rotated = ctrl->currentRotation();
    ctrl->setCurrentRotation(0);

    // Assert
    EXPECT_EQ(rotated, 90);
    EXPECT_EQ(ctrl->currentRotation(), 0);
    EXPECT_EQ(rotateFileCalls, 2);  // 一次 90 度请求 + 一次 0 度复位请求
}

TEST_F(GlobalControlTest, CurrentSource_InitialState_ReturnsEmptyUrl)
{
    // Arrange（SetUp 已构造）
    ASSERT_NE(ctrl, nullptr);

    // Act
    const QUrl source = ctrl->currentSource();

    // Assert
    EXPECT_TRUE(source.isEmpty());
    EXPECT_EQ(ctrl->currentIndex(), 0);
}

TEST_F(GlobalControlTest, CurrentSource_AfterOpenFile_ReturnsCurrentFileUrl)
{
    // Arrange
    const QList<QUrl> urls = loadThreeImages();

    // Act
    const QUrl source = ctrl->currentSource();

    // Assert
    EXPECT_EQ(source, urls.at(1));
    EXPECT_EQ(ctrl->currentIndex(), 1);
}

// ── enableMultiThread ──

TEST_P(GlobalControlThreadCountTest, EnableMultiThread_IdealThreadCountBoundary_ReturnsExpected)
{
    // Arrange：sc_MaxThreadCountLimit = 2，判定 idealThreadCount() > 2（覆盖 1/2/3/16 边界）
    const ThreadCountCase &c = GetParam();
    stub.set_lamda(static_cast<int (*)()>(&QThread::idealThreadCount),
                   [&c]() -> int { return c.idealThreads; });

    // Act
    const bool ret = ctrl->enableMultiThread();

    // Assert
    EXPECT_EQ(ret, c.expected);  // branch: QThread::idealThreadCount() > 2
    EXPECT_EQ(QThread::idealThreadCount(), c.idealThreads);  // stub 生效校验
}

INSTANTIATE_TEST_SUITE_P(IdealThreadCountBoundary, GlobalControlThreadCountTest,
                         ::testing::Values(ThreadCountCase{1, false},
                                           ThreadCountCase{2, false},
                                           ThreadCountCase{3, true},
                                           ThreadCountCase{16, true}));

// ── firstImage / lastImage ──

TEST_F(GlobalControlTest, FirstImage_EmptyModel_ReturnsFalse)
{
    // Arrange（空模型）
    ASSERT_EQ(ctrl->imageCount(), 0);

    // Act
    const bool ret = ctrl->firstImage();

    // Assert
    EXPECT_FALSE(ret);  // branch: sourceModel->rowCount() == 0
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(ctrl->imageCount(), 0);
}

TEST_F(GlobalControlTest, FirstImage_FromLastImage_MovesToFirstAndEmitsSignals)
{
    // Arrange：三张图打开末张
    const QList<QUrl> urls = loadThreeImages();
    ctrl->setCurrentIndex(2);
    ASSERT_EQ(ctrl->currentIndex(), 2);
    QSignalSpy spyIndex(ctrl, &GlobalControl::currentIndexChanged);
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);

    // Act
    const bool ret = ctrl->firstImage();

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(ctrl->currentSource(), urls.at(0));
    EXPECT_EQ(spyIndex.count(), 1);
    EXPECT_EQ(spySource.count(), 1);
}

TEST_F(GlobalControlTest, LastImage_EmptyModel_ReturnsFalse)
{
    // Arrange（空模型）
    ASSERT_EQ(ctrl->imageCount(), 0);

    // Act
    const bool ret = ctrl->lastImage();

    // Assert
    EXPECT_FALSE(ret);  // branch: count == 0
    EXPECT_EQ(ctrl->imageCount(), 0);
    EXPECT_EQ(ctrl->currentIndex(), 0);
}

TEST_F(GlobalControlTest, LastImage_NormalImage_MovesToLastIndexWithZeroFrame)
{
    // Arrange：三张图打开中间张 b.png（index 1）
    const QList<QUrl> urls = loadThreeImages();
    ASSERT_EQ(ctrl->currentIndex(), 1);

    // Act
    const bool ret = ctrl->lastImage();

    // Assert：普通单帧图，帧索引固定 0
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->currentIndex(), 2);
    EXPECT_EQ(ctrl->currentFrameIndex(), 0);
    EXPECT_EQ(ctrl->currentSource(), urls.at(2));
}

TEST_F(GlobalControlTest, LastImage_MultiImage_SetsFrameIndexToLastFrame)
{
    // Arrange：多帧图（frameCount=3），type 通过 stub 标记为 MultiImage
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    stub.set_lamda(VADDR(ImageInfo, type),
                   [](ImageInfo *) -> int { return static_cast<int>(Types::MultiImage); });
    const QList<QUrl> urls = loadThreeImages();

    // Act
    const bool ret = ctrl->lastImage();

    // Assert：末张 + 末帧（frameCount-1）
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->currentIndex(), 2);
    EXPECT_EQ(ctrl->currentFrameIndex(), 2);  // branch: MultiImage → frameIndex = 3-1
    EXPECT_EQ(ctrl->currentSource(), urls.at(2));
}

// ── nextImage / previousImage ──

TEST_F(GlobalControlTest, NextImage_SingleImage_ReturnsFalse)
{
    // Arrange：仅一张图且停在唯一索引
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    QStringList paths{urlA.toString()};  // file:// 形态与模型存储一致
    ctrl->setImageFiles(paths, paths.at(0));
    ASSERT_FALSE(ctrl->hasNextImage());

    // Act
    const bool ret = ctrl->nextImage();

    // Assert
    EXPECT_FALSE(ret);  // branch: hasNextImage() == false
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(ctrl->currentSource(), urlA);
}

TEST_F(GlobalControlTest, NextImage_MiddleImage_AdvancesToNextIndex)
{
    // Arrange：三张图打开中间张
    const QList<QUrl> urls = loadThreeImages();

    // Act
    const bool ret = ctrl->nextImage();

    // Assert：普通图直接索引 +1、帧归 0
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->currentIndex(), 2);
    EXPECT_EQ(ctrl->currentFrameIndex(), 0);
    EXPECT_EQ(ctrl->currentSource(), urls.at(2));
}

TEST_F(GlobalControlTest, NextImage_LastIndex_ReturnsFalse)
{
    // Arrange：三张图打开末张
    loadThreeImages();
    ctrl->setCurrentIndex(2);
    ASSERT_FALSE(ctrl->hasNextImage());

    // Act
    const bool ret = ctrl->nextImage();

    // Assert：末张且非多帧 → 无下一张
    EXPECT_FALSE(ret);
    EXPECT_EQ(ctrl->currentIndex(), 2);
    EXPECT_EQ(ctrl->imageCount(), 3);
}

TEST_F(GlobalControlTest, NextImage_MultiImage_AdvancesFrameBeforeIndex)
{
    // Arrange：多帧图（frameCount=3）停在首帧
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    stub.set_lamda(VADDR(ImageInfo, type),
                   [](ImageInfo *) -> int { return static_cast<int>(Types::MultiImage); });
    const QList<QUrl> urls = loadThreeImages();
    ASSERT_EQ(ctrl->currentIndex(), 1);
    ASSERT_EQ(ctrl->currentFrameIndex(), 0);

    // Act
    const bool ret = ctrl->nextImage();

    // Assert：帧内前进优先，索引不变
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->currentIndex(), 1);
    EXPECT_EQ(ctrl->currentFrameIndex(), 1);  // branch: MultiImage && curFrameIndex < frameCount-1
    EXPECT_EQ(ctrl->currentSource(), urls.at(1));
}

TEST_F(GlobalControlTest, NextImage_MultiImageLastFrame_MovesToNextImageFrameZero)
{
    // Arrange：多帧图（frameCount=3）停在末帧
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    stub.set_lamda(VADDR(ImageInfo, type),
                   [](ImageInfo *) -> int { return static_cast<int>(Types::MultiImage); });
    const QList<QUrl> urls = loadThreeImages();
    ctrl->setCurrentFrameIndex(2);  // b.png 末帧
    ASSERT_EQ(ctrl->currentIndex(), 1);
    ASSERT_EQ(ctrl->currentFrameIndex(), 2);

    // Act
    const bool ret = ctrl->nextImage();

    // Assert：末帧时帧内不可前进，落到下一张图、帧归 0
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->currentIndex(), 2);
    EXPECT_EQ(ctrl->currentFrameIndex(), 0);  // branch: curIndex < rowCount-1
    EXPECT_EQ(ctrl->currentSource(), urls.at(2));
}

TEST_F(GlobalControlTest, PreviousImage_FirstImage_ReturnsFalse)
{
    // Arrange：三张图打开首张
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    const QUrl urlB = makeImageFile(QStringLiteral("b.png"));
    QStringList paths{urlA.toString(), urlB.toString()};  // file:// 形态与模型存储一致
    ctrl->setImageFiles(paths, paths.at(0));
    ASSERT_FALSE(ctrl->hasPreviousImage());

    // Act
    const bool ret = ctrl->previousImage();

    // Assert
    EXPECT_FALSE(ret);  // branch: hasPreviousImage() == false
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(ctrl->currentSource(), urlA);
}

TEST_F(GlobalControlTest, PreviousImage_MiddleImage_MovesToPreviousIndex)
{
    // Arrange：三张图打开中间张（普通单帧图）
    const QList<QUrl> urls = loadThreeImages();

    // Act
    const bool ret = ctrl->previousImage();

    // Assert：INT_MAX 帧索引被 qBound 钳到 0（前一张未知类型时从尾部开始的约定）
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(ctrl->currentFrameIndex(), 0);  // branch: curIndex > 0
    EXPECT_EQ(ctrl->currentSource(), urls.at(0));
}

TEST_F(GlobalControlTest, PreviousImage_EmptyModel_ReturnsFalse)
{
    // Arrange（空模型）
    ASSERT_EQ(ctrl->imageCount(), 0);

    // Act
    const bool ret = ctrl->previousImage();

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(ctrl->imageCount(), 0);
    EXPECT_EQ(ctrl->currentIndex(), 0);
}

TEST_F(GlobalControlTest, PreviousImage_MultiImage_RetreatsFrameBeforeIndex)
{
    // Arrange：多帧图（frameCount=3）停在末帧
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    stub.set_lamda(VADDR(ImageInfo, type),
                   [](ImageInfo *) -> int { return static_cast<int>(Types::MultiImage); });
    const QList<QUrl> urls = loadThreeImages();
    ctrl->setCurrentFrameIndex(2);
    ASSERT_EQ(ctrl->currentIndex(), 1);
    ASSERT_EQ(ctrl->currentFrameIndex(), 2);

    // Act
    const bool ret = ctrl->previousImage();

    // Assert：帧内后退优先，索引不变
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->currentIndex(), 1);
    EXPECT_EQ(ctrl->currentFrameIndex(), 1);  // branch: MultiImage && curFrameIndex > 0
    EXPECT_EQ(ctrl->currentSource(), urls.at(1));
}

TEST_F(GlobalControlTest, PreviousImage_MultiImageFirstFrame_MovesToPreviousImage)
{
    // Arrange：多帧图（frameCount=3）停在首帧
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    stub.set_lamda(VADDR(ImageInfo, type),
                   [](ImageInfo *) -> int { return static_cast<int>(Types::MultiImage); });
    const QList<QUrl> urls = loadThreeImages();
    ASSERT_EQ(ctrl->currentIndex(), 1);
    ASSERT_EQ(ctrl->currentFrameIndex(), 0);

    // Act
    const bool ret = ctrl->previousImage();

    // Assert：首帧时帧内不可后退，落到前一张图、INT_MAX 钳到末帧 2
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(ctrl->currentFrameIndex(), 2);  // branch: curIndex > 0（INT_MAX → frameCount-1）
    EXPECT_EQ(ctrl->currentSource(), urls.at(0));
}

// ── hasNextImage / hasPreviousImage / imageCount ──

TEST_F(GlobalControlTest, HasNextImage_EmptyModel_ReturnsFalse)
{
    // Arrange（空模型）
    ASSERT_EQ(ctrl->imageCount(), 0);

    // Act
    const bool hasNext = ctrl->hasNextImage();

    // Assert
    EXPECT_FALSE(hasNext);
    EXPECT_EQ(ctrl->imageCount(), 0);
}

TEST_F(GlobalControlTest, HasNextImage_MiddleImage_ReturnsTrue)
{
    // Arrange
    loadThreeImages();

    // Act
    const bool hasNext = ctrl->hasNextImage();

    // Assert
    EXPECT_TRUE(hasNext);
    EXPECT_EQ(ctrl->currentIndex(), 1);
}

TEST_F(GlobalControlTest, HasNextImage_LastImage_ReturnsFalse)
{
    // Arrange
    loadThreeImages();
    ctrl->setCurrentIndex(2);

    // Act
    const bool hasNext = ctrl->hasNextImage();

    // Assert
    EXPECT_FALSE(hasNext);
    EXPECT_EQ(ctrl->currentIndex(), 2);
}

TEST_F(GlobalControlTest, HasPreviousImage_FirstImage_ReturnsFalse)
{
    // Arrange：两张图打开首张
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    const QUrl urlB = makeImageFile(QStringLiteral("b.png"));
    QStringList paths{urlA.toString(), urlB.toString()};  // file:// 形态与模型存储一致
    ctrl->setImageFiles(paths, paths.at(0));

    // Act
    const bool hasPrev = ctrl->hasPreviousImage();

    // Assert
    EXPECT_FALSE(hasPrev);
    EXPECT_EQ(ctrl->currentIndex(), 0);
}

TEST_F(GlobalControlTest, HasPreviousImage_MiddleImage_ReturnsTrue)
{
    // Arrange
    loadThreeImages();

    // Act
    const bool hasPrev = ctrl->hasPreviousImage();

    // Assert
    EXPECT_TRUE(hasPrev);
    EXPECT_EQ(ctrl->currentIndex(), 1);
}

TEST_F(GlobalControlTest, HasPreviousImage_EmptyModel_ReturnsFalse)
{
    // Arrange（空模型）
    ASSERT_EQ(ctrl->imageCount(), 0);

    // Act
    const bool hasPrev = ctrl->hasPreviousImage();

    // Assert
    EXPECT_FALSE(hasPrev);
    EXPECT_EQ(ctrl->imageCount(), 0);
}

TEST_F(GlobalControlTest, ImageCount_EmptyModel_ReturnsZero)
{
    // Arrange（空模型）
    ASSERT_EQ(ctrl->imageCount(), 0);

    // Act
    const int count = ctrl->imageCount();

    // Assert
    EXPECT_EQ(count, 0);
    EXPECT_NE(ctrl->globalModel(), nullptr);
}

TEST_F(GlobalControlTest, ImageCount_AfterSetImageFiles_ReturnsListSize)
{
    // Arrange
    loadThreeImages();

    // Act
    const int count = ctrl->imageCount();

    // Assert
    EXPECT_EQ(count, 3);
    EXPECT_EQ(ctrl->currentIndex(), 1);
}

// ── removeImage（high）──

TEST_F(GlobalControlTest, RemoveImage_CurrentMiddleImage_ShiftsToFollowingImage)
{
    // Arrange：三张图打开中间张 b.png
    const QList<QUrl> urls = loadThreeImages();
    ASSERT_EQ(ctrl->currentIndex(), 1);
    QSignalSpy spyCount(ctrl, &GlobalControl::imageCountChanged);
    QSignalSpy spyNext(ctrl, &GlobalControl::hasNextImageChanged);

    // Act：删除当前图 b.png
    ctrl->removeImage(urls.at(1));

    // Assert：删除后索引不变并指向原下一张 c.png
    EXPECT_EQ(ctrl->imageCount(), 2);
    EXPECT_EQ(ctrl->currentIndex(), 1);
    EXPECT_EQ(ctrl->currentSource(), urls.at(2));  // branch: !atEnd → 读取 index(curIndex)
    EXPECT_EQ(spyCount.count(), 1);
    EXPECT_EQ(spyNext.count(), 1);  // 末尾位置 → hasNext 翻转为 false
}

TEST_F(GlobalControlTest, RemoveImage_CurrentLastImage_MovesToPreviousImage)
{
    // Arrange：三张图打开末张 c.png
    const QList<QUrl> urls = loadThreeImages();
    ctrl->setCurrentIndex(2);
    ASSERT_EQ(ctrl->currentIndex(), 2);
    QSignalSpy spyCount(ctrl, &GlobalControl::imageCountChanged);

    // Act：删除尾部当前图
    ctrl->removeImage(urls.at(2));

    // Assert：退到前一张 b.png
    EXPECT_EQ(ctrl->imageCount(), 2);
    EXPECT_EQ(ctrl->currentIndex(), 1);  // branch: atEnd && rowCount != 0 → curIndex-1
    EXPECT_EQ(ctrl->currentSource(), urls.at(1));
    EXPECT_TRUE(ctrl->hasPreviousImage());
    EXPECT_FALSE(ctrl->hasNextImage());
    EXPECT_EQ(spyCount.count(), 1);
}

TEST_F(GlobalControlTest, RemoveImage_LastRemainingImage_ClearsCurrentState)
{
    // Arrange：仅一张图且为当前图
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    QStringList paths{urlA.toString()};  // file:// 形态与模型存储一致
    ctrl->setImageFiles(paths, paths.at(0));
    QSignalSpy spyCount(ctrl, &GlobalControl::imageCountChanged);
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);

    // Act：删除最后一张
    ctrl->removeImage(urlA);

    // Assert：模型清空，导航标志复位（atEnd 且 rowCount==0 落入 else 分支）；
    // 修复语义：currentImage 同步清空，currentSource() 不再返回已删除路径
    EXPECT_EQ(ctrl->imageCount(), 0);
    EXPECT_FALSE(ctrl->hasNextImage());
    EXPECT_FALSE(ctrl->hasPreviousImage());
    EXPECT_TRUE(ctrl->currentSource().isEmpty());
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(spyCount.count(), 1);
    EXPECT_EQ(spySource.count(), 1);
}

TEST_F(GlobalControlTest, RemoveImage_WithPendingRotation_SubmitsRotationBeforeRemoval)
{
    // Arrange：挂起 90 度旋转后删除当前图
    const QList<QUrl> urls = loadThreeImages();
    ctrl->setCurrentRotation(90);
    ASSERT_EQ(rotateFileCalls, 1);
    QSignalSpy spyRotate(ctrl, &GlobalControl::requestRotateImage);

    // Act
    ctrl->removeImage(urls.at(1));

    // Assert：先经 setCurrentRotation(0) 复位旋转（追加一次 0 度复位请求）再执行删除；
    // 挂起的 90 度不会生成 requestRotateImage（文件即将删除，无需落盘）
    EXPECT_EQ(ctrl->currentRotation(), 0);  // branch: 0 != currentRotation()
    EXPECT_EQ(rotateFileCalls, 2);
    EXPECT_EQ(spyRotate.count(), 0);
    EXPECT_EQ(ctrl->imageCount(), 2);
}

TEST_F(GlobalControlTest, RemoveImage_UnknownImage_KeepsCurrentStateIntact)
{
    // Arrange：两张图打开首张 a.png；待删除的 URL 不在模型中
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    const QUrl urlB = makeImageFile(QStringLiteral("b.png"));
    QStringList paths{urlA.toString(), urlB.toString()};  // file:// 形态与模型存储一致
    ctrl->setImageFiles(paths, paths.at(0));
    const QUrl unknown = QUrl::fromLocalFile(tempDir.filePath(QStringLiteral("ghost.png")));
    QSignalSpy spyCount(ctrl, &GlobalControl::imageCountChanged);
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);

    // Act
    ctrl->removeImage(unknown);

    // Assert：无匹配项被移除，当前图状态保持（强异常安全）
    EXPECT_EQ(ctrl->imageCount(), 2);
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(ctrl->currentSource(), urlA);
    EXPECT_EQ(spyCount.count(), 0);   // 修复语义：图片未从模型删除，不发 imageCountChanged
    EXPECT_EQ(spySource.count(), 1);  // 注：!atEnd 分支仍无条件补发双信号（D1 未修复项）
}

// ── renameImage ──

TEST_F(GlobalControlTest, RenameImage_CurrentImage_UpdatesSourceAndReloads)
{
    // Arrange：两张图打开首张 a.png；隔离 reloadData 的缓存异步加载
    stub.set_lamda(VADDR(ImageInfo, reloadData), [this](ImageInfo *) { ++reloadCalls; });
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    const QUrl urlB = makeImageFile(QStringLiteral("b.png"));
    QStringList paths{urlA.toString(), urlB.toString()};  // file:// 形态与模型存储一致
    ctrl->setImageFiles(paths, paths.at(0));
    const QUrl urlA2 = QUrl::fromLocalFile(tempDir.filePath(QStringLiteral("a2.png")));
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);

    // Act：重命名当前图
    ctrl->renameImage(urlA, urlA2);

    // Assert：模型与当前源全部指向新名，且强制 reload 一次
    EXPECT_EQ(ctrl->currentSource(), urlA2);  // branch: oldName == currentImage.source()
    EXPECT_EQ(ctrl->globalModel()->indexForImagePath(urlA2), 0);
    EXPECT_EQ(ctrl->globalModel()->indexForImagePath(urlA), -1);
    EXPECT_EQ(ctrl->imageCount(), 2);
    EXPECT_EQ(reloadCalls, 1);
    EXPECT_EQ(spySource.count(), 1);
}

TEST_F(GlobalControlTest, RenameImage_UnknownImage_LeavesStateUnchanged)
{
    // Arrange
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    QStringList paths{urlA.toString()};  // file:// 形态与模型存储一致
    ctrl->setImageFiles(paths, paths.at(0));
    const QUrl unknown = QUrl::fromLocalFile(tempDir.filePath(QStringLiteral("ghost.png")));
    const QUrl renamed = QUrl::fromLocalFile(tempDir.filePath(QStringLiteral("ghost2.png")));
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);

    // Act：重命名不在模型中的图片
    ctrl->renameImage(unknown, renamed);

    // Assert：indexForImagePath 返回 -1，全部状态不变
    EXPECT_EQ(ctrl->currentSource(), urlA);
    EXPECT_EQ(ctrl->imageCount(), 1);
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(spySource.count(), 0);
}

// ── setCurrentFrameIndex / setCurrentIndex ──

TEST_F(GlobalControlTest, SetCurrentFrameIndex_ValidIndex_UpdatesAndEmits)
{
    // Arrange：多帧图（frameCount=3）
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    loadThreeImages();
    QSignalSpy spyFrame(ctrl, &GlobalControl::currentFrameIndexChanged);
    QSignalSpy spyIndex(ctrl, &GlobalControl::currentIndexChanged);

    // Act
    ctrl->setCurrentFrameIndex(2);

    // Assert
    EXPECT_EQ(ctrl->currentFrameIndex(), 2);
    EXPECT_EQ(spyFrame.count(), 1);
    EXPECT_EQ(spyIndex.count(), 0);  // 仅帧变化，索引信号不应发出
}

TEST_F(GlobalControlTest, SetCurrentFrameIndex_OutOfRange_ClampsToLastFrame)
{
    // Arrange：多帧图（frameCount=3）
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    loadThreeImages();

    // Act：越界帧索引（上边界外 / 下边界外）
    ctrl->setCurrentFrameIndex(99);
    const int high = ctrl->currentFrameIndex();
    ctrl->setCurrentFrameIndex(-1);
    const int low = ctrl->currentFrameIndex();

    // Assert：qBound 钳制到 [0, frameCount-1]
    EXPECT_EQ(high, 2);
    EXPECT_EQ(low, 0);
}

TEST_F(GlobalControlTest, SetCurrentIndex_ValidIndex_UpdatesCurrentSource)
{
    // Arrange
    const QList<QUrl> urls = loadThreeImages();
    QSignalSpy spyIndex(ctrl, &GlobalControl::currentIndexChanged);
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);

    // Act
    ctrl->setCurrentIndex(2);

    // Assert
    EXPECT_EQ(ctrl->currentIndex(), 2);
    EXPECT_EQ(ctrl->currentSource(), urls.at(2));
    EXPECT_EQ(spyIndex.count(), 1);
    EXPECT_EQ(spySource.count(), 1);
}

TEST_F(GlobalControlTest, SetCurrentIndex_OutOfRangeIndex_ClampsToValidRange)
{
    // Arrange
    loadThreeImages();

    // Act：上越界 / 下越界各设一次
    ctrl->setCurrentIndex(99);
    const int high = ctrl->currentIndex();
    ctrl->setCurrentIndex(-1);
    const int low = ctrl->currentIndex();

    // Assert：修复语义（原 D5）——validIndex 落库，越界索引被钳制到 [0, count-1]
    EXPECT_EQ(high, 2);   // branch: this->curIndex = validIndex（钳制值）
    EXPECT_EQ(low, 0);
    EXPECT_EQ(ctrl->imageCount(), 3);
}

// ── setCurrentRotation ──

TEST_F(GlobalControlTest, SetCurrentRotation_NinetyDegrees_UpdatesStateEmitsAndSwapsSize)
{
    // Arrange：记录 swap 分支调用次数
    stub.set_lamda(VADDR(ImageInfo, swapWidthAndHeight), [this](ImageInfo *) { ++swapCalls; });
    loadThreeImages();
    QSignalSpy spyBegin(ctrl, &GlobalControl::changeRotationCacheBegin);
    QSignalSpy spyCache(ctrl, &GlobalControl::requestRotateCacheImage);
    QSignalSpy spyRotation(ctrl, &GlobalControl::currentRotationChanged);

    // Act
    ctrl->setCurrentRotation(90);

    // Assert：0→90 相差 90 度（%180 非 0）→ 需要交换宽高
    EXPECT_EQ(ctrl->currentRotation(), 90);
    EXPECT_EQ(swapCalls, 1);  // branch: needSwap
    EXPECT_EQ(spyBegin.count(), 1);
    EXPECT_EQ(spyCache.count(), 1);
    EXPECT_EQ(spyRotation.count(), 1);
    EXPECT_EQ(rotateFileCalls, 1);
}

TEST_F(GlobalControlTest, SetCurrentRotation_SameAngle_SkipsUpdate)
{
    // Arrange
    loadThreeImages();
    ctrl->setCurrentRotation(90);
    ASSERT_EQ(rotateFileCalls, 1);
    QSignalSpy spyRotation(ctrl, &GlobalControl::currentRotationChanged);
    QSignalSpy spyBegin(ctrl, &GlobalControl::changeRotationCacheBegin);

    // Act：重复设置相同角度
    ctrl->setCurrentRotation(90);

    // Assert：角度未变分支（else），不做任何处理
    EXPECT_EQ(ctrl->currentRotation(), 90);
    EXPECT_EQ(spyRotation.count(), 0);
    EXPECT_EQ(spyBegin.count(), 0);
    EXPECT_EQ(rotateFileCalls, 1);
}

TEST_F(GlobalControlTest, SetCurrentRotation_ResetToZero_EmitsWithoutSwap)
{
    // Arrange
    stub.set_lamda(VADDR(ImageInfo, swapWidthAndHeight), [this](ImageInfo *) { ++swapCalls; });
    loadThreeImages();
    ctrl->setCurrentRotation(90);
    ASSERT_EQ(swapCalls, 1);
    QSignalSpy spyRotation(ctrl, &GlobalControl::currentRotationChanged);

    // Act：复位到 0（angle 为 0 时特殊处理，不交换宽高）
    ctrl->setCurrentRotation(0);

    // Assert
    EXPECT_EQ(ctrl->currentRotation(), 0);
    EXPECT_EQ(swapCalls, 1);  // branch: needSwap 为假（angle == 0）
    EXPECT_EQ(spyRotation.count(), 1);
    EXPECT_EQ(rotateFileCalls, 2);  // 复位同样会下发一次 0 度旋转请求
}

TEST_F(GlobalControlTest, SetCurrentRotation_AngleNotMultipleOfNinety_StillApplied)
{
    // Arrange：45 度非 90 倍数（仅告警不拒绝）
    stub.set_lamda(VADDR(ImageInfo, swapWidthAndHeight), [this](ImageInfo *) { ++swapCalls; });
    loadThreeImages();

    // Act
    ctrl->setCurrentRotation(45);

    // Assert：B2 告警分支不中断流程，角度照常生效
    EXPECT_EQ(ctrl->currentRotation(), 45);  // branch: 0 != angle % 90 → 仅 qCWarning
    EXPECT_EQ(swapCalls, 1);                 // (45-0)%180 = 45 → 仍需交换宽高
    EXPECT_EQ(rotateFileCalls, 1);
}

// ── setCurrentSource ──

TEST_F(GlobalControlTest, SetCurrentSource_KnownImage_UpdatesIndexAndFrame)
{
    // Arrange
    const QList<QUrl> urls = loadThreeImages();
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);
    QSignalSpy spyIndex(ctrl, &GlobalControl::currentIndexChanged);

    // Act：切换到已知图片 c.png
    ctrl->setCurrentSource(urls.at(2));

    // Assert
    EXPECT_EQ(ctrl->currentIndex(), 2);  // branch: -1 != index
    EXPECT_EQ(ctrl->currentFrameIndex(), 0);
    EXPECT_EQ(ctrl->currentSource(), urls.at(2));
    EXPECT_EQ(spySource.count(), 1);
    EXPECT_EQ(spyIndex.count(), 1);
}

TEST_F(GlobalControlTest, SetCurrentSource_SameImage_SkipsUpdate)
{
    // Arrange：当前已是 b.png
    const QList<QUrl> urls = loadThreeImages();
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);
    QSignalSpy spyIndex(ctrl, &GlobalControl::currentIndexChanged);

    // Act：重复设置当前源
    ctrl->setCurrentSource(urls.at(1));

    // Assert：相同源早退，不触发任何信号
    EXPECT_EQ(ctrl->currentIndex(), 1);  // branch: currentImage.source() == source → return
    EXPECT_EQ(spySource.count(), 0);
    EXPECT_EQ(spyIndex.count(), 0);
}

TEST_F(GlobalControlTest, SetCurrentSource_UnknownImage_KeepsIndex)
{
    // Arrange
    loadThreeImages();
    const QUrl unknown = QUrl::fromLocalFile(tempDir.filePath(QStringLiteral("ghost.png")));
    QSignalSpy spyIndex(ctrl, &GlobalControl::currentIndexChanged);

    // Act：设置不在模型中的源
    ctrl->setCurrentSource(unknown);

    // Assert：indexForImagePath == -1，状态保持
    EXPECT_EQ(ctrl->currentIndex(), 1);  // branch: -1 == index → 不更新
    EXPECT_EQ(ctrl->imageCount(), 3);
    EXPECT_EQ(spyIndex.count(), 0);
}

// ── setImageFiles ──

TEST_F(GlobalControlTest, SetImageFiles_EmptyList_ReturnsFalse)
{
    // Arrange
    QSignalSpy spyCount(ctrl, &GlobalControl::imageCountChanged);

    // Act
    const bool ret = ctrl->setImageFiles(QStringList(), QStringLiteral("a.png"));

    // Assert
    EXPECT_FALSE(ret);  // branch: filePaths.isEmpty()
    EXPECT_EQ(ctrl->imageCount(), 0);
    EXPECT_EQ(spyCount.count(), 0);
}

TEST_F(GlobalControlTest, SetImageFiles_OpenFileMissingFromList_ReturnsFalse)
{
    // Arrange：openFile 不在列表中
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    QStringList paths{urlA.toString()};  // file:// 形态与模型存储一致

    // Act
    const bool ret = ctrl->setImageFiles(paths, tempDir.filePath(QStringLiteral("zz.png")));

    // Assert
    EXPECT_FALSE(ret);  // branch: filePaths.indexOf(openFile) == -1
    EXPECT_EQ(ctrl->imageCount(), 0);
    EXPECT_TRUE(ctrl->currentSource().isEmpty());
}

TEST_F(GlobalControlTest, SetImageFiles_ValidList_ReturnsTrueAndOpensTarget)
{
    // Arrange：全新对象打开首张 a.png（currentImage 尚为空 → 必走 setSource 分支）
    const QList<QUrl> urls = {makeImageFile(QStringLiteral("a.png")),
                              makeImageFile(QStringLiteral("b.png")),
                              makeImageFile(QStringLiteral("c.png"))};
    QStringList paths;
    for (const QUrl &u : urls)
        paths << u.toString();
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);
    QSignalSpy spyCount(ctrl, &GlobalControl::imageCountChanged);
    QSignalSpy spyNext(ctrl, &GlobalControl::hasNextImageChanged);

    // Act：openFile 为列表首项
    const bool ret = ctrl->setImageFiles(paths, paths.at(0));

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->imageCount(), 3);
    EXPECT_EQ(ctrl->currentIndex(), 0);
    EXPECT_EQ(ctrl->currentSource(), urls.at(0));  // branch: currentImage.source() != currentSource → setSource
    EXPECT_EQ(spySource.count(), 1);               // 修复语义：源实际变更后才发 currentSourceChanged
    EXPECT_EQ(spyCount.count(), 1);
    EXPECT_EQ(spyNext.count(), 1);                 // 首张 → next 翻转为 true
}

TEST_F(GlobalControlTest, SetImageFiles_RepeatedCall_KeepsStateConsistent)
{
    // Arrange：同一列表连续设置两次
    const QList<QUrl> urls = loadThreeImages();
    QStringList paths;
    for (const QUrl &u : urls)
        paths << u.toString();
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);
    QSignalSpy spyIndex(ctrl, &GlobalControl::currentIndexChanged);
    QSignalSpy spyCount(ctrl, &GlobalControl::imageCountChanged);

    // Act：第二次设置（currentImage.source() 已等于目标 → B3 为假）
    const bool ret = ctrl->setImageFiles(paths, paths.at(1));

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(ctrl->currentIndex(), 1);
    EXPECT_EQ(ctrl->currentSource(), urls.at(1));
    EXPECT_EQ(ctrl->imageCount(), 3);
    EXPECT_EQ(spyIndex.count(), 0);   // 索引未变，不发索引信号
    EXPECT_EQ(spySource.count(), 0);  // 修复语义：源未变，不发源信号
    EXPECT_EQ(spyCount.count(), 0);   // 修复语义：数量未变（3→3），不发数量信号
}

// ── setIndexAndFrameIndex ──

TEST_F(GlobalControlTest, SetIndexAndFrameIndex_NewIndex_UpdatesSourceAndEmitsBoth)
{
    // Arrange
    const QList<QUrl> urls = loadThreeImages();
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);
    QSignalSpy spyIndex(ctrl, &GlobalControl::currentIndexChanged);

    // Act
    ctrl->setIndexAndFrameIndex(2, 0);

    // Assert
    EXPECT_EQ(ctrl->currentIndex(), 2);
    EXPECT_EQ(ctrl->currentFrameIndex(), 0);
    EXPECT_EQ(ctrl->currentSource(), urls.at(2));
    EXPECT_EQ(spySource.count(), 1);
    EXPECT_EQ(spyIndex.count(), 1);
}

TEST_F(GlobalControlTest, SetIndexAndFrameIndex_SameValues_EmitsNothing)
{
    // Arrange
    loadThreeImages();  // (1, 0)
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);
    QSignalSpy spyIndex(ctrl, &GlobalControl::currentIndexChanged);
    QSignalSpy spyFrame(ctrl, &GlobalControl::currentFrameIndexChanged);

    // Act：重复设置相同值
    ctrl->setIndexAndFrameIndex(1, 0);

    // Assert：两个 if 条件均为假，无信号
    EXPECT_EQ(ctrl->currentIndex(), 1);
    EXPECT_EQ(ctrl->currentFrameIndex(), 0);
    EXPECT_EQ(spySource.count(), 0);
    EXPECT_EQ(spyIndex.count(), 0);
    EXPECT_EQ(spyFrame.count(), 0);
}

TEST_F(GlobalControlTest, SetIndexAndFrameIndex_OutOfRange_BothIndexAndFrameClamped)
{
    // Arrange：多帧图（frameCount=3）
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    loadThreeImages();

    // Act：双重越界（索引与帧）
    ctrl->setIndexAndFrameIndex(99, 99);
    const int highIndex = ctrl->currentIndex();
    const int highFrame = ctrl->currentFrameIndex();
    ctrl->setIndexAndFrameIndex(-1, -1);
    const int lowIndex = ctrl->currentIndex();
    const int lowFrame = ctrl->currentFrameIndex();

    // Assert：修复语义（原 D5）——索引与帧索引均经 qBound 钳制后落库
    EXPECT_EQ(highIndex, 2);  // branch: this->curIndex = validIndex（钳制值）
    EXPECT_EQ(highFrame, 2);  // qBound(0, frameIndex, frameCount-1) 钳制
    EXPECT_EQ(lowIndex, 0);   // 同上，索引下界钳制
    EXPECT_EQ(lowFrame, 0);   // 钳制生效
}

TEST_F(GlobalControlTest, SetIndexAndFrameIndex_FrameOnlyChange_EmitsOnlyFrameSignal)
{
    // Arrange：多帧图（frameCount=3）
    stub.set_lamda(VADDR(ImageInfo, frameCount), [](ImageInfo *) -> int { return 3; });
    loadThreeImages();
    QSignalSpy spySource(ctrl, &GlobalControl::currentSourceChanged);
    QSignalSpy spyIndex(ctrl, &GlobalControl::currentIndexChanged);
    QSignalSpy spyFrame(ctrl, &GlobalControl::currentFrameIndexChanged);

    // Act：索引不变仅改帧
    ctrl->setIndexAndFrameIndex(1, 2);

    // Assert
    EXPECT_EQ(ctrl->currentFrameIndex(), 2);
    EXPECT_EQ(ctrl->currentIndex(), 1);
    EXPECT_EQ(spyFrame.count(), 1);
    EXPECT_EQ(spyIndex.count(), 0);
    EXPECT_EQ(spySource.count(), 0);
}

// ── submitImageChangeImmediately ──

TEST_F(GlobalControlTest, SubmitImageChangeImmediately_ZeroRotation_ReturnsWithoutRequest)
{
    // Arrange（未设置过旋转）
    loadThreeImages();
    QSignalSpy spyRotate(ctrl, &GlobalControl::requestRotateImage);

    // Act
    ctrl->submitImageChangeImmediately();

    // Assert：0 度直接早退，不下发旋转请求
    EXPECT_EQ(spyRotate.count(), 0);  // branch: 0 == rotation
    EXPECT_EQ(ctrl->currentRotation(), 0);
    EXPECT_EQ(rotateFileCalls, 0);
}

TEST_F(GlobalControlTest, SubmitImageChangeImmediately_NinetyRotation_EmitsRequestAndResets)
{
    // Arrange
    const QList<QUrl> urls = loadThreeImages();
    ctrl->setCurrentRotation(90);
    QSignalSpy spyRotate(ctrl, &GlobalControl::requestRotateImage);

    // Act
    ctrl->submitImageChangeImmediately();

    // Assert：发出 (路径, 90) 旋转请求并复位角度
    EXPECT_EQ(spyRotate.count(), 1);  // branch: 0 != rotation % 360
    EXPECT_EQ(spyRotate.at(0).at(0).toString(), urls.at(1).toLocalFile());
    EXPECT_EQ(spyRotate.at(0).at(1).toInt(), 90);
    EXPECT_EQ(ctrl->currentRotation(), 0);
}

TEST_F(GlobalControlTest, SubmitImageChangeImmediately_FullTurnRotation_SkipsRequestButResets)
{
    // Arrange：累计 360 度（90 倍数合法，%360 == 0）
    loadThreeImages();
    ctrl->setCurrentRotation(360);
    ASSERT_EQ(ctrl->currentRotation(), 360);
    QSignalSpy spyRotate(ctrl, &GlobalControl::requestRotateImage);

    // Act
    ctrl->submitImageChangeImmediately();

    // Assert：整圈旋转无需落盘，仅复位角度
    EXPECT_EQ(spyRotate.count(), 0);  // branch: 0 == rotation % 360
    EXPECT_EQ(ctrl->currentRotation(), 0);
}

// ── timerEvent ──

TEST_F(GlobalControlTest, TimerEvent_SubmitTimerExpired_SubmitsPendingRotation)
{
    // Arrange：挂起 90 度旋转（submitTimer 由 setCurrentRotation 启动）
    const QList<QUrl> urls = loadThreeImages();
    ctrl->setCurrentRotation(90);
    ASSERT_TRUE(ctrl->submitTimer.isActive());
    QSignalSpy spyRotate(ctrl, &GlobalControl::requestRotateImage);

    // Act：伪造 submitTimer 超时事件
    QTimerEvent event(ctrl->submitTimer.timerId());
    ctrl->timerEvent(&event);

    // Assert
    EXPECT_EQ(spyRotate.count(), 1);  // branch: submitTimer.timerId() == event->timerId()
    EXPECT_EQ(spyRotate.at(0).at(1).toInt(), 90);
    EXPECT_EQ(ctrl->currentRotation(), 0);
    // 真实行为：submit 内部 setCurrentRotation(0) 会再次 submitTimer.start()，
    // 提交后定时器被重新武装（下次到期时旋转为 0 → 早退），并非停止态
    EXPECT_TRUE(ctrl->submitTimer.isActive());
}

TEST_F(GlobalControlTest, TimerEvent_SwitchCheckTimerExpired_RechecksNavigationFlags)
{
    // Arrange：通过 infoChanged 信号启动 switchCheckTimer（构造函数中的防抖连接）
    const QUrl urlA = makeImageFile(QStringLiteral("a.png"));
    QStringList paths{urlA.toString()};  // file:// 形态与模型存储一致
    ctrl->setImageFiles(paths, paths.at(0));
    Q_EMIT ctrl->currentImage.infoChanged();
    ASSERT_TRUE(ctrl->switchCheckTimer.isActive());

    // Act：伪造 switchCheckTimer 超时事件
    QTimerEvent event(ctrl->switchCheckTimer.timerId());
    ctrl->timerEvent(&event);

    // Assert：单张普通图，两个方向均不可用
    EXPECT_FALSE(ctrl->switchCheckTimer.isActive());  // branch: switchCheckTimer 命中
    EXPECT_FALSE(ctrl->hasNextImage());
    EXPECT_FALSE(ctrl->hasPreviousImage());
    EXPECT_EQ(ctrl->imageCount(), 1);
}

TEST_F(GlobalControlTest, TimerEvent_ViewModelSyncTimerExpired_SyncsViewModel)
{
    // Arrange：setImageFiles 内部的 setIndexAndFrameIndex 已启动 viewModelSyncTimer
    loadThreeImages();
    ASSERT_TRUE(ctrl->viewModelSyncTimer.isActive());

    // Act：伪造 viewModelSyncTimer 超时事件（真实代理模型同步，同索引 → Current 无动作）
    QTimerEvent event(ctrl->viewModelSyncTimer.timerId());
    ctrl->timerEvent(&event);

    // Assert
    EXPECT_FALSE(ctrl->viewModelSyncTimer.isActive());  // branch: viewModelSyncTimer 命中
    EXPECT_EQ(ctrl->currentIndex(), 1);
    EXPECT_NE(ctrl->viewModel(), nullptr);
}

TEST_F(GlobalControlTest, TimerEvent_UnknownTimerId_IgnoresEventSafely)
{
    // Arrange：挂起旋转并保持三个定时器状态
    loadThreeImages();
    ctrl->setCurrentRotation(90);
    ASSERT_TRUE(ctrl->submitTimer.isActive());
    QSignalSpy spyRotate(ctrl, &GlobalControl::requestRotateImage);

    // Act：未知 timerId 的事件应被忽略
    QTimerEvent event(-1);
    ctrl->timerEvent(&event);

    // Assert：三个分支均未命中，状态原样
    EXPECT_EQ(spyRotate.count(), 0);
    EXPECT_EQ(ctrl->currentRotation(), 90);
    EXPECT_TRUE(ctrl->submitTimer.isActive());
}

// ── forceExit / globalModel / viewModel ──

TEST_F(GlobalControlTest, ForceExit_InChildProcess_ExitsWithCodeZero)
{
    // Arrange：forceExit 依次调用 QApplication::exit(0) 与 _Exit(0)。
    // 1) ::_Exit 的函数入口改写（stub_ext）在 ASan/UBsan 下会写坏 libc 代码段
    //    （addrof 解析到未对齐地址触发 SEGV），不可用；
    // 2) fork 后直接执行真实 QApplication::exit 在重负载前置用例后可能因 fork 时刻
    //    Qt 内部锁被占用而死锁，故子进程内仅对 QApplication::exit 打 stub（已验证安全），
    //    _Exit(0) 保持真实路径以验证退出码；
    // 3) _Exit 跳过 exit 处理不会刷 gcov 计数（FNDA 恒为 0），故在 QApplication::exit
    //    桩内先 __gcov_dump() 落盘——此时 forceExit 已执行到桩调用点，计数完整。
    ASSERT_NE(ctrl, nullptr);
    EXPECT_EQ(ctrl->currentRotation(), 0);

    // Act
    const pid_t pid = fork();
    ASSERT_GE(pid, 0);
    if (pid == 0) {
        // 子进程：仅隔离会触碰 Qt 内部锁的 QApplication::exit，_Exit 走真实路径；
        // 桩内先转储 gcov 计数再返回，使 forceExit 的覆盖数据在 _Exit 前落盘
        stub_ext::StubExt childStub;
        childStub.set_lamda(static_cast<void (*)(int)>(&QApplication::exit),
                            [](int) {
                                UT_GCOV_DUMP();
                            });
        ctrl->forceExit();  // 子进程不应返回：_Exit(0) 直接终止（计数已在桩内转储）
        _exit(1);           // 防御：意外返回时以非零码退出以便区分
    }
    // 父进程：看门狗等待（2 秒上限），任何意外情况都不允许挂死测试进程
    int status = 0;
    bool reaped = false;
    for (int i = 0; i < 200 && !reaped; ++i) {
        if (waitpid(pid, &status, WNOHANG) == pid) {
            reaped = true;
            break;
        }
        usleep(10000);
    }
    if (!reaped) {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
    }

    // Assert：子进程及时正常终止且退出码为 0（对应 _Exit(0)），父进程对象不受影响
    EXPECT_TRUE(reaped);
    EXPECT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);
    EXPECT_EQ(ctrl->currentRotation(), 0);
}

TEST_F(GlobalControlTest, GlobalModel_AfterConstruction_ReturnsEmptyModelInstance)
{
    // Arrange（SetUp 已构造）
    ASSERT_NE(ctrl, nullptr);

    // Act
    ImageSourceModel *model = ctrl->globalModel();

    // Assert
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(model->rowCount(), 0);
    EXPECT_EQ(ctrl->globalModel(), model);  // 稳定返回同一实例
}

TEST_F(GlobalControlTest, ViewModel_AfterConstruction_ReturnsValidProxyInstance)
{
    // Arrange（SetUp 已构造）
    ASSERT_NE(ctrl, nullptr);

    // Act
    PathViewProxyModel *proxy = ctrl->viewModel();

    // Assert
    EXPECT_NE(proxy, nullptr);
    EXPECT_EQ(proxy->rowCount(QModelIndex()), 0);  // 未 reset 前队列为空
    EXPECT_EQ(ctrl->viewModel(), proxy);
}

TEST_F(GlobalControlTest, ViewModel_AfterSetImageFiles_StillReturnsSameInstance)
{
    // Arrange
    loadThreeImages();

    // Act
    PathViewProxyModel *proxy = ctrl->viewModel();

    // Assert：模型数据重置不更换代理实例；resetModel 以固定容量（maxCount=5，
    // radius=2）循环填充代理队列，rowCount 恒为 5 而非源数据条数
    EXPECT_NE(proxy, nullptr);
    EXPECT_EQ(proxy->rowCount(QModelIndex()), 5);
    EXPECT_EQ(ctrl->viewModel(), proxy);
}

// ── 构造函数 rotateImageFinished 连接（补测：对 sender 单例直接 emit 驱动 ctor lambda）───
// 分支（来源：get_code_snippet GlobalControl ctor globalcontrol.cpp:38-44）：
// B1: path == currentImage.source().toLocalFile() → submitImageChangeImmediately()
// B2: 路径不匹配 → 仅记录日志，无副作用
// 映射： GlobalControl_RotateFinishedHook_MatchingPathSubmitsPendingRotation    → B1
//        GlobalControl_RotateFinishedHook_MismatchedPathKeepsPendingRotation    → B2

TEST_F(GlobalControlTest, GlobalControl_RotateFinishedHook_MatchingPathSubmitsPendingRotation)
{
    // Arrange：三张图打开 b.png 并挂起 90 度未提交旋转；lambda 已在构造函数中连接到
    // RotateImageHelper::instance() 的 rotateImageFinished 信号
    const QList<QUrl> urls = loadThreeImages();
    ctrl->setCurrentRotation(90);
    ASSERT_EQ(ctrl->currentRotation(), 90);
    QSignalSpy spyRotate(ctrl, &GlobalControl::requestRotateImage);

    // Act：对 sender 单例直接 emit，路径与当前图一致
    Q_EMIT RotateImageHelper::instance()->rotateImageFinished(urls.at(1).toLocalFile(), true);

    // Assert：B1 命中 → submitImageChangeImmediately 发出 (路径, 90) 请求并复位角度
    EXPECT_EQ(spyRotate.count(), 1);
    EXPECT_EQ(spyRotate.at(0).at(0).toString(), urls.at(1).toLocalFile());
    EXPECT_EQ(spyRotate.at(0).at(1).toInt(), 90);
    EXPECT_EQ(ctrl->currentRotation(), 0);
}

TEST_F(GlobalControlTest, GlobalControl_RotateFinishedHook_MismatchedPathKeepsPendingRotation)
{
    // Arrange：挂起 90 度旋转，准备与当前图不同的路径
    const QList<QUrl> urls = loadThreeImages();
    ctrl->setCurrentRotation(90);
    ASSERT_EQ(ctrl->currentRotation(), 90);
    QSignalSpy spyRotate(ctrl, &GlobalControl::requestRotateImage);
    const QString otherPath = tempDir.filePath(QStringLiteral("other.png"));

    // Act：emit 的路径与当前图不匹配
    Q_EMIT RotateImageHelper::instance()->rotateImageFinished(otherPath, true);

    // Assert：B2 —— 不提交，挂起旋转与当前源原样保留（强异常安全）
    EXPECT_EQ(spyRotate.count(), 0);
    EXPECT_EQ(ctrl->currentRotation(), 90);
    EXPECT_EQ(ctrl->currentSource(), urls.at(1));
}

// ═══════════════════ 源码缺陷清单（fix/scan-defects 分支修复状态）═══════════════════
// D1（未修复，仍标红）: removeImage 删除「非当前/不存在」的图片时仍走更新分支
//     证据：globalcontrol.cpp（!atEnd 分支无条件 Q_EMIT currentSourceChanged/
//     currentIndexChanged）；单图列表（curIndex==0）传入不存在的 URL 时 else-if 分支
//     读取 index(curIndex-1) = index(-1) 得空 QUrl，会把当前源清成 null。
//     固化用例：RemoveImage_UnknownImage_KeepsCurrentStateIntact（断言状态保持 +
//     源/索引双信号仍发出，数量信号已随 D3 修复不再发出）。
//
// D2（已修复）: removeImage 删除最后一张图后 currentImage 未清空
//     修复：else 分支 setSource("") + curIndex 归 0 + emit 双信号。
//     用例：RemoveImage_LastRemainingImage_ClearsCurrentState（currentSource 为空）。
//
// D3（已修复）: removeImage / setImageFiles 无条件发出 imageCountChanged
//     修复：均改为模型 count 前后对比，仅数量实际变化才发。
//     用例：RemoveImage_UnknownImage_KeepsCurrentStateIntact（数量未变 count==0）、
//     SetImageFiles_RepeatedCall_KeepsStateConsistent（3→3 不发）。
//
// D4（已修复）: setImageFiles 无条件发出 currentSourceChanged
//     修复：Q_EMIT 移入 currentImage.source() != currentSource 守卫内。
//     用例：SetImageFiles_RepeatedCall_KeepsStateConsistent（源未变 count==0）。
//
// D5（已修复）: setIndexAndFrameIndex 将原始 index 落库而非钳制后的 validIndex
//     修复：this->curIndex = validIndex，与帧索引 validFrameIndex 的做法一致。
//     用例：SetCurrentIndex_OutOfRangeIndex_ClampsToValidRange、
//     SetIndexAndFrameIndex_OutOfRange_BothIndexAndFrameClamped。
//
// 另（真实行为注记，未单列缺陷）：submitImageChangeImmediately 内部经 setCurrentRotation(0)
// 复位旋转时会重新 start(submitTimer)，提交完成后定时器处于重新武装状态而非停止态。
// 固化用例：TimerEvent_SubmitTimerExpired_SubmitsPendingRotation。
// ═══════════════════════════════════════════════════════════════════════════════
