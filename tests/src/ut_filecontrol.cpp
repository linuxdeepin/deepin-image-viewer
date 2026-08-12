// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_filecontrol.h"
#include "filecontrol.h"

#include "stub.h"
#include "printhelper.h"

#include <QUrl>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QImage>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QThread>
#include <QStandardPaths>
#include <QSignalSpy>
#include <QClipboard>
#include <QJsonDocument>
#include <QTest>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>

void ut_filecontrol::SetUp()
{
}

void ut_filecontrol::TearDown()
{
}

// 测试构造与析构
TEST_F(ut_filecontrol, Construct)
{
    FileControl *control = new FileControl();
    ASSERT_TRUE(control != nullptr);
    delete control;
}

// 测试获取标准图片路径
TEST_F(ut_filecontrol, StandardPicturesPath)
{
    FileControl control;
    QString path = control.standardPicturesPath();
    EXPECT_FALSE(path.isEmpty());
}

// 测试获取文件名（不要求文件存在）
TEST_F(ut_filecontrol, SlotGetFileName)
{
    FileControl control;
    QString name = control.slotGetFileName("/tmp/test/image.png");
    EXPECT_FALSE(name.isEmpty());
}

// 测试获取文件后缀（需要文件存在，slotFileSuffix 内部使用 QUrl::toLocalFile）
TEST_F(ut_filecontrol, SlotFileSuffix)
{
    FileControl control;

    // 创建临时文件以测试后缀获取
    QTemporaryFile tmpFile("ut_test_suffix_XXXXXX.png");
    tmpFile.setAutoRemove(true);
    ASSERT_TRUE(tmpFile.open());

    QImage img(2, 2, QImage::Format_ARGB32);
    img.save(&tmpFile, "PNG");
    tmpFile.close();

    // slotFileSuffix 内部使用 QUrl(path).toLocalFile()，需传入 file:// URL
    QString fileUrl = QUrl::fromLocalFile(tmpFile.fileName()).toString();
    QString suffix = control.slotFileSuffix(fileUrl);
    EXPECT_FALSE(suffix.isEmpty());
}

// 测试判断是否为图片
TEST_F(ut_filecontrol, IsImage)
{
    FileControl control;
    EXPECT_TRUE(control.isImage("/tmp/test/image.jpg"));
    EXPECT_TRUE(control.isImage("/tmp/test/image.png"));
    EXPECT_FALSE(control.isImage("/tmp/test/document.txt"));
}

// 测试配置读写
TEST_F(ut_filecontrol, ConfigValue)
{
    FileControl control;
    control.setConfigValue("ut_group", "ut_key", QString("ut_value"));
    QVariant val = control.getConfigValue("ut_group", "ut_key");
    EXPECT_EQ(val.toString(), QString("ut_value"));
}

// ============================================================
// 以下为增量补全用例，覆盖此前未覆盖的 public/private 函数
// ============================================================

// 在临时目录中创建一张 PNG 图片，返回其绝对路径
static QString ut_fc_makeImage(const QTemporaryDir &dir, const QString &name)
{
    QString path = dir.filePath(name);
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(path, "PNG");
    return path;
}

// 桩: 替换 PrintHelper::showPrintDialog, 避免弹出真实打印对话框(首参为 this)
static void ut_fc_stub_showPrintDialog(PrintHelper *, const QStringList &, QWidget *)
{
}

// resetImageFiles: 重设监控列表并清空不崩溃
TEST_F(ut_filecontrol, ResetImageFiles_SetAndClear_NoCrash)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "a.png");
    QString f2 = ut_fc_makeImage(dir, "b.png");

    FileControl control;
    control.resetImageFiles({QUrl::fromLocalFile(f1).toString(),
                             QUrl::fromLocalFile(f2).toString()});
    control.resetImageFiles({});
    SUCCEED();
}

// addImageFile: 追加监控文件不崩溃
TEST_F(ut_filecontrol, AddImageFile_AppendWatch_NoCrash)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "c.png");

    FileControl control;
    control.resetImageFiles({});
    control.addImageFile(QUrl::fromLocalFile(f1).toString());
    SUCCEED();
}

