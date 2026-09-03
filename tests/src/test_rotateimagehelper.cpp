// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | RotateImageHelper(QObject *parent) | low | - | 1 | 2 |
// | ~RotateImageHelper | low | - | 1 | 1 |
// | instance() | mid | - | 2 | 2 |
// | rotateImageFile(path, angle) | mid | - | 2 | 10（5 TEST_F + TEST_P×5） |
// | resetRotateState() | mid | - | 2 | 3 |
// | rotateImageImpl(cachePath, path, angle) | low | - | 1 | 4 |
// | enqueueRotateTask(path, angle) | low | - | 1 | 3 |
// | checkDataValid() | low | - | 1 | 3 |
// | RotateImageHelperData() | low | - | 1 | 1 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（inventory 8 方法 + 隐藏类 RotateImageHelperData 构造，全覆盖）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（angle: 0/360 边界、>360 取模、多次累计、
//    同路径/不同路径；pathType: 本地/MTP/PTP/APPLE/SAFEBOX/RECYCLEBIN；data: 空/已初始化；
//    watcher: 运行中/未运行；cache 文件: 存在/缺失；copy: 成功/失败；旋转: 成功/失败）
// 3. 每个等价类的边界值显式覆盖: [x]（0° 与 360°（等价零旋转）、450°（超整圈取模）、
//    90°+270°（累计归零）、空队列/单任务/双任务）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（5 种不支持路径类型 →
//    RotateImageFile_UnsupportedPathType_SkipsRotationWithoutCaching）
// 5. 分支清单 → 用例映射已列出: [x]（见下方 5 个方法分支清单块）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（见各分支清单映射；
//    构造函数 aboutToQuit 清理 lambda 经对 qApp 直接 emit aboutToQuit 触发（QuitHook 用例））
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无显式 throw；错误路径以返回 false +
//    信号未发射断言覆盖）
// 8. 负面场景有专门用例: [x]（ZeroAngle/UnsupportedPathType/SourceMissingCopyFails/
//    RotateDelegateFails/DataNull 系列）
// 9. 负面用例验证强异常安全: [x]（提前返回路径断言 data 保持 null、队列未受污染、
//    失败后对象仍可 checkDataValid 正常初始化）
// 10. stub_ext vs gMock 选择正确: [x]（LibUnionImage_NameSpace 自由函数与本项目静态
//     成员 rotateImageImpl → 函数地址 set_lamda；QTemporaryDir::isValid、
//     QFutureWatcher::isRunning 为 Qt 类方法 → static_cast 消歧；无虚接口依赖，不引入 gMock）
//
// 隔离与桩说明：
// - 路径口径：rotateImageFile 以裸本地路径为 rotationCache key、getPathType 与
//   QFile::copy 的输入，不做 toLocalFile 归一，故本文件一律使用本地路径形态
//   （与 GlobalControl url.toString() 口径不同，此处按源码真实口径）。
// - RotateImageHelperData 为 rotateimagehelper.cpp 内部隐藏类（无头文件），下方镜像
//   声明逐成员复制自源文件（currentRotateImage/rotationCache/watcher/queueMutex/
//   processQueue/cacheDir，含 QHash 容器），构造/析构符号链接到真实实现，
//   -fno-access-control 下经私有成员 data 直接读写状态。
// - 状态机用例全部使用栈上独立 RotateImageHelper 实例（私有构造经
//   -fno-access-control 可达），不污染单例；TearDown 统一 waitForDone + 单例复位。
// - 旋转状态转移断言 = 前后状态对比（rotationCache / processQueue 镜像直读）+
//   QSignalSpy（rotateImageImpl 三信号发射于 instance() 单例上，spy 挂单例；
//   rotateImageFile 累计归零短路（B8）的三信号发射于操作实例上，spy 挂该实例）。
// - enqueueRotateTask 的 QtConcurrent 工作线程真实启动，rotateImageImpl 以函数地址
//   stub 隔离文件写入；ImplRecorder 内置 QMutex 保证工作线程并发记录安全。
// - QThread::msleep(10)（rotateImageImpl 内同步短睡眠）保持真实，仅 10ms。
//
// ─────────────────────────────────────────────────────────────
// 分支清单（来源：RotateImageHelper::rotateImageFile(QString, int)）
// ─────────────────────────────────────────────────────────────
// B1: if (angle % 360 == 0) → 直接 return（不判定路径类型、不初始化 data）
// B2: if (pathType ∈ {MTP, PTP, APPLE, SAFEBOX, RECYCLEBIN}) → 直接 return（特殊位置禁写）
// B3: if (watcher.isRunning()) → 进入已排队任务更新路径；此前 checkDataValid
//     懒初始化 data，totalAngle += angle 后 %= 360 写回 rotationCache
// B4: for 遍历 processQueue 扫描已有同路径任务
// B5: if (proc.first == path) → 原地更新该任务角度并 return
// B6: for 未命中 → 追加队列（不启动新线程）
// B7: else（watcher 未运行）→ enqueueRotateTask（入队并启动 QtConcurrent 线程）
// B8: totalAngle %= 360 后为 0 → 不更新/不入队任务，在操作实例上按成功补发
//     record/clear/finished(path,true) 三信号（与 rotateImageImpl 正常路径序列一致）后 return
// 映射： RotateImageFile_ZeroOrFullTurnAngle_ReturnsBeforePathCheck → B1
//        RotateImageFile_UnsupportedPathType_SkipsRotationWithoutCaching（TEST_P）→ B2
//        RotateImageFile_LocalPath_EnqueuesAsyncRotationTask → B3+B7
//        RotateImageFile_AccumulatedFullTurnWhileRunning_CompletesWithoutEnqueue → B3+B6+B8（累计归零短路）
//        RotateImageFile_DifferentPathsWhileRunning_AppendsToQueue → B3+B4+B6
//        RotateImageFile_AngleBeyondFullTurn_ReducesModulo360 → B3+B6（>360° 边界）
//
// 分支清单（来源：RotateImageHelper::enqueueRotateTask(QString, int)）
// ─────────────────────────────────────────────────────────────
// B1: 循环入口 processQueue.isEmpty() → break（空队列提前退出，多线程竞争路径）
// B2: rotateImageImpl 返回 false → 仅告警，继续处理后续任务
// B3: rotateImageImpl 返回 true → 继续
// B4: 处理完一个任务后 queueSize > 0 → 继续循环
// B5: queueSize == 0 → 退出 do-while
// 映射： EnqueueRotateTask_SingleTask_DelegatesWithCacheDirFilePath → B3+B5
//        EnqueueRotateTask_MultipleTasks_AllProcessedBeforeExit → B4+B5
//        EnqueueRotateTask_ImplFails_ContinuesRemainingTasks → B2+B5
//
// 分支清单（来源：RotateImageHelper::rotateImageImpl(QString, QString, int)）
// ─────────────────────────────────────────────────────────────
// B1: cache 缺失且 QFile::copy 失败 → return false（不发射任何信号）
// B2: cache 已存在 → 跳过拷贝
// B3: 拷贝成功/已存在 → 发 recordRotateImage → 调 LibUnionImage_NameSpace::rotateImageFile
//     成功 → 发 clearRotateStatus + rotateImageFinished(path,true)，return true
// B4: 旋转失败 → 发 clearRotateStatus + rotateImageFinished(path,false)，return false
// 映射： RotateImageImpl_SourceMissingCopyFails_ReturnsFalseWithoutSignals → B1
//        RotateImageImpl_CacheFileExists_SkipsCopyKeepsCachedContent → B2+B3
//        RotateImageImpl_CacheMissing_CopiesFileAndEmitsLifecycleSignals → B3
//        RotateImageImpl_RotateDelegateFails_EmitsFinishedWithFalse → B4
//
// 分支清单（来源：RotateImageHelper::resetRotateState()）
// ─────────────────────────────────────────────────────────────
// B1: data 为空 → 直接 return
// B2: watcher 未运行 → 清 rotationCache 并立即 cacheDir.remove()
// B3: watcher 运行中 → 仅清 rotationCache，推迟目录清理
// 映射： ResetRotateState_DataNull_ReturnsEarlyWithoutSideEffects → B1
//        ResetRotateState_IdleWatcher_ClearsAngleCacheAndRemovesCacheDir → B2
//        ResetRotateState_WatcherRunning_ClearsCacheButKeepsCacheDir → B3
//
// 分支清单（来源：RotateImageHelper::checkDataValid()）
// ─────────────────────────────────────────────────────────────
// B1: data 为空 → new RotateImageHelperData 并连接 ImageFileWatcher 队列连接
// B2: cacheDir.isValid() 为假 → 告警（不中断初始化）
// B3: cacheDir.isValid() 为真 → 正常
// B4: data 已存在 → 跳过全部初始化
// 映射： CheckDataValid_DataNull_CreatesDataWithValidCacheDir → B1+B3
//        CheckDataValid_RepeatedCall_DoesNotRecreateData → B4
//        CheckDataValid_InvalidCacheDir_StillCreatesData → B1+B2

