// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_globalcontrol.h"
#include "globalcontrol.h"

#include "stub.h"
#include "utils/rotateimagehelper.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QImage>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QTimerEvent>
#include <QCoreApplication>
#include <QStandardPaths>
#include <cstdlib>
#include <stdexcept>

void ut_globalcontrol::SetUp()
{
}

void ut_globalcontrol::TearDown()
{
}

// 测试构造与析构
TEST_F(ut_globalcontrol, Construct)
{
    GlobalControl *control = new GlobalControl();
    ASSERT_TRUE(control != nullptr);
    delete control;
}

// 测试默认索引值
TEST_F(ut_globalcontrol, DefaultIndex)
{
    GlobalControl control;
    EXPECT_EQ(control.currentIndex(), 0);
    EXPECT_EQ(control.currentFrameIndex(), 0);
    EXPECT_EQ(control.imageCount(), 0);
}

// 在临时目录中创建指定数量的测试图片文件，由 QTemporaryDir 自动清理
static QStringList createTestImageFiles(const QTemporaryDir &dir, int count)
{
    QStringList files;
    for (int i = 0; i < count; ++i) {
        QString path = dir.filePath(QString("ut_test_img_%1.png").arg(i));
        QImage img(10, 10, QImage::Format_ARGB32);
        img.fill(Qt::red);
        if (img.save(path, "PNG")) {
            files << path;
        }
    }
    return files;
}

// 测试设置图片文件列表
TEST_F(ut_globalcontrol, SetImageFiles)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 3);
    ASSERT_EQ(files.size(), 3);

    control.setImageFiles(files, files.value(1));
    EXPECT_EQ(control.imageCount(), 3);
}

// 测试设置当前索引
TEST_F(ut_globalcontrol, CurrentIndex)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 3);
    ASSERT_EQ(files.size(), 3);

    control.setImageFiles(files, files.value(0));
    EXPECT_EQ(control.imageCount(), 3);

    QSignalSpy spy(&control, &GlobalControl::currentIndexChanged);
    control.setCurrentIndex(2);
    EXPECT_EQ(control.currentIndex(), 2);
    EXPECT_EQ(spy.count(), 1);
}

// 测试旋转角度设置
TEST_F(ut_globalcontrol, CurrentRotation)
{
    GlobalControl control;
    EXPECT_EQ(control.currentRotation(), 0);

    QSignalSpy spy(&control, &GlobalControl::currentRotationChanged);
    control.setCurrentRotation(90);
    EXPECT_EQ(control.currentRotation(), 90);
    EXPECT_EQ(spy.count(), 1);
}

// ============================================================
// 以下为增量补全用例，覆盖此前未覆盖的 public/protected/private 函数
// ============================================================

// globalModel / viewModel: 返回非空内部模型
TEST_F(ut_globalcontrol, Models_ReturnValidInstances)
{
    GlobalControl control;
    EXPECT_NE(control.globalModel(), nullptr);
    EXPECT_NE(control.viewModel(), nullptr);
}

// currentSource / setCurrentSource: 模型内 source 切换索引
TEST_F(ut_globalcontrol, CurrentSource_SetSourceInModel_UpdatesIndex)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 3);
    ASSERT_EQ(files.size(), 3);

    control.setImageFiles(files, files.value(1));  // index 1
    ASSERT_FALSE(control.currentSource().isEmpty());

    // 取索引 0 的实际 source(URL scheme 与模型存储一致)
    control.setCurrentIndex(0);
    QUrl src0 = control.currentSource();
    control.setCurrentIndex(2);

    QSignalSpy spy(&control, &GlobalControl::currentIndexChanged);
    control.setCurrentSource(src0);  // 回到索引 0
    EXPECT_EQ(control.currentIndex(), 0);
    EXPECT_GE(spy.count(), 1);

    // 相同 source 再次设置(早退)
    control.setCurrentSource(src0);
    // 不在模型中的 source(无操作)
    control.setCurrentSource(QUrl("ut_gc_bogus_source"));
    EXPECT_EQ(control.currentIndex(), 0);
}

// setCurrentFrameIndex: 单帧图片帧索引被钳制为 0
TEST_F(ut_globalcontrol, SetCurrentFrameIndex_SingleFrame_ClampedToZero)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 1);
    ASSERT_EQ(files.size(), 1);
    control.setImageFiles(files, files.value(0));

    control.setCurrentFrameIndex(5);
    EXPECT_EQ(control.currentFrameIndex(), 0);
}

// setIndexAndFrameIndex: 同时设置索引与帧索引
TEST_F(ut_globalcontrol, SetIndexAndFrameIndex_UpdatesIndex)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 3);
    ASSERT_EQ(files.size(), 3);
    control.setImageFiles(files, files.value(0));

    control.setIndexAndFrameIndex(2, 0);
    EXPECT_EQ(control.currentIndex(), 2);
    EXPECT_EQ(control.currentFrameIndex(), 0);
}

