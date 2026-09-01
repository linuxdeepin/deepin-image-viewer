// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | ImageFileWatcher(parent) | low | - | 1 | 1 |
// | ~ImageFileWatcher | low | - | 1 | 1 |
// | addImageFile(filePath) | low | - | 1 | 2 |
// | clearRotateStatus(targetPath) | mid | - | 2 | 2 |
// | fileRename(oldPath,newPath) | low | - | 1 | 2 |
// | instance() | low | - | 1 | 1 |
// | isCurrentDir(filePath) | low | - | 1 | 1 |
// | onImageDirChanged(dir) | mid | - | 2 | 2 |
// | onImageFileChanged(file) | low | - | 1 | 4 |
// | recordRotateImage(targetPath) | low | - | 1 | 1 |
// | resetImageFiles(filePaths) | mid | - | 2 | 5 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（11 个方法全覆盖，含构造/析构）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（存在/缺失/目录路径；空列表/单/多文件；
//    本地路径/URL 形态；已缓存/未缓存/旋转中文件）
// 3. 每个等价类的边界值显式覆盖: [x]（空列表、单文件、混合存在性、重复重置、
//    移除后恢复的目录变更边界）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（本类输入空间为互异行为场景，无 ≥3 组同断言逻辑的
//    同质输入，故用多个独立 TEST_F 分别覆盖各分支，不适用 TEST_P）
// 5. 分支清单 → 用例映射已列出: [x]（见下方 onImageFileChanged/resetImageFiles
//    分支清单块，其余方法分支数 <3 且 complexity<10，按 §4.1 允许省略）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（见各分支清单映射）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无显式 throw；错误路径以
//    无信号/无缓存记录断言覆盖）
// 8. 负面场景有专门用例: [x]（Missing/Uncached/NonExisting/EmptyList 系列）
// 9. 负面用例验证强异常安全: [x]（未命中路径处理后观察列表/缓存保持原状断言）
// 10. stub_ext vs gMock 选择正确: [x]（ImageInfo 为项目内部类、非虚、无注入点 →
//     VADDR；文件系统经 QTemporaryDir 隔离，不 stub Qt 类）
//
// 隔离与桩说明：
// - 输入形态约定：addImageFile/resetImageFiles 内部做 QUrl(filePath).toLocalFile()
//   归一，本 Qt6 环境 QUrl(裸绝对路径).toLocalFile() 返回空串（裸路径会静默空转），
//   故这两个人口一律经 watchUrl() 以 "file://..." URL 形态输入（与生产链路
//   GlobalControl::setImageFiles 存 url.toString() 的形态一致）；
//   fileRename/onImageFileChanged/recordRotateImage/isCurrentDir 以本地路径为缓存
//   key，直接使用本地路径。
// - onImageFileChanged 尾部构造 ImageInfo 并 setSource/clearCurrentCache/reloadData，
//   真实实现会触发 ImageInfoCache 后台加载；三项均以 VADDR stub 计数隔离。
// - 文件监听用真实 QFileSystemWatcher + QTemporaryDir 落盘文件（仅同步增删路径，
//   不等待异步信号），用例内不调用 processEvents，避免事件循环引入的不确定时序。
// - ScopedDebugLogSuppressor：onImageDirChanged 在 erase 返回 end() 后仍以
//   qCDebug 流式输出 itr.key()（疑似缺陷 D1，见类内注释）。ut_main 开启了 debug
//   日志，触发恢复分支的用例需临时关闭该分类以保证流参数不求值、迭代器不解引用。
//
// ─────────────────────────────────────────────────────────────
// 分支清单（来源：ImageFileWatcher::onImageFileChanged(QString)）
// ─────────────────────────────────────────────────────────────
// B1: rotateImagePathSet.contains(file) → return（旋转处理中，忽略变更）
// B2: cacheFileInfo.contains(file) → 检查 QFile::exists；
//     !isExist → removedFile.insert（移除/移动记录）
// B3: isExist → 仅记录日志；两路均 Q_EMIT imageFileChanged 并走 ImageInfo 重载链；
//     未缓存 → 无任何动作
// 映射： OnImageFileChanged_RotatingFile_ChangeIgnored → B1
//        OnImageFileChanged_CachedFileDeleted_StillEmitsAndRecordsRemoval → B2
//        OnImageFileChanged_CachedExistingFile_EmitsAndReloadsInfo → B3
//        OnImageFileChanged_UncachedFile_IgnoresChange → B3（未缓存侧）
//        OnImageDirChanged_* → B2+B3（经恢复链路间接覆盖）
//
// 分支清单（来源：ImageFileWatcher::resetImageFiles(QStringList)）
// ─────────────────────────────────────────────────────────────
// B1: filePaths.isEmpty() → 清空三份缓存记录
// B2: files 非空 → removePaths(files)
// B3: directories 非空 → removePaths(directories) → return
// B4: isCurrentDir(filePaths.first()) → return（同目录重复重置跳过）
// B5: for 遍历 filePaths
// B6: info.exists() → cacheFileInfo.insert + addPath
// B7: 文件不存在 → 跳过（告警）
// B8: fileList 非空 → addPath(父目录)；否则跳过目录观察
// 映射： ResetImageFiles_EmptyList_UnwatchesAllPaths → B1+B2+B3
//        ResetImageFiles_SameDirectoryAgain_SkipsRewatchButClearsCache → B4
//        ResetImageFiles_ExistingPaths_WatchesFilesAndParentDirectory → B5+B6+B8
//        ResetImageFiles_MixedExistence_OnlyExistingFilesWatched → B5+B6+B7+B8

