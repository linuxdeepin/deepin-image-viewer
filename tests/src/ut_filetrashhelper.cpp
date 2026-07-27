// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_filetrashhelper.h"
#include "filetrashhelper.h"
#include "unionimage/baseutils.h"

#include <QUrl>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryFile>

#include "stub.h"

// 与 filetrashhelper.cpp 中的 DeletionResult 枚举值保持一致
// kTrashTimeout = 0, kTrashInterfaceError = 1, kTrashInvalidUrl = 2, kTrashSuccess = 3
static const int kUtTrashTimeout = 0;
static const int kUtTrashInterfaceError = 1;
static const int kUtTrashInvalidUrl = 2;
static const int kUtTrashSuccess = 3;

void ut_filetrashhelper::SetUp() {}
void ut_filetrashhelper::TearDown() {}

// ==================== 构造函数 ====================

// 测试构造函数初始化 DBus 设备管理接口
TEST_F(ut_filetrashhelper, Constructor_InitializesDeviceManager)
{
    FileTrashHelper helper;
    // m_dfmDeviceManager 在构造函数中通过 reset() 创建
    EXPECT_NE(helper.m_dfmDeviceManager.data(), nullptr);
}

// ==================== removeFile ====================

// 测试删除不存在的文件返回 false
TEST_F(ut_filetrashhelper, RemoveFile_NonExistentFile_ReturnsFalse)
{
    FileTrashHelper helper;
    QUrl url = QUrl::fromLocalFile("/tmp/ut_nonexistent_file_12345_not_exist.png");
    EXPECT_FALSE(helper.removeFile(url));
}

// 测试删除存在的文件返回 true 且文件被删除
TEST_F(ut_filetrashhelper, RemoveFile_ExistingFile_ReturnsTrue)
{
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    tmpFile.write("test content");
    tmpFile.close();
    QString path = tmpFile.fileName();
    ASSERT_TRUE(QFileInfo::exists(path));

    FileTrashHelper helper;
    QUrl url = QUrl::fromLocalFile(path);
    EXPECT_TRUE(helper.removeFile(url));
    EXPECT_FALSE(QFileInfo::exists(path));
}

// ==================== resetMountInfo ====================

// 测试 resetMountInfo 清空挂载数据
TEST_F(ut_filetrashhelper, ResetMountInfo_ClearsState)
{
    FileTrashHelper helper;
    // 借助 -fno-access-control 访问私有成员
    helper.initData = true;
    helper.mountDevices.insert("dev1", "/mnt/usb1");
    helper.mountDevices.insert("dev2", "/mnt/usb2");

    helper.resetMountInfo();

    EXPECT_FALSE(helper.initData);
    EXPECT_TRUE(helper.mountDevices.isEmpty());
}

// ==================== isGvfsFile (private, const) ====================

// 测试无效 URL 返回 false
TEST_F(ut_filetrashhelper, IsGvfsFile_InvalidUrl_ReturnsFalse)
{
    FileTrashHelper helper;
    QUrl invalidUrl;
    EXPECT_FALSE(helper.isGvfsFile(invalidUrl));
}

// 测试普通本地路径返回 false
TEST_F(ut_filetrashhelper, IsGvfsFile_NormalPath_ReturnsFalse)
{
    FileTrashHelper helper;
    EXPECT_FALSE(helper.isGvfsFile(QUrl::fromLocalFile("/tmp/test.png")));
    EXPECT_FALSE(helper.isGvfsFile(QUrl::fromLocalFile("/home/user/image.jpg")));
}

// 测试 /run/user/<uid>/gvfs/ 路径返回 true
TEST_F(ut_filetrashhelper, IsGvfsFile_RunUserGvfsPath_ReturnsTrue)
{
    FileTrashHelper helper;
    QUrl url = QUrl::fromLocalFile("/run/user/1000/gvfs/smb:test/file.png");
    EXPECT_TRUE(helper.isGvfsFile(url));
}