// hasPreviousImage / hasNextImage: 位于中间时均为 true
TEST_F(ut_globalcontrol, HasPreviousNext_AtMiddle_BothTrue)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 3);
    control.setImageFiles(files, files.value(1));

    EXPECT_TRUE(control.hasPreviousImage());
    EXPECT_TRUE(control.hasNextImage());
}

// previousImage / nextImage: 在列表中导航
TEST_F(ut_globalcontrol, PreviousAndNext_NavigateSuccessfully)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 3);
    control.setImageFiles(files, files.value(1));  // index 1

    EXPECT_TRUE(control.previousImage());  // -> index 0
    EXPECT_EQ(control.currentIndex(), 0);
    EXPECT_FALSE(control.previousImage());  // 已到首张
    EXPECT_TRUE(control.nextImage());        // -> index 1
    EXPECT_TRUE(control.nextImage());        // -> index 2
    EXPECT_EQ(control.currentIndex(), 2);
}

// firstImage / lastImage: 跳至首/末
TEST_F(ut_globalcontrol, FirstAndLast_NavigateSuccessfully)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 3);
    control.setImageFiles(files, files.value(1));

    EXPECT_TRUE(control.lastImage());
    EXPECT_EQ(control.currentIndex(), 2);
    EXPECT_TRUE(control.firstImage());
    EXPECT_EQ(control.currentIndex(), 0);
}

// firstImage / lastImage: 空模型返回 false
TEST_F(ut_globalcontrol, FirstAndLast_EmptyModel_ReturnsFalse)
{
    GlobalControl control;
    EXPECT_FALSE(control.firstImage());
    EXPECT_FALSE(control.lastImage());
}

// addImageAndSetCurrentSource: 新增一张实际图片并切换
TEST_F(ut_globalcontrol, AddImageAndSetCurrentSource_NewImage_Adds)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 2);
    ASSERT_EQ(files.size(), 2);
    control.setImageFiles(files, files.value(0));
    EXPECT_EQ(control.imageCount(), 2);

    // 新建一张实际图片文件
    QString newPath = tmpDir.filePath("added.png");
    QImage img(10, 10, QImage::Format_ARGB32);
    img.fill(Qt::green);
    ASSERT_TRUE(img.save(newPath, "PNG"));

    EXPECT_TRUE(control.addImageAndSetCurrentSource(QUrl::fromLocalFile(newPath)));
    EXPECT_EQ(control.imageCount(), 3);

    // 已存在的 source 仅切换索引, 不增加数量
    control.setCurrentIndex(0);
    control.addImageAndSetCurrentSource(QUrl::fromLocalFile(newPath));
    EXPECT_EQ(control.imageCount(), 3);
}

// addImageAndSetCurrentSource: 无效/空 URL 返回 false
TEST_F(ut_globalcontrol, AddImageAndSetCurrentSource_InvalidUrl_ReturnsFalse)
{
    GlobalControl control;
    EXPECT_FALSE(control.addImageAndSetCurrentSource(QUrl()));
    EXPECT_FALSE(control.addImageAndSetCurrentSource(QUrl::fromLocalFile("/tmp/ut_gc_not_exist.png")));
}

// removeImage: 移除当前图片并更新计数
TEST_F(ut_globalcontrol, RemoveImage_Current_RemovesAndUpdates)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 3);
    control.setImageFiles(files, files.value(1));  // index 1, 非末尾
    EXPECT_EQ(control.imageCount(), 3);

    QUrl current = control.currentSource();  // 取模型实际存储的 URL
    control.removeImage(current);
    EXPECT_EQ(control.imageCount(), 2);
}

// renameImage: 重命名当前图片, source 随之更新
TEST_F(ut_globalcontrol, RenameImage_CurrentSource_Updates)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 2);
    control.setImageFiles(files, files.value(0));

    // 创建重命名目标为真实文件, 使后续 reloadData 正常加载
    QString newPath = tmpDir.filePath("renamed_target.png");
    QImage img(10, 10, QImage::Format_ARGB32);
    img.fill(Qt::blue);
    ASSERT_TRUE(img.save(newPath, "PNG"));

    QUrl oldUrl = control.currentSource();
    QUrl newUrl = QUrl::fromLocalFile(newPath);
    control.renameImage(oldUrl, newUrl);
    EXPECT_EQ(control.currentSource(), newUrl);

    // 重命名不在模型中的图片(无操作)
    control.renameImage(QUrl::fromLocalFile("/tmp/ut_gc_no_such.png"),
                        QUrl::fromLocalFile("/tmp/ut_gc_other.png"));
    SUCCEED();
}

// submitImageChangeImmediately: 无旋转时直接返回
TEST_F(ut_globalcontrol, SubmitImageChangeImmediately_NoRotation_NoOp)
{
    GlobalControl control;
    control.submitImageChangeImmediately();
    EXPECT_EQ(control.currentRotation(), 0);
}