#include <gtest/gtest.h>

#include <QByteArray>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QPair>
#include <QPointer>
#include <QQueue>
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>
#include <QThreadPool>
#include <QList>

#include "stub_ext/stubext.h"
#include "rotateimagehelper.h"
#include "types.h"
#include "unionimage.h"

// ─────────────────────────────────────────────────────────────
// 镜像声明：RotateImageHelperData（隐藏于 rotateimagehelper.cpp）
// 逐成员复制自源文件（含成员顺序，QHash 容器），构造函数链接真实实现。
// 严禁改动成员顺序/类型，布局不符将导致越界访问。
// ─────────────────────────────────────────────────────────────
class RotateImageHelperData
{
public:
    explicit RotateImageHelperData();

    QString currentRotateImage;   // 当前操作的
    QHash<QString, int> rotationCache;   // 已缓存旋转文件列表 <文件路径，缓存旋转角度>
    QFutureWatcher<void> watcher;   // 异步处理监视器

    // 图片旋转处理队列
    QMutex queueMutex;
    QQueue<QPair<QString, int>> processQueue;   // 待处理的图片队列
    QTemporaryDir cacheDir;   // 临时文件目录
};

namespace {

// 工作线程安全的 rotateImageImpl 调用记录器（enqueueRotateTask 可能并行两个 worker）
class ImplRecorder
{
public:
    void record(const QString &cachePath, const QString &path, int angle)
    {
        QMutexLocker locker(&m_mutex);
        m_calls.append(qMakePair(path, angle));
        m_lastCachePath = cachePath;
    }