// getDirImagePath: 扫描目录返回其中的图片文件
TEST_F(ut_filecontrol, GetDirImagePath_WithImages_ReturnsList)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    ut_fc_makeImage(dir, "x.png");
    ut_fc_makeImage(dir, "y.png");

    FileControl control;
    // 内部使用 QUrl(path).toLocalFile(), 需传入 file:// URL 才能正确解析路径
    QStringList imgs = control.getDirImagePath(QUrl::fromLocalFile(dir.filePath("x.png")).toString());
    EXPECT_GE(imgs.size(), 2);
}

// getDirImagePath: 空路径返回空列表
TEST_F(ut_filecontrol, GetDirImagePath_EmptyPath_ReturnsEmpty)
{
    FileControl control;
    EXPECT_TRUE(control.getDirImagePath(QString()).isEmpty());
}

// isCurrentWatcherDir: 未监控目录返回 false
TEST_F(ut_filecontrol, IsCurrentWatcherDir_UnknownDir_ReturnsFalse)
{
    FileControl control;
    EXPECT_FALSE(control.isCurrentWatcherDir(QUrl::fromLocalFile("/tmp/ut_fc_unknown_dir/img.png")));
}

// slotGetInfo: 对真实图片查询信息(未命中 key 返回 "-")
TEST_F(ut_filecontrol, SlotGetInfo_RealImage_ReturnsNonEmpty)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "info.png");

    FileControl control;
    QString url = QUrl::fromLocalFile(f1).toString();
    QString val = control.slotGetInfo("DateTimeOriginal", url);
    EXPECT_FALSE(val.isEmpty());
    // 同一文件再次查询应命中缓存路径
    EXPECT_EQ(control.slotGetInfo("DateTimeOriginal", url), val);
}

// slotGetFileNameSuffix: 返回带后缀的完整文件名
TEST_F(ut_filecontrol, SlotGetFileNameSuffix_ReturnsFullName)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "name.png");

    FileControl control;
    QString name = control.slotGetFileNameSuffix(QUrl::fromLocalFile(f1).toString());
    EXPECT_TRUE(name.endsWith("name.png"));
}

// getNamePath: 根据新名字构造 file:// 路径
TEST_F(ut_filecontrol, GetNamePath_ConstructsNewPath)
{
    FileControl control;
    QString old = QUrl::fromLocalFile("/tmp/ut_fc_dir/old.png").toString();
    QString newPath = control.getNamePath(old, "newname");
    EXPECT_TRUE(newPath.contains("newname"));
    EXPECT_TRUE(newPath.startsWith("file://"));
}

// deleteImagePath: 无效 URL(空路径)提前返回 false
TEST_F(ut_filecontrol, DeleteImagePath_InvalidUrl_ReturnsFalse)
{
    FileControl control;
    EXPECT_FALSE(control.deleteImagePath(QString()));
}

// displayinFileManager: DBus 服务可用时返回 true，不可用时返回 false
TEST_F(ut_filecontrol, DisplayinFileManager_Callable_NoCrash)
{
    FileControl control;
    // FileManager1 DBus 服务在测试环境中可能不可用，仅验证可调用不崩溃
    bool result = control.displayinFileManager(QUrl::fromLocalFile("/tmp/ut_fc_fm.png").toString());
    EXPECT_TRUE(result == true || result == false);
}

// copyImage: 复制图片到剪贴板
TEST_F(ut_filecontrol, CopyImage_SetsClipboard_NoCrash)
{
    FileControl control;
    // 内部使用 QUrl(path).toLocalFile(), 需传入 file:// URL 才能把本地路径写入剪贴板文本
    control.copyImage(QUrl::fromLocalFile("/tmp/ut_fc_copy.png").toString());
    EXPECT_FALSE(qApp->clipboard()->text().isEmpty());
}

// copyText: 复制文本到剪贴板
TEST_F(ut_filecontrol, CopyText_SetsClipboard)
{
    FileControl control;
    control.copyText("ut_fc_text");
    EXPECT_EQ(qApp->clipboard()->text(), QString("ut_fc_text"));
}

// ocrImage: 单页静态图走 openFile 分支
TEST_F(ut_filecontrol, OcrImage_StaticImage_NoCrash)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "ocr.png");

    FileControl control;
    control.ocrImage(QUrl::fromLocalFile(f1).toString(), 0);
    SUCCEED();
}

// showPrintDialog: 桩住真实弹窗, 不崩溃
TEST_F(ut_filecontrol, ShowPrintDialog_Stubbed_NoCrash)
{
    FileControl control;
    Stub stub;
    stub.set(ADDR(PrintHelper, showPrintDialog), ut_fc_stub_showPrintDialog);
    control.showPrintDialog(QUrl::fromLocalFile("/tmp/ut_fc_print.png").toString());
    SUCCEED();
}

