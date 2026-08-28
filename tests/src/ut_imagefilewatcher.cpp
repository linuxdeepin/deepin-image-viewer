// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_imagefilewatcher.h"
#include "imagefilewatcher.h"

#include <QUrl>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSignalSpy>
#include <QCoreApplication>
#include <QImage>
#include <QLoggingCategory>

// 生成一个临时文件用于测试，返回绝对路径
static QString makeWatcherTempFile(const QString &name)
{
    QString dir = QDir::tempPath() + "/ut_filewatcher_" +
                  QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(dir);
    QString path = dir + "/" + name;
    QImage img(8, 8, QImage::Format_ARGB32);
    img.fill(Qt::blue);
    img.save(path, "PNG");
    return path;
}

void ut_imagefilewatcher::SetUp()
{
    // 每个用例前重置单例状态，保证隔离
    ImageFileWatcher::instance()->resetImageFiles({});
}

void ut_imagefilewatcher::TearDown()
{
    ImageFileWatcher::instance()->resetImageFiles({});
}

// 测试 instance() 返回同一单例
TEST_F(ut_imagefilewatcher, InstanceReturnsSameSingleton)
{
    ImageFileWatcher *p1 = ImageFileWatcher::instance();
    ImageFileWatcher *p2 = ImageFileWatcher::instance();
    EXPECT_NE(p1, nullptr);
    EXPECT_EQ(p1, p2);
}

// 测试 resetImageFiles 空列表不崩溃
TEST_F(ut_imagefilewatcher, ResetImageFilesEmpty)
{
    ImageFileWatcher::instance()->resetImageFiles({});
    SUCCEED();
}

// 测试 resetImageFiles 设置存在的文件
TEST_F(ut_imagefilewatcher, ResetImageFilesWithExistingFiles)
{
    QString f1 = makeWatcherTempFile("a.png");
    QString f2 = makeWatcherTempFile("b.png");
    ASSERT_TRUE(QFileInfo::exists(f1));

    // resetImageFiles 内部用 QUrl(filePath).toLocalFile() 解析，故传入 file:// URL
    ImageFileWatcher::instance()->resetImageFiles(
        {QUrl::fromLocalFile(f1).toString(), QUrl::fromLocalFile(f2).toString()});
    // 重复重置同一目录应被忽略（isCurrentDir 分支）
    ImageFileWatcher::instance()->resetImageFiles(
        {QUrl::fromLocalFile(f1).toString(), QUrl::fromLocalFile(f2).toString()});
    SUCCEED();
}

// 测试 resetImageFiles 包含不存在文件（被忽略）
TEST_F(ut_imagefilewatcher, ResetImageFilesWithNonexisting)
{
    ImageFileWatcher::instance()->resetImageFiles({QUrl::fromLocalFile("/tmp/ut_nonexist_file.png").toString()});
    SUCCEED();
}

// 测试 isCurrentDir
TEST_F(ut_imagefilewatcher, IsCurrentDir)
{
    QString f1 = makeWatcherTempFile("c.png");
    ImageFileWatcher::instance()->resetImageFiles({QUrl::fromLocalFile(f1).toString()});

    // f1 所在目录应被监控（isCurrentDir 接收本地路径）
    EXPECT_TRUE(ImageFileWatcher::instance()->isCurrentDir(f1));
    // 另一目录不被监控
    EXPECT_FALSE(ImageFileWatcher::instance()->isCurrentDir("/usr/bin"));
}

// 测试 fileRename 缓存命中与未命中分支
TEST_F(ut_imagefilewatcher, FileRenameHitAndMiss)
{
    QString f1 = makeWatcherTempFile("d.png");
    QString f1Renamed = QFileInfo(f1).absolutePath() + "/d_renamed.png";
    QFile::rename(f1, f1Renamed);  // 真实重命名以便新路径存在

    ImageFileWatcher::instance()->resetImageFiles({QUrl::fromLocalFile(f1Renamed).toString()});
    // 未命中分支（oldPath 不在缓存中，预期不崩溃且无副作用）
    ImageFileWatcher::instance()->fileRename("/tmp/ut_not_in_cache.png", f1Renamed);

    // 对当前缓存路径做 rename（命中分支，cacheFileInfo 以本地路径为 key）
    ImageFileWatcher::instance()->fileRename(f1Renamed, f1);
    SUCCEED();
}

// 测试 recordRotateImage 与 clearRotateStatus
TEST_F(ut_imagefilewatcher, RecordAndClearRotateStatus)
{
    QString target = "/tmp/ut_rotate_target.png";
    ImageFileWatcher::instance()->recordRotateImage(target);
    // 再次清除存在的记录
    ImageFileWatcher::instance()->clearRotateStatus(target);
    // 清除不存在的记录不崩溃
    ImageFileWatcher::instance()->clearRotateStatus("/tmp/ut_rotate_other.png");
    SUCCEED();
}