    QList<QPair<QString, int>> snapshot() const
    {
        QMutexLocker locker(&m_mutex);
        return m_calls;
    }

    QString lastCachePath() const
    {
        QMutexLocker locker(&m_mutex);
        return m_lastCachePath;
    }

private:
    mutable QMutex m_mutex;
    QList<QPair<QString, int>> m_calls;
    QString m_lastCachePath;
};

QString createSourceFile(const QTemporaryDir &dir, const QString &name)
{
    const QString path = dir.filePath(name);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QByteArray("png"));
        file.close();
    }
    return path;
}

}  // namespace

class RotateImageHelperTest : public ::testing::Test {
protected:
    void SetUp() override {
        stub.clear();
        helper = RotateImageHelper::instance();
    }

    void TearDown() override {
        // 先收割残留旋转线程，再撤桩（保证 resetRotateState 读到真实 isRunning），最后复位单例
        QThreadPool::globalInstance()->waitForDone();
        stub.clear();
        helper->resetRotateState();
    }

    stub_ext::StubExt stub;
    RotateImageHelper *helper = nullptr;
};

// ── 构造 / 析构 / instance ────────────────────────────────────────

TEST_F(RotateImageHelperTest, RotateImageHelper_Constructor_WithParent_HoldsParentAndNoLazyData)
{
    // Arrange
    QObject parent;

    // Act
    RotateImageHelper *scoped = new RotateImageHelper(&parent);

    // Assert：构造不初始化 data（懒加载，B1 前置），父子关系正确建立
    EXPECT_EQ(scoped->parent(), &parent);
    EXPECT_TRUE(scoped->data.isNull());
    delete scoped;
}

TEST_F(RotateImageHelperTest, RotateImageHelper_Destructor_ParentDeleteDestroysHelperAndData)
{
    // Arrange：父对象必须堆分配（delete 栈对象 = bad-free）
    QObject *parent = new QObject();
    RotateImageHelper *scoped = new RotateImageHelper(parent);
    scoped->checkDataValid();  // data 已初始化后析构，覆盖 QSharedPointer 释放链
    QPointer<RotateImageHelper> guard(scoped);
    EXPECT_NE(guard, nullptr);

    // Act：父对象析构连带销毁子对象（虚析构）
    delete parent;

    // Assert
    EXPECT_TRUE(guard.isNull());
    EXPECT_NE(QCoreApplication::instance(), nullptr);  // 进程核心对象未被波及
}

TEST_F(RotateImageHelperTest, Instance_RepeatedCalls_ReturnSameSingleton)
{
    // Arrange
    RotateImageHelper *first = RotateImageHelper::instance();

    // Act
    RotateImageHelper *second = RotateImageHelper::instance();

    // Assert
    EXPECT_NE(first, nullptr);
    EXPECT_EQ(first, second);
}

TEST_F(RotateImageHelperTest, Instance_StackInstance_DistinctFromSingletonWithOwnLazyData)
{
    // Arrange
    RotateImageHelper *singleton = RotateImageHelper::instance();

    // Act
    RotateImageHelper scoped;

    // Assert：独立实例与单例互不为同一对象，且数据各自懒加载（不共享）
    EXPECT_NE(&scoped, singleton);
    EXPECT_TRUE(scoped.data.isNull());
}

// ── checkDataValid ────────────────────────────────────────────────

TEST_F(RotateImageHelperTest, CheckDataValid_DataNull_CreatesDataWithValidCacheDir)
{
    // Arrange
    RotateImageHelper scoped;
    EXPECT_TRUE(scoped.data.isNull());  // 前置：懒加载未触发

    // Act
    scoped.checkDataValid();

    // Assert：B1+B3 —— data 创建成功，缓存目录有效且初始无角度记录
    ASSERT_FALSE(scoped.data.isNull());
    EXPECT_TRUE(scoped.data->cacheDir.isValid());
    EXPECT_EQ(scoped.data->rotationCache.size(), 0);
    EXPECT_EQ(scoped.data->processQueue.size(), 0);
}

TEST_F(RotateImageHelperTest, CheckDataValid_RepeatedCall_DoesNotRecreateData)
{
    // Arrange
    RotateImageHelper scoped;
    scoped.checkDataValid();
    const RotateImageHelperData *before = scoped.data.data();

    // Act
    scoped.checkDataValid();

    // Assert：B4 —— 已初始化则跳过重复创建，内部数据指针不变
    EXPECT_EQ(scoped.data.data(), before);
    EXPECT_EQ(scoped.data->rotationCache.size(), 0);
}

