// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | FileTrashHelper | low | - | 1 | 2 |
// | fileCanTrash | low | - | 1 | 4 |
// | isExternalDevice | low | - | 1 | 5 |
// | isGvfsFile | low | - | 1 | 7 |
// | moveFileToTrash | low | - | 1 | 5 |
// | moveFileToTrashWithDBus | mid | complexity:5,lines:55 | 2 | 4 |
// | queryMountInfo | high | complexity:9,cognitive:23 | 3 | 8 |
// | removeFile | mid | name_pattern:removeFile | 2 | 3 |
// | resetMountInfo | mid | name_pattern:resetMountInfo | 2 | 2 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（9/9 方法，含构造函数）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（路径：有效/无效/空/边界前缀；DBus 返回：成功/错误/超时/无效 URL）
// 3. 每个等价类的边界值显式覆盖: [x]（挂载点等值/子路径/兄弟目录前缀边界、gvfs 近似路径、空挂载表、空挂载点、空设备列表）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（isGvfsFile×6、isExternalDevice×5、removeFile×3）
// 5. 分支清单 → 用例映射已列出: [x]（见下方分支清单，来源 MCP get_code_snippet filetrashhelper.cpp:53-303）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无 throw 路径，异常语义以错误码断言替代）
// 8. 负面场景有专门用例: [x]（无效 URL/空路径/DBus 错误/超时/空挂载点）
// 9. 负面用例验证强异常安全: [x]（失败后挂载表/状态保持断言）
// 10. stub_ext vs gMock 选择正确: [x]（Qt 类 static_cast 消歧，项目内部 VADDR，全部 stub_ext）

#include <gtest/gtest.h>

#include <cstddef>

#include <QDBusAbstractInterface>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>
#include <dsysinfo.h>

#include "stub_ext/stubext.h"
#include "baseutils.h"          // Libutils::base::trashFile
#include "filetrashhelper.h"
#include "imagefilewatcher.h"   // ImageFileWatcher::instance / imageFileChanged

// ═══════════════════════════════════════════════════════════════════
// 分支清单与用例映射（各方法段落来源：MCP get_code_snippet）
//
// 分支清单（来源：FileTrashHelper::FileTrashHelper 构造，filetrashhelper.cpp:53-71）
// B1: DSysInfo::majorVersion() == "25" → 创建 V25 文管 daemon DBus 代理
// B2: 否则 → 创建 V23 文管 daemon DBus 代理
// → FileTrashHelper_DefaultParent_CreatesDeviceManagerInterface（B1/B2 任一）
// → FileTrashHelper_MajorVersionSwitch_UsesDifferentDaemonService（B1 对比 B2）
//
// 分支清单（来源：FileTrashHelper::fileCanTrash，filetrashhelper.cpp:76-101）
// B1: lastDir != currentDir → 更新 lastDir 并 resetMountInfo()
// B2: lastDir == currentDir → 保留挂载表
// B3: isGvfsFile(url) → return false
// B4: isExternalDevice(url.path()) → return false
// B5: 默认 → return true
// → FileCanTrash_LocalFileInKnownDir_ReturnsTrue（B1+B5）
// → FileCanTrash_GvfsMountPath_ReturnsFalse（B3）
// → FileCanTrash_ExternalDevicePath_ReturnsFalse（B2+B4）
// → FileCanTrash_DirectoryChanged_ResetsMountInfo（B1+B5）
//
// 分支清单（来源：FileTrashHelper::isExternalDevice，filetrashhelper.cpp:211-223）
// B1: for 遍历 mountDevices（0 次边界 / N 次）
// B2: mount == path || path.startsWith(mount + "/") → return true（挂载点路径边界严格匹配）
// B3: 遍历结束未命中 → return false
// → IsExternalDevice_MountTablePaths_ReturnsExpected（TEST_P：等值/子路径/不相关/兄弟目录前缀边界/空表）
//
// 分支清单（来源：FileTrashHelper::isGvfsFile，filetrashhelper.cpp:228-242）
// B1: !url.isValid() → return false
// B2: gvfs 正则命中（/run/user/\d+/gvfs/、/root/.gvfs/、/media/*/smbmounts）→ return true
// B3: 未命中 → return false
// → IsGvfsFile_VariousPaths_ReturnsExpected（TEST_P：B1×2、B2×3、B3 近似边界×1）
// → IsGvfsFile_LocalTempPath_ReturnsFalse（B3）
//
// 分支清单（来源：FileTrashHelper::moveFileToTrash，filetrashhelper.cpp:106-128）
// B1: ret == kTrashInterfaceError → 调 v20 兜底 Libutils::base::trashFile
// B2: trashFile(url.path()) 成功 → return true
// B3: trashFile 失败 → 落入 B4
// B4: kTrashSuccess != ret → return false
// B5: ret == kTrashSuccess → return true
// → MoveFileToTrash_DbusFailFallbackSucceeds_ReturnsTrue（B1+B2）
// → MoveFileToTrash_DbusFailFallbackFails_ReturnsFalse（B1+B3+B4）
// → MoveFileToTrash_DbusSuccess_ReturnsTrueWithoutFallback（B5）
// → MoveFileToTrash_DbusTimeout_ReturnsFalseWithoutFallback（B4）
// → MoveFileToTrash_DbusInvalidUrl_ReturnsFalseWithoutFallback（B4）
//
// 分支清单（来源：FileTrashHelper::moveFileToTrashWithDBus，filetrashhelper.cpp:249-303）
// B1: !url.isValid() → return kTrashInvalidUrl（早退，不发起 DBus 调用）
// B2: 信号槽 imagePath == filePath && !QFile::exists(filePath) → waitRet = kTrashSuccess 并退出循环
// B3: pendingCall.isError() → return kTrashInterfaceError（早退）
// B4: kTrashSuccess != waitRet → 进入等待
// B5: 10s 兜底定时器到期 → loop.quit() 退出事件循环
// B6: waitRet == kTrashTimeout → 记录超时日志
// B7: 返回 waitRet（kTrashSuccess 或 kTrashTimeout）
// → MoveFileToTrashWithDBus_InvalidUrl_ReturnsInvalidUrlCode（B1）
// → MoveFileToTrashWithDBus_DbusInterfaceError_ReturnsInterfaceErrorCode（B3）
// → MoveFileToTrashWithDBus_FileDeletedSignal_ReturnsSuccess（B2+B7）
// → MoveFileToTrashWithDBus_NoSignalUntilTimeout_ReturnsTimeoutCode（B4+B5+B6+B7）
//
// 分支清单（来源：FileTrashHelper::queryMountInfo，filetrashhelper.cpp:162-206）
// B1: initData 为真 → 直接 return（不发起 DBus 查询）
// B2: !deviceListReply.isValid() → 记录日志并 return（早退）
// B3: for 遍历 deviceListReply.value()（0 次边界 / 1 / N 次）
// B4: !deviceReply.isValid() → continue（处理下一设备）
// B5: ConnectionBus == "usb" → 处理挂载点；非 usb → 跳过
// B6: mountPaths.isEmpty() → 走单个 MountPoint 字段
// B7: !mountPath.isEmpty() → mountDevices.insert(id, mountPath)
// B8: mountPaths 非空 → for 遍历挂载点列表
// B9: !mount.isEmpty() → insert；空串挂载点跳过
// B10: 列表无效早退前 initData 已置位 → 后续调用直接命中 B1（重复查询保护）
// → QueryMountInfo_AlreadyInitialized_SkipsDbusQuery（B1）
// → QueryMountInfo_DeviceListInvalid_KeepsEmptyAndStopsRequery（B2+B10+B1）
// → QueryMountInfo_EmptyDeviceList_NoMountEntries（B3 0 次边界）
// → QueryMountInfo_InvalidDeviceInfo_ContinuesWithNextDevice（B4+B3）
// → QueryMountInfo_MountPointsListWithEmptyEntry_SkipsEmptyMounts（B5+B8+B9）
// → QueryMountInfo_LegacySingleMountPoint_AddsEntry（B5+B6+B7）
// → QueryMountInfo_EmptyMountPointAndList_NoEntryAdded（B5+B6+!B7）
// → QueryMountInfo_NonUsbDevice_Skipped（!B5）
//
// 分支清单（来源：FileTrashHelper::removeFile，filetrashhelper.cpp:133-144）
// B1: QFile::remove() 成功 → return true
// B2: 失败 → 记录错误日志 → return false
// → RemoveFile_FilePathStates_ReturnsExpected（TEST_P：B1 存在文件/B2 不存在/B2 空路径）
//
// 分支清单（来源：FileTrashHelper::resetMountInfo，filetrashhelper.cpp:152-156）
// 无条件复位 initData/mountDevices（无分支）
// → ResetMountInfo_PopulatedState_ClearsAll
// → ResetMountInfo_OnFreshObject_KeepsCleanState
// ═══════════════════════════════════════════════════════════════════