#include <gtest/gtest.h>

#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QLoggingCategory>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>

#include "imagefilewatcher.h"
#include "imageinfo.h"
#include "stub_ext/stubext.h"

namespace {
// 在临时目录下落盘一个可被 QFileInfo::isFile 识别的占位文件，返回绝对路径
QString createWatchableFile(const QTemporaryDir &dir, const QString &name)
{
    const QString path = dir.filePath(name);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
        file.write("placeholder");
    return path;
}

// 环境坑（同批次简报 #6）：QUrl(裸绝对路径).toLocalFile() 在本 Qt6 环境返回空串，
// 生产链路（GlobalControl::setImageFiles → FileControl::resetImageFiles）以
// url.toString() 即 "file:///..." 形态喂给观察器。因此凡经 addImageFile /
// resetImageFiles 入口（内部做 QUrl(filePath).toLocalFile() 归一）的输入一律
// 使用该 URL 形态；fileRename/onImageFileChanged/isCurrentDir 等以本地路径为
// 缓存 key 的接口仍直接使用本地路径。
QString watchUrl(const QString &localPath)
{
    return QUrl::fromLocalFile(localPath).toString();
}

// 疑似缺陷 D1 防护：onImageDirChanged 恢复分支中 itr = removedFile.erase(itr) 之后
// 仍执行 qCDebug(...) << itr.key()；当被擦除元素是迭代最后一个时 erase 返回 end()，
// 解引用 end() 为 UB。ut_main 默认开启 debug 日志会使流参数被求值，故在触发该分支的
// 用例期间临时关闭本日志分类（qCDebug 宏在 isDebugEnabled()==false 时不求值参数），
// 退出时恢复为 true（与 ut_main 的运行期设定一致）。
class ScopedDebugLogSuppressor {
public:
    ScopedDebugLogSuppressor()
    {
        QLoggingCategory::setFilterRules(
                QStringLiteral("org.deepin.dde.imageviewer.debug=false"));
    }
    ~ScopedDebugLogSuppressor()
    {
        QLoggingCategory::setFilterRules(
                QStringLiteral("org.deepin.dde.imageviewer.debug=true"));
    }
};
}  // namespace

class ImageFileWatcherTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        watcher = new ImageFileWatcher();
        // onImageFileChanged 尾部 ImageInfo 重载链隔离：只计数，不触盘
        stub.set_lamda(VADDR(ImageInfo, setSource),
                       [this](ImageInfo *, const QUrl &) { ++setSourceCount; });
        stub.set_lamda(VADDR(ImageInfo, clearCurrentCache),
                       [this](ImageInfo *) { ++clearCacheCount; });
        stub.set_lamda(VADDR(ImageInfo, reloadData),
                       [this](ImageInfo *) { ++reloadCount; });
    }

    void TearDown() override
    {
        delete watcher;
        watcher = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    QTemporaryDir tmpDir;
    ImageFileWatcher *watcher = nullptr;

    // ImageInfo 链路 stub 计数
    int setSourceCount = 0;
    int clearCacheCount = 0;
    int reloadCount = 0;
};

// ── 构造 / 析构 / instance ────────────────────────────────────────

TEST_F(ImageFileWatcherTest, ImageFileWatcher_Constructor_StartsWithNoWatchedPaths)
{
    // Arrange：SetUp 已构造新实例，取其内部 QFileSystemWatcher 子对象
    QFileSystemWatcher *inner = watcher->fileWatcher;

    // Act
    const int watchedFiles = inner->files().size();
    const int watchedDirs = inner->directories().size();

    // Assert：初始不观察任何文件与目录
    EXPECT_EQ(watchedFiles, 0);
    EXPECT_EQ(watchedDirs, 0);
}

TEST_F(ImageFileWatcherTest, ImageFileWatcher_Destructor_WithWatchedPathsEndsCleanly)
{
    // Arrange
    const QString path = createWatchableFile(tmpDir, QStringLiteral("a.png"));
    ImageFileWatcher *scoped = new ImageFileWatcher();
    scoped->addImageFile(watchUrl(path));
    EXPECT_EQ(scoped->fileWatcher->files().size(), 1);

    // Act：delete 触发析构（连带销毁子对象 QFileSystemWatcher）
    EXPECT_NO_THROW(delete scoped);

    // Assert：fixture 实例不受影响
    EXPECT_EQ(watcher->fileWatcher->files().size(), 0);
}

TEST_F(ImageFileWatcherTest, Instance_RepeatedCalls_ReturnSameSingleton)
{
    // Arrange
    ImageFileWatcher *first = ImageFileWatcher::instance();

    // Act
    ImageFileWatcher *second = ImageFileWatcher::instance();
    second->resetImageFiles({});  // 单例状态复位，避免污染其它用例

    // Assert
    EXPECT_NE(first, nullptr);
    EXPECT_EQ(first, second);
    EXPECT_EQ(first->fileWatcher->files().size(), 0);
}

// ── addImageFile ──────────────────────────────────────────────────

TEST_F(ImageFileWatcherTest, AddImageFile_ExistingFile_WatchedWithoutDuplicates)
{
    // Arrange
    const QString path = createWatchableFile(tmpDir, QStringLiteral("a.png"));

    // Act：URL 形态添加两次（内部经 toLocalFile 归一到同一路径）
    watcher->addImageFile(watchUrl(path));
    watcher->addImageFile(watchUrl(path));

    // Assert：同一路径只观察一次
    EXPECT_TRUE(watcher->fileWatcher->files().contains(path));
    EXPECT_EQ(watcher->fileWatcher->files().size(), 1);
}

TEST_F(ImageFileWatcherTest, AddImageFile_MissingOrDirectoryPath_Ignored)
{
    // Arrange
    const QString ghost = tmpDir.filePath(QStringLiteral("ghost.png"));
    const QString dirPath = tmpDir.path();

    // Act：不存在的文件与目录路径（负面：isFile 为假）
    watcher->addImageFile(watchUrl(ghost));
    watcher->addImageFile(watchUrl(dirPath));

    // Assert：均不进入观察列表
    EXPECT_FALSE(watcher->fileWatcher->files().contains(ghost));
    EXPECT_EQ(watcher->fileWatcher->files().size(), 0);
}