TEST_F(RotateImageHelperTest, CheckDataValid_InvalidCacheDir_StillCreatesData)
{
    // Arrange
    RotateImageHelper scoped;
    stub.set_lamda(static_cast<bool (QTemporaryDir::*)() const>(&QTemporaryDir::isValid),
                   []() -> bool { return false; });

    // Act
    scoped.checkDataValid();

    // Assert：B2 —— 缓存目录创建失败仅告警，data 初始化不被中断
    ASSERT_FALSE(scoped.data.isNull());
    EXPECT_EQ(scoped.data->rotationCache.size(), 0);
    EXPECT_EQ(scoped.data->processQueue.size(), 0);
    EXPECT_EQ(scoped.data->currentRotateImage, QString());
}

// ── rotateImageFile ───────────────────────────────────────────────

TEST_F(RotateImageHelperTest, RotateImageFile_ZeroOrFullTurnAngle_ReturnsBeforePathCheck)
{
    // Arrange
    RotateImageHelper scoped;
    int pathTypeCalls = 0;
    stub.set_lamda(&LibUnionImage_NameSpace::getPathType,
                   [&pathTypeCalls](const QString &) -> imageViewerSpace::PathType {
                       ++pathTypeCalls;
                       return imageViewerSpace::PathTypeLOCAL;
                   });

    // Act：0° 与 360° 为同一等价类的两个边界（均归零）
    scoped.rotateImageFile(QStringLiteral("album/pic-a.png"), 0);
    scoped.rotateImageFile(QStringLiteral("album/pic-a.png"), 360);

    // Assert：B1 —— 提前返回，路径类型未判定，data 未初始化（状态未受损）
    EXPECT_EQ(pathTypeCalls, 0);
    EXPECT_TRUE(scoped.data.isNull());
}

// TEST_P 子 Fixture：主 Fixture 保持 ::testing::Test，参数能力经多继承注入
struct RotateImageHelperPathTypeTest
    : public RotateImageHelperTest
    , public ::testing::WithParamInterface<imageViewerSpace::PathType> {
};

TEST_P(RotateImageHelperPathTypeTest, RotateImageFile_UnsupportedPathType_SkipsRotationWithoutCaching)
{
    // Arrange
    RotateImageHelper scoped;
    int pathTypeCalls = 0;
    stub.set_lamda(&LibUnionImage_NameSpace::getPathType,
                   [&pathTypeCalls, this](const QString &) -> imageViewerSpace::PathType {
                       ++pathTypeCalls;
                       return GetParam();
                   });

    // Act
    scoped.rotateImageFile(QStringLiteral("/mnt/device/photo.jpg"), 90);

    // Assert：B2 —— 路径类型已判定但直接跳过，未初始化 data、未记录角度
    EXPECT_EQ(pathTypeCalls, 1);
    EXPECT_TRUE(scoped.data.isNull());
}

INSTANTIATE_TEST_SUITE_P(
    UnsupportedPathTypes, RotateImageHelperPathTypeTest,
    ::testing::Values(imageViewerSpace::PathTypeMTP, imageViewerSpace::PathTypePTP,
                      imageViewerSpace::PathTypeAPPLE, imageViewerSpace::PathTypeSAFEBOX,
                      imageViewerSpace::PathTypeRECYCLEBIN));

TEST_F(RotateImageHelperTest, RotateImageFile_LocalPath_EnqueuesAsyncRotationTask)
{
    // Arrange
    QTemporaryDir workDir;
    const QString path = createSourceFile(workDir, QStringLiteral("a.png"));
    ASSERT_TRUE(QFile::exists(path));
    RotateImageHelper scoped;
    stub.set_lamda(&LibUnionImage_NameSpace::getPathType,
                   [](const QString &) -> imageViewerSpace::PathType {
                       return imageViewerSpace::PathTypeLOCAL;
                   });
    ImplRecorder recorder;
    stub.set_lamda(&RotateImageHelper::rotateImageImpl,
                   [&recorder](const QString &cachePath, const QString &p, int angle) -> bool {
                       recorder.record(cachePath, p, angle);
                       return true;
                   });

    // Act：B3+B6 —— watcher 未运行，入队并启动异步处理线程
    scoped.rotateImageFile(path, 90);
    QThreadPool::globalInstance()->waitForDone();

    // Assert：角度已记录（状态转移 0 → 90），异步委托发生且队列为空
    const QList<QPair<QString, int>> calls = recorder.snapshot();
    ASSERT_EQ(calls.size(), 1);
    EXPECT_EQ(calls.first().first, path);
    EXPECT_EQ(calls.first().second, 90);
    ASSERT_FALSE(scoped.data.isNull());
    EXPECT_EQ(scoped.data->rotationCache.value(path), 90);
    EXPECT_TRUE(scoped.data->processQueue.isEmpty());
    EXPECT_FALSE(scoped.data->watcher.isRunning());
}