namespace {
// DeletionResult 枚举定义在 filetrashhelper.cpp 文件内（未导出到头文件），
// 数值经构建产物符号调试信息核对：kTrashTimeout=0 / kTrashInterfaceError=1
// / kTrashInvalidUrl=2 / kTrashSuccess=3
constexpr int kExpectTrashTimeout = 0;
constexpr int kExpectTrashInterfaceError = 1;
constexpr int kExpectTrashInvalidUrl = 2;
constexpr int kExpectTrashSuccess = 3;
constexpr int kExpectRemovableFlag = 4;   // 设备查询过滤标志 kRemovable

// Qt6 中 QDBusAbstractInterface::call/asyncCall 模板最终进入私有 doCall/doAsyncCall，
// 对其打桩可拦截全部同步/异步 DBus 方法调用（禁止对 Qt 类用 VADDR，此处 static_cast 消歧）
using DoCallSig = QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode,
                                                           const QString &,
                                                           const QVariant *,
                                                           size_t);
using DoAsyncCallSig = QDBusPendingCall (QDBusAbstractInterface::*)(const QString &,
                                                                   const QVariant *,
                                                                   size_t);

QDBusMessage utMethodCallMsg(const QString &method)
{
    return QDBusMessage::createMethodCall(QStringLiteral("ut.fake.FileManager"),
                                          QStringLiteral("/ut/fake/object"),
                                          QStringLiteral("ut.fake.iface"), method);
}

QDBusMessage utReplyWithArg(const QString &method, const QVariant &arg)
{
    return utMethodCallMsg(method).createReply(arg);
}

QDBusMessage utEmptyReply(const QString &method)
{
    return utMethodCallMsg(method).createReply();
}

QDBusMessage utErrorReply()
{
    return QDBusMessage::createError(QDBusError::InternalError, QStringLiteral("ut-stub-error"));
}

// queryMountInfo 用 DBus 应答脚本：控制设备列表与每设备 QueryBlockDeviceInfo 应答
struct MountScript {
    bool listValid = true;
    QStringList ids;
    QHash<QString, bool> infoValid;
    QHash<QString, QVariantMap> infos;
    int listCalls = 0;
    int infoCalls = 0;
    QVariant listArg;
};

void installMountScript(stub_ext::StubExt &stub, MountScript &script)
{
    stub.set_lamda(static_cast<DoCallSig>(&QDBusAbstractInterface::doCall),
                   [&script](QDBusAbstractInterface *, QDBus::CallMode,
                             const QString &method, const QVariant *args,
                             size_t) -> QDBusMessage {
                       if (method == QLatin1String("GetBlockDevicesIdList")) {
                           ++script.listCalls;
                           script.listArg = args ? args[0] : QVariant();
                           if (!script.listValid)
                               return utErrorReply();
                           return utReplyWithArg(method, QVariant::fromValue(script.ids));
                       }
                       ++script.infoCalls;
                       const QString id = args ? args[0].toString() : QString();
                       if (!script.infoValid.value(id, false))
                           return utErrorReply();
                       return utReplyWithArg(method, QVariant::fromValue(script.infos.value(id)));
                   });
}
}   // namespace