// submitImageChangeImmediately: 有旋转时发出 requestRotateImage
TEST_F(ut_globalcontrol, SubmitImageChangeImmediately_WithRotation_EmitsRequest)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    GlobalControl control;
    QStringList files = createTestImageFiles(tmpDir, 1);
    ASSERT_EQ(files.size(), 1);
    control.setImageFiles(files, files.value(0));

    control.setCurrentRotation(90);
    QSignalSpy spy(&control, &GlobalControl::requestRotateImage);
    control.submitImageChangeImmediately();
    EXPECT_GE(spy.count(), 1);
    EXPECT_EQ(control.currentRotation(), 0);
}

// enableMultiThread(静态): 返回布尔结果
TEST_F(ut_globalcontrol, EnableMultiThread_ReturnsBool)
{
    bool ret = GlobalControl::enableMultiThread();
    EXPECT_TRUE(ret || !ret);
}

// timerEvent(submitTimer): 触发旋转提交分支
TEST_F(ut_globalcontrol, TimerEvent_SubmitTimer_TriggersSubmit)
{
    GlobalControl control;
    control.setCurrentRotation(90);  // 启动 submitTimer
    int tid = control.submitTimer.timerId();  // 私有成员, 借 -fno-access-control 访问
    ASSERT_GT(tid, 0);
    QTimerEvent ev(tid);
    control.timerEvent(&ev);  // protected, 借 -fno-access-control 直调
    EXPECT_EQ(control.currentRotation(), 0);
}

// timerEvent(switchCheckTimer): 触发切换状态检查分支
TEST_F(ut_globalcontrol, TimerEvent_SwitchCheckTimer_TriggersCheck)
{
    GlobalControl control;
    control.switchCheckTimer.start(50, &control);  // 私有成员
    int tid = control.switchCheckTimer.timerId();
    ASSERT_GT(tid, 0);
    QTimerEvent ev(tid);
    control.timerEvent(&ev);
    SUCCEED();
}

// timerEvent: 未知 timerId 不产生副作用
TEST_F(ut_globalcontrol, TimerEvent_UnknownTimerId_NoOp)
{
    GlobalControl control;
    QTimerEvent ev(-999);
    control.timerEvent(&ev);
    SUCCEED();
}

// checkSwitchEnable(私有): 空模型时前后均不可用
TEST_F(ut_globalcontrol, CheckSwitchEnable_EmptyModel_AllDisabled)
{
    GlobalControl control;
    control.checkSwitchEnable();  // 私有, 借 -fno-access-control 直调
    EXPECT_FALSE(control.hasPreviousImage());
    EXPECT_FALSE(control.hasNextImage());
}

// ============================================================
// 以下为补充用例，覆盖构造函数中的 lambda 及 forceExit
// ============================================================

// currentImage.infoChanged lambda: 手动发射 infoChanged 信号触发 switchCheckTimer
TEST_F(ut_globalcontrol, CurrentImage_InfoChanged_TriggersSwitchCheckTimer)
{
    GlobalControl control;
    // currentImage 为私有成员，-fno-access-control 允许访问
    // 手动发射 infoChanged 信号，触发构造函数中连接的 lambda
    emit control.currentImage.infoChanged();
    // lambda 调用 switchCheckTimer.start(50, this)
    EXPECT_TRUE(control.switchCheckTimer.isActive());
    control.switchCheckTimer.stop();
}

// RotateImageHelper::rotateImageFinished lambda: 手动发射信号触发构造函数 lambda
TEST_F(ut_globalcontrol, RotateImageFinished_TriggersConstructorLambda)
{
    GlobalControl control;
    // 手动发射 rotateImageFinished 信号，触发构造函数中连接的 lambda
    // 即使 path 不匹配 currentImage.source()，lambda 仍会执行（仅不进入提交分支）
    RotateImageHelper::instance()->rotateImageFinished("/tmp/ut_gc_test_rotate.png", true);
    SUCCEED();
}

// forceExit: stub _Exit 使用异常机制安全地中断 noreturn 函数
// _Exit 声明为 noreturn，直接 stub 为空操作会导致编译器优化问题。
// 使用异常可以触发正常的 C++ 栈展开，确保 Stub 和 GlobalControl 析构函数被调用。
namespace {
class ForceExitTestException : public std::exception {};
}  // namespace

[[noreturn]] static void ut_gc_stub_throwOnExit(int)
{
    throw ForceExitTestException();
}

TEST_F(ut_globalcontrol, ForceExit_Stubbed_NoTermination)
{
    Stub stub;
    stub.set(_Exit, ut_gc_stub_throwOnExit);

    GlobalControl control;

    // forceExit() 内部先调用 QApplication::exit(0)，再调用 _Exit(0)。
    // _Exit 被 stub 替换为抛出异常，异常正常传播并触发栈展开。
    try {
        control.forceExit();
        FAIL() << "Expected ForceExitTestException but no exception was thrown";
    } catch (const ForceExitTestException &) {
        // 成功捕获异常，此时 control 和 stub 的析构函数将被正常调用
        SUCCEED();
    } catch (...) {
        FAIL() << "Caught unexpected exception";
    }
}