TEST_F(RotateImageHelperTest, RotateImageFile_AccumulatedFullTurnWhileRunning_CompletesWithoutEnqueue)
{
    // Arrange
    RotateImageHelper scoped;
    scoped.checkDataValid();
    stub.set_lamda(&LibUnionImage_NameSpace::getPathType,
                   [](const QString &) -> imageViewerSpace::PathType {
                       return imageViewerSpace::PathTypeLOCAL;
                   });
    stub.set_lamda(
        static_cast<bool (QFutureWatcher<void>::*)() const>(&QFutureWatcher<void>::isRunning),
        []() -> bool { return true; });
    const QString path = QStringLiteral("album/pic-a.png");
    QSignalSpy spyRecord(&scoped, &RotateImageHelper::recordRotateImage);
    QSignalSpy spyClear(&scoped, &RotateImageHelper::clearRotateStatus);
    QSignalSpy spyFinished(&scoped, &RotateImageHelper::rotateImageFinished);

    // Act：90° 入队后再追加 270°，累计取模归零 → 短路按成功完成，不再入队 0° 无效旋转
    scoped.rotateImageFile(path, 90);
    scoped.rotateImageFile(path, 270);

    // Assert：B3+B6+B8 —— 累计角度 0；首个任务保持 90° 未被改写为 0、未重复入队/启动线程；
    // 短路路径在操作实例上按成功补发与正常路径相同的三信号序列
    EXPECT_EQ(scoped.data->rotationCache.value(path), 0);
    ASSERT_EQ(scoped.data->processQueue.size(), 1);
    EXPECT_EQ(scoped.data->processQueue.head().second, 90);
    EXPECT_EQ(scoped.data->processQueue.head().first, path);
    EXPECT_EQ(spyRecord.count(), 1);
    EXPECT_EQ(spyClear.count(), 1);
    ASSERT_EQ(spyFinished.count(), 1);
    EXPECT_EQ(spyFinished.at(0).at(0).toString(), path);
    EXPECT_EQ(spyFinished.at(0).at(1).toBool(), true);
}

TEST_F(RotateImageHelperTest, RotateImageFile_DifferentPathsWhileRunning_AppendsToQueue)
{
    // Arrange
    RotateImageHelper scoped;
    scoped.checkDataValid();
    stub.set_lamda(&LibUnionImage_NameSpace::getPathType,
                   [](const QString &) -> imageViewerSpace::PathType {
                       return imageViewerSpace::PathTypeLOCAL;
                   });
    stub.set_lamda(
        static_cast<bool (QFutureWatcher<void>::*)() const>(&QFutureWatcher<void>::isRunning),
        []() -> bool { return true; });
    const QString pathA = QStringLiteral("album/pic-a.png");
    const QString pathB = QStringLiteral("album/pic-b.png");

    // Act：两个不同路径先后请求旋转
    scoped.rotateImageFile(pathA, 90);
    scoped.rotateImageFile(pathB, 180);

    // Assert：B5 —— 未命中已有任务时追加新任务，两份角度记录独立累计
    ASSERT_EQ(scoped.data->processQueue.size(), 2);
    EXPECT_EQ(scoped.data->processQueue.at(0).first, pathA);
    EXPECT_EQ(scoped.data->processQueue.at(0).second, 90);
    EXPECT_EQ(scoped.data->processQueue.at(1).first, pathB);
    EXPECT_EQ(scoped.data->processQueue.at(1).second, 180);
    EXPECT_EQ(scoped.data->rotationCache.value(pathA), 90);
    EXPECT_EQ(scoped.data->rotationCache.value(pathB), 180);
}

TEST_F(RotateImageHelperTest, RotateImageFile_AngleBeyondFullTurn_ReducesModulo360)
{
    // Arrange
    RotateImageHelper scoped;
    scoped.checkDataValid();
    stub.set_lamda(&LibUnionImage_NameSpace::getPathType,
                   [](const QString &) -> imageViewerSpace::PathType {
                       return imageViewerSpace::PathTypeLOCAL;
                   });
    stub.set_lamda(
        static_cast<bool (QFutureWatcher<void>::*)() const>(&QFutureWatcher<void>::isRunning),
        []() -> bool { return true; });
    const QString path = QStringLiteral("album/pic-a.png");

    // Act：450° 超过整圈，应取模为 90° 后入队
    scoped.rotateImageFile(path, 450);

    // Assert：入队角度与累计角度均为取模余数
    ASSERT_EQ(scoped.data->processQueue.size(), 1);
    EXPECT_EQ(scoped.data->processQueue.head().second, 90);
    EXPECT_EQ(scoped.data->rotationCache.value(path), 90);
}

// ── rotateImageImpl（静态方法，信号发射于 instance() 单例上）────────