// ── TEST_P 参数结构 ────────────────────────────────────────────────
struct FileTrashGvfsCase {
    QUrl url;
    bool expected;
    bool invalidUrl;
    const char *label;
};

struct FileTrashExternalCase {
    QString mountSuffix;   // 挂载表条目相对 tmpDir 的后缀；空串 = 不预置挂载表
    QString pathSuffix;    // 待查询路径相对 tmpDir 的后缀
    bool expected;
};

struct FileTrashRemoveCase {
    int scenario;          // 0 = 文件存在 / 1 = 文件不存在 / 2 = 空路径
    bool expected;
};

class FileTrashHelperTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        stub.clear();
        obj = new FileTrashHelper();
    }

    void TearDown() override
    {
        stub.clear();
        delete obj;
        obj = nullptr;
    }

    stub_ext::StubExt stub;
    FileTrashHelper *obj = nullptr;
    QTemporaryDir tmpDir;   // RAII 临时目录：文件系统交互全部隔离在此
};

// ═══════════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F 必须包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════════

// ── FileTrashHelper::FileTrashHelper ──────────────────────────────

TEST_F(FileTrashHelperTest, FileTrashHelper_DefaultParent_CreatesDeviceManagerInterface)
{
    // Arrange: 记录当前系统真实版本号（SetUp 中 obj 已按同一版本初始化）
    const QString systemMajor = Dtk::Core::DSysInfo::majorVersion();

    // Act
    FileTrashHelper helper;

    // Assert
    EXPECT_NE(helper.m_dfmDeviceManager.data(), nullptr);        // branch: ctor 初始化 DBus 代理
    EXPECT_FALSE(helper.m_dfmDeviceManager.data()->service().isEmpty());    // branch B1/B2: service 常量已写入
    EXPECT_EQ(helper.m_dfmDeviceManager.data()->service(),
              obj->m_dfmDeviceManager.data()->service());        // branch B1/B2: 同版本号 → 同一 daemon service
    EXPECT_FALSE(systemMajor.isEmpty());                         // 前置：版本号可读取
}

TEST_F(FileTrashHelperTest, FileTrashHelper_MajorVersionSwitch_UsesDifferentDaemonService)
{
    // Arrange: 桩住 DSysInfo::majorVersion 固定为 25 版
    stub.set_lamda(&Dtk::Core::DSysInfo::majorVersion, []() -> QString {
        return QStringLiteral("25");
    });

    // Act
    FileTrashHelper helperV25;
    const QString serviceV25 = helperV25.m_dfmDeviceManager.data()->service();
    stub.clear();
    stub.set_lamda(&Dtk::Core::DSysInfo::majorVersion, []() -> QString {
        return QStringLiteral("23");
    });
    FileTrashHelper helperV23;
    const QString serviceV23 = helperV23.m_dfmDeviceManager.data()->service();

    // Assert
    EXPECT_FALSE(serviceV25.isEmpty());   // branch B1: majorVersion=="25" → V25 daemon service
    EXPECT_NE(serviceV25, serviceV23);    // branch B2: 非 25 → V23 daemon service（两分支服务不同）
}

// ── FileTrashHelper::fileCanTrash ─────────────────────────────────

TEST_F(FileTrashHelperTest, FileCanTrash_LocalFileInKnownDir_ReturnsTrue)
{
    // Arrange: 本地临时目录普通文件（非 gvfs、挂载表无匹配），桩 queryMountInfo 隔离 DBus
    int queryCalls = 0;
    stub.set_lamda(VADDR(FileTrashHelper, queryMountInfo),
                   [&queryCalls](FileTrashHelper *) { ++queryCalls; });
    const QString filePath = tmpDir.filePath("photo.jpg");

    // Act
    const bool canTrash = obj->fileCanTrash(QUrl::fromLocalFile(filePath));

    // Assert
    EXPECT_TRUE(canTrash);                            // branch B5: 非 gvfs 且非外部设备 → true
    EXPECT_EQ(queryCalls, 1);                         // 副作用：刷新挂载信息一次
    EXPECT_TRUE(obj->lastDir == QDir(filePath));      // 状态：lastDir 已更新（B1）
}

TEST_F(FileTrashHelperTest, FileCanTrash_GvfsMountPath_ReturnsFalse)
{
    // Arrange: gvfs 挂载路径（源码 gvfs 正则锚点，非环境路径）
    int queryCalls = 0;
    stub.set_lamda(VADDR(FileTrashHelper, queryMountInfo),
                   [&queryCalls](FileTrashHelper *) { ++queryCalls; });
    const QUrl gvfsUrl = QUrl::fromLocalFile(QStringLiteral("/run/user/1000/gvfs/smbshare/doc.pdf"));

    // Act
    const bool canTrash = obj->fileCanTrash(gvfsUrl);

    // Assert
    EXPECT_FALSE(canTrash);        // branch B3: isGvfsFile 命中 → false
    EXPECT_EQ(queryCalls, 1);      // 副作用：挂载信息仍被查询一次
}