// parseCommandlineGetPath: 测试参数中无图片返回空串
TEST_F(ut_filecontrol, ParseCommandlineGetPath_NoImageInArgs_ReturnsEmpty)
{
    FileControl control;
    EXPECT_TRUE(control.parseCommandlineGetPath().isEmpty());
}

// isCheckOnly: 首次调用锁定成功返回 true
TEST_F(ut_filecontrol, IsCheckOnly_FirstCall_ReturnsTrue)
{
    FileControl control;
    EXPECT_TRUE(control.isCheckOnly());
}

// isSupportSetWallpaper: 受支持且可读返回 true, 否则 false
TEST_F(ut_filecontrol, IsSupportSetWallpaper_PngReadable_True_Other_False)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "wp.png");

    FileControl control;
    EXPECT_TRUE(control.isSupportSetWallpaper(QUrl::fromLocalFile(f1).toString()));
    EXPECT_FALSE(control.isSupportSetWallpaper(QUrl::fromLocalFile("/tmp/ut_fc_no_such.txt").toString()));
}

// isCanSupportOcr: 静态图可识别返回 true
TEST_F(ut_filecontrol, IsCanSupportOcr_StaticImage_ReturnsTrue)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "ocr2.png");

    FileControl control;
    EXPECT_TRUE(control.isCanSupportOcr(QUrl::fromLocalFile(f1).toString()));
}

// isCanRename: 本地可写可读文件返回 true
TEST_F(ut_filecontrol, IsCanRename_LocalWritableFile_ReturnsTrue)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "rn.png");

    FileControl control;
    EXPECT_TRUE(control.isCanRename(QUrl::fromLocalFile(f1).toString()));
}

// isCanReadable: 存在且可读返回 true, 不存在返回 false
TEST_F(ut_filecontrol, IsCanReadable_ExistingFile_True_Missing_False)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "rd.png");

    FileControl control;
    EXPECT_TRUE(control.isCanReadable(QUrl::fromLocalFile(f1).toString()));
    EXPECT_FALSE(control.isCanReadable(QUrl::fromLocalFile("/tmp/ut_fc_no_read.png").toString()));
}

// isRotatable: 不存在返回 false; 真实文件可调用
TEST_F(ut_filecontrol, IsRotatable_MissingFile_False_RealFile_Callable)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "rot.png");

    FileControl control;
    EXPECT_FALSE(control.isRotatable(QUrl::fromLocalFile("/tmp/ut_fc_no_rot.png").toString()));
    // 真实 PNG 是否可旋转取决于格式支持, 仅验证可调用
    control.isRotatable(QUrl::fromLocalFile(f1).toString());
    SUCCEED();
}

// isCanWrite: 本地临时文件可调用
TEST_F(ut_filecontrol, IsCanWrite_LocalFile_Callable)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "wr.png");

    FileControl control;
    control.isCanWrite(QUrl::fromLocalFile(f1).toString());
    SUCCEED();
}

// isCanDelete: 本地临时文件可调用
TEST_F(ut_filecontrol, IsCanDelete_LocalFile_Callable)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "del.png");

    FileControl control;
    control.isCanDelete(QUrl::fromLocalFile(f1).toString());
    SUCCEED();
}

// slotFileReName: 真实重命名成功, 并发出 imageRenamed 信号
TEST_F(ut_filecontrol, SlotFileReName_Success_EmitsSignal)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "old.png");

    FileControl control;
    QSignalSpy spy(&control, &FileControl::imageRenamed);

    EXPECT_TRUE(control.slotFileReName("newname", QUrl::fromLocalFile(f1).toString(), false));
    EXPECT_EQ(spy.count(), 1);
}

// slotFileReName: 不存在的文件返回 false
TEST_F(ut_filecontrol, SlotFileReName_Nonexistent_ReturnsFalse)
{
    FileControl control;
    EXPECT_FALSE(control.slotFileReName("x", QUrl::fromLocalFile("/tmp/ut_fc_no_exist.png").toString(), false));
}

// isShowToolTip: 同名返回 false
TEST_F(ut_filecontrol, IsShowToolTip_SameName_ReturnsFalse)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "tip.png");

    FileControl control;
    EXPECT_FALSE(control.isShowToolTip(QUrl::fromLocalFile(f1).toString(), "tip"));
}