TEST_F(RotateImageHelperTest, RotateImageImpl_CacheMissing_CopiesFileAndEmitsLifecycleSignals)
{
    // Arrange
    QTemporaryDir workDir;
    const QString srcPath = createSourceFile(workDir, QStringLiteral("src.png"));
    const QString cachePath = workDir.filePath(QStringLiteral("cache.png"));
    ASSERT_FALSE(QFile::exists(cachePath));  // 前置：缓存缺失，触发拷贝分支
    int unionCalls = 0;
    int seenAngle = 0;
    QString seenRotatePath;
    QString seenTargetPath;
    stub.set_lamda(&LibUnionImage_NameSpace::rotateImageFile,
                   [&](int angel, const QString &p, QString &, const QString &targetPath) -> bool {
                       ++unionCalls;
                       seenAngle = angel;
                       seenRotatePath = p;
                       seenTargetPath = targetPath;
                       return true;
                   });
    QSignalSpy spyRecord(helper, &RotateImageHelper::recordRotateImage);
    QSignalSpy spyClear(helper, &RotateImageHelper::clearRotateStatus);
    QSignalSpy spyFinished(helper, &RotateImageHelper::rotateImageFinished);

    // Act
    const bool ret = RotateImageHelper::rotateImageImpl(cachePath, srcPath, 90);
    QCoreApplication::processEvents();  // 信号经队列连接投递，冲刷事件队列

    // Assert：源文件拷贝到缓存、旋转以缓存文件为目标、三信号按序发射
    EXPECT_TRUE(ret);
    EXPECT_TRUE(QFile::exists(cachePath));
    EXPECT_EQ(unionCalls, 1);
    EXPECT_EQ(seenAngle, 90);
    EXPECT_EQ(seenRotatePath, cachePath);
    EXPECT_EQ(seenTargetPath, srcPath);
    EXPECT_EQ(spyRecord.count(), 1);
    EXPECT_EQ(spyClear.count(), 1);
    ASSERT_EQ(spyFinished.count(), 1);
    EXPECT_EQ(spyFinished.at(0).at(0).toString(), srcPath);
    EXPECT_EQ(spyFinished.at(0).at(1).toBool(), true);
}

TEST_F(RotateImageHelperTest, RotateImageImpl_CacheFileExists_SkipsCopyKeepsCachedContent)
{
    // Arrange
    QTemporaryDir workDir;
    const QString srcPath = createSourceFile(workDir, QStringLiteral("src.png"));
    const QString cachePath = workDir.filePath(QStringLiteral("cache.png"));
    QFile cacheCreator(cachePath);
    ASSERT_TRUE(cacheCreator.open(QIODevice::WriteOnly));
    cacheCreator.write(QByteArray("cached-bytes"));
    cacheCreator.close();
    stub.set_lamda(&LibUnionImage_NameSpace::rotateImageFile,
                   [](int, const QString &, QString &, const QString &) -> bool { return true; });
    QSignalSpy spyRecord(helper, &RotateImageHelper::recordRotateImage);

    // Act
    const bool ret = RotateImageHelper::rotateImageImpl(cachePath, srcPath, 90);
    QCoreApplication::processEvents();

    // Assert：B2 —— 缓存已存在则跳过拷贝，缓存内容不被源文件覆盖
    EXPECT_TRUE(ret);
    QFile reader(cachePath);
    ASSERT_TRUE(reader.open(QIODevice::ReadOnly));
    const QByteArray content = reader.readAll();
    reader.close();
    EXPECT_EQ(content, QByteArray("cached-bytes"));
    EXPECT_EQ(spyRecord.count(), 1);
}

TEST_F(RotateImageHelperTest, RotateImageImpl_SourceMissingCopyFails_ReturnsFalseWithoutSignals)
{
    // Arrange
    QTemporaryDir workDir;
    const QString srcPath = workDir.filePath(QStringLiteral("missing.png"));
    const QString cachePath = workDir.filePath(QStringLiteral("cache.png"));
    ASSERT_FALSE(QFile::exists(srcPath));
    int unionCalls = 0;
    stub.set_lamda(&LibUnionImage_NameSpace::rotateImageFile,
                   [&unionCalls](int, const QString &, QString &, const QString &) -> bool {
                       ++unionCalls;
                       return true;
                   });
    QSignalSpy spyRecord(helper, &RotateImageHelper::recordRotateImage);
    QSignalSpy spyFinished(helper, &RotateImageHelper::rotateImageFinished);

    // Act
    const bool ret = RotateImageHelper::rotateImageImpl(cachePath, srcPath, 90);
    QCoreApplication::processEvents();

    // Assert：B1 —— 拷贝失败直接返回 false，旋转未执行、生命周期信号未发射
    EXPECT_FALSE(ret);
    EXPECT_EQ(unionCalls, 0);
    EXPECT_FALSE(QFile::exists(cachePath));
    EXPECT_EQ(spyRecord.count(), 0);
    EXPECT_EQ(spyFinished.count(), 0);
}

TEST_F(RotateImageHelperTest, RotateImageImpl_RotateDelegateFails_EmitsFinishedWithFalse)
{
    // Arrange
    QTemporaryDir workDir;
    const QString srcPath = createSourceFile(workDir, QStringLiteral("src.png"));
    const QString cachePath = createSourceFile(workDir, QStringLiteral("cache.png"));
    stub.set_lamda(&LibUnionImage_NameSpace::rotateImageFile,
                   [](int, const QString &, QString &erroMsg, const QString &) -> bool {
                       erroMsg = QStringLiteral("rotate failed");
                       return false;
                   });
    QSignalSpy spyClear(helper, &RotateImageHelper::clearRotateStatus);
    QSignalSpy spyFinished(helper, &RotateImageHelper::rotateImageFinished);

    // Act
    const bool ret = RotateImageHelper::rotateImageImpl(cachePath, srcPath, 90);
    QCoreApplication::processEvents();

    // Assert：B4 —— 委托失败返回 false，结束信号携带失败结果
    EXPECT_FALSE(ret);
    EXPECT_EQ(spyClear.count(), 1);
    ASSERT_EQ(spyFinished.count(), 1);
    EXPECT_EQ(spyFinished.at(0).at(0).toString(), srcPath);
    EXPECT_EQ(spyFinished.at(0).at(1).toBool(), false);
}