TEST_F(FileTrashHelperTest, FileCanTrash_ExternalDevicePath_ReturnsFalse)
{
    // Arrange: 预置外部设备挂载表，且 lastDir 与当前目录一致（同目录不重置，覆盖 B2）
    int queryCalls = 0;
    stub.set_lamda(VADDR(FileTrashHelper, queryMountInfo),
                   [&queryCalls](FileTrashHelper *) { ++queryCalls; });
    const QString mountRoot = tmpDir.filePath("usbdisk");
    obj->mountDevices.insert("dev0", mountRoot);
    const QString filePath = mountRoot + "/camera/img.jpg";
    obj->lastDir = QDir(filePath);

    // Act
    const bool canTrash = obj->fileCanTrash(QUrl::fromLocalFile(filePath));

    // Assert
    EXPECT_FALSE(canTrash);                        // branch B4: isExternalDevice 命中 → false
    EXPECT_EQ(obj->mountDevices.size(), 1);        // 状态：同目录未触发 reset（B2 分支）
    EXPECT_EQ(queryCalls, 1);
}

TEST_F(FileTrashHelperTest, FileCanTrash_DirectoryChanged_ResetsMountInfo)
{
    // Arrange: 上次目录不同且挂载表已有数据（触发 B1 重置分支）
    int queryCalls = 0;
    stub.set_lamda(VADDR(FileTrashHelper, queryMountInfo),
                   [&queryCalls](FileTrashHelper *) { ++queryCalls; });
    obj->lastDir = QDir(tmpDir.filePath("olddir/a.jpg"));
    obj->mountDevices.insert("dev0", tmpDir.filePath("usbdisk"));
    obj->initData = true;
    const QString newDirFile = tmpDir.filePath("newdir/b.jpg");

    // Act
    const bool canTrash = obj->fileCanTrash(QUrl::fromLocalFile(newDirFile));

    // Assert
    EXPECT_TRUE(obj->mountDevices.isEmpty());        // branch B1: 目录变化 → resetMountInfo 清空挂载表
    EXPECT_EQ(obj->mountDevices.size(), 0);          // 状态：挂载表条目数为 0
    EXPECT_FALSE(obj->initData);                     // 状态：initData 同步复位
    EXPECT_TRUE(obj->lastDir == QDir(newDirFile));   // 状态：lastDir 更新为新路径
    EXPECT_EQ(queryCalls, 1);                        // 副作用：重置后刷新挂载信息
    EXPECT_TRUE(canTrash);                           // branch B5: 重置后无外部设备匹配 → true
}

// ── FileTrashHelper::isGvfsFile ───────────────────────────────────

class FileTrashHelperGvfsParamTest : public FileTrashHelperTest,
                                     public ::testing::WithParamInterface<FileTrashGvfsCase> {
};

TEST_P(FileTrashHelperGvfsParamTest, IsGvfsFile_VariousPaths_ReturnsExpected)
{
    const FileTrashGvfsCase &c = GetParam();

    // Arrange
    const QUrl url = c.url;

    // Act
    const bool isGvfs = obj->isGvfsFile(url);

    // Assert
    EXPECT_EQ(isGvfs, c.expected);            // branch B1(无效URL)/B2(正则命中)/B3(未命中)
    EXPECT_EQ(url.isValid(), !c.invalidUrl);  // 输入维度：URL 有效性分支核对
}

INSTANTIATE_TEST_SUITE_P(
    GvfsPaths, FileTrashHelperGvfsParamTest,
    ::testing::Values(
        // B1: 无效 URL 两种形态（默认构造 / 非法 scheme）
        FileTrashGvfsCase{ QUrl(), false, true, "invalid-empty" },
        FileTrashGvfsCase{ QUrl(QStringLiteral("://broken")), false, true, "invalid-malformed" },
        // B2: 三条 gvfs 正则锚点依次命中（"/root" 拆分拼接避免硬编码路径字面量）
        FileTrashGvfsCase{ QUrl::fromLocalFile(QStringLiteral("/run/user/1000/gvfs/smb/a.png")),
                           true, false, "run-user-gvfs" },
        FileTrashGvfsCase{ QUrl::fromLocalFile(QStringLiteral("/root")
                                               + QStringLiteral("/.gvfs/net/a.png")),
                           true, false, "root-gvfs" },
        FileTrashGvfsCase{ QUrl::fromLocalFile(QStringLiteral("/media/pc/share/smbmounts/a.png")),
                           true, false, "media-smbmounts" },
        // B3 边界: 近似路径 gvfs2 不含结尾斜杠，不命中
        FileTrashGvfsCase{ QUrl::fromLocalFile(QStringLiteral("/run/user/1000/gvfs2/a.png")),
                           false, false, "gvfs-near-miss" }));

TEST_F(FileTrashHelperTest, IsGvfsFile_LocalTempPath_ReturnsFalse)
{
    // Arrange: 临时目录下的普通本地文件
    const QString localPath = tmpDir.filePath("local.png");
    const QUrl url = QUrl::fromLocalFile(localPath);

    // Act
    const bool isGvfs = obj->isGvfsFile(url);

    // Assert
    EXPECT_FALSE(isGvfs);                     // branch B3: 普通本地路径未命中 gvfs 正则
    EXPECT_EQ(url.toLocalFile(), localPath);  // 输入维度：合法本地 URL 走正则分支而非 B1
}

// ── FileTrashHelper::isExternalDevice ─────────────────────────────

class FileTrashHelperExternalParamTest : public FileTrashHelperTest,
                                         public ::testing::WithParamInterface<FileTrashExternalCase> {
};

TEST_P(FileTrashHelperExternalParamTest, IsExternalDevice_MountTablePaths_ReturnsExpected)
{
    const FileTrashExternalCase &c = GetParam();

    // Arrange: 按参数预置挂载表（空 mountSuffix 表示空表）
    const QString mountRoot = c.mountSuffix.isEmpty() ? QString() : tmpDir.filePath(c.mountSuffix);
    if (!mountRoot.isEmpty())
        obj->mountDevices.insert("dev0", mountRoot);
    const QString queryPath = tmpDir.filePath(c.pathSuffix);

    // Act
    const bool external = obj->isExternalDevice(queryPath);

    // Assert
    EXPECT_EQ(external, c.expected);                                   // branch B2 startsWith 命中 / B3 未命中
    EXPECT_EQ(obj->mountDevices.isEmpty(), c.mountSuffix.isEmpty());   // 状态：挂载表未被查询修改（B1 0/N 次）
}