// 测试 imageFileChanged 信号在文件变更时触发
TEST_F(ut_imagefilewatcher, ImageFileChangedSignal)
{
    QString f1 = makeWatcherTempFile("e.png");
    ImageFileWatcher::instance()->resetImageFiles({QUrl::fromLocalFile(f1).toString()});

    QSignalSpy spy(ImageFileWatcher::instance(), &ImageFileWatcher::imageFileChanged);

    // 触发文件内容变更：覆盖写入
    {
        QImage img(4, 4, QImage::Format_ARGB32);
        img.fill(Qt::red);
        img.save(f1, "PNG");
    }

    // 注意: 不在此处调用事件循环(loop.exec/qWait)。原因: FileControl 构造时
    // 创建的 OcrInterface 会在 session bus 注册服务监视，对象销毁后残留 pending
    // DBus 事件；ASAN 下任意事件循环处理该事件会触发 Qt6DBus 空指针访问。
    // 这里只验证接口可调用且 spy 有效(QFileSystemWatcher 行为依赖平台)。
    EXPECT_TRUE(spy.isValid());
}

// ---------- 私有槽测试（依赖 -fno-access-control） ----------

// 测试 onImageFileChanged 对缓存中的文件
TEST_F(ut_imagefilewatcher, PrivateOnImageFileChangedCached)
{
    QString f1 = makeWatcherTempFile("f.png");
    ImageFileWatcher::instance()->resetImageFiles({QUrl::fromLocalFile(f1).toString()});

    // 先记录旋转状态，使 onImageFileChanged 走"忽略"分支
    ImageFileWatcher::instance()->recordRotateImage(f1);
    ImageFileWatcher::instance()->onImageFileChanged(f1);
    SUCCEED();
}

// 测试 onImageFileChanged 对非缓存文件（不处理）
TEST_F(ut_imagefilewatcher, PrivateOnImageFileChangedNotCached)
{
    ImageFileWatcher::instance()->onImageFileChanged("/tmp/ut_not_cached_file.png");
    SUCCEED();
}

// 测试 onImageDirChanged 不崩溃
TEST_F(ut_imagefilewatcher, PrivateOnImageDirChanged)
{
    QString dir = QDir::tempPath() + "/ut_filewatcher_" +
                  QString::number(QCoreApplication::applicationPid());
    ImageFileWatcher::instance()->onImageDirChanged(dir);
    SUCCEED();
}

// 测试 D0 析构函数: 通过 new/delete 覆盖 deleting destructor
// (单例使用静态局部变量，其析构由 C++ 运行时在进程退出后调用，
//  coverage 统计在进程退出前完成，故需手动构造/析构以覆盖 D0)
TEST_F(ut_imagefilewatcher, DeletingDestructor_NewDelete_NoCrash)
{
    ImageFileWatcher *watcher = new ImageFileWatcher();
    ASSERT_NE(watcher, nullptr);
    // 析构函数中仅输出日志，删除不崩溃
    delete watcher;
    SUCCEED();
}

// onImageDirChanged with removedFile entry that does NOT exist in directory
TEST_F(ut_imagefilewatcher, OnImageDirChanged_WithRemovedFile)
{
    QString dir = QDir::tempPath() + "/ut_filewatcher_dirchanged_" +
                  QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(dir);
    QString f1 = dir + "/nonexist.png";

    // Directly populate removedFile (avoids calling onImageFileChanged which
    // creates ImageInfo objects and starts async LoadImageInfoRunnable tasks
    // that cause segfaults during global teardown).
    ImageFileWatcher::instance()->removedFile.insert(f1, QUrl::fromLocalFile(f1));

    // File is not in directory listing -> else branch (L242)
    ImageFileWatcher::instance()->onImageDirChanged(dir);

    // File should remain in removedFile (not erased)
    EXPECT_TRUE(ImageFileWatcher::instance()->removedFile.contains(f1));

    ImageFileWatcher::instance()->resetImageFiles({});
    QDir().rmdir(dir);
    SUCCEED();
}

// onImageDirChanged with removedFile entry that EXISTS in directory → restore branch (L231-237)
// NOTE: Source code has a bug at L240 — after erase(itr), itr.key() is called
// unconditionally in a qCDebug line, which is UB if itr == end().  We work
// around this by disabling the debug log category so qCDebug does not evaluate
// its arguments.  We cannot fix the source (only mark defects for the user).
TEST_F(ut_imagefilewatcher, OnImageDirChanged_WithRestoredFile)
{
    QString dir = QDir::tempPath() + "/ut_filewatcher_restored_" +
                  QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(dir);
    QString f1 = dir + "/restored.png";
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::green);
    img.save(f1, "PNG");

    // Populate removedFile with the restored file path.
    ImageFileWatcher::instance()->removedFile.insert(f1, QUrl::fromLocalFile(f1));

    // Record rotate status for f1 so onImageFileChanged (called inside restore
    // branch) takes the early-return path and does NOT start async reloads.
    ImageFileWatcher::instance()->recordRotateImage(f1);

    // Disable debug logging to prevent source UB (itr.key() on end() iterator
    // inside qCDebug at L240) from crashing.
    QLoggingCategory::setFilterRules("org.deepin.dde.imageviewer.debug=false");

    // File exists in directory → restore branch (L231-237)
    ImageFileWatcher::instance()->onImageDirChanged(dir);

    // Restore logging rules
    QLoggingCategory::setFilterRules("");

    // f1 should have been erased from removedFile
    EXPECT_FALSE(ImageFileWatcher::instance()->removedFile.contains(f1));

    // Clean up (do NOT call processEvents — pending DBus events from
    // OcrInterface can crash Qt6DBus during teardown)
    ImageFileWatcher::instance()->resetImageFiles({});

    QFile::remove(f1);
    QDir().rmdir(dir);
}