// ── fileRename ────────────────────────────────────────────────────

TEST_F(ImageFileWatcherTest, FileRename_CachedPath_MovesWatchToNewPath)
{
    // Arrange
    const QString oldPath = createWatchableFile(tmpDir, QStringLiteral("old.png"));
    const QString newPath = createWatchableFile(tmpDir, QStringLiteral("new.png"));
    watcher->addImageFile(watchUrl(oldPath));
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act
    watcher->fileRename(oldPath, newPath);

    // Assert：观察与缓存均迁移到新路径（新路径变更会触发信号与重载链）
    EXPECT_FALSE(watcher->fileWatcher->files().contains(oldPath));
    EXPECT_TRUE(watcher->fileWatcher->files().contains(newPath));
    watcher->onImageFileChanged(newPath);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(reloadCount, 1);
}

TEST_F(ImageFileWatcherTest, FileRename_UncachedPath_LeavesWatchUnchanged)
{
    // Arrange
    const QString ghost = tmpDir.filePath(QStringLiteral("ghost.png"));
    const QString other = tmpDir.filePath(QStringLiteral("other.png"));
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act：旧路径未在缓存中（负面）
    watcher->fileRename(ghost, other);
    watcher->onImageFileChanged(other);

    // Assert：观察列表与信号均无变化（强异常安全）
    EXPECT_EQ(watcher->fileWatcher->files().size(), 0);
    EXPECT_EQ(spy.count(), 0);
}

// ── isCurrentDir ──────────────────────────────────────────────────

TEST_F(ImageFileWatcherTest, IsCurrentDir_WatchedDirectory_TrueForItsFiles)
{
    // Arrange
    const QString watched = createWatchableFile(tmpDir, QStringLiteral("a.png"));
    watcher->resetImageFiles({watchUrl(watched)});
    QTemporaryDir otherDir;
    ASSERT_TRUE(otherDir.isValid());
    const QString elsewhere = createWatchableFile(otherDir, QStringLiteral("b.png"));

    // Act
    const bool inWatchedDir = watcher->isCurrentDir(watched);
    const bool inOtherDir = watcher->isCurrentDir(elsewhere);

    // Assert：被观察目录内文件返回 true，其它目录文件返回 false
    EXPECT_TRUE(inWatchedDir);
    EXPECT_FALSE(inOtherDir);
    EXPECT_EQ(watcher->fileWatcher->directories().size(), 1);
}

// ── recordRotateImage / clearRotateStatus ─────────────────────────

TEST_F(ImageFileWatcherTest, RecordRotateImage_WhileRotating_SuppressesFileChange)
{
    // Arrange
    const QString path = createWatchableFile(tmpDir, QStringLiteral("rot.png"));
    watcher->addImageFile(watchUrl(path));
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act：登记旋转处理后触发文件变更
    watcher->recordRotateImage(path);
    watcher->onImageFileChanged(path);

    // Assert：旋转中的变更被忽略（不发信号、不重载）
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(reloadCount, 0);
}

TEST_F(ImageFileWatcherTest, ClearRotateStatus_TrackedPath_ChangeProcessedAgain)
{
    // Arrange：进入旋转态并确认变更被抑制
    const QString path = createWatchableFile(tmpDir, QStringLiteral("rot.png"));
    watcher->addImageFile(watchUrl(path));
    watcher->recordRotateImage(path);
    watcher->onImageFileChanged(path);
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act：清除旋转标记后再次触发变更
    watcher->clearRotateStatus(path);
    watcher->onImageFileChanged(path);

    // Assert：恢复正常通知
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(reloadCount, 1);
}