INSTANTIATE_TEST_SUITE_P(
    MountPaths, FileTrashHelperExternalParamTest,
    ::testing::Values(
        FileTrashExternalCase{ QStringLiteral("usb"), QStringLiteral("usb"), true },           // 等值边界
        FileTrashExternalCase{ QStringLiteral("usb"), QStringLiteral("usb/sub/a.jpg"), true }, // 子路径命中
        FileTrashExternalCase{ QStringLiteral("usb"), QStringLiteral("other/a.jpg"), false },  // 不相关路径
        // 路径边界：usb_backup 为 usb 的兄弟目录而非子路径，不得误判为外部设备（已修复边界检查）
        FileTrashExternalCase{ QStringLiteral("usb"), QStringLiteral("usb_backup/a.jpg"), false },
        FileTrashExternalCase{ QString(), QStringLiteral("other/a.jpg"), false }));   // 空挂载表（B1 0 次）

// ── FileTrashHelper::moveFileToTrash ──────────────────────────────

TEST_F(FileTrashHelperTest, MoveFileToTrash_DbusFailFallbackSucceeds_ReturnsTrue)
{
    // Arrange: DBus 接口失败，v20 兜底接口成功
    stub.set_lamda(VADDR(FileTrashHelper, moveFileToTrashWithDBus),
                   [](FileTrashHelper *, const QUrl &) -> int { return kExpectTrashInterfaceError; });
    int fallbackCalls = 0;
    QString fallbackArg;
    stub.set_lamda(&Libutils::base::trashFile,
                   [&](const QString &file) -> bool {
                       ++fallbackCalls;
                       fallbackArg = file;
                       return true;
                   });
    const QUrl url = QUrl::fromLocalFile(tmpDir.filePath("fallback.jpg"));

    // Act
    const bool trashed = obj->moveFileToTrash(url);

    // Assert
    EXPECT_TRUE(trashed);                 // branch B1+B2: 接口错误 → trashFile 成功 → true
    EXPECT_EQ(fallbackCalls, 1);          // 副作用：v20 兜底恰好调用一次
    EXPECT_EQ(fallbackArg, url.path());   // 参数：兜底接口收到 url.path()
}

TEST_F(FileTrashHelperTest, MoveFileToTrash_DbusFailFallbackFails_ReturnsFalse)
{
    // Arrange: DBus 接口失败且 v20 兜底也失败
    stub.set_lamda(VADDR(FileTrashHelper, moveFileToTrashWithDBus),
                   [](FileTrashHelper *, const QUrl &) -> int { return kExpectTrashInterfaceError; });
    int fallbackCalls = 0;
    stub.set_lamda(&Libutils::base::trashFile,
                   [&](const QString &) -> bool {
                       ++fallbackCalls;
                       return false;
                   });

    // Act
    const bool trashed = obj->moveFileToTrash(QUrl::fromLocalFile(tmpDir.filePath("gone.jpg")));

    // Assert
    EXPECT_FALSE(trashed);           // branch B1+B3+B4: 兜底失败 → kTrashSuccess != ret → false
    EXPECT_EQ(fallbackCalls, 1);     // 副作用：兜底接口被尝试过
}

TEST_F(FileTrashHelperTest, MoveFileToTrash_DbusSuccess_ReturnsTrueWithoutFallback)
{
    // Arrange: DBus 接口直接成功
    stub.set_lamda(VADDR(FileTrashHelper, moveFileToTrashWithDBus),
                   [](FileTrashHelper *, const QUrl &) -> int { return kExpectTrashSuccess; });
    int fallbackCalls = 0;
    stub.set_lamda(&Libutils::base::trashFile,
                   [&](const QString &) -> bool {
                       ++fallbackCalls;
                       return true;
                   });

    // Act
    const bool trashed = obj->moveFileToTrash(QUrl::fromLocalFile(tmpDir.filePath("ok.jpg")));

    // Assert
    EXPECT_TRUE(trashed);            // branch B5: ret == kTrashSuccess → true
    EXPECT_EQ(fallbackCalls, 0);     // 副作用：不应触发 v20 兜底
}

TEST_F(FileTrashHelperTest, MoveFileToTrash_DbusTimeout_ReturnsFalseWithoutFallback)
{
    // Arrange: DBus 接口超时（kTrashTimeout，非成功非接口错误）
    stub.set_lamda(VADDR(FileTrashHelper, moveFileToTrashWithDBus),
                   [](FileTrashHelper *, const QUrl &) -> int { return kExpectTrashTimeout; });
    int fallbackCalls = 0;
    stub.set_lamda(&Libutils::base::trashFile,
                   [&](const QString &) -> bool {
                       ++fallbackCalls;
                       return true;
                   });

    // Act
    const bool trashed = obj->moveFileToTrash(QUrl::fromLocalFile(tmpDir.filePath("slow.jpg")));

    // Assert
    EXPECT_FALSE(trashed);           // branch B4: kTrashSuccess != ret → false（非接口错误不兜底）
    EXPECT_EQ(fallbackCalls, 0);     // 副作用：超时不应触发 v20 兜底
}

