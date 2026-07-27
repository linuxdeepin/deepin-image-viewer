// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_rotateimagehelper.h"
#include "rotateimagehelper.h"

#include <QImage>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QTest>

void ut_rotateimagehelper::SetUp()
{
    // 重置单例旋转状态，保证用例间隔离
    RotateImageHelper::instance()->resetRotateState();
}

void ut_rotateimagehelper::TearDown() {}

// ==================== instance() ====================

// 测试 instance() 返回非空单例
TEST_F(ut_rotateimagehelper, Instance_ReturnsNonNull)
{
    RotateImageHelper *inst = RotateImageHelper::instance();
    EXPECT_NE(inst, nullptr);
}

// 测试 instance() 多次调用返回同一单例
TEST_F(ut_rotateimagehelper, Instance_ReturnsSameSingleton)
{
    RotateImageHelper *inst1 = RotateImageHelper::instance();
    RotateImageHelper *inst2 = RotateImageHelper::instance();
    EXPECT_EQ(inst1, inst2);
}

// ==================== rotateImageFile ====================

// 测试 0 度旋转被跳过（提前返回）
TEST_F(ut_rotateimagehelper, RotateImageFile_ZeroAngle_ReturnsEarly)
{
    RotateImageHelper::instance()->rotateImageFile("/tmp/ut_rotate_zero.png", 0);
    SUCCEED();
}

// 测试 360 度等同于 0 度，被跳过
TEST_F(ut_rotateimagehelper, RotateImageFile_FullRotation_ReturnsEarly)
{
    RotateImageHelper::instance()->rotateImageFile("/tmp/ut_rotate_360.png", 360);
    SUCCEED();
}

// 测试对真实文件执行旋转，最终发送 rotateImageFinished 信号
TEST_F(ut_rotateimagehelper, RotateImageFile_RealFile_EmitsFinishedSignal)
{
    // 准备临时图片文件
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString srcPath = tmpDir.filePath("rotate_src.png");
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::red);
    ASSERT_TRUE(img.save(srcPath, "PNG"));

    QSignalSpy spy(RotateImageHelper::instance(), &RotateImageHelper::rotateImageFinished);

    RotateImageHelper::instance()->rotateImageFile(srcPath, 90);

    // 等待异步任务完成（最多 3 秒）
    ASSERT_TRUE(spy.wait(3000));
    ASSERT_GE(spy.count(), 1);

    // 验证信号参数：path 和 ret
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), srcPath);
    EXPECT_TRUE(args.at(1).toBool());
}

// ==================== resetRotateState ====================

// 测试在单例上调用 resetRotateState 不崩溃
TEST_F(ut_rotateimagehelper, ResetRotateState_OnSingleton_NoCrash)
{
    RotateImageHelper::instance()->resetRotateState();
    SUCCEED();
}

// 测试 data 为 null 时调用 resetRotateState 不崩溃（借助 -fno-access-control 构造私有构造函数）
TEST_F(ut_rotateimagehelper, ResetRotateState_NullData_NoCrash)
{
    // 直接构造新实例（构造函数私有，-fno-access-control 允许访问）
    RotateImageHelper *fresh = new RotateImageHelper();
    // data 此时为 null
    fresh->resetRotateState();
    delete fresh;
    SUCCEED();
}

// 测试 data 已初始化时调用 resetRotateState 不崩溃
// (RotateImageHelperData 在 .cpp 中定义，无法直接访问其成员，
//  此处验证 watcher 未运行时 resetRotateState 的 cacheDir.remove() 分支)
TEST_F(ut_rotateimagehelper, ResetRotateState_WithDataInitialized_NoCrash)
{
    RotateImageHelper *fresh = new RotateImageHelper();
    fresh->checkDataValid();
    fresh->resetRotateState();  // watcher 未运行，触发 cacheDir.remove() 分支
    delete fresh;
    SUCCEED();
}

// ==================== rotateImageImpl (static) ====================