// 测试 /root/.gvfs/ 路径返回 true
TEST_F(ut_filetrashhelper, IsGvfsFile_RootGvfsPath_ReturnsTrue)
{
    FileTrashHelper helper;
    QUrl url = QUrl::fromLocalFile("/root/.gvfs/test/file.png");
    EXPECT_TRUE(helper.isGvfsFile(url));
}

// 测试 /media/.../smbmounts 路径返回 true
TEST_F(ut_filetrashhelper, IsGvfsFile_SmbMountsPath_ReturnsTrue)
{
    FileTrashHelper helper;
    QUrl url = QUrl::fromLocalFile("/media/smb/smbmounts/share/file.png");
    EXPECT_TRUE(helper.isGvfsFile(url));
}

// ==================== isExternalDevice (private) ====================

// 测试无挂载设备时返回 false
TEST_F(ut_filetrashhelper, IsExternalDevice_NoMounts_ReturnsFalse)
{
    FileTrashHelper helper;
    helper.mountDevices.clear();
    EXPECT_FALSE(helper.isExternalDevice("/mnt/usb/file.png"));
}

// 测试路径匹配外部设备时返回 true
TEST_F(ut_filetrashhelper, IsExternalDevice_PathOnExternal_ReturnsTrue)
{
    FileTrashHelper helper;
    helper.mountDevices.clear();
    helper.mountDevices.insert("dev1", "/mnt/usb");
    EXPECT_TRUE(helper.isExternalDevice("/mnt/usb/file.png"));
    EXPECT_TRUE(helper.isExternalDevice("/mnt/usb/subdir/deep/file.png"));
}

// 测试路径不匹配任何外部设备时返回 false
TEST_F(ut_filetrashhelper, IsExternalDevice_PathNotOnExternal_ReturnsFalse)
{
    FileTrashHelper helper;
    helper.mountDevices.clear();
    helper.mountDevices.insert("dev1", "/mnt/usb");
    EXPECT_FALSE(helper.isExternalDevice("/home/user/file.png"));
    EXPECT_FALSE(helper.isExternalDevice("/mnt/other/file.png"));
}

// ==================== queryMountInfo (private) ====================

// 测试 initData 为 true 时不调用 DBus（提前返回）
TEST_F(ut_filetrashhelper, QueryMountInfo_AlreadyInitialized_NoDBusCall)
{
    FileTrashHelper helper;
    helper.initData = true;
    helper.queryMountInfo();
    EXPECT_TRUE(helper.initData);
}

// 测试 initData 为 false 时调用 DBus（测试环境 DBus 不可用，不应崩溃）
TEST_F(ut_filetrashhelper, QueryMountInfo_DBusFails_NoCrash)
{
    FileTrashHelper helper;
    helper.initData = false;
    helper.queryMountInfo();
    // 无论 DBus 是否成功，initData 都应被设为 true
    EXPECT_TRUE(helper.initData);
}

// ==================== fileCanTrash ====================

// 测试 GVFS 路径不可回收（提前返回 false，不触发 DBus 查询）
TEST_F(ut_filetrashhelper, FileCanTrash_GvfsPath_ReturnsFalse)
{
    FileTrashHelper helper;
    QUrl url = QUrl::fromLocalFile("/run/user/1000/gvfs/smb:test/file.png");
    EXPECT_FALSE(helper.fileCanTrash(url));
}

// 测试普通本地路径可回收
TEST_F(ut_filetrashhelper, FileCanTrash_NormalLocalPath_ReturnsTrue)
{
    FileTrashHelper helper;
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    tmpFile.close();

    QUrl url = QUrl::fromLocalFile(tmpFile.fileName());
    // 本地路径，非外部设备，非 GVFS → 可回收
    EXPECT_TRUE(helper.fileCanTrash(url));
}

// 测试同一目录连续调用不重复重置挂载信息
TEST_F(ut_filetrashhelper, FileCanTrash_SameDirNotResetMountInfo)
{
    FileTrashHelper helper;
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    tmpFile.close();
    QUrl url = QUrl::fromLocalFile(tmpFile.fileName());

    // 第一次调用：lastDir 与 currentDir 不同，会触发 resetMountInfo
    helper.fileCanTrash(url);
    QDir firstDir = helper.lastDir;

    // 第二次调用同一目录：lastDir 与 currentDir 相同，不重置
    helper.fileCanTrash(url);
    EXPECT_EQ(helper.lastDir, firstDir);
}