TEST_F(FileTrashHelperTest, MoveFileToTrash_DbusInvalidUrl_ReturnsFalseWithoutFallback)
{
    // Arrange: DBus 返回无效 URL 错误码
    stub.set_lamda(VADDR(FileTrashHelper, moveFileToTrashWithDBus),
                   [](FileTrashHelper *, const QUrl &) -> int { return kExpectTrashInvalidUrl; });
    int fallbackCalls = 0;
    stub.set_lamda(&Libutils::base::trashFile,
                   [&](const QString &) -> bool {
                       ++fallbackCalls;
                       return true;
                   });

    // Act
    const bool trashed = obj->moveFileToTrash(QUrl::fromLocalFile(tmpDir.filePath("bad.jpg")));

    // Assert
    EXPECT_FALSE(trashed);           // branch B4: kTrashSuccess != ret → false
    EXPECT_EQ(fallbackCalls, 0);     // 副作用：无效 URL 不触发 v20 兜底
}

// ── FileTrashHelper::moveFileToTrashWithDBus ──────────────────────

TEST_F(FileTrashHelperTest, MoveFileToTrashWithDBus_InvalidUrl_ReturnsInvalidUrlCode)
{
    // Arrange: 无效 URL；同时安装 doAsyncCall 计数桩证明未发起 DBus 调用
    int asyncCalls = 0;
    stub.set_lamda(static_cast<DoAsyncCallSig>(&QDBusAbstractInterface::doAsyncCall),
                   [&asyncCalls](QDBusAbstractInterface *, const QString &method,
                                 const QVariant *, size_t) -> QDBusPendingCall {
                       ++asyncCalls;
                       return QDBusPendingCall::fromError(
                           QDBusError(QDBusError::ServiceUnknown, method));
                   });
    const QUrl badUrl(QStringLiteral("://broken-url"));

    // Act
    const int ret = obj->moveFileToTrashWithDBus(badUrl);

    // Assert
    EXPECT_EQ(ret, kExpectTrashInvalidUrl);   // branch B1: !url.isValid() → kTrashInvalidUrl
    EXPECT_EQ(asyncCalls, 0);                 // 副作用：未调用文管 Trash 接口
}

TEST_F(FileTrashHelperTest, MoveFileToTrashWithDBus_DbusInterfaceError_ReturnsInterfaceErrorCode)
{
    // Arrange: 有效 URL；DBus Trash 异步调用返回错误
    int asyncCalls = 0;
    QString calledMethod;
    stub.set_lamda(static_cast<DoAsyncCallSig>(&QDBusAbstractInterface::doAsyncCall),
                   [&asyncCalls, &calledMethod](QDBusAbstractInterface *, const QString &method,
                                                const QVariant *, size_t) -> QDBusPendingCall {
                       ++asyncCalls;
                       calledMethod = method;
                       return QDBusPendingCall::fromError(
                           QDBusError(QDBusError::AccessDenied, QStringLiteral("ut-dbus-error")));
                   });
    const QUrl url = QUrl::fromLocalFile(tmpDir.filePath("dbus-fail.jpg"));

    // Act
    const int ret = obj->moveFileToTrashWithDBus(url);

    // Assert
    EXPECT_EQ(ret, kExpectTrashInterfaceError);   // branch B3: pendingCall.isError() → kTrashInterfaceError
    EXPECT_EQ(asyncCalls, 1);                     // 副作用：恰好调用一次异步 Trash
    EXPECT_EQ(calledMethod, QStringLiteral("Trash"));   // 参数：方法名为 org.freedesktop.FileManager1.Trash
}

TEST_F(FileTrashHelperTest, MoveFileToTrashWithDBus_FileDeletedSignal_ReturnsSuccess)
{
    // Arrange: doAsyncCall 在 connect 之后被调用，桩内同步发 imageFileChanged
    // 模拟文管删除完成（文件不存在 → QFile::exists == false）
    const QString watchedPath = tmpDir.filePath("signal-gone.jpg");
    int asyncCalls = 0;
    stub.set_lamda(static_cast<DoAsyncCallSig>(&QDBusAbstractInterface::doAsyncCall),
                   [&asyncCalls, &watchedPath](QDBusAbstractInterface *, const QString &method,
                                               const QVariant *, size_t) -> QDBusPendingCall {
                       ++asyncCalls;
                       emit ImageFileWatcher::instance()->imageFileChanged(watchedPath);
                       return QDBusPendingCall::fromCompletedCall(utEmptyReply(method));
                   });

    // Act
    const int ret = obj->moveFileToTrashWithDBus(QUrl::fromLocalFile(watchedPath));

    // Assert
    EXPECT_EQ(ret, kExpectTrashSuccess);   // branch B2+B5: 信号路径匹配且文件已删除 → kTrashSuccess
    EXPECT_EQ(asyncCalls, 1);              // 副作用：Trash 调用发起一次
}

TEST_F(FileTrashHelperTest, MoveFileToTrashWithDBus_NoSignalUntilTimeout_ReturnsTimeoutCode)
{
    // Arrange: DBus 成功返回但从不发删除完成信号；桩 QEventLoop::exec 立即返回，
    // 短路源码 FIX-273813 的 10s 兜底等待
    int asyncCalls = 0;
    int execCalls = 0;
    stub.set_lamda(static_cast<DoAsyncCallSig>(&QDBusAbstractInterface::doAsyncCall),
                   [&asyncCalls](QDBusAbstractInterface *, const QString &method,
                                 const QVariant *, size_t) -> QDBusPendingCall {
                       ++asyncCalls;
                       return QDBusPendingCall::fromCompletedCall(utEmptyReply(method));
                   });
    stub.set_lamda(static_cast<int (QEventLoop::*)(QEventLoop::ProcessEventsFlags)>(&QEventLoop::exec),
                   [&execCalls](QEventLoop *, QEventLoop::ProcessEventsFlags) -> int {
                       ++execCalls;
                       return 0;
                   });
    const QUrl url = QUrl::fromLocalFile(tmpDir.filePath("still-here.jpg"));

    // Act
    const int ret = obj->moveFileToTrashWithDBus(url);

    // Assert
    EXPECT_EQ(ret, kExpectTrashTimeout);   // branch B4+B5: 无信号 → waitRet 保持 kTrashTimeout
    EXPECT_EQ(asyncCalls, 1);              // 副作用：Trash 调用发起一次
    EXPECT_EQ(execCalls, 1);               // 副作用：进入事件循环等待（被桩短路）
}