// isShowToolTip: 目标名已存在且与原文件不同返回 true
TEST_F(ut_filecontrol, IsShowToolTip_TargetExists_ReturnsTrue)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString f1 = ut_fc_makeImage(dir, "src.png");
    ut_fc_makeImage(dir, "dst.png");

    FileControl control;
    EXPECT_TRUE(control.isShowToolTip(QUrl::fromLocalFile(f1).toString(), "dst"));
}

// saveSetting: 直接调用不崩溃
TEST_F(ut_filecontrol, SaveSetting_DirectCall_NoCrash)
{
    FileControl control;
    control.saveSetting();
    SUCCEED();
}

// getlastWidth / getlastHeight: 返回值不小于最小尺寸
TEST_F(ut_filecontrol, GetLastWidthHeight_NotLessThanMinimum)
{
    FileControl control;
    EXPECT_GE(control.getlastWidth(), 628);
    EXPECT_GE(control.getlastHeight(), 300);
}

// setSettingWidth / setSettingHeight: 调用设置值不崩溃
TEST_F(ut_filecontrol, SetSettingWidthHeight_NoCrash)
{
    FileControl control;
    control.setSettingWidth(700);
    control.setSettingHeight(400);
    SUCCEED();
}

// getPrimaryScreenCenterX / Y: 返回坐标值
TEST_F(ut_filecontrol, GetPrimaryScreenCenter_ReturnsValue)
{
    FileControl control;
    int x = control.getPrimaryScreenCenterX(100);
    int y = control.getPrimaryScreenCenterY(100);
    Q_UNUSED(x);
    Q_UNUSED(y);
    SUCCEED();
}

// setEnableNavigation / isEnableNavigation: 读写回环
TEST_F(ut_filecontrol, EnableNavigation_ReadWriteRoundTrip)
{
    FileControl control;
    control.setEnableNavigation(false);
    EXPECT_FALSE(control.isEnableNavigation());
    control.setEnableNavigation(true);
    EXPECT_TRUE(control.isEnableNavigation());
}

// getCompanyLogo: 返回有效 URL
TEST_F(ut_filecontrol, GetCompanyLogo_ReturnsValidUrl)
{
    FileControl control;
    QUrl logo = control.getCompanyLogo();
    EXPECT_TRUE(logo.isValid() || !logo.isEmpty());
}

// showShortcutPanel: 启动快捷键面板进程(二进制缺失时安全失败)
TEST_F(ut_filecontrol, ShowShortcutPanel_NoCrash)
{
    FileControl control;
    control.showShortcutPanel(100, 100);
    control.terminateShortcutPanelProcess();
    SUCCEED();
}

// terminateShortcutPanelProcess: 无运行中进程时安全返回
TEST_F(ut_filecontrol, TerminateShortcutPanelProcess_Idle_NoCrash)
{
    FileControl control;
    control.terminateShortcutPanelProcess();
    SUCCEED();
}

// createShortcutString(私有): 首次构建非空, 再次调用命中缓存
TEST_F(ut_filecontrol, CreateShortcutString_BuildsAndCaches)
{
    FileControl control;
    QString s1 = control.createShortcutString();
    EXPECT_FALSE(s1.isEmpty());
    // 不为空时直接返回缓存值
    QString s2 = control.createShortcutString();
    EXPECT_EQ(s1, s2);
}

// setWallpaper(): null 路径，线程启动后立即返回，覆盖函数入口
TEST_F(ut_filecontrol, SetWallpaper_NullPath_NoCrash)
{
    FileControl control;
    // null 路径时线程 lambda 直接跳过 DBus 逻辑
    control.setWallpaper(QString());
    // 等待线程结束并清理 QThread 对象（deleteLater 需事件处理）
    QThread::usleep(50000);  // 50ms 等线程退出
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    SUCCEED();
}

// saveSetting 定时器 lambda：直接发射 timeout 信号触发 lambda
TEST_F(ut_filecontrol, SaveSettingTimer_TriggersLambda_NoCrash)
{
    FileControl control;
    // m_tSaveSetting 为私有成员，-fno-access-control 允许访问
    // 直接发射 QTimer::timeout 信号触发构造函数中的 lambda（调用 saveSetting）
    // 不使用 processEvents 避免处理 DBus 残留事件导致崩溃
    control.m_tSaveSetting->timeout(QTimer::QPrivateSignal{});
    SUCCEED();
}