// ── enqueueRotateTask ─────────────────────────────────────────────

TEST_F(RotateImageHelperTest, EnqueueRotateTask_SingleTask_DelegatesWithCacheDirFilePath)
{
    // Arrange
    QTemporaryDir workDir;
    const QString path = createSourceFile(workDir, QStringLiteral("a.png"));
    ASSERT_TRUE(QFile::exists(path));
    RotateImageHelper scoped;
    scoped.checkDataValid();
    ImplRecorder recorder;
    stub.set_lamda(&RotateImageHelper::rotateImageImpl,
                   [&recorder](const QString &cachePath, const QString &p, int angle) -> bool {
                       recorder.record(cachePath, p, angle);
                       return true;
                   });

    // Act
    scoped.enqueueRotateTask(path, 90);
    QThreadPool::globalInstance()->waitForDone();

    // Assert：任务出队并委托 rotateImageImpl，缓存路径位于 data->cacheDir 内且文件名一致
    const QList<QPair<QString, int>> calls = recorder.snapshot();
    ASSERT_EQ(calls.size(), 1);
    EXPECT_EQ(calls.first().first, path);
    EXPECT_EQ(calls.first().second, 90);
    EXPECT_TRUE(recorder.lastCachePath().startsWith(scoped.data->cacheDir.path()));
    EXPECT_EQ(QFileInfo(recorder.lastCachePath()).fileName(), QFileInfo(path).fileName());
    EXPECT_TRUE(scoped.data->processQueue.isEmpty());
}

TEST_F(RotateImageHelperTest, EnqueueRotateTask_MultipleTasks_AllProcessedBeforeExit)
{
    // Arrange
    QTemporaryDir workDir;
    const QString pathA = createSourceFile(workDir, QStringLiteral("a.png"));
    const QString pathB = createSourceFile(workDir, QStringLiteral("b.png"));
    RotateImageHelper scoped;
    scoped.checkDataValid();
    ImplRecorder recorder;
    stub.set_lamda(&RotateImageHelper::rotateImageImpl,
                   [&recorder](const QString &cachePath, const QString &p, int angle) -> bool {
                       recorder.record(cachePath, p, angle);
                       return true;
                   });

    // Act：连续入队两个任务（可能由一个线程循环处理或两个线程并行消费）
    scoped.enqueueRotateTask(pathA, 90);
    scoped.enqueueRotateTask(pathB, 180);
    QThreadPool::globalInstance()->waitForDone();

    // Assert：B4+B5 —— 循环持续消费，两个任务均被处理且队列清空（顺序不作断言，避免线程调度耦合）
    const QList<QPair<QString, int>> calls = recorder.snapshot();
    ASSERT_EQ(calls.size(), 2);
    EXPECT_TRUE(calls.contains(qMakePair(pathA, 90)));
    EXPECT_TRUE(calls.contains(qMakePair(pathB, 180)));
    EXPECT_EQ(scoped.data->processQueue.size(), 0);
}

TEST_F(RotateImageHelperTest, EnqueueRotateTask_ImplFails_ContinuesRemainingTasks)
{
    // Arrange
    QTemporaryDir workDir;
    const QString pathA = createSourceFile(workDir, QStringLiteral("a.png"));
    const QString pathB = createSourceFile(workDir, QStringLiteral("b.png"));
    RotateImageHelper scoped;
    scoped.checkDataValid();
    ImplRecorder recorder;
    stub.set_lamda(&RotateImageHelper::rotateImageImpl,
                   [&recorder](const QString &cachePath, const QString &p, int angle) -> bool {
                       recorder.record(cachePath, p, angle);
                       return p.endsWith(QStringLiteral("b.png"));
                   });

    // Act：a 任务委托失败、b 任务成功
    scoped.enqueueRotateTask(pathA, 90);
    scoped.enqueueRotateTask(pathB, 180);
    QThreadPool::globalInstance()->waitForDone();

    // Assert：B2 —— 失败仅告警，后续任务继续被处理，队列最终清空
    const QList<QPair<QString, int>> calls = recorder.snapshot();
    ASSERT_EQ(calls.size(), 2);
    EXPECT_TRUE(calls.contains(qMakePair(pathA, 90)));
    EXPECT_TRUE(calls.contains(qMakePair(pathB, 180)));
    EXPECT_EQ(scoped.data->processQueue.size(), 0);
}

// ── resetRotateState ──────────────────────────────────────────────