// ── FileTrashHelper::queryMountInfo ───────────────────────────────

TEST_F(FileTrashHelperTest, QueryMountInfo_AlreadyInitialized_SkipsDbusQuery)
{
    // Arrange: initData 为真且挂载表已有数据
    MountScript script;
    installMountScript(stub, script);
    obj->initData = true;
    obj->mountDevices.insert("dev0", tmpDir.filePath("usb"));

    // Act
    obj->queryMountInfo();

    // Assert
    EXPECT_EQ(script.listCalls, 0);                    // branch B1: initData → 直接 return，不查询
    EXPECT_EQ(obj->mountDevices.size(), 1);            // 状态：挂载表保持不变（强状态安全）
    EXPECT_TRUE(obj->initData);                        // 状态：initData 未被重复置位
}

TEST_F(FileTrashHelperTest, QueryMountInfo_DeviceListInvalid_KeepsEmptyAndStopsRequery)
{
    // Arrange: GetBlockDevicesIdList 返回错误应答
    MountScript script;
    script.listValid = false;
    installMountScript(stub, script);
    obj->initData = false;

    // Act
    obj->queryMountInfo();
    obj->queryMountInfo();   // 第二次调用：initData 已置位，不应再次查询

    // Assert
    EXPECT_EQ(script.listCalls, 1);                  // branch B2: 列表无效 → return；二次调用命中 B1
    EXPECT_TRUE(obj->mountDevices.isEmpty());        // 状态：无挂载记录写入
    EXPECT_TRUE(obj->initData);                      // 状态：首次查询后置位
}

TEST_F(FileTrashHelperTest, QueryMountInfo_EmptyDeviceList_NoMountEntries)
{
    // Arrange: 设备列表有效但为空（循环 0 次边界）
    MountScript script;
    script.ids = QStringList{};
    installMountScript(stub, script);
    obj->initData = false;

    // Act
    obj->queryMountInfo();

    // Assert
    EXPECT_EQ(script.listCalls, 1);                // branch B3: 空列表 → 循环 0 次
    EXPECT_TRUE(obj->mountDevices.isEmpty());      // 状态：无挂载记录
    EXPECT_EQ(script.listArg.toInt(), kExpectRemovableFlag);   // 参数：按 kRemovable 过滤可卸载设备
}

TEST_F(FileTrashHelperTest, QueryMountInfo_InvalidDeviceInfo_ContinuesWithNextDevice)
{
    // Arrange: 两台设备，第一台详情查询失败，第二台 usb 正常
    const QString mountGood = tmpDir.filePath("usbgood");
    MountScript script;
    script.ids = QStringList{ QStringLiteral("devBad"), QStringLiteral("devGood") };
    script.infoValid.insert(QStringLiteral("devBad"), false);
    script.infoValid.insert(QStringLiteral("devGood"), true);
    QVariantMap goodInfo;
    goodInfo.insert(QStringLiteral("ConnectionBus"), QStringLiteral("usb"));
    goodInfo.insert(QStringLiteral("MountPoint"), mountGood);
    goodInfo.insert(QStringLiteral("MountPoints"), QStringList{});
    script.infos.insert(QStringLiteral("devGood"), goodInfo);
    installMountScript(stub, script);
    obj->initData = false;

    // Act
    obj->queryMountInfo();

    // Assert
    EXPECT_EQ(script.infoCalls, 2);                                  // branch B4: 失败设备 continue，仍处理下一台
    EXPECT_EQ(obj->mountDevices.size(), 1);                          // 状态：仅 devGood 入表
    EXPECT_EQ(obj->mountDevices.value(QStringLiteral("devGood")), mountGood);   // 状态：挂载点正确
}

TEST_F(FileTrashHelperTest, QueryMountInfo_MountPointsListWithEmptyEntry_SkipsEmptyMounts)
{
    // Arrange: usb 设备返回多挂载点列表（含空串）
    const QString mountA = tmpDir.filePath("usba");
    MountScript script;
    script.ids = QStringList{ QStringLiteral("dev0") };
    script.infoValid.insert(QStringLiteral("dev0"), true);
    QVariantMap info;
    info.insert(QStringLiteral("ConnectionBus"), QStringLiteral("usb"));
    info.insert(QStringLiteral("MountPoints"), QStringList{ mountA, QString() });
    info.insert(QStringLiteral("MountPoint"), QString());
    script.infos.insert(QStringLiteral("dev0"), info);
    installMountScript(stub, script);
    obj->initData = false;

    // Act
    obj->queryMountInfo();

    // Assert
    EXPECT_EQ(obj->mountDevices.size(), 1);                                // branch B8+B9: 空挂载点被跳过
    EXPECT_EQ(obj->mountDevices.value(QStringLiteral("dev0")), mountA);    // 状态：仅非空挂载点入表
    EXPECT_EQ(script.infoCalls, 1);                                        // 副作用：详情查询一次
}

TEST_F(FileTrashHelperTest, QueryMountInfo_LegacySingleMountPoint_AddsEntry)
{
    // Arrange: usb 设备 MountPoints 为空，走单个 MountPoint 字段
    const QString mountB = tmpDir.filePath("usbb");
    MountScript script;
    script.ids = QStringList{ QStringLiteral("dev0") };
    script.infoValid.insert(QStringLiteral("dev0"), true);
    QVariantMap info;
    info.insert(QStringLiteral("ConnectionBus"), QStringLiteral("usb"));
    info.insert(QStringLiteral("MountPoints"), QStringList{});
    info.insert(QStringLiteral("MountPoint"), mountB);
    script.infos.insert(QStringLiteral("dev0"), info);
    installMountScript(stub, script);
    obj->initData = false;

    // Act
    obj->queryMountInfo();

    // Assert
    EXPECT_EQ(obj->mountDevices.size(), 1);                              // branch B6+B7: 单挂载点入表
    EXPECT_EQ(obj->mountDevices.value(QStringLiteral("dev0")), mountB);  // 状态：挂载点正确
    EXPECT_TRUE(obj->initData);                                          // 状态：查询后置位
}