// ==================== moveFileToTrashWithDBus (private) ====================

// 测试无效 URL 返回 kTrashInvalidUrl（提前返回，不触发 DBus）
TEST_F(ut_filetrashhelper, MoveFileToTrashWithDBus_InvalidUrl_ReturnsInvalidUrlError)
{
    FileTrashHelper helper;
    QUrl invalidUrl;
    int ret = helper.moveFileToTrashWithDBus(invalidUrl);
    EXPECT_EQ(ret, kUtTrashInvalidUrl);
}

// ==================== moveFileToTrash ====================

// 桩：moveFileToTrashWithDBus 返回可配置的值
static int g_ut_stubMoveDBusRet = kUtTrashSuccess;
static int ut_stub_moveFileToTrashWithDBus(FileTrashHelper *, const QUrl &)
{
    return g_ut_stubMoveDBusRet;
}

// 测试 DBus 调用成功时 moveFileToTrash 返回 true
TEST_F(ut_filetrashhelper, MoveFileToTrash_DBusSuccess_ReturnsTrue)
{
    FileTrashHelper helper;
    Stub stub;
    g_ut_stubMoveDBusRet = kUtTrashSuccess;
    stub.set(ADDR(FileTrashHelper, moveFileToTrashWithDBus), ut_stub_moveFileToTrashWithDBus);

    QUrl url = QUrl::fromLocalFile("/tmp/ut_filetrashhelper_test.png");
    EXPECT_TRUE(helper.moveFileToTrash(url));
}

// 测试 DBus 接口失败且回退接口(trashFile)也失败时返回 false
TEST_F(ut_filetrashhelper, MoveFileToTrash_DBusErrorAndFallbackFails_ReturnsFalse)
{
    FileTrashHelper helper;
    Stub stub;
    g_ut_stubMoveDBusRet = kUtTrashInterfaceError;
    stub.set(ADDR(FileTrashHelper, moveFileToTrashWithDBus), ut_stub_moveFileToTrashWithDBus);

    // 不存在的文件，trashFile 会失败
    QUrl url = QUrl::fromLocalFile("/tmp/ut_filetrashhelper_nonexistent_12345.png");
    EXPECT_FALSE(helper.moveFileToTrash(url));
}

// 桩：trashFile 返回 true
static bool ut_stub_trashFile_success(const QString &)
{
    return true;
}

// 测试 DBus 接口失败但回退接口(trashFile)成功时返回 true
TEST_F(ut_filetrashhelper, MoveFileToTrash_DBusErrorAndFallbackSucceeds_ReturnsTrue)
{
    FileTrashHelper helper;
    Stub stub;
    g_ut_stubMoveDBusRet = kUtTrashInterfaceError;
    stub.set(ADDR(FileTrashHelper, moveFileToTrashWithDBus), ut_stub_moveFileToTrashWithDBus);
    stub.set(Libutils::base::trashFile, ut_stub_trashFile_success);

    QUrl url = QUrl::fromLocalFile("/tmp/ut_filetrashhelper_fallback_test.png");
    EXPECT_TRUE(helper.moveFileToTrash(url));
}

// 测试 DBus 返回 kTrashInvalidUrl（非 InterfaceError 非 Success）时返回 false
TEST_F(ut_filetrashhelper, MoveFileToTrash_DBusInvalidUrl_ReturnsFalse)
{
    FileTrashHelper helper;
    Stub stub;
    g_ut_stubMoveDBusRet = kUtTrashInvalidUrl;
    stub.set(ADDR(FileTrashHelper, moveFileToTrashWithDBus), ut_stub_moveFileToTrashWithDBus);

    QUrl url = QUrl::fromLocalFile("/tmp/ut_filetrashhelper_invalidurl_test.png");
    EXPECT_FALSE(helper.moveFileToTrash(url));
}