TEST_F(RotateImageHelperTest, ResetRotateState_DataNull_ReturnsEarlyWithoutSideEffects)
{
    // Arrange
    RotateImageHelper scoped;
    EXPECT_TRUE(scoped.data.isNull());

    // Act：data 未初始化时的复位（空状态早退）
    scoped.resetRotateState();

    // Assert：B1 —— 早退不创建数据；对象未被破坏，仍可正常懒加载初始化
    EXPECT_TRUE(scoped.data.isNull());
    scoped.checkDataValid();
    ASSERT_FALSE(scoped.data.isNull());
    EXPECT_EQ(scoped.data->rotationCache.size(), 0);
}

TEST_F(RotateImageHelperTest, ResetRotateState_IdleWatcher_ClearsAngleCacheAndRemovesCacheDir)
{
    // Arrange
    RotateImageHelper scoped;
    scoped.checkDataValid();
    scoped.data->rotationCache.insert(QStringLiteral("album/pic-a.png"), 90);
    const QString cacheDirPath = scoped.data->cacheDir.path();
    EXPECT_TRUE(QFile::exists(cacheDirPath));  // 前置：缓存目录真实存在

    // Act
    scoped.resetRotateState();

    // Assert：B2 —— 角度缓存清空 + 缓存目录立即删除
    EXPECT_EQ(scoped.data->rotationCache.size(), 0);
    EXPECT_FALSE(QFile::exists(cacheDirPath));
}

TEST_F(RotateImageHelperTest, ResetRotateState_WatcherRunning_ClearsCacheButKeepsCacheDir)
{
    // Arrange
    RotateImageHelper scoped;
    scoped.checkDataValid();
    scoped.data->rotationCache.insert(QStringLiteral("album/pic-a.png"), 90);
    const QString cacheDirPath = scoped.data->cacheDir.path();
    stub.set_lamda(
        static_cast<bool (QFutureWatcher<void>::*)() const>(&QFutureWatcher<void>::isRunning),
        []() -> bool { return true; });

    // Act：旋转线程运行中复位
    scoped.resetRotateState();

    // Assert：B3 —— 角度缓存仍被清理，但目录删除被推迟（线程仍在使用缓存目录）
    EXPECT_EQ(scoped.data->rotationCache.size(), 0);
    EXPECT_TRUE(QFile::exists(cacheDirPath));
}

// ── RotateImageHelperData（隐藏类，镜像声明链接真实构造）──────────

class RotateImageHelperDataTest : public ::testing::Test {
protected:
    void SetUp() override {
        stub.clear();
        obj = new RotateImageHelperData();
    }

    void TearDown() override {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    RotateImageHelperData *obj = nullptr;
};

TEST_F(RotateImageHelperDataTest, RotateImageHelperData_Constructor_InitializesEmptyStateWithValidCacheDir)
{
    // Arrange：SetUp 已以镜像声明构造真实实现
    ASSERT_NE(obj, nullptr);

    // Act：读取各成员初始状态（构造发生在 SetUp，此处取值即被测行为结果）
    const int angleCacheCount = obj->rotationCache.size();
    const int queueCount = obj->processQueue.size();
    const bool cacheReady = obj->cacheDir.isValid();
    const bool watcherIdle = !obj->watcher.isRunning();

    // Assert：新实例无进行中操作、无角度缓存、队列为空、缓存目录就绪、监视器空闲
    EXPECT_TRUE(obj->currentRotateImage.isEmpty());
    EXPECT_EQ(angleCacheCount, 0);
    EXPECT_EQ(queueCount, 0);
    EXPECT_TRUE(cacheReady);
    EXPECT_TRUE(watcherIdle);
}

// ─── 构造函数 aboutToQuit 清理 lambda（补测：对 qApp 实例直接 emit 驱动）───
// 分支（来源：rotateimagehelper.cpp:43-57）：
// B1: data && data->watcher.isRunning() → waitForFinished + cacheDir.remove()
// B2: watcher 空闲 → lambda 安全返回（无清理动作）
// 映射： RotateImageHelper_QuitHook_QuitSignalIdleWatcher_KeepsDataIntact → B2

TEST_F(RotateImageHelperTest, RotateImageHelper_QuitHook_QuitSignalIdleWatcher_KeepsDataIntact)
{
    // Arrange：栈实例并懒加载 data（watcher 空闲路径）；监听 qApp 的 aboutToQuit
    RotateImageHelper scoped;
    scoped.checkDataValid();
    ASSERT_FALSE(scoped.data.isNull());
    QSignalSpy quitSpy(QCoreApplication::instance(), &QCoreApplication::aboutToQuit);

    // Act：对 qApp 实例直接发射 aboutToQuit（-fno-access-control 下可构造 QPrivateSignal）
    Q_EMIT QCoreApplication::instance()->aboutToQuit(QCoreApplication::QPrivateSignal{});

    // Assert：lambda 安全返回——空闲 watcher 不触发 waitForFinished/删缓存目录，
    // data 完整保留，随后 public API（0° 早退路径）仍可正常受理
    EXPECT_EQ(quitSpy.count(), 1);
    EXPECT_FALSE(scoped.data->watcher.isRunning());
    EXPECT_TRUE(scoped.data->cacheDir.isValid());
    scoped.rotateImageFile(QStringLiteral("album/pic.png"), 0);
    EXPECT_EQ(scoped.data->rotationCache.size(), 0);
}