// 测试源文件不存在时 rotateImageImpl 返回 false（copy 失败）
TEST_F(ut_rotateimagehelper, RotateImageImpl_NonExistentSource_ReturnsFalse)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString cachePath = tmpDir.filePath("nonexistent_cache.png");
    QString sourcePath = tmpDir.filePath("nonexistent_source.png");
    QFile::remove(cachePath);
    QFile::remove(sourcePath);
    ASSERT_FALSE(QFileInfo::exists(sourcePath));

    bool ret = RotateImageHelper::rotateImageImpl(cachePath, sourcePath, 90);
    EXPECT_FALSE(ret);
}

// 测试对真实文件旋转成功
TEST_F(ut_rotateimagehelper, RotateImageImpl_ValidFile_ReturnsTrue)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString sourcePath = tmpDir.filePath("valid_source.png");
    QString cachePath = tmpDir.filePath("valid_cache.png");
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::blue);
    ASSERT_TRUE(img.save(sourcePath, "PNG"));
    QFile::remove(cachePath);
    ASSERT_FALSE(QFileInfo::exists(cachePath));

    QSignalSpy spy(RotateImageHelper::instance(), &RotateImageHelper::rotateImageFinished);

    bool ret = RotateImageHelper::rotateImageImpl(cachePath, sourcePath, 90);
    EXPECT_TRUE(ret);

    // 等待 queued 信号送达（rotateImageImpl 发射 3 个信号）
    spy.wait(500);
}

// 测试 cachePath 已存在时跳过拷贝直接旋转
TEST_F(ut_rotateimagehelper, RotateImageImpl_CacheExists_SkipsCopy)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString sourcePath = tmpDir.filePath("skip_copy_source.png");
    QString cachePath = tmpDir.filePath("skip_copy_cache.png");

    // 同时创建 source 和 cache
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::green);
    ASSERT_TRUE(img.save(sourcePath, "PNG"));
    ASSERT_TRUE(img.save(cachePath, "PNG"));
    ASSERT_TRUE(QFileInfo::exists(cachePath));

    QSignalSpy spy(RotateImageHelper::instance(), &RotateImageHelper::rotateImageFinished);

    bool ret = RotateImageHelper::rotateImageImpl(cachePath, sourcePath, 90);
    EXPECT_TRUE(ret);

    spy.wait(500);
}

// ==================== checkDataValid (private) ====================

// 测试 checkDataValid 在新实例上创建 data
TEST_F(ut_rotateimagehelper, CheckDataValid_NewInstance_CreatesData)
{
    RotateImageHelper *fresh = new RotateImageHelper();
    EXPECT_EQ(fresh->data.data(), nullptr);

    fresh->checkDataValid();

    // data 指针应为非空（RotateImageHelperData 为不完整类型，仅比较指针）
    EXPECT_NE(fresh->data.data(), nullptr);
    delete fresh;
}

// 测试 checkDataValid 在已初始化实例上不重复创建
TEST_F(ut_rotateimagehelper, CheckDataValid_AlreadyValid_NoDuplicateCreation)
{
    RotateImageHelper *fresh = new RotateImageHelper();
    fresh->checkDataValid();
    auto dataPtr = fresh->data.data();

    fresh->checkDataValid();
    // 指针应未变（未重新创建）
    EXPECT_EQ(fresh->data.data(), dataPtr);
    delete fresh;
}

// ==================== enqueueRotateTask (private) ====================

// 测试 enqueueRotateTask 将任务加入队列并异步处理
TEST_F(ut_rotateimagehelper, EnqueueRotateTask_QueuesAndProcesses)
{
    RotateImageHelper *fresh = new RotateImageHelper();
    fresh->checkDataValid();

    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString srcPath = tmpDir.filePath("enqueue_test.png");
    QImage img(2, 2, QImage::Format_ARGB32);
    img.fill(Qt::yellow);
    ASSERT_TRUE(img.save(srcPath, "PNG"));

    QSignalSpy spy(RotateImageHelper::instance(), &RotateImageHelper::rotateImageFinished);

    fresh->enqueueRotateTask(srcPath, 90);

    // 等待 rotateImageFinished 信号（由 rotateImageImpl 在子线程中发射）
    ASSERT_TRUE(spy.wait(3000));
    EXPECT_GE(spy.count(), 1);

    // 给后台任务额外时间完成循环退出（无法直接访问 RotateImageHelperData::watcher）
    QTest::qSleep(200);

    delete fresh;
    SUCCEED();
}