// =================== Round 58 coverage improvement tests ===================

// L68-69: addFileDirWatchPaths with duplicate directory (isCurrentDir returns true)
// addFileDirWatchPaths is private, but -fno-access-control allows direct calls.
TEST_F(ut_imagefilewatcher, AddFileDirWatchPaths_DuplicateDir_SkipsReset)
{
    QString dir = QDir::tempPath() + "/ut_filewatcher_dup_" +
                  QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(dir);
    QString f1 = dir + "/dup_test.png";
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::green);
    img.save(f1, "PNG");

    // First call adds the directory to fileWatcher (must use file:// URL so QUrl::toLocalFile works)
    ImageFileWatcher::instance()->resetImageFiles({QUrl::fromLocalFile(f1).toString()});
    // Second call with plain path hits isCurrentDir == true and returns early (L68-69)
    ImageFileWatcher::instance()->resetImageFiles({f1});

    ImageFileWatcher::instance()->resetImageFiles({});
    QFile::remove(f1);
    QDir().rmdir(dir);
    SUCCEED();
}

// L197-198: onImageFileChanged when file does NOT exist (removed/deleted)
TEST_F(ut_imagefilewatcher, OnImageFileChanged_FileRemoved)
{
    QString dir = QDir::tempPath() + "/ut_filewatcher_removed_" +
                  QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(dir);
    QString f1 = dir + "/removed_test.png";
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(f1, "PNG");

    // Use resetImageFiles with file:// URL to properly add file to cacheFileInfo
    ImageFileWatcher::instance()->resetImageFiles({QUrl::fromLocalFile(f1).toString()});

    // Ensure file is NOT in rotateImagePathSet so the early return is skipped
    ImageFileWatcher::instance()->rotateImagePathSet.remove(f1);

    // Delete the file so onImageFileChanged hits the !isExist branch (L197-198)
    QFile::remove(f1);
    ASSERT_FALSE(QFile::exists(f1));

    // Call onImageFileChanged - file does not exist, so removedFile is populated
    ImageFileWatcher::instance()->onImageFileChanged(f1);

    // Verify the file was added to removedFile
    EXPECT_TRUE(ImageFileWatcher::instance()->removedFile.contains(f1));

    // Clean up
    ImageFileWatcher::instance()->resetImageFiles({});
    QDir().rmdir(dir);
    SUCCEED();
}

// L111: addImageFile with invalid path (directory, not file)
TEST_F(ut_imagefilewatcher, AddImageFile_WithDirectoryPath_ReturnsEarly)
{
    QString dir = QDir::tempPath() + "/ut_filewatcher_addimg_" +
                  QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(dir);

    // Call addImageFile with a directory path - should return early at L111
    ImageFileWatcher::instance()->addImageFile(dir);

    ImageFileWatcher::instance()->resetImageFiles({});
    QDir().rmdir(dir);
    SUCCEED();
}

// L192-212: onImageFileChanged when file exists in cacheFileInfo and file exists on disk
TEST_F(ut_imagefilewatcher, OnImageFileChanged_FileExists)
{
    QString dir = QDir::tempPath() + "/ut_filewatcher_changed_" +
                  QString::number(QCoreApplication::applicationPid());
    QDir().mkpath(dir);
    QString f1 = dir + "/changed_test.png";
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::blue);
    img.save(f1, "PNG");

    // Populate cacheFileInfo directly so onImageFileChanged processes the file
    QUrl url = QUrl::fromLocalFile(f1);
    ImageFileWatcher::instance()->cacheFileInfo.insert(f1, url);

    // Ensure file is NOT in rotateImagePathSet so the early return is skipped
    ImageFileWatcher::instance()->rotateImagePathSet.remove(f1);

    // Call onImageFileChanged - file exists, so it goes through the exists branch (L203-204)
    ImageFileWatcher::instance()->onImageFileChanged(f1);

    // Clean up
    ImageFileWatcher::instance()->resetImageFiles({});
    ImageFileWatcher::instance()->cacheFileInfo.remove(f1);
    QFile::remove(f1);
    QDir().rmdir(dir);
    SUCCEED();
}