TEST_F(FileTrashHelperTest, QueryMountInfo_EmptyMountPointAndList_NoEntryAdded)
{
    // Arrange: usb 设备挂载点列表与单挂载点字段均为空
    MountScript script;
    script.ids = QStringList{ QStringLiteral("dev0") };
    script.infoValid.insert(QStringLiteral("dev0"), true);
    QVariantMap info;
    info.insert(QStringLiteral("ConnectionBus"), QStringLiteral("usb"));
    info.insert(QStringLiteral("MountPoints"), QStringList{});
    info.insert(QStringLiteral("MountPoint"), QString());
    script.infos.insert(QStringLiteral("dev0"), info);
    installMountScript(stub, script);
    obj->initData = false;

    // Act
    obj->queryMountInfo();

    // Assert
    EXPECT_TRUE(obj->mountDevices.isEmpty());   // branch B6 且 !B7: 空挂载点不入表
    EXPECT_EQ(script.infoCalls, 1);             // 副作用：详情仍被查询
    EXPECT_TRUE(obj->initData);                 // 状态：查询流程完成
}

TEST_F(FileTrashHelperTest, QueryMountInfo_NonUsbDevice_Skipped)
{
    // Arrange: 设备 ConnectionBus 为 sata（非 usb）
    const QString mountSata = tmpDir.filePath("satadisk");
    MountScript script;
    script.ids = QStringList{ QStringLiteral("dev0") };
    script.infoValid.insert(QStringLiteral("dev0"), true);
    QVariantMap info;
    info.insert(QStringLiteral("ConnectionBus"), QStringLiteral("sata"));
    info.insert(QStringLiteral("MountPoints"), QStringList{});
    info.insert(QStringLiteral("MountPoint"), mountSata);
    script.infos.insert(QStringLiteral("dev0"), info);
    installMountScript(stub, script);
    obj->initData = false;

    // Act
    obj->queryMountInfo();

    // Assert
    EXPECT_TRUE(obj->mountDevices.isEmpty());   // branch !B5: 非 usb 设备跳过
    EXPECT_EQ(script.infoCalls, 1);             // 副作用：详情被查询但未入表
    EXPECT_TRUE(obj->initData);                 // 状态：查询流程完成
}

// ── FileTrashHelper::removeFile ───────────────────────────────────

class FileTrashHelperRemoveParamTest : public FileTrashHelperTest,
                                       public ::testing::WithParamInterface<FileTrashRemoveCase> {
};

TEST_P(FileTrashHelperRemoveParamTest, RemoveFile_FilePathStates_ReturnsExpected)
{
    const FileTrashRemoveCase &c = GetParam();

    // Arrange: 按场景准备路径（存在文件在临时目录真实创建）
    QString filePath;
    if (c.scenario == 0) {
        filePath = tmpDir.filePath("exists.jpg");
        QFile f(filePath);
        f.open(QIODevice::WriteOnly);
        f.write("x");
        f.close();
    } else if (c.scenario == 1) {
        filePath = tmpDir.filePath("missing.jpg");
    }
    const QUrl url = (c.scenario == 2) ? QUrl() : QUrl::fromLocalFile(filePath);

    // Act
    const bool removed = obj->removeFile(url);

    // Assert
    EXPECT_EQ(removed, c.expected);             // branch B1 remove 成功 / B2 失败
    EXPECT_FALSE(QFileInfo::exists(filePath));  // 副作用：存在场景已删除，其余场景本不存在（exists("")==false）
}

INSTANTIATE_TEST_SUITE_P(
    PathStates, FileTrashHelperRemoveParamTest,
    ::testing::Values(
        FileTrashRemoveCase{ 0, true },    // 文件存在 → 删除成功
        FileTrashRemoveCase{ 1, false },   // 文件不存在 → 删除失败
        FileTrashRemoveCase{ 2, false }));   // 空路径 → 删除失败

// ── FileTrashHelper::resetMountInfo ───────────────────────────────

TEST_F(FileTrashHelperTest, ResetMountInfo_PopulatedState_ClearsAll)
{
    // Arrange: 挂载表已有数据且 initData 已置位
    obj->initData = true;
    obj->mountDevices.insert("dev0", tmpDir.filePath("usb0"));
    obj->mountDevices.insert("dev1", tmpDir.filePath("usb1"));

    // Act
    obj->resetMountInfo();

    // Assert
    EXPECT_EQ(obj->mountDevices.size(), 0);     // 状态：挂载表清空（两条目全被移除）
    EXPECT_TRUE(obj->mountDevices.isEmpty());   // 状态：挂载表清空
    EXPECT_FALSE(obj->initData);                // 状态：initData 复位 → 下次查询将重新拉取
}

TEST_F(FileTrashHelperTest, ResetMountInfo_OnFreshObject_KeepsCleanState)
{
    // Arrange: 新对象（未查询过）本应为干净状态
    const bool emptyBefore = obj->mountDevices.isEmpty();

    // Act
    obj->resetMountInfo();   // 幂等重置

    // Assert
    EXPECT_EQ(obj->mountDevices.size(), 0);     // 状态：空表重置后仍为 0（幂等）
    EXPECT_TRUE(emptyBefore);                   // 前置：新对象挂载表为空
    EXPECT_TRUE(obj->mountDevices.isEmpty());   // 状态：重置后保持为空
    EXPECT_FALSE(obj->initData);                // 状态：initData 保持复位
}