TEST_F(ImageFileWatcherTest, ClearRotateStatus_UntrackedPath_SafeNoOp)
{
    // Arrange：从未登记过的路径（负面：else 分支）
    watcher->clearRotateStatus(tmpDir.filePath(QStringLiteral("unknown.png")));
    const QString path = createWatchableFile(tmpDir, QStringLiteral("a.png"));
    watcher->addImageFile(watchUrl(path));
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act
    watcher->onImageFileChanged(path);

    // Assert：观察器功能不受影响
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(setSourceCount, 1);
}

// ── onImageFileChanged ────────────────────────────────────────────

TEST_F(ImageFileWatcherTest, OnImageFileChanged_CachedExistingFile_EmitsAndReloadsInfo)
{
    // Arrange
    const QString path = createWatchableFile(tmpDir, QStringLiteral("ok.png"));
    watcher->addImageFile(watchUrl(path));
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act：文件仍然存在（普通变更）
    watcher->onImageFileChanged(path);

    // Assert：发信号 + 走 ImageInfo 清理重载链 + 观察保持（计数先断言，避免越界访问）
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), path);
    EXPECT_EQ(clearCacheCount, 1);
    EXPECT_EQ(reloadCount, 1);
    EXPECT_TRUE(watcher->fileWatcher->files().contains(path));
}

TEST_F(ImageFileWatcherTest, OnImageFileChanged_UncachedFile_IgnoresChange)
{
    // Arrange
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act：未加入缓存的文件（负面）
    watcher->onImageFileChanged(tmpDir.filePath(QStringLiteral("stranger.png")));

    // Assert：完全无动作
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(reloadCount, 0);
}

TEST_F(ImageFileWatcherTest, OnImageFileChanged_RotatingFile_ChangeIgnored)
{
    // Arrange
    const QString path = createWatchableFile(tmpDir, QStringLiteral("spin.png"));
    watcher->addImageFile(watchUrl(path));
    watcher->recordRotateImage(path);
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act：旋转中的文件变更（B1 早退）
    watcher->onImageFileChanged(path);

    // Assert：旋转分支优先于缓存分支
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(setSourceCount, 0);
}

TEST_F(ImageFileWatcherTest, OnImageFileChanged_CachedFileDeleted_StillEmitsAndKeepsWatch)
{
    // Arrange
    const QString path = createWatchableFile(tmpDir, QStringLiteral("del.png"));
    watcher->addImageFile(watchUrl(path));
    ASSERT_TRUE(QFile::remove(path));
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act：文件已被删除（B2：记录 removedFile）
    watcher->onImageFileChanged(path);

    // Assert：删除场景仍发信号并走重载链（记录移除由 OnImageDirChanged 用例验证）
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(reloadCount, 1);
    EXPECT_TRUE(watcher->fileWatcher->files().contains(path));  // 观察项未摘除
}

// ── onImageDirChanged ─────────────────────────────────────────────

TEST_F(ImageFileWatcherTest, OnImageDirChanged_RemovedFileRestored_RewatchedAndEmits)
{
    // Arrange：文件删除 → onImageFileChanged 记入 removedFile → 文件恢复
    ScopedDebugLogSuppressor suppressor;  // D1：erase 后解引用 end() 的 UB 防护
    const QString path = createWatchableFile(tmpDir, QStringLiteral("res.png"));
    watcher->addImageFile(watchUrl(path));
    ASSERT_TRUE(QFile::remove(path));
    watcher->onImageFileChanged(path);
    createWatchableFile(tmpDir, QStringLiteral("res.png"));
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act：目录变更扫描发现被移除文件已恢复
    watcher->onImageDirChanged(tmpDir.path());

    // Assert：重新观察 + 转发文件变更 + 记录被消费（二次扫描不再发信号）
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(watcher->fileWatcher->files().contains(path));
    watcher->onImageDirChanged(tmpDir.path());
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ImageFileWatcherTest, OnImageDirChanged_StillMissingFile_RecordRetainedSilently)
{
    // Arrange：文件删除后保持缺失
    const QString path = createWatchableFile(tmpDir, QStringLiteral("gone.png"));
    watcher->addImageFile(watchUrl(path));
    ASSERT_TRUE(QFile::remove(path));
    watcher->onImageFileChanged(path);
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act：目录变更扫描（else 分支：文件名不在目录中）
    watcher->onImageDirChanged(tmpDir.path());

    // Assert：静默保留移除记录；文件恢复后再次扫描仍能触发（记录未丢失）
    EXPECT_EQ(spy.count(), 0);
    ScopedDebugLogSuppressor suppressor;  // D1：下方恢复路径含 erase
    createWatchableFile(tmpDir, QStringLiteral("gone.png"));
    watcher->onImageDirChanged(tmpDir.path());
    EXPECT_EQ(spy.count(), 1);
}

// ── resetImageFiles ───────────────────────────────────────────────

TEST_F(ImageFileWatcherTest, ResetImageFiles_ExistingPaths_WatchesFilesAndParentDirectory)
{
    // Arrange
    const QString first = createWatchableFile(tmpDir, QStringLiteral("a.png"));
    const QString second = createWatchableFile(tmpDir, QStringLiteral("b.png"));

    // Act
    watcher->resetImageFiles({watchUrl(first), watchUrl(second)});

    // Assert：两个文件与所在父目录均被观察
    EXPECT_TRUE(watcher->fileWatcher->files().contains(first));
    EXPECT_TRUE(watcher->fileWatcher->files().contains(second));
    EXPECT_EQ(watcher->fileWatcher->directories().size(), 1);
    EXPECT_TRUE(watcher->fileWatcher->directories().contains(tmpDir.path()));
}

TEST_F(ImageFileWatcherTest, ResetImageFiles_EmptyList_UnwatchesAllPaths)
{
    // Arrange：先建立观察
    const QString path = createWatchableFile(tmpDir, QStringLiteral("a.png"));
    watcher->resetImageFiles({watchUrl(path)});
    ASSERT_EQ(watcher->fileWatcher->files().size(), 1);

    // Act：空列表（边界）
    watcher->resetImageFiles({});

    // Assert：文件与目录观察全部摘除
    EXPECT_EQ(watcher->fileWatcher->files().size(), 0);
    EXPECT_EQ(watcher->fileWatcher->directories().size(), 0);
}

TEST_F(ImageFileWatcherTest, ResetImageFiles_SameDirectoryAgain_SkipsRewatchButClearsCache)
{
    // Arrange：URL 形态建立观察（内部归一为本地路径后 addPath）
    const QString path = createWatchableFile(tmpDir, QStringLiteral("a.png"));
    watcher->resetImageFiles({watchUrl(path)});
    QSignalSpy spy(watcher, &ImageFileWatcher::imageFileChanged);

    // Act：同目录重复重置（B4：isCurrentDir 以本地路径判定命中，直接返回）
    watcher->resetImageFiles({path});

    // Assert：观察列表不重复；但缓存记录已被清空（该路径变更不再发信号）
    EXPECT_EQ(watcher->fileWatcher->files().size(), 1);
    EXPECT_EQ(watcher->fileWatcher->directories().size(), 1);
    watcher->onImageFileChanged(path);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(ImageFileWatcherTest, ResetImageFiles_MixedExistence_OnlyExistingFilesWatched)
{
    // Arrange
    const QString existing = createWatchableFile(tmpDir, QStringLiteral("real.png"));
    const QString ghost = tmpDir.filePath(QStringLiteral("ghost.png"));

    // Act：存在与不存在路径混合（均以 URL 形态传入）
    watcher->resetImageFiles({watchUrl(ghost), watchUrl(existing)});

    // Assert：仅存在文件进入观察，父目录以首个被观察文件为准
    EXPECT_EQ(watcher->fileWatcher->files().size(), 1);
    EXPECT_TRUE(watcher->fileWatcher->files().contains(existing));
    EXPECT_TRUE(watcher->fileWatcher->directories().contains(tmpDir.path()));
}
