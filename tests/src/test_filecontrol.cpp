// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | UrlInfo (free) | low | - | 1 | 3 |
// | compareByFileInfo (free) | low | - | 1 | 4 |
// | FileControl(parent) ctor | low | - | 1 | 1 |
// | ~FileControl | low | - | 1 | 1 |
// | addImageFile | low | - | 1 | 1 |
// | copyImage | low | - | 1 | 2 |
// | copyText | low | - | 1 | 1 |
// | createShortcutString | mid | - | 2 | 2 |
// | deleteImagePath | mid | - | 2 | 4 |
// | displayinFileManager | low | - | 1 | 2 |
// | getCompanyLogo | low | - | 1 | 1 |
// | getConfigValue | low | - | 1 | 1 |
// | getDirImagePath | high | - | 3 | 4 |
// | getNamePath | low | - | 1 | 3 |
// | getPrimaryScreenCenterX | low | - | 1 | 2 |
// | getPrimaryScreenCenterY | low | - | 1 | 2 |
// | getlastHeight | low | - | 1 | 2 |
// | getlastWidth | low | - | 1 | 4 |
// | isCanDelete | mid | - | 2 | 3 |
// | isCanReadable | low | - | 1 | 2 |
// | isCanRename | mid | - | 2 | 3 |
// | isCanSupportOcr | mid | - | 2 | 3 |
// | isCanWrite | mid | - | 2 | 2 |
// | isCheckOnly | low | - | 1 | 1 |
// | isCurrentWatcherDir | low | - | 1 | 1 |
// | isEnableNavigation | low | - | 1 | 1 |
// | isImage | mid | - | 2 | 5 |
// | isRotatable | mid | - | 2 | 3 |
// | isShowToolTip | mid | - | 2 | 3 |
// | isSupportSetWallpaper | low | - | 1 | 3 |
// | ocrImage | mid | - | 2 | 2 |
// | parseCommandlineGetPath | low | - | 1 | 2 |
// | resetImageFiles | mid | - | 2 | 2 |
// | saveSetting | mid | - | 2 | 2 |
// | setConfigValue | low | - | 1 | 1 |
// | setEnableNavigation | low | - | 1 | 1 |
// | setSettingHeight | low | - | 1 | 1 |
// | setSettingWidth | low | - | 1 | 1 |
// | setWallpaper | high | - | 3 | 5 |
// | showPrintDialog | low | - | 1 | 1 |
// | showShortcutPanel | low | - | 1 | 1 |
// | slotFileReName | mid | - | 2 | 4 |
// | slotFileSuffix | mid | - | 2 | 4 |
// | slotGetFileName | mid | - | 2 | 2 |
// | slotGetFileNameSuffix | mid | - | 2 | 2 |
// | slotGetInfo | low | - | 1 | 2 |
// | standardPicturesPath | low | - | 1 | 1 |
// | terminateShortcutPanelProcess | mid | - | 2 | 2 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（46 类方法 + 2 自由函数全覆盖）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]（空/缺失/单元素/多元素/极值）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（compareByFileInfo/getNamePath/isImage/slotFileSuffix）
// 5. 分支清单 → 用例映射已列出: [x]（见下方分支清单块）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（B3 类防御分支见各清单说明）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无显式 throw；错误路径以返回值/状态断言覆盖）
// 8. 负面场景有专门用例: [x]（Invalid/Missing/Empty 系列）
// 9. 负面用例验证强异常安全: [x]（失败后成员状态/文件仍在断言）
// 10. stub_ext vs gMock 选择正确: [x]（Qt 类/DBus 全 static_cast；项目内部类 VADDR；无虚接口注入点，不适用 gMock）
//
// ─────────────────────────────────────────────────────────────
// 分支清单 → 用例映射（来源：get_code_snippet 真实源码）
// ─────────────────────────────────────────────────────────────
// QUrl UrlInfo(QString path) [自由函数]
// B1: QFile::exists(path) → fromLocalFile(QDir::current().absoluteFilePath(path))
// B2: 正则 ":(\d+)(?::(\d+))?:?$" 命中 → path.chop(裁剪行/列号)
// B3: fromUserInput 结果 !isValid() → fromLocalFile 兜底（防御分支：QUrl::fromUserInput
//     对任意输入几乎均产出合法 URL，无法稳定构造 invalid 输入，标记不可达/note）
// B4: 正常 fromUserInput(path, currentPath, AssumeLocalFile)
// 映射： UrlInfo_ExistingFile_ReturnsAbsoluteFileUrl→B1
//        UrlInfo_LineColumnSuffix_StrippedBeforeConversion→B2+B4
//        UrlInfo_NonLocalInput_ConvertedFromUserInput→B4
//
// bool compareByFileInfo(const QFileInfo&, const QFileInfo&) [自由函数]
// B1: collator.compare(base1, base2) < 0 → true（numericMode 数字序）
// B2: >= 0 → false
// 映射： CompareByFileInfo_NumericAndLexicalOrder_ParamSet→B1+B2（TEST_P 4 组）
//
// FileControl::FileControl(parent)
// B1: !m_tSaveSetting → new QTimer + connect 防抖保存
// B2: listsupportWallPaper 填充 11 项
// 映射： FileControl_Constructor_InitializesMembersAndFormats→B1+B2
//
// FileControl::~FileControl()
// B1: 析构 → saveSetting()（待保存尺寸经 setConfigValue 落库）
// 映射： FileControl_Destructor_SavesPendingWindowSize→B1
//
// FileControl::addImageFile(filePath)
// B1: 委托 imageFileWatcher->addImageFile 透传
// 映射： AddImageFile_DelegatesToWatcher_ParamPassed→B1
//
// FileControl::copyImage(path)
// B1: localPath 非空 → text += localPath+'\n'（setText 去尾换行）
// B2: localPath 为空 → text 为空串；gnome 格式仍写入 "copy\n"
// 映射： CopyImage_LocalPath_FillsClipboardMime→B1
//        CopyImage_EmptyLocalPath_CopiesEmptyText→B2
//
// FileControl::copyText(str)
// B1: clipboard()->setText(str)
// 映射： CopyText_PlainText_StoredInClipboard→B1
//
// QString FileControl::createShortcutString()
// B1: m_shortcutString 非空 → 返回缓存
// B2: 为空 → 构建 3 组 22 项 JSON
// 映射： CreateShortcutString_FirstCall_BuildsGroupedJson→B2
//        CreateShortcutString_SecondCall_ReturnsCachedString→B1
//
// 分支清单（来源：FileControl::deleteImagePath）
// B1: !displayUrl.isValid() → return false（末尾兜底）
// B2: Trash 异步调用发起（asyncCallWithArgumentList）
// B3: while !pendingCall.isFinished() → processEvents 等待
// B4: pendingCall.isError() → return false
// B5: isError 假 → 落到文件存在性检查
// B6: QFile::exists(toLocalFile) 仍真 → return false
// B7: exists 假（已删除）→ return true
// 映射： DeleteImagePath_InvalidUrl_ReturnsFalse→B1
//        DeleteImagePath_DbusError_ReturnsFalse→B4
//        DeleteImagePath_FileStillExists_ReturnsFalse→B6
//        DeleteImagePath_TrashSucceeds_ReturnsTrue→B7
//
// bool FileControl::displayinFileManager(path)
// B1: !interface.isValid() → return false（bRet 初值）
// B2: isValid 且 ShowItems 非 ErrorMessage → true
// 映射： DisplayinFileManager_InterfaceInvalid_ReturnsFalse→B1
//        DisplayinFileManager_ShowItemsSucceeds_ReturnsTrue→B2
//
// QUrl FileControl::getCompanyLogo()
// B1: DSysInfo::distributionOrgLogo 取路径 → QUrl::fromLocalFile 包装
// 映射： GetCompanyLogo_DistributionLogoPath_WrappedAsFileUrl→B1
//
// QVariant FileControl::getConfigValue(group,key,default)
// B1: m_config->value 透传并返回
// 映射： GetConfigValue_DelegatesToSetter_ReturnsDefault→B1
//
// 分支清单（来源：FileControl::getDirImagePath）
// B1: path.isEmpty() → return {}（早退）
// B2: for 遍历 entryInfoList 排序结果
// B3: tmpPath.isEmpty() → continue（防御，难稳定构造）
// B4: isImage(tmpPath) 真 → 追加 file URL
// B5: 非图片 → 跳过
// 映射： GetDirImagePath_EmptyPath_ReturnsEmptyList→B1
//        GetDirImagePath_NoImageFiles_ReturnsEmptyList→B5
//        GetDirImagePath_MixedFiles_ReturnsNumericallySortedUrls→B2+B4（1/2/10 数字序）
//        GetDirImagePath_SingleImage_ReturnsOneEntry→B2+B4（循环 1 次边界）
//
// QString FileControl::getNamePath(oldPath,newName)
// B1: old.startsWith("file://") → 转 localFile
// B2: now(newName) startsWith("file://") → 转 localFile（结果被丢弃，
//     newPath 仍拼 newName 原串——疑似缺陷 D2，按现状断言并标红）
// 映射： GetNamePath_ParamSet（TEST_P）→B1/B2 组合 3 组
//
// int FileControl::getPrimaryScreenCenterX(windowWidth)
// B1: !screen → return 0
// B2: 正常 → geometry.x()+width/2-windowWidth/2
// 映射： GetPrimaryScreenCenterX_NullScreen_ReturnsZero→B1
//        GetPrimaryScreenCenterX_RealScreen_MatchesGeometryCalc→B2
// （getPrimaryScreenCenterY 同构：CenterY 两用例）
//
// 分支清单（来源：FileControl::getlastWidth）
// B1: !screen → return MAINWIDGET_MINIMUN_WIDTH（早退）
// B2: screens().size()>1 → defaultW=size().width()*0.6
// B3: 单屏 → defaultW=geometry().width()*0.6
// B4: ww >= MIN → reWidth=ww
// B5: ww < MIN → reWidth=MIN
// 映射： GetlastWidth_ConfigAboveDefault_ReturnsConfigured→B4（B3）
//        GetlastWidth_SmallConfigValue_ClampedToMinimum→B5
//        GetlastWidth_NullScreen_ReturnsMinimum→B1
//        GetlastWidth_MultipleScreens_UsesSizeBasedDefault→B2+B4
//
// 分支清单（来源：FileControl::getlastHeight）
// B1: !screen → return MAINWIDGET_MINIMUN_HEIGHT（早退）
// B2: screens().size()>1 → defaultH=size().height()*0.6
// B3: 单屏 → defaultH=geometry().height()*0.6
// B4: wh >= MIN → reHeight=wh
// B5: wh < MIN → reHeight=MIN
// 映射： GetlastHeight_ConfigAboveDefault_ReturnsConfigured→B4（B3）
//        GetlastHeight_SmallConfigValue_ClampedToMinimum→B5
//
// bool FileControl::isCanDelete(path)
// B1: 非特殊 PathType && isWritable && isReadable → true
// B2: 特殊 PathType（MTP 等）→ false
// B3: 不可写 → false（isAlbum 恒 false，第二短路分支永不生效——疑似缺陷 D4）
// 映射： IsCanDelete_LocalWritableFile_ReturnsTrue→B1
//        IsCanDelete_MtpPathType_ReturnsFalse→B2
//        IsCanDelete_ReadOnlyFile_ReturnsFalse→B3
//
// bool FileControl::isCanReadable(path)
// B1: info.isReadable() → true
// B2: 不可读/不存在 → false
// 映射： IsCanReadable_ExistingFile_ReturnsTrue→B1
//        IsCanReadable_MissingFile_ReturnsFalse→B2
//
// bool FileControl::isCanRename(path)
// B1: 可读&&可写&&非 MTP/PTP/APPLE → true
// B2: MTP 类型 → false
// B3: 文件不存在 → false
// 映射： IsCanRename_LocalWritableFile_ReturnsTrue→B1
//        IsCanRename_MtpPathType_ReturnsFalse→B2
//        IsCanRename_MissingFile_ReturnsFalse→B3
//
// bool FileControl::isCanSupportOcr(path)
// B1: type != Dynamic && isReadable → true
// B2: type == Dynamic → false
// B3: 不可读 → false
// 映射： IsCanSupportOcr_StaticImageReadable_ReturnsTrue→B1
//        IsCanSupportOcr_DynamicImageType_ReturnsFalse→B2
//        IsCanSupportOcr_MissingFile_ReturnsFalse→B3
//
// bool FileControl::isCanWrite(path)
// B1: 文件可写 && 父目录可写 → true
// B2: 文件只读 → false
// 映射： IsCanWrite_WritableFileAndDir_ReturnsTrue→B1
//        IsCanWrite_ReadOnlyFile_ReturnsFalse→B2
//
// 分支清单（来源：FileControl::isCheckOnly）
// B1: 锁目录不存在 → mkpath
// B2: open 锁文件 → 返回 fd（fd==-1 见 B5）
// B3: lockf 失败(flock==-1) → return false（同进程二次加锁触发）
// B4: 成功 → return true（fd 未 close、锁未释放——疑似缺陷 D3，行为按现状断言）
// B5: open 失败(fd==-1) → return false（环境异常分支，无法稳定注入，note）
// 映射： IsCheckOnly_RepeatedLockAttempts_BothSucceed→B4（B3 仅跨进程触发，单进程内不可达）
//
// bool FileControl::isCurrentWatcherDir(path)
// B1: 委托 imageFileWatcher->isCurrentDir
// 映射： IsCurrentWatcherDir_DelegatesToWatcher_BothBranches→B1（true/false 两态）
//
// bool FileControl::isEnableNavigation()
// B1: getConfigValue(...,true) 透传
// 映射： IsEnableNavigation_ConfigDefaultTrue_ReturnsTrue→B1
//
// bool FileControl::isImage(path)
// B1: url.isLocalFile() → toLocalFile
// B2: mt(内容) image/* 或 video/x-mng → true
// B3: mt1(扩展) image/* 或 video/x-mng → true
// B4: 均否 → false
// 映射： IsImage_ParamSet→B1~B4（TEST_P 5 组：真图/纯文本/图内容伪装扩展/文本伪装图扩展/URL）
//
// bool FileControl::isRotatable(path)
// B1: !isFile || !exists || !writable → false
// B2: 依赖 isImageSupportRotate=true → true
// B3: isImageSupportRotate=false → false
// 映射： IsRotatable_MissingFile_ReturnsFalse→B1
//        IsRotatable_SupportRotateTrue_ReturnsTrue→B2
//        IsRotatable_SupportRotateFalse_ReturnsFalse→B3
//
// 分支清单（来源：FileControl::isShowToolTip）
// B1: completeBaseName == name → return false（早退）
// B2: 目标存在且 != path → true
// B3: 目标不存在或与 path 相同 → false
// 映射： IsShowToolTip_SameBaseName_ReturnsFalse→B1
//        IsShowToolTip_TargetExists_ReturnsTrue→B2
//        IsShowToolTip_TargetMissing_ReturnsFalse→B3
//
// bool FileControl::isSupportSetWallpaper(path)
// B1: 后缀(小写)在 11 项列表 && 可读 → true
// B2: 否则 → false
// 映射： IsSupportSetWallpaper_SupportedSuffix_ReturnsTrue→B1
//        IsSupportSetWallpaper_UnsupportedSuffix_ReturnsFalse→B2
//        IsSupportSetWallpaper_UpperCaseSuffix_NormalizedToLower→B1（大小写归一）
//
// void FileControl::ocrImage(path,index)
// B1: type != MultiImage → 直接 openFile(localPath)
// B2: type == MultiImage → jumpToImage/read → 缓存目录存 rec.png → openFile(rec.png)
// 映射： OcrImage_SinglePage_OpensLocalPathViaDbus→B1
//        OcrImage_MultiPage_SavesCacheAndOpensTemp→B2
//
// 分支清单（来源：FileControl::parseCommandlineGetPath）
// B1: for 遍历 QCoreApplication::arguments()
// B2: QFileInfo(path).isFile() 假 → 跳过该参数
// B3: isImage(path) 真 → return file URL（早退）
// B4: 循环自然结束（无图片）→ return ""
// 映射： ParseCommandlineGetPath_ImageArgument_ReturnsFileUrl→B1+B2+B3
//        ParseCommandlineGetPath_NoImageArgument_ReturnsEmpty→B1+B2+B4
//
// void FileControl::resetImageFiles(filePaths)
// B1: imageFileWatcher->resetImageFiles 透传
// B2: ImageInfo::clearCache()
// 映射： ResetImageFiles_PathList_DelegatesAndClearsCache→B1+B2
//        ResetImageFiles_EmptyList_StillResetsAndClears→B1+B2（空集边界）
//
// void FileControl::saveSetting()
// B1: m_lastSaveWidth != m_windowWidth → 写 W 并同步 lastSave
// B2: 相等 → 跳过
// B3: m_lastSaveHeight != m_windowHeight → 写 H 并同步
// B4: 相等 → 跳过
// 映射： SaveSetting_BothSizesChanged_WritesEachOnce→B1+B3+B2+B4（二次调用去重）
//        SaveSetting_OnlyHeightChanged_WritesHeightOnly→B3+B2
//
// void FileControl::setConfigValue(group,key,value)
// B1: m_config->setValue 透传
// 映射： SetConfigValue_DelegatesToSetter_ValueRecorded→B1
//
// void FileControl::setEnableNavigation(b)
// B1: setConfigValue(...,b) 透传
// 映射： SetEnableNavigation_BoolValue_PassedToConfig→B1（true/false 两态）
//
// void FileControl::setSettingHeight(height)
// B1: m_windowHeight=height + 单次定时器 start(1000)
// 映射： SetSettingHeight_NewHeight_UpdatesMemberAndArmsTimer→B1
// （setSettingWidth 同构：SetSettingWidth_NewWidth_UpdatesMemberAndArmsTimer→B1）
//
// 分支清单（来源：FileControl::setWallpaper）
// B1: imgPath.isNull() → 线程体无操作
// B2: 外层 FLATPAK 恒真 if → 进入 DBus 设置路径
// B3: interfaceV23/V20 均 !isValid() → 仅告警不调用
// B4: 会话判定（XDG_SESSION_TYPE/WAYLAND_DISPLAY 是否含 wayland）
// B5: isWayland=true → Wayland Display 接口取主屏名（V23 valid 走 property / else V20）
// B6: isWayland=false（X11）→ primaryScreen()->name()（未判空——疑似缺陷 D1）
// B7: 三元日志描述（isWayland ? "Wayland" : "X11"）
// B8: interfaceV23.isValid() → SetMonitorBackground
// B9: V23 应答 error → settingSucc=false
// B10: interfaceV20.isValid() && !settingSucc → V20 兜底调用
// 映射： SetWallpaper_NullPath_SkipsAllDbusCalls→B1
//        SetWallpaper_AllInterfacesInvalid_NoDbusCall→B2+B3
//        SetWallpaper_X11Session_CallsV23WithImagePath→B4+B6+B8（成功态）
//        SetWallpaper_WaylandSession_ReadsPrimaryAndCallsV23→B4+B5+B8
//        SetWallpaper_V23Fails_FallsBackToV20→B9+B10
//
// void FileControl::showPrintDialog(path)
// B1: PrintHelper::getIntance()->showPrintDialog({localPath})
// 映射： ShowPrintDialog_LocalImagePath_DelegatesToHelper→B1
//
// void FileControl::showShortcutPanel(x,y)
// B1: 构造 "-j=json" "-p=x,y" 并 start("deepin-shortcut-viewer")（先 terminate 旧进程）
// 映射： ShowShortcutPanel_CenterPosition_StartsViewerWithJsonAndPos→B1
//
// 分支清单（来源：FileControl::slotFileReName）
// B1: file.exists() 假 → return false（末尾兜底）
// B2: isSuffix=true → newName=path+"/"+name
// B3: isSuffix=false → newName=path+"/"+name+"."+suffix
// B4: rename 成功 → watcher 通知 + imageRenamed 信号 + return true（早退）
// B5: rename 失败 → return false
// 映射： SlotFileReName_WithSuffixTrue_RenamesAndEmitsSignal→B2+B4
//        SlotFileReName_WithoutSuffix_KeepsOldSuffix→B3+B4
//        SlotFileReName_MissingFile_ReturnsFalse→B1
//        SlotFileReName_TargetDirMissing_ReturnsFalse→B5
//
// QString FileControl::slotFileSuffix(path,ret)
// B1: path 空 || 文件不存在 → ""
// B2: ret=true → "."+completeSuffix
// B3: ret=false → completeSuffix
// 映射： SlotFileSuffix_ParamSet→B1+B2+B3（TEST_P 4 组）
//
// QString FileControl::slotGetFileName(path)
// B1: startsWith("file://") → toLocalFile
// B2: completeBaseName
// 映射： SlotGetFileName_PlainPath_ReturnsBaseName→B2
//        SlotGetFileName_FileUrl_ReturnsBaseName→B1+B2
// （slotGetFileNameSuffix 同构两用例）
//
// QString FileControl::slotGetInfo(key,path)
// B1: localPath != m_currentPath → 刷新 getAllMetaData 缓存
// B2: 相同 → 复用缓存
// B3: value 为空 → 返回 "-"
// 映射： SlotGetInfo_FirstLookup_LoadsMetadataOnce→B1（+B2 复用计数不变）
//        SlotGetInfo_UnknownKey_ReturnsDash→B3
//
// QString FileControl::standardPicturesPath()
// B1: QStandardPaths 图片可写目录（writableLocation of PicturesLocation）
// 映射： StandardPicturesPath_PicturesLocation_MatchesQtValue→B1
//
// void FileControl::terminateShortcutPanelProcess()
// B1: terminate() 后 waitForFinished(2000)
// 映射： TerminateShortcutPanelProcess_IdleProcess_TerminatesAndWaits→B1
//        TerminateShortcutPanelProcess_WaitReturnsTrue_CompletesQuickly→B1（waitFor 成功态）
//
// ─────────────────────────────────────────────────────────────

#include <gtest/gtest.h>

#include <QApplication>
#include <QClipboard>
#include <QCollator>
#include <QCoreApplication>
#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QProcess>
#include <QScreen>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVariant>

#include <atomic>

#include <DSysInfo>

#include "stub_ext/stubext.h"

#include "configsetter.h"
#include "filecontrol.h"
#include "imagefilewatcher.h"
#include "imageinfo.h"
#include "ocrinterface.h"
#include "printhelper.h"
#include "types.h"
#include "unionimage.h"
#include "unionimage_global.h"

// filecontrol.cpp 内的自由函数原型（inventory (free) 条目，供直接测试）
QUrl UrlInfo(QString path);
bool compareByFileInfo(const QFileInfo &str1, const QFileInfo &str2);

namespace {

// 在指定目录下生成一张真实 PNG 图片，返回绝对路径
QString makePngFile(const QString &dirPath, const QString &baseName)
{
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::red);
    const QString path = QDir(dirPath).filePath(baseName);
    img.save(path, "PNG");
    return path;
}

// 在指定目录下生成一个纯文本文件，返回绝对路径
QString makeTextFile(const QString &dirPath, const QString &baseName)
{
    const QString path = QDir(dirPath).filePath(baseName);
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write("plain text payload");
        f.close();
    }
    return path;
}

QString localUrl(const QString &path)
{
    return QUrl::fromLocalFile(path).toString();
}

// 探测 sessionBus 上某服务是否已注册（无 DBus 会话时安全返回 false）。
// setWallpaper/displayinFileManager 的 B3~B6 分支可达性依赖桌面服务在位，
// 用例按探测结果自适应断言，保证任意环境（本机桌面 / 干净 CI）稳定。
bool serviceRegistered(const QString &name)
{
    QDBusConnectionInterface *iface = QDBusConnection::sessionBus().interface();
    if (iface == nullptr)
        return false;
    const QDBusReply<bool> reply = iface->isServiceRegistered(name);
    return reply.isValid() && reply.value();
}

}  // namespace

class FileControlTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        // 记录环境变量原值，TearDown 恢复（setWallpaper 会话类型用例会改写）
        origSessionType = qEnvironmentVariable("XDG_SESSION_TYPE");
        origWaylandDisplay = qEnvironmentVariable("WAYLAND_DISPLAY");

        obj = new FileControl();

        // 隔离 LibConfigSetter 单例的真实配置读写，避免污染用户配置
        stub.set_lamda(VADDR(LibConfigSetter, value),
                       [](LibConfigSetter *, const QString &, const QString &,
                          const QVariant &defaultValue) -> QVariant {
                           return defaultValue;
                       });
        stub.set_lamda(VADDR(LibConfigSetter, setValue),
                       [this](LibConfigSetter *, const QString &group, const QString &key,
                              const QVariant &value) {
                               ++cfgWriteCount;
                               lastCfgGroup = group;
                               lastCfgKey = key;
                               lastCfgValue = value;
                           });
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
        if (!origSessionType.isNull())
            qputenv("XDG_SESSION_TYPE", origSessionType.toUtf8());
        else
            qunsetenv("XDG_SESSION_TYPE");
        if (!origWaylandDisplay.isNull())
            qputenv("WAYLAND_DISPLAY", origWaylandDisplay.toUtf8());
        else
            qunsetenv("WAYLAND_DISPLAY");
    }

    // 等待 setWallpaper 的后台线程完成（线程体内均为快速同步调用，stub 拦截全部 DBus
    // 出口，不依赖主线程事件循环）。
    // 禁止在此处 processEvents/sendPostedEvents：stub 窗口内驱动事件分发会把其它
    // 测试 TU 挂接在真实 sessionBus 上的 DBus watcher posted 事件投递到已受 stub
    // 干扰的连接/对象（QDBusConnection 私有事件 delivery 读野指针，全量运行时曾
    // 在此段崩溃）。积压事件由 stub 恢复后的后续正常事件循环消化。
    void waitDbusCalls(int expectedCalls)
    {
        for (int i = 0; i < 600 && dbusCallCount.load() < expectedCalls; ++i)
            QThread::msleep(10);
        QThread::msleep(100);
    }

    stub_ext::StubExt stub;
    FileControl *obj = nullptr;
    QTemporaryDir tmpDir;

    // LibConfigSetter::setValue stub 记录
    int cfgWriteCount = 0;
    QString lastCfgGroup;
    QString lastCfgKey;
    QVariant lastCfgValue;

    // FileControl::setConfigValue / getConfigValue stub 记录
    int innerCfgSetCount = 0;
    QVariant lastInnerCfgValue;
    QVariant lastNavDefault;
    int capturedDefault = 0;

    // DBus stub 计数/记录（callWithArgumentList / asyncCallWithArgumentList）
    std::atomic<int> dbusCallCount{0};
    QString lastDbusMethod;
    QList<QVariant> lastDbusArgs;
    bool failNextDbusCall = false;
    bool trashCallSucceeds = true;

    // ImageFileWatcher stub 记录
    int watcherAddCount = 0;
    QString lastWatcherAddPath;
    int watcherResetCount = 0;
    QStringList lastResetFiles;
    int watcherRenameCount = 0;
    QString lastWatcherRenameOld;
    QString lastWatcherRenameNew;
    int watcherIsCurrentCount = 0;
    bool watcherIsCurrentRet = true;

    // 其它依赖 stub 记录
    int cacheClearCount = 0;
    int metaCallCount = 0;
    QMap<QString, QString> fakeMetaData;
    int printCallCount = 0;
    QStringList lastPrintPaths;
    int procStartCount = 0;
    QString lastProgram;
    QStringList lastProcArgs;
    int procTermCount = 0;
    int procWaitCount = 0;
    int lastWaitMs = 0;
    int timerStartCount = 0;
    int lastTimerMs = 0;
    QStringList fakeArguments;

    QString origSessionType;
    QString origWaylandDisplay;
};

// ═════════════════ 自由函数 UrlInfo ═════════════════

TEST_F(FileControlTest, UrlInfo_ExistingFile_ReturnsAbsoluteFileUrl)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "exists.png");
    ASSERT_TRUE(QFile::exists(png));

    // Act
    const QUrl url = UrlInfo(png);

    // Assert（B1：存在的文件 → fromLocalFile(current.absoluteFilePath)）
    EXPECT_TRUE(url.isLocalFile());
    EXPECT_EQ(url.toLocalFile(), QDir::current().absoluteFilePath(png));
}

TEST_F(FileControlTest, UrlInfo_LineColumnSuffix_StrippedBeforeConversion)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "lines.png");
    // 拼接 ":12:34" 后该路径不存在，触发正则裁剪分支
    const QString withSuffix = png + ":12:34";
    ASSERT_FALSE(QFile::exists(withSuffix));

    // Act
    const QUrl url = UrlInfo(withSuffix);

    // Assert（B2：行/列号被 chop 后按本地文件转换）
    EXPECT_EQ(url.toLocalFile(), png);
    EXPECT_TRUE(url.isLocalFile());
}

TEST_F(FileControlTest, UrlInfo_NonLocalInput_ConvertedFromUserInput)
{
    // Arrange
    const QString missing = QDir(tmpDir.path()).filePath("no-such-file.png");
    ASSERT_FALSE(QFile::exists(missing));

    // Act
    const QUrl url = UrlInfo(missing);

    // Assert（B4：不存在的本地路径经 fromUserInput(AssumeLocalFile) 转换）
    EXPECT_TRUE(url.isValid());
    EXPECT_EQ(url.toLocalFile(), missing);
}

// ═════════════════ 自由函数 compareByFileInfo ═════════════════

namespace {
struct CompareByNameCase {
    QString firstName;
    QString secondName;
    bool expectedLess;
};
}  // namespace

class CompareByFileInfoParamTest : public FileControlTest,
                                   public ::testing::WithParamInterface<CompareByNameCase> {
};

TEST_P(CompareByFileInfoParamTest, CompareByFileInfo_NumericAndLexicalOrder_ParamSet)
{
    // Arrange
    const auto &c = GetParam();
    const QFileInfo first(QDir(tmpDir.path()).filePath(c.firstName));
    const QFileInfo second(QDir(tmpDir.path()).filePath(c.secondName));

    // Act
    const bool less = compareByFileInfo(first, second);

    // Assert（B1/B2：numericMode 下 2<10；同 baseName 严格小于为假——
    // 反向比较仅在两名不同且正向为假时才为真，相同名两向均为假）
    EXPECT_EQ(less, c.expectedLess);
    EXPECT_EQ(compareByFileInfo(second, first), !c.expectedLess && c.firstName != c.secondName);
}

INSTANTIATE_TEST_SUITE_P(
        BasicCases, CompareByFileInfoParamTest,
        ::testing::Values(
                CompareByNameCase{QStringLiteral("2.png"), QStringLiteral("10.png"), true},
                CompareByNameCase{QStringLiteral("10.png"), QStringLiteral("2.png"), false},
                CompareByNameCase{QStringLiteral("a.png"), QStringLiteral("b.png"), true},
                CompareByNameCase{QStringLiteral("same.png"), QStringLiteral("same.png"), false}));

// ═════════════════ 构造 / 析构 ═════════════════

TEST_F(FileControlTest, FileControl_Constructor_InitializesMembersAndFormats)
{
    // Arrange（SetUp 已构造 obj）
    ASSERT_NE(obj, nullptr);
    const QStringList wallpaperFormats = obj->listsupportWallPaper;

    // Act：构造结果经成员状态断言
    const bool timerReady = (obj->m_tSaveSetting != nullptr);

    // Assert（B1：防抖定时器已建；B2：11 种壁纸后缀）
    EXPECT_TRUE(timerReady);
    EXPECT_NE(obj->m_ocrInterface, nullptr);
    EXPECT_NE(obj->m_shortcutViewProcess, nullptr);
    EXPECT_NE(obj->m_config, nullptr);
    EXPECT_NE(obj->imageFileWatcher, nullptr);
    EXPECT_NE(obj->m_tSaveSetting, nullptr);
    EXPECT_EQ(obj->listsupportWallPaper.size(), 11);
    EXPECT_EQ(obj->parent(), nullptr);
}

TEST_F(FileControlTest, FileControl_Destructor_SavesPendingWindowSize)
{
    // Arrange
    obj->m_windowWidth = 888;
    obj->m_windowHeight = 666;
    cfgWriteCount = 0;

    // Act
    delete obj;
    obj = nullptr;

    // Assert（B1：析构 → saveSetting → 配置写入两次）
    EXPECT_EQ(cfgWriteCount, 2);
    EXPECT_EQ(lastCfgValue.toInt(), 666);
}

// ═════════════════ addImageFile ═════════════════

TEST_F(FileControlTest, AddImageFile_DelegatesToWatcher_ParamPassed)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "watch.png");
    stub.set_lamda(VADDR(ImageFileWatcher, addImageFile),
                   [this](ImageFileWatcher *, const QString &filePath) {
                       ++watcherAddCount;
                       lastWatcherAddPath = filePath;
                   });

    // Act
    obj->addImageFile(png);

    // Assert（B1：路径透传且仅一次）
    EXPECT_EQ(watcherAddCount, 1);
    EXPECT_EQ(lastWatcherAddPath, png);
}

// ═════════════════ copyImage / copyText ═════════════════

TEST_F(FileControlTest, CopyImage_LocalPath_FillsClipboardMime)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "copy.png");

    // Act
    obj->copyImage(localUrl(png));

    // Assert（B1：text/urls/gnome 三类 mime 均写入）
    const QMimeData *md = QGuiApplication::clipboard()->mimeData();
    EXPECT_EQ(QGuiApplication::clipboard()->text(), png);
    ASSERT_EQ(md->urls().size(), 1);
    EXPECT_EQ(md->urls().at(0), QUrl::fromLocalFile(png));
    EXPECT_EQ(md->data("x-special/gnome-copied-files"),
              QByteArray("copy\n") + QUrl::fromLocalFile(png).toEncoded());
}

TEST_F(FileControlTest, CopyImage_EmptyLocalPath_CopiesEmptyText)
{
    // Arrange：非本地 URL → toLocalFile 为空
    const QString remoteUrl = QStringLiteral("http://example.com/img.png");

    // Act
    obj->copyImage(remoteUrl);

    // Assert（B2：text 为空串；gnome 格式仅剩头）
    const QMimeData *md = QGuiApplication::clipboard()->mimeData();
    EXPECT_TRUE(QGuiApplication::clipboard()->text().isEmpty());
    EXPECT_EQ(md->data("x-special/gnome-copied-files"), QByteArray("copy\n"));
}

TEST_F(FileControlTest, CopyText_PlainText_StoredInClipboard)
{
    // Arrange
    const QString payload = QStringLiteral("hello 图片文本");

    // Act
    obj->copyText(payload);

    // Assert
    EXPECT_EQ(QGuiApplication::clipboard()->text(), payload);
    EXPECT_TRUE(QGuiApplication::clipboard()->mimeData()->hasText());
}

// ═════════════════ createShortcutString ═════════════════

TEST_F(FileControlTest, CreateShortcutString_FirstCall_BuildsGroupedJson)
{
    // Arrange（m_shortcutString 初始为空）
    ASSERT_TRUE(obj->m_shortcutString.isEmpty());

    // Act
    const QString json = obj->createShortcutString();

    // Assert（B2：3 组 22 项的合法 JSON）
    EXPECT_FALSE(json.isEmpty());
    EXPECT_EQ(obj->m_shortcutString, json);
    QJsonParseError parseErr;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseErr);
    ASSERT_EQ(parseErr.error, QJsonParseError::NoError);
    const QJsonArray groups = doc.object().value("shortcut").toArray();
    ASSERT_EQ(groups.size(), 3);
    EXPECT_EQ(groups.at(0).toObject().value("groupItems").toArray().size(), 18);
    EXPECT_TRUE(json.contains(QLatin1String("F11")));
}

TEST_F(FileControlTest, CreateShortcutString_SecondCall_ReturnsCachedString)
{
    // Arrange
    const QString first = obj->createShortcutString();
    ASSERT_FALSE(first.isEmpty());

    // Act
    const QString second = obj->createShortcutString();

    // Assert（B1：命中缓存，与首次完全一致）
    EXPECT_EQ(second, first);
    EXPECT_EQ(obj->m_shortcutString, second);
}

// ═════════════════ deleteImagePath ═════════════════

TEST_F(FileControlTest, DeleteImagePath_InvalidUrl_ReturnsFalse)
{
    // Arrange：空串构造的 QUrl 无效
    dbusCallCount = 0;

    // Act
    const bool ok = obj->deleteImagePath(QString());

    // Assert（B1）
    EXPECT_FALSE(ok);
    EXPECT_EQ(dbusCallCount.load(), 0);
}

TEST_F(FileControlTest, DeleteImagePath_DbusError_ReturnsFalse)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "err.png");
    trashCallSucceeds = false;
    dbusCallCount = 0;
    stub.set_lamda(
            static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &)>(
                    &QDBusAbstractInterface::asyncCallWithArgumentList),
            [this](QDBusAbstractInterface *, const QString &method, const QList<QVariant> &) {
                if (method != QStringLiteral("Trash")) {
                    // 非被测调用：透传错误 pendingCall，不计入计数
                    return QDBusPendingCall::fromCompletedCall(
                            QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                       QStringLiteral("skip")));
                }
                ++dbusCallCount;
                lastDbusMethod = method;
                return trashCallSucceeds
                               ? QDBusPendingCall::fromCompletedCall(
                                         QDBusMessage().createReply(QVariant()))
                               : QDBusPendingCall::fromCompletedCall(
                                         QDBusMessage::createError(QStringLiteral("test.trash"),
                                                                   QStringLiteral("failed")));
            });

    // Act
    const bool ok = obj->deleteImagePath(localUrl(png));

    // Assert（B2）
    EXPECT_FALSE(ok);
    EXPECT_EQ(lastDbusMethod, QStringLiteral("Trash"));
}

TEST_F(FileControlTest, DeleteImagePath_FileStillExists_ReturnsFalse)
{
    // Arrange：Trash 调用成功但文件仍存在（模拟删除未生效）
    const QString png = makePngFile(tmpDir.path(), "still.png");
    trashCallSucceeds = true;
    stub.set_lamda(
            static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &)>(
                    &QDBusAbstractInterface::asyncCallWithArgumentList),
            [](QDBusAbstractInterface *, const QString &method, const QList<QVariant> &) {
                if (method != QStringLiteral("Trash")) {
                    return QDBusPendingCall::fromCompletedCall(
                            QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                       QStringLiteral("skip")));
                }
                return QDBusPendingCall::fromCompletedCall(QDBusMessage().createReply(QVariant()));
            });
    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::exists),
                   [](const QString &) { return true; });

    // Act
    const bool ok = obj->deleteImagePath(localUrl(png));

    // Assert（B3）
    EXPECT_EQ(ok, false);
    EXPECT_EQ(QFile::exists(png), true);
}

TEST_F(FileControlTest, DeleteImagePath_TrashSucceeds_ReturnsTrue)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "gone.png");
    dbusCallCount = 0;
    stub.set_lamda(
            static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &)>(
                    &QDBusAbstractInterface::asyncCallWithArgumentList),
            [this](QDBusAbstractInterface *, const QString &method, const QList<QVariant> &args) {
                if (method != QStringLiteral("Trash")) {
                    return QDBusPendingCall::fromCompletedCall(
                            QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                       QStringLiteral("skip")));
                }
                ++dbusCallCount;
                lastDbusMethod = method;
                lastDbusArgs = args;
                return QDBusPendingCall::fromCompletedCall(QDBusMessage().createReply(QVariant()));
            });
    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::exists),
                   [](const QString &) { return false; });

    // Act
    const bool ok = obj->deleteImagePath(localUrl(png));

    // Assert（B4：Trash 一次且参数含文件 URL）
    EXPECT_TRUE(ok);
    EXPECT_EQ(dbusCallCount.load(), 1);
    ASSERT_EQ(lastDbusArgs.size(), 1);
    EXPECT_EQ(lastDbusArgs.at(0).toString(), localUrl(png));
}

// ═════════════════ displayinFileManager ═════════════════

TEST_F(FileControlTest, DisplayinFileManager_InterfaceInvalid_ReturnsFalse)
{
    // Arrange：连接层隔离——sessionBus 置为无效连接，QDBusInterface::isValid 恒 false。
    // （不直接 stub QDBusAbstractInterface::isValid：DTK DConfig 后台线程并发调用该方法，
    //   运行时补丁存在竞态，曾引发 SEGV 与计数污染）
    const QString png = makePngFile(tmpDir.path(), "show.png");
    dbusCallCount = 0;
    // 空名 QDBusConnection = 无效连接，接口 isValid 恒 false
    stub.set_lamda(static_cast<QDBusConnection (*)()>(&QDBusConnection::sessionBus),
                   []() -> QDBusConnection { return QDBusConnection(QString()); });
    stub.set_lamda(
            static_cast<QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &,
                                                                 const QList<QVariant> &)>(
                    &QDBusAbstractInterface::callWithArgumentList),
            [this](QDBusAbstractInterface *, QDBus::CallMode, const QString &method,
                   const QList<QVariant> &) {
                if (method != QStringLiteral("ShowItems")) {
                    // 非被测调用（DTK DConfig 后台线程轮询）：透传错误让其降级，不计入计数
                    return QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                     QStringLiteral("skip"));
                }
                ++dbusCallCount;
                return QDBusMessage().createReply(QVariant());
            });

    // Act
    const bool ok = obj->displayinFileManager(localUrl(png));

    // Assert（B1：接口无效直接 false，不发起调用）
    EXPECT_EQ(ok, false);
    EXPECT_EQ(dbusCallCount.load(), 0);
}

TEST_F(FileControlTest, DisplayinFileManager_ShowItemsSucceeds_ReturnsTrue)
{
    // Arrange：真实 sessionBus + call 层拦截（不会真正打开文件管理器）；
    // 分支可达性依赖 org.freedesktop.FileManager1 在位，按探测结果断言
    const QString png = makePngFile(tmpDir.path(), "show2.png");
    dbusCallCount = 0;
    const bool fileManagerUp = serviceRegistered(QStringLiteral("org.freedesktop.FileManager1"));
    stub.set_lamda(
            static_cast<QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &,
                                                                 const QList<QVariant> &)>(
                    &QDBusAbstractInterface::callWithArgumentList),
            [this](QDBusAbstractInterface *, QDBus::CallMode, const QString &method,
                   const QList<QVariant> &) {
                if (method != QStringLiteral("ShowItems")) {
                    return QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                     QStringLiteral("skip"));
                }
                ++dbusCallCount;
                lastDbusMethod = method;
                return QDBusMessage().createReply(QVariant());
            });

    // Act
    const bool ok = obj->displayinFileManager(localUrl(png));

    // Assert（B2：服务在位时 ShowItems 成功应答 → true；服务不在位时走 B1 路径 → false）
    EXPECT_EQ(ok, fileManagerUp);
    if (fileManagerUp) {
        EXPECT_EQ(lastDbusMethod, QStringLiteral("ShowItems"));
        EXPECT_EQ(dbusCallCount.load(), 1);
    } else {
        EXPECT_EQ(dbusCallCount.load(), 0);
    }
}

// ═════════════════ getCompanyLogo ═════════════════

TEST_F(FileControlTest, GetCompanyLogo_DistributionLogoPath_WrappedAsFileUrl)
{
    // Arrange
    const QString expectPath = Dtk::Core::DSysInfo::distributionOrgLogo(
            Dtk::Core::DSysInfo::Distribution, Dtk::Core::DSysInfo::Light,
            QStringLiteral(":/assets/images/deepin-logo.svg"));

    // Act
    const QUrl logo = obj->getCompanyLogo();

    // Assert（B1）
    EXPECT_EQ(logo, QUrl::fromLocalFile(expectPath));
    EXPECT_FALSE(logo.toString().isEmpty());
}

// ═════════════════ getConfigValue / setConfigValue ═════════════════

TEST_F(FileControlTest, GetConfigValue_DelegatesToSetter_ReturnsDefault)
{
    // Arrange（SetUp 已 stub LibConfigSetter::value 透传默认值）
    cfgWriteCount = 0;

    // Act
    const QVariant intVal = obj->getConfigValue(QStringLiteral("G1"), QStringLiteral("K1"),
                                                QVariant(42));
    const QVariant strVal = obj->getConfigValue(QStringLiteral("G2"), QStringLiteral("K2"),
                                                QVariant(QStringLiteral("def")));

    // Assert（B1：原样返回默认值——stub 隔离下不会读到真实配置）
    EXPECT_EQ(intVal, QVariant(42));
    EXPECT_EQ(strVal, QVariant(QStringLiteral("def")));
}

TEST_F(FileControlTest, SetConfigValue_DelegatesToSetter_ValueRecorded)
{
    // Arrange
    cfgWriteCount = 0;

    // Act
    obj->setConfigValue(QStringLiteral("G"), QStringLiteral("K"), QVariant(7));

    // Assert（B1：group/key/value 透传到 LibConfigSetter）
    EXPECT_EQ(cfgWriteCount, 1);
    EXPECT_EQ(lastCfgGroup, QStringLiteral("G"));
    EXPECT_EQ(lastCfgKey, QStringLiteral("K"));
    EXPECT_EQ(lastCfgValue, QVariant(7));
}

// ═════════════════ getDirImagePath ═════════════════

TEST_F(FileControlTest, GetDirImagePath_EmptyPath_ReturnsEmptyList)
{
    // Arrange（空目录已备）
    ASSERT_TRUE(tmpDir.isValid());

    // Act
    const QStringList result = obj->getDirImagePath(QString());

    // Assert（B1）
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(FileControlTest, GetDirImagePath_NoImageFiles_ReturnsEmptyList)
{
    // Arrange：目录内只有文本文件；入参传目录内文件 URL（源码按"文件所在目录"解析，
    // 直接传目录 URL 会取到父目录——疑似缺陷 D6，见文末）
    const QString txt = makeTextFile(tmpDir.path(), "note.txt");
    makeTextFile(tmpDir.path(), "readme.md");

    // Act
    const QStringList result = obj->getDirImagePath(localUrl(txt));

    // Assert（B5：全部被 isImage 过滤）
    EXPECT_EQ(result.isEmpty(), true);
    EXPECT_EQ(result.size(), 0);
}

TEST_F(FileControlTest, GetDirImagePath_MixedFiles_ReturnsNumericallySortedUrls)
{
    // Arrange：图片与文本混合，文件名特意用 1/2/10 区分数字序与字典序；
    // 入参传目录内任一文件 URL（源码解析其所在目录）
    const QString first = makePngFile(tmpDir.path(), "1.png");
    makePngFile(tmpDir.path(), "10.png");
    makePngFile(tmpDir.path(), "2.png");
    makeTextFile(tmpDir.path(), "note.txt");

    // Act
    const QStringList result = obj->getDirImagePath(localUrl(first));

    // Assert（B4：仅 3 张图，且数字序 1<2<10，而非字典序 1,10,2）
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(QUrl(result.at(0)).toLocalFile(), QDir(tmpDir.path()).filePath(QStringLiteral("1.png")));
    EXPECT_EQ(QUrl(result.at(1)).toLocalFile(), QDir(tmpDir.path()).filePath(QStringLiteral("2.png")));
    EXPECT_EQ(QUrl(result.at(2)).toLocalFile(), QDir(tmpDir.path()).filePath(QStringLiteral("10.png")));
}

TEST_F(FileControlTest, GetDirImagePath_SingleImage_ReturnsOneEntry)
{
    // Arrange：单元素循环边界；入参传该文件 URL
    const QString only = makePngFile(tmpDir.path(), "only.png");

    // Act
    const QStringList result = obj->getDirImagePath(localUrl(only));

    // Assert（B4：循环 1 次）
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(QUrl(result.at(0)).toLocalFile(), QDir(tmpDir.path()).filePath(QStringLiteral("only.png")));
}

// ═════════════════ getNamePath ═════════════════

namespace {
struct GetNamePathCase {
    bool oldAsUrl;
    QString newNameInput;  // 传给方法的原始参数
    QString expectedNewName;  // 拼入新路径的名字（按当前实现：原样 newName）
};
}  // namespace

class GetNamePathParamTest : public FileControlTest,
                             public ::testing::WithParamInterface<GetNamePathCase> {
};

TEST_P(GetNamePathParamTest, GetNamePath_ParamSet_BuildsNewPathInSameDir)
{
    // Arrange
    const auto &c = GetParam();
    const QString oldFile = makePngFile(tmpDir.path(), "origin.png");
    const QString oldInput = c.oldAsUrl ? localUrl(oldFile) : oldFile;
    const QString expectUrl = QUrl::fromLocalFile(
            QDir(tmpDir.path()).filePath(c.expectedNewName + QStringLiteral(".png")))
                                      .toString();

    // Act
    const QString got = obj->getNamePath(oldInput, c.newNameInput);

    // Assert（B1/B2：file:// 旧路径被转换；新路径 = 同目录 + 名字 + 原后缀）
    EXPECT_EQ(got, expectUrl);
    EXPECT_TRUE(QUrl(got).isLocalFile());
}

INSTANTIATE_TEST_SUITE_P(
        PathForms, GetNamePathParamTest,
        ::testing::Values(
                // 普通路径 + 普通名
                GetNamePathCase{false, QStringLiteral("renamed"), QStringLiteral("renamed")},
                // file:// 旧路径 + 普通名
                GetNamePathCase{true, QStringLiteral("b"), QStringLiteral("b")},
                // file:// 旧路径 + file:// 新名：入参与期望均用字面量构造。
                // 注意不可用 localUrl("c") 生成——QUrl::fromLocalFile 对相对路径返回
                // "file:c"（无 authority），与实现的拼接产物不一致；当前实现拼接的是
                // newName 原串（now 转换结果被丢弃，疑似缺陷 D2），期望按现状锁定
                GetNamePathCase{true, QStringLiteral("file:///c"), QStringLiteral("file:///c")}));

// ═════════════════ getPrimaryScreenCenterX / Y ═════════════════

TEST_F(FileControlTest, GetPrimaryScreenCenterX_NullScreen_ReturnsZero)
{
    // Arrange
    stub.set_lamda(static_cast<QScreen *(*)()>(&QGuiApplication::primaryScreen),
                   []() -> QScreen * { return nullptr; });

    // Act
    const int x = obj->getPrimaryScreenCenterX(800);

    // Assert（B1）
    EXPECT_EQ(x, 0);
    EXPECT_EQ(obj->getPrimaryScreenCenterX(0), 0);
}

TEST_F(FileControlTest, GetPrimaryScreenCenterX_RealScreen_MatchesGeometryCalc)
{
    // Arrange
    QScreen *screen = QGuiApplication::primaryScreen();
    ASSERT_NE(screen, nullptr);
    const QRect g = screen->geometry();
    const int expected = g.x() + g.width() / 2 - 800 / 2;

    // Act
    const int x = obj->getPrimaryScreenCenterX(800);

    // Assert（B2）
    EXPECT_EQ(x, expected);
    EXPECT_EQ(obj->getPrimaryScreenCenterX(0), g.x() + g.width() / 2);
}

TEST_F(FileControlTest, GetPrimaryScreenCenterY_NullScreen_ReturnsZero)
{
    // Arrange
    stub.set_lamda(static_cast<QScreen *(*)()>(&QGuiApplication::primaryScreen),
                   []() -> QScreen * { return nullptr; });

    // Act
    const int y = obj->getPrimaryScreenCenterY(600);

    // Assert（B1）
    EXPECT_EQ(y, 0);
    EXPECT_EQ(obj->getPrimaryScreenCenterY(100), 0);
}

TEST_F(FileControlTest, GetPrimaryScreenCenterY_RealScreen_MatchesGeometryCalc)
{
    // Arrange
    QScreen *screen = QGuiApplication::primaryScreen();
    ASSERT_NE(screen, nullptr);
    const QRect g = screen->geometry();
    const int expected = g.y() + g.height() / 2 - 600 / 2;

    // Act
    const int y = obj->getPrimaryScreenCenterY(600);

    // Assert（B2）
    EXPECT_EQ(y, expected);
    EXPECT_EQ(obj->getPrimaryScreenCenterY(0), g.y() + g.height() / 2);
}

// ═════════════════ getlastWidth / getlastHeight ═════════════════

TEST_F(FileControlTest, GetlastWidth_ConfigAboveDefault_ReturnsConfigured)
{
    // Arrange：配置值 = 屏幕默认宽 + 1000，必然高于下限
    stub.set_lamda(VADDR(FileControl, getConfigValue),
                   [this](FileControl *, const QString &, const QString &,
                          const QVariant &defaultValue) -> QVariant {
                       capturedDefault = defaultValue.toInt();
                       return defaultValue.toInt() + 1000;
                   });

    // Act
    const int w = obj->getlastWidth();

    // Assert（B4：精确回传配置值并同步成员）
    EXPECT_GT(capturedDefault, 0);
    EXPECT_EQ(w, capturedDefault + 1000);
    EXPECT_EQ(obj->m_windowWidth, w);
}

TEST_F(FileControlTest, GetlastWidth_SmallConfigValue_ClampedToMinimum)
{
    // Arrange：配置返回 -1，应被钳制到最小宽度
    stub.set_lamda(VADDR(FileControl, getConfigValue),
                   [](FileControl *, const QString &, const QString &, const QVariant &) -> QVariant {
                       return QVariant(-1);
                   });

    // Act
    const int w = obj->getlastWidth();

    // Assert（B5：返回正的最小宽，成员同步）
    EXPECT_GT(w, 0);
    EXPECT_EQ(obj->m_windowWidth, w);
}

TEST_F(FileControlTest, GetlastWidth_NullScreen_ReturnsMinimum)
{
    // Arrange
    stub.set_lamda(static_cast<QScreen *(*)()>(&QGuiApplication::primaryScreen),
                   []() -> QScreen * { return nullptr; });

    // Act
    const int w = obj->getlastWidth();

    // Assert（B1：直接返回最小宽，m_windowWidth 不被更新）
    EXPECT_GT(w, 0);
    EXPECT_EQ(obj->m_windowWidth, 0);
}

TEST_F(FileControlTest, GetlastWidth_MultipleScreens_UsesSizeBasedDefault)
{
    // Arrange：多屏走 screen->size() 分支
    stub.set_lamda(static_cast<QList<QScreen *> (*)()>(&QGuiApplication::screens),
                   []() -> QList<QScreen *> {
                       QScreen *s = QGuiApplication::primaryScreen();
                       return QList<QScreen *>{ s, s };
                   });
    stub.set_lamda(VADDR(FileControl, getConfigValue),
                   [this](FileControl *, const QString &, const QString &,
                          const QVariant &defaultValue) -> QVariant {
                       capturedDefault = defaultValue.toInt();
                       return defaultValue.toInt() + 1000;
                   });

    // Act
    const int w = obj->getlastWidth();

    // Assert（B2+B4）
    EXPECT_GT(capturedDefault, 0);
    EXPECT_EQ(w, capturedDefault + 1000);
}

TEST_F(FileControlTest, GetlastHeight_ConfigAboveDefault_ReturnsConfigured)
{
    // Arrange
    stub.set_lamda(VADDR(FileControl, getConfigValue),
                   [this](FileControl *, const QString &, const QString &,
                          const QVariant &defaultValue) -> QVariant {
                       capturedDefault = defaultValue.toInt();
                       return defaultValue.toInt() + 1000;
                   });

    // Act
    const int h = obj->getlastHeight();

    // Assert（B4）
    EXPECT_GT(capturedDefault, 0);
    EXPECT_EQ(h, capturedDefault + 1000);
    EXPECT_EQ(obj->m_windowHeight, h);
}

TEST_F(FileControlTest, GetlastHeight_SmallConfigValue_ClampedToMinimum)
{
    // Arrange
    stub.set_lamda(VADDR(FileControl, getConfigValue),
                   [](FileControl *, const QString &, const QString &, const QVariant &) -> QVariant {
                       return QVariant(-1);
                   });

    // Act
    const int h = obj->getlastHeight();

    // Assert（B5）
    EXPECT_GT(h, 0);
    EXPECT_EQ(obj->m_windowHeight, h);
}

// ═════════════════ isCanDelete ═════════════════

TEST_F(FileControlTest, IsCanDelete_LocalWritableFile_ReturnsTrue)
{
    // Arrange：临时目录内真实可读写文件（PathType 为本地）
    const QString png = makePngFile(tmpDir.path(), "del.png");
    ASSERT_EQ(QFile::exists(png), true);

    // Act
    const bool can = obj->isCanDelete(localUrl(png));

    // Assert（B1：URL 输入判 true；裸路径无 scheme → toLocalFile 为空 → false，输入约定为
    // file:// URL 形式（与 isImage 的 fallback 不一致——疑似缺陷 D7，按真实行为锁定）
    EXPECT_EQ(can, true);
    EXPECT_EQ(obj->isCanDelete(png), false);
}

TEST_F(FileControlTest, IsCanDelete_MtpPathType_ReturnsFalse)
{
    // Arrange：MTP 设备路径类型被排除
    const QString png = makePngFile(tmpDir.path(), "mtp.png");
    QString capturedPath;
    stub.set_lamda(&LibUnionImage_NameSpace::getPathType,
                   [&capturedPath](const QString &imagepath) -> imageViewerSpace::PathType {
                       capturedPath = imagepath;
                       return imageViewerSpace::PathTypeMTP;
                   });

    // Act
    const bool can = obj->isCanDelete(localUrl(png));

    // Assert（B2：本地路径被送入类型判定且结果为拒绝）
    EXPECT_EQ(can, false);
    EXPECT_EQ(capturedPath, png);
}

TEST_F(FileControlTest, IsCanDelete_ReadOnlyFile_ReturnsFalse)
{
    // Arrange：文件设为只读
    const QString png = makePngFile(tmpDir.path(), "ro.png");
    ASSERT_TRUE(QFile::setPermissions(png, QFileDevice::ReadOwner | QFileDevice::ReadUser));

    // Act
    const bool can = obj->isCanDelete(localUrl(png));

    // Assert（B3：不可写 → false，文件状态未受影响）
    EXPECT_EQ(can, false);
    EXPECT_EQ(QFile::exists(png), true);
}

// ═════════════════ isCanReadable ═════════════════

TEST_F(FileControlTest, IsCanReadable_ExistingFile_ReturnsTrue)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "read.png");
    ASSERT_EQ(QFile::exists(png), true);

    // Act
    const bool can = obj->isCanReadable(localUrl(png));

    // Assert（B1：URL 输入判 true；裸路径无 scheme → toLocalFile 为空 → false（D7）
    EXPECT_EQ(can, true);
    EXPECT_EQ(obj->isCanReadable(png), false);
}

TEST_F(FileControlTest, IsCanReadable_MissingFile_ReturnsFalse)
{
    // Arrange
    const QString missing = QDir(tmpDir.path()).filePath("missing.png");

    // Act
    const bool can = obj->isCanReadable(localUrl(missing));

    // Assert（B2）
    EXPECT_EQ(can, false);
    EXPECT_EQ(QFile::exists(missing), false);
}

// ═════════════════ isCanRename ═════════════════

TEST_F(FileControlTest, IsCanRename_LocalWritableFile_ReturnsTrue)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "ren.png");
    ASSERT_EQ(QFile::exists(png), true);

    // Act
    const bool can = obj->isCanRename(localUrl(png));

    // Assert（B1：URL 输入判 true；裸路径无 scheme → false（D7）
    EXPECT_EQ(can, true);
    EXPECT_EQ(obj->isCanRename(png), false);
}

TEST_F(FileControlTest, IsCanRename_MtpPathType_ReturnsFalse)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "renmtp.png");
    QString capturedPath;
    stub.set_lamda(&LibUnionImage_NameSpace::getPathType,
                   [&capturedPath](const QString &imagepath) -> imageViewerSpace::PathType {
                       capturedPath = imagepath;
                       return imageViewerSpace::PathTypeMTP;
                   });

    // Act
    const bool can = obj->isCanRename(localUrl(png));

    // Assert（B2）
    EXPECT_EQ(can, false);
    EXPECT_EQ(capturedPath, png);
}

TEST_F(FileControlTest, IsCanRename_MissingFile_ReturnsFalse)
{
    // Arrange
    const QString missing = QDir(tmpDir.path()).filePath("norename.png");

    // Act
    const bool can = obj->isCanRename(localUrl(missing));

    // Assert（B3）
    EXPECT_EQ(can, false);
    EXPECT_EQ(QFile::exists(missing), false);
}

// ═════════════════ isCanSupportOcr ═════════════════

TEST_F(FileControlTest, IsCanSupportOcr_StaticImageReadable_ReturnsTrue)
{
    // Arrange：真实 PNG → getImageType 为非动态类型
    const QString png = makePngFile(tmpDir.path(), "ocr.png");
    ASSERT_EQ(QFileInfo(png).isReadable(), true);

    // Act
    const bool can = obj->isCanSupportOcr(localUrl(png));

    // Assert（B1：URL 输入判 true；裸路径无 scheme → false（D7）
    EXPECT_EQ(can, true);
    EXPECT_EQ(obj->isCanSupportOcr(png), false);
}

TEST_F(FileControlTest, IsCanSupportOcr_DynamicImageType_ReturnsFalse)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "dyn.png");
    QString capturedPath;
    stub.set_lamda(&LibUnionImage_NameSpace::getImageType,
                   [&capturedPath](const QString &imagepath) -> imageViewerSpace::ImageType {
                       capturedPath = imagepath;
                       return imageViewerSpace::ImageTypeDynamic;
                   });

    // Act
    const bool can = obj->isCanSupportOcr(localUrl(png));

    // Assert（B2）
    EXPECT_EQ(can, false);
    EXPECT_EQ(capturedPath, png);
}

TEST_F(FileControlTest, IsCanSupportOcr_MissingFile_ReturnsFalse)
{
    // Arrange：不可读路径
    const QString missing = QDir(tmpDir.path()).filePath("noocr.png");

    // Act
    const bool can = obj->isCanSupportOcr(localUrl(missing));

    // Assert（B3）
    EXPECT_EQ(can, false);
    EXPECT_EQ(QFile::exists(missing), false);
}

// ═════════════════ isCanWrite ═════════════════

TEST_F(FileControlTest, IsCanWrite_WritableFileAndDir_ReturnsTrue)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "w.png");
    ASSERT_EQ(QFile::exists(png), true);

    // Act
    const bool can = obj->isCanWrite(localUrl(png));

    // Assert（B1：URL 输入判 true；裸路径无 scheme → false（D7）
    EXPECT_EQ(can, true);
    EXPECT_EQ(obj->isCanWrite(png), false);
}

TEST_F(FileControlTest, IsCanWrite_ReadOnlyFile_ReturnsFalse)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "wro.png");
    ASSERT_TRUE(QFile::setPermissions(png, QFileDevice::ReadOwner | QFileDevice::ReadUser));

    // Act
    const bool can = obj->isCanWrite(localUrl(png));

    // Assert（B2）
    EXPECT_EQ(can, false);
    EXPECT_EQ(QFile::exists(png), true);
}

// ═════════════════ isCheckOnly ═════════════════

TEST_F(FileControlTest, IsCheckOnly_RepeatedLockAttempts_BothSucceed)
{
    // Arrange：同进程首次调用即持有锁（源码未 close/unlock，fd 泄漏——疑似缺陷 D3）
    ASSERT_EQ(cfgWriteCount, 0);

    // Act
    const bool first = obj->isCheckOnly();
    const bool second = obj->isCheckOnly();

    // Assert（B4 首次成功；B3 不触发——lockf 为 POSIX record lock，按进程互斥，
    // 同进程对同一文件的重复 F_TLOCK 请求成功而非 EWOULDBLOCK）
    EXPECT_EQ(first, true);
    EXPECT_EQ(second, true);
}

// ═════════════════ isCurrentWatcherDir ═════════════════

TEST_F(FileControlTest, IsCurrentWatcherDir_DelegatesToWatcher_BothBranches)
{
    // Arrange
    const QString dir = tmpDir.path();
    stub.set_lamda(VADDR(ImageFileWatcher, isCurrentDir),
                   [this](ImageFileWatcher *, const QString &) -> bool {
                       ++watcherIsCurrentCount;
                       return watcherIsCurrentRet;
                   });

    // Act
    const bool yes = obj->isCurrentWatcherDir(QUrl::fromLocalFile(dir));

    // Assert（B1 true 态）
    EXPECT_TRUE(yes);
    EXPECT_EQ(watcherIsCurrentCount, 1);

    // Act（覆盖 stub 为 false 再测 B1 false 态）
    stub.set_lamda(VADDR(ImageFileWatcher, isCurrentDir),
                   [this](ImageFileWatcher *, const QString &) -> bool {
                       ++watcherIsCurrentCount;
                       return false;
                   });
    const bool no = obj->isCurrentWatcherDir(QUrl::fromLocalFile(dir));

    // Assert
    EXPECT_FALSE(no);
    EXPECT_EQ(watcherIsCurrentCount, 2);
}

// ═════════════════ isEnableNavigation ═════════════════

TEST_F(FileControlTest, IsEnableNavigation_ConfigDefaultTrue_ReturnsTrue)
{
    // Arrange：拦截 getConfigValue 验证默认值透传
    stub.set_lamda(VADDR(FileControl, getConfigValue),
                   [this](FileControl *, const QString &, const QString &,
                          const QVariant &defaultValue) -> QVariant {
                       lastNavDefault = defaultValue;
                       return defaultValue;
                   });

    // Act
    const bool enabled = obj->isEnableNavigation();

    // Assert（B1：默认 true 透传）
    EXPECT_EQ(enabled, true);
    EXPECT_EQ(lastNavDefault, QVariant(true));
}

// ═════════════════ isImage ═════════════════

namespace {
struct IsImageCase {
    const char *kind;   // real_png / text_txt / png_as_txt / text_as_png / real_png_url
    bool expected;
};
}  // namespace

class IsImageParamTest : public FileControlTest, public ::testing::WithParamInterface<IsImageCase> {
};

TEST_P(IsImageParamTest, IsImage_ParamSet_DetectsByContentAndExtension)
{
    // Arrange
    const auto &c = GetParam();
    const std::string kind = c.kind;
    QString path;
    if (kind == "real_png") {
        path = makePngFile(tmpDir.path(), "real.png");
    } else if (kind == "text_txt") {
        path = makeTextFile(tmpDir.path(), "note.txt");
    } else if (kind == "png_as_txt") {
        path = makePngFile(tmpDir.path(), "masked.txt");  // 图片内容伪装 txt 扩展
    } else if (kind == "text_as_png") {
        path = makeTextFile(tmpDir.path(), "fake.png");  // 文本内容伪装 png 扩展
    } else {
        path = localUrl(makePngFile(tmpDir.path(), "viurl.png"));  // file:// URL 形式
    }
    ASSERT_FALSE(path.isEmpty());

    // Act
    const bool isImg = obj->isImage(path);

    // Assert（B1~B4 见参数注释）
    EXPECT_EQ(isImg, c.expected);
    EXPECT_EQ(obj->isImage(path), c.expected);  // 判定稳定可重复
}

INSTANTIATE_TEST_SUITE_P(
        ContentAndExtension, IsImageParamTest,
        ::testing::Values(
                IsImageCase{"real_png", true},      // B2 内容命中
                IsImageCase{"text_txt", false},     // B4 均不命中
                IsImageCase{"png_as_txt", true},    // B2 内容命中（扩展不符）
                IsImageCase{"text_as_png", true},   // B3 扩展命中（内容不符）
                IsImageCase{"real_png_url", true}   // B1 URL 转换后命中
                ));

// ═════════════════ isRotatable ═════════════════

TEST_F(FileControlTest, IsRotatable_MissingFile_ReturnsFalse)
{
    // Arrange
    const QString missing = QDir(tmpDir.path()).filePath("norotate.png");

    // Act
    const bool can = obj->isRotatable(localUrl(missing));

    // Assert（B1）
    EXPECT_EQ(can, false);
    EXPECT_EQ(QFile::exists(missing), false);
}

TEST_F(FileControlTest, IsRotatable_SupportRotateTrue_ReturnsTrue)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "rot.png");
    QString capturedPath;
    stub.set_lamda(&LibUnionImage_NameSpace::isImageSupportRotate,
                   [&capturedPath](const QString &path) -> bool {
                       capturedPath = path;
                       return true;
                   });

    // Act
    const bool can = obj->isRotatable(localUrl(png));

    // Assert（B2：本地路径送检且可旋转）
    EXPECT_EQ(can, true);
    EXPECT_EQ(capturedPath, png);
}

TEST_F(FileControlTest, IsRotatable_SupportRotateFalse_ReturnsFalse)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "rot2.png");
    stub.set_lamda(&LibUnionImage_NameSpace::isImageSupportRotate,
                   [](const QString &) -> bool { return false; });

    // Act
    const bool can = obj->isRotatable(localUrl(png));

    // Assert（B3：文件完好，仅判定不可旋转）
    EXPECT_EQ(can, false);
    EXPECT_EQ(QFile::exists(png), true);
}

// ═════════════════ isShowToolTip ═════════════════

TEST_F(FileControlTest, IsShowToolTip_SameBaseName_ReturnsFalse)
{
    // Arrange：新名字与现名相同
    const QString png = makePngFile(tmpDir.path(), "same.png");
    ASSERT_EQ(QFileInfo(png).completeBaseName(), QStringLiteral("same"));

    // Act
    const bool show = obj->isShowToolTip(localUrl(png), QStringLiteral("same"));

    // Assert（B1）
    EXPECT_EQ(show, false);
    EXPECT_EQ(QFile::exists(png), true);
}

TEST_F(FileControlTest, IsShowToolTip_TargetExists_ReturnsTrue)
{
    // Arrange：目录下已有同名冲突文件 b.png
    const QString png = makePngFile(tmpDir.path(), "a.png");
    const QString conflict = makePngFile(tmpDir.path(), "b.png");

    // Act
    const bool show = obj->isShowToolTip(localUrl(png), QStringLiteral("b"));

    // Assert（B2：冲突文件真实存在）
    EXPECT_EQ(show, true);
    EXPECT_EQ(QFile::exists(conflict), true);
}

TEST_F(FileControlTest, IsShowToolTip_TargetMissing_ReturnsFalse)
{
    // Arrange：目标名不冲突
    const QString png = makePngFile(tmpDir.path(), "c.png");
    const QString target = QDir(tmpDir.path()).filePath("fresh.png");

    // Act
    const bool show = obj->isShowToolTip(localUrl(png), QStringLiteral("fresh"));

    // Assert（B3）
    EXPECT_EQ(show, false);
    EXPECT_EQ(QFile::exists(target), false);
}

// ═════════════════ isSupportSetWallpaper ═════════════════

TEST_F(FileControlTest, IsSupportSetWallpaper_SupportedSuffix_ReturnsTrue)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "wall.png");
    ASSERT_EQ(QFileInfo(png).suffix(), QStringLiteral("png"));

    // Act
    const bool can = obj->isSupportSetWallpaper(localUrl(png));

    // Assert（B1：URL 输入判 true；裸路径无 scheme → false（D7）
    EXPECT_EQ(can, true);
    EXPECT_EQ(obj->isSupportSetWallpaper(png), false);
}

TEST_F(FileControlTest, IsSupportSetWallpaper_UnsupportedSuffix_ReturnsFalse)
{
    // Arrange：webp 不在 11 项支持列表
    const QString fake = QDir(tmpDir.path()).filePath("pic.webp");
    QFile f(fake);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("fake");
    f.close();

    // Act
    const bool can = obj->isSupportSetWallpaper(localUrl(fake));

    // Assert（B2）
    EXPECT_EQ(can, false);
    EXPECT_EQ(QFileInfo(fake).suffix().toLower(), QStringLiteral("webp"));
}

TEST_F(FileControlTest, IsSupportSetWallpaper_UpperCaseSuffix_NormalizedToLower)
{
    // Arrange：大写后缀经 toLower 归一后命中列表
    const QString png = makePngFile(tmpDir.path(), "upper.PNG");
    ASSERT_EQ(QFileInfo(png).suffix(), QStringLiteral("PNG"));

    // Act
    const bool can = obj->isSupportSetWallpaper(localUrl(png));

    // Assert（B1：大小写归一生效）
    EXPECT_EQ(can, true);
    EXPECT_EQ(obj->listsupportWallPaper.contains(QStringLiteral("png")), true);
}

// ═════════════════ ocrImage ═════════════════

TEST_F(FileControlTest, OcrImage_SinglePage_OpensLocalPathViaDbus)
{
    // Arrange：不存在的文件 → ImageInfo 类型非多页 → 走单页分支
    const QString missing = QDir(tmpDir.path()).filePath("single.png");
    dbusCallCount = 0;
    stub.set_lamda(
            static_cast<QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &,
                                                                 const QList<QVariant> &)>(
                    &QDBusAbstractInterface::callWithArgumentList),
            [this](QDBusAbstractInterface *, QDBus::CallMode, const QString &method,
                   const QList<QVariant> &args) {
                if (method != QStringLiteral("openFile")) {
                    // 非被测调用（DTK DConfig 后台线程轮询）：透传错误，不计入计数
                    return QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                     QStringLiteral("skip"));
                }
                ++dbusCallCount;
                lastDbusMethod = method;
                lastDbusArgs = args;
                return QDBusMessage().createReply(QVariant());
            });

    // Act
    obj->ocrImage(localUrl(missing), 0);

    // Assert（B1：openFile(localPath) 一次）
    EXPECT_EQ(dbusCallCount.load(), 1);
    EXPECT_EQ(lastDbusMethod, QStringLiteral("openFile"));
    ASSERT_EQ(lastDbusArgs.size(), 1);
    EXPECT_EQ(lastDbusArgs.at(0).toString(), missing);
}

TEST_F(FileControlTest, OcrImage_MultiPage_SavesCacheAndOpensTemp)
{
    // Arrange：构造多页图场景
    const QString multi = makePngFile(tmpDir.path(), "multi.tif");
    dbusCallCount = 0;
    stub.set_lamda(VADDR(ImageInfo, type), [](ImageInfo *) -> int {
        return Types::MultiImage;
    });
    stub.set_lamda(static_cast<bool (QImageReader::*)(int)>(&QImageReader::jumpToImage),
                   [](QImageReader *, int) -> bool { return true; });
    stub.set_lamda(static_cast<QImage (QImageReader::*)()>(&QImageReader::read),
                   [](QImageReader *) -> QImage { return QImage(3, 3, QImage::Format_RGB32); });
    stub.set_lamda(
            static_cast<QString (*)(QStandardPaths::StandardLocation)>(
                    &QStandardPaths::writableLocation),
            [this](QStandardPaths::StandardLocation) -> QString { return tmpDir.path(); });
    stub.set_lamda(
            static_cast<QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &,
                                                                 const QList<QVariant> &)>(
                    &QDBusAbstractInterface::callWithArgumentList),
            [this](QDBusAbstractInterface *, QDBus::CallMode, const QString &method,
                   const QList<QVariant> &args) {
                if (method != QStringLiteral("openFile")) {
                    return QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                     QStringLiteral("skip"));
                }
                ++dbusCallCount;
                lastDbusMethod = method;
                lastDbusArgs = args;
                return QDBusMessage().createReply(QVariant());
            });
    const QString recPath = QDir(tmpDir.path()).filePath("rec.png");

    // Act
    obj->ocrImage(localUrl(multi), 2);

    // Assert（B2：缓存图已落盘并交给 OCR）
    EXPECT_EQ(dbusCallCount.load(), 1);
    EXPECT_EQ(lastDbusMethod, QStringLiteral("openFile"));
    ASSERT_EQ(lastDbusArgs.size(), 1);
    EXPECT_EQ(lastDbusArgs.at(0).toString(), recPath);
    EXPECT_TRUE(QFile::exists(recPath));
}

// ═════════════════ parseCommandlineGetPath ═════════════════

TEST_F(FileControlTest, ParseCommandlineGetPath_ImageArgument_ReturnsFileUrl)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "arg.png");
    const QString txt = makeTextFile(tmpDir.path(), "arg.txt");
    fakeArguments = QStringList{ QStringLiteral("app-name"), png, txt };
    stub.set_lamda(static_cast<QStringList (*)()>(&QCoreApplication::arguments),
                   [this]() -> QStringList { return fakeArguments; });

    // Act
    const QString got = obj->parseCommandlineGetPath();

    // Assert（B1：首个图片参数转成 file URL）
    EXPECT_EQ(got, localUrl(png));
    EXPECT_TRUE(QUrl(got).isLocalFile());
}

TEST_F(FileControlTest, ParseCommandlineGetPath_NoImageArgument_ReturnsEmpty)
{
    // Arrange：仅有非图片参数
    const QString txt = makeTextFile(tmpDir.path(), "plain.txt");
    fakeArguments = QStringList{ QStringLiteral("app-name"), txt };
    stub.set_lamda(static_cast<QStringList (*)()>(&QCoreApplication::arguments),
                   [this]() -> QStringList { return fakeArguments; });

    // Act
    const QString got = obj->parseCommandlineGetPath();

    // Assert（B2）
    EXPECT_EQ(got.isEmpty(), true);
    EXPECT_EQ(got, QString());
}

// ═════════════════ resetImageFiles ═════════════════

TEST_F(FileControlTest, ResetImageFiles_PathList_DelegatesAndClearsCache)
{
    // Arrange
    const QStringList files{ makePngFile(tmpDir.path(), "r1.png"),
                             makePngFile(tmpDir.path(), "r2.png") };
    stub.set_lamda(VADDR(ImageFileWatcher, resetImageFiles),
                   [this](ImageFileWatcher *, const QStringList &filePaths) {
                       ++watcherResetCount;
                       lastResetFiles = filePaths;
                   });
    stub.set_lamda(&ImageInfo::clearCache, [this]() { ++cacheClearCount; });

    // Act
    obj->resetImageFiles(files);

    // Assert（B1+B2）
    EXPECT_EQ(watcherResetCount, 1);
    EXPECT_EQ(lastResetFiles, files);
    EXPECT_EQ(cacheClearCount, 1);
}

TEST_F(FileControlTest, ResetImageFiles_EmptyList_StillResetsAndClears)
{
    // Arrange
    stub.set_lamda(VADDR(ImageFileWatcher, resetImageFiles),
                   [this](ImageFileWatcher *, const QStringList &filePaths) {
                       ++watcherResetCount;
                       lastResetFiles = filePaths;
                   });
    stub.set_lamda(&ImageInfo::clearCache, [this]() { ++cacheClearCount; });

    // Act
    obj->resetImageFiles(QStringList());

    // Assert（B1+B2 空集边界）
    EXPECT_EQ(watcherResetCount, 1);
    EXPECT_TRUE(lastResetFiles.isEmpty());
    EXPECT_EQ(cacheClearCount, 1);
}

// ═════════════════ saveSetting ═════════════════

TEST_F(FileControlTest, SaveSetting_BothSizesChanged_WritesEachOnce)
{
    // Arrange
    stub.set_lamda(VADDR(FileControl, setConfigValue),
                   [this](FileControl *, const QString &, const QString &, const QVariant &value) {
                       ++innerCfgSetCount;
                       lastInnerCfgValue = value;
                   });
    obj->m_windowWidth = 800;
    obj->m_windowHeight = 600;
    innerCfgSetCount = 0;

    // Act
    obj->saveSetting();
    const int writesAfterFirst = innerCfgSetCount;
    obj->saveSetting();

    // Assert（B1+B3 首次各写一次；B2+B4 二次调用去重不再写）
    EXPECT_EQ(writesAfterFirst, 2);
    EXPECT_EQ(innerCfgSetCount, 2);
    EXPECT_EQ(obj->m_lastSaveWidth, 800);
    EXPECT_EQ(obj->m_lastSaveHeight, 600);
}

TEST_F(FileControlTest, SaveSetting_OnlyHeightChanged_WritesHeightOnly)
{
    // Arrange：宽未变（与 lastSave 相同），仅高变化
    stub.set_lamda(VADDR(FileControl, setConfigValue),
                   [this](FileControl *, const QString &, const QString &, const QVariant &value) {
                       ++innerCfgSetCount;
                       lastInnerCfgValue = value;
                   });
    obj->m_lastSaveWidth = obj->m_windowWidth = 1024;
    obj->m_windowHeight = 400;
    innerCfgSetCount = 0;

    // Act
    obj->saveSetting();

    // Assert（B2 跳过写宽 + B3 只写高）
    EXPECT_EQ(innerCfgSetCount, 1);
    EXPECT_EQ(lastInnerCfgValue, QVariant(400));
    EXPECT_EQ(obj->m_lastSaveHeight, 400);
}

// ═════════════════ setEnableNavigation ═════════════════

TEST_F(FileControlTest, SetEnableNavigation_BoolValue_PassedToConfig)
{
    // Arrange
    stub.set_lamda(VADDR(FileControl, setConfigValue),
                   [this](FileControl *, const QString &, const QString &, const QVariant &value) {
                       ++innerCfgSetCount;
                       lastInnerCfgValue = value;
                   });

    // Act
    obj->setEnableNavigation(false);
    const QVariant firstValue = lastInnerCfgValue;
    obj->setEnableNavigation(true);

    // Assert（B1：布尔值原样透传）
    EXPECT_EQ(firstValue, QVariant(false));
    EXPECT_EQ(lastInnerCfgValue, QVariant(true));
    EXPECT_EQ(innerCfgSetCount, 2);
}

// ═════════════════ setSettingWidth / setSettingHeight ═════════════════

TEST_F(FileControlTest, SetSettingWidth_NewWidth_UpdatesMemberAndArmsTimer)
{
    // Arrange
    stub.set_lamda(static_cast<void (QTimer::*)(int)>(&QTimer::start),
                   [this](QTimer *, int msec) {
                       ++timerStartCount;
                       lastTimerMs = msec;
                   });

    // Act
    obj->setSettingWidth(1234);

    // Assert（B1）
    EXPECT_EQ(obj->m_windowWidth, 1234);
    EXPECT_EQ(timerStartCount, 1);
    EXPECT_EQ(lastTimerMs, 1000);
}

TEST_F(FileControlTest, SetSettingHeight_NewHeight_UpdatesMemberAndArmsTimer)
{
    // Arrange
    stub.set_lamda(static_cast<void (QTimer::*)(int)>(&QTimer::start),
                   [this](QTimer *, int msec) {
                       ++timerStartCount;
                       lastTimerMs = msec;
                   });

    // Act
    obj->setSettingHeight(567);

    // Assert（B1）
    EXPECT_EQ(obj->m_windowHeight, 567);
    EXPECT_EQ(timerStartCount, 1);
    EXPECT_EQ(lastTimerMs, 1000);
}

// ═════════════════ setWallpaper ═════════════════

TEST_F(FileControlTest, SetWallpaper_NullPath_SkipsAllDbusCalls)
{
    // Arrange：连接层隔离——sessionBus 置为无效连接（所有 QDBusInterface 恒 invalid）；
    // 不直接 stub QDBusAbstractInterface::isValid：DTK DConfig 后台线程并发调用该方法，
    // 运行时补丁存在竞态，曾引发 SEGV 与计数污染
    dbusCallCount = 0;
    // 空名 QDBusConnection = 无效连接，接口 isValid 恒 false
    stub.set_lamda(static_cast<QDBusConnection (*)()>(&QDBusConnection::sessionBus),
                   []() -> QDBusConnection { return QDBusConnection(QString()); });
    stub.set_lamda(
            static_cast<QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &,
                                                                 const QList<QVariant> &)>(
                    &QDBusAbstractInterface::callWithArgumentList),
            [this](QDBusAbstractInterface *, QDBus::CallMode, const QString &method,
                   const QList<QVariant> &) {
                if (method != QStringLiteral("SetMonitorBackground")) {
                    // 非被测调用（DTK DConfig 后台线程轮询）：透传错误让其降级，不计入计数
                    return QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                     QStringLiteral("skip"));
                }
                ++dbusCallCount;
                return QDBusMessage().createReply(QVariant());
            });

    // Act
    obj->setWallpaper(QString());
    waitDbusCalls(0);

    // Assert（B1：null 路径 → 线程体无任何 DBus 交互）
    EXPECT_EQ(dbusCallCount.load(), 0);
    EXPECT_EQ(lastDbusMethod, QString());
}

TEST_F(FileControlTest, SetWallpaper_AllInterfacesInvalid_NoDbusCall)
{
    // Arrange：连接层隔离——接口全部 invalid，走"Failed to initialize"告警分支
    const QString png = makePngFile(tmpDir.path(), "wall0.png");
    dbusCallCount = 0;
    // 空名 QDBusConnection = 无效连接，接口 isValid 恒 false
    stub.set_lamda(static_cast<QDBusConnection (*)()>(&QDBusConnection::sessionBus),
                   []() -> QDBusConnection { return QDBusConnection(QString()); });
    stub.set_lamda(
            static_cast<QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &,
                                                                 const QList<QVariant> &)>(
                    &QDBusAbstractInterface::callWithArgumentList),
            [this](QDBusAbstractInterface *, QDBus::CallMode, const QString &method,
                   const QList<QVariant> &) {
                if (method != QStringLiteral("SetMonitorBackground")) {
                    // 非被测调用（DTK DConfig 后台线程轮询）：透传错误让其降级，不计入计数
                    return QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                     QStringLiteral("skip"));
                }
                ++dbusCallCount;
                return QDBusMessage().createReply(QVariant());
            });

    // Act
    obj->setWallpaper(png);
    waitDbusCalls(0);

    // Assert（B2+B3：Appearance 接口均无效 → 仅告警，无 SetMonitorBackground 调用）
    EXPECT_EQ(dbusCallCount.load(), 0);
    EXPECT_EQ(lastDbusMethod, QString());
}

TEST_F(FileControlTest, SetWallpaper_X11Session_CallsV23WithImagePath)
{
    // Arrange：真实 sessionBus + call 层拦截（不会真正下发壁纸设置）；
    // B6/B8 分支可达性依赖 org.deepin.dde.Appearance1 在位，按探测结果断言
    const QString png = makePngFile(tmpDir.path(), "wall1.png");
    dbusCallCount = 0;
    qputenv("XDG_SESSION_TYPE", "x11");
    qunsetenv("WAYLAND_DISPLAY");
    const bool appearanceUp = serviceRegistered(QStringLiteral("org.deepin.dde.Appearance1"));
    stub.set_lamda(
            static_cast<QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &,
                                                                 const QList<QVariant> &)>(
                    &QDBusAbstractInterface::callWithArgumentList),
            [this](QDBusAbstractInterface *, QDBus::CallMode, const QString &method,
                   const QList<QVariant> &args) {
                if (method != QStringLiteral("SetMonitorBackground")) {
                    // 非被测调用（DTK DConfig 后台线程轮询）：透传错误，不计入计数
                    return QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                     QStringLiteral("skip"));
                }
                ++dbusCallCount;
                lastDbusMethod = method;
                lastDbusArgs = args;
                return QDBusMessage().createReply(QVariant());
            });

    // Act
    obj->setWallpaper(png);
    waitDbusCalls(1);

    // Assert（B4+B6+B8：服务在位时经 V23 SetMonitorBackground 传图路径且不再走 V20；
    // 服务不在位时 B2 兜底路径 0 次调用）
    EXPECT_EQ(dbusCallCount.load(), appearanceUp ? 1 : 0);
    if (appearanceUp) {
        EXPECT_EQ(lastDbusMethod, QStringLiteral("SetMonitorBackground"));
        ASSERT_EQ(lastDbusArgs.size(), 2);
        EXPECT_EQ(lastDbusArgs.at(1).toString(), png);
    }
}

TEST_F(FileControlTest, SetWallpaper_WaylandSession_ReadsPrimaryAndCallsV23)
{
    // Arrange：wayland 会话判定走 Display1 接口取主屏名（无服务时 property 立即返回空，安全）
    const QString png = makePngFile(tmpDir.path(), "wall2.png");
    dbusCallCount = 0;
    qputenv("XDG_SESSION_TYPE", "wayland");
    qunsetenv("WAYLAND_DISPLAY");
    const bool appearanceUp = serviceRegistered(QStringLiteral("org.deepin.dde.Appearance1"));
    stub.set_lamda(
            static_cast<QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &,
                                                                 const QList<QVariant> &)>(
                    &QDBusAbstractInterface::callWithArgumentList),
            [this](QDBusAbstractInterface *, QDBus::CallMode, const QString &method,
                   const QList<QVariant> &args) {
                if (method != QStringLiteral("SetMonitorBackground")) {
                    return QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                     QStringLiteral("skip"));
                }
                ++dbusCallCount;
                lastDbusMethod = method;
                lastDbusArgs = args;
                return QDBusMessage().createReply(QVariant());
            });

    // Act
    obj->setWallpaper(png);
    waitDbusCalls(1);

    // Assert（B5+B8：wayland 判定后仍经 V23 调用；服务不在位时 B2 路径 0 次调用）
    EXPECT_EQ(dbusCallCount.load(), appearanceUp ? 1 : 0);
    if (appearanceUp) {
        EXPECT_EQ(lastDbusMethod, QStringLiteral("SetMonitorBackground"));
        ASSERT_EQ(lastDbusArgs.size(), 2);
        EXPECT_EQ(lastDbusArgs.at(1).toString(), png);
    }
}

TEST_F(FileControlTest, SetWallpaper_V23Fails_FallsBackToV20)
{
    // Arrange：V23 应答失败（首败次成），观察 V20 兜底是否发生
    const QString png = makePngFile(tmpDir.path(), "wall3.png");
    dbusCallCount = 0;
    failNextDbusCall = true;
    qputenv("XDG_SESSION_TYPE", "x11");
    qunsetenv("WAYLAND_DISPLAY");
    const bool v23Up = serviceRegistered(QStringLiteral("org.deepin.dde.Appearance1"));
    const bool v20Up = serviceRegistered(QStringLiteral("com.deepin.daemon.Appearance"));
    stub.set_lamda(
            static_cast<QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &,
                                                                 const QList<QVariant> &)>(
                    &QDBusAbstractInterface::callWithArgumentList),
            [this](QDBusAbstractInterface *, QDBus::CallMode, const QString &method,
                   const QList<QVariant> &args) {
                if (method != QStringLiteral("SetMonitorBackground")) {
                    return QDBusMessage::createError(QStringLiteral("test.passthrough"),
                                                     QStringLiteral("skip"));
                }
                ++dbusCallCount;
                lastDbusMethod = method;
                lastDbusArgs = args;
                if (failNextDbusCall) {
                    failNextDbusCall = false;
                    return QDBusMessage::createError(QStringLiteral("test.appearance"),
                                                     QStringLiteral("v23 down"));
                }
                return QDBusMessage().createReply(QVariant());
            });

    // Act
    obj->setWallpaper(png);
    waitDbusCalls(2);

    // Assert（B9+B10：V23 失败后 V20 兜底（服务在位时共两次）；V20 不在位则仅一次）
    if (v23Up && v20Up) {
        EXPECT_EQ(dbusCallCount.load(), 2);
        EXPECT_EQ(lastDbusArgs.at(1).toString(), png);
    } else if (v23Up) {
        EXPECT_EQ(dbusCallCount.load(), 1);  // V20 服务不在位，无兜底调用
        EXPECT_EQ(lastDbusMethod, QStringLiteral("SetMonitorBackground"));
    } else {
        EXPECT_EQ(dbusCallCount.load(), 0);  // Appearance 服务均不在位，走 B2 告警路径
        EXPECT_EQ(lastDbusMethod, QString());
    }
}

// ═════════════════ showPrintDialog ═════════════════

TEST_F(FileControlTest, ShowPrintDialog_LocalImagePath_DelegatesToHelper)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "print.png");
    stub.set_lamda(VADDR(PrintHelper, showPrintDialog),
                   [this](PrintHelper *, const QStringList &paths, QWidget *) {
                       ++printCallCount;
                       lastPrintPaths = paths;
                   });

    // Act
    obj->showPrintDialog(localUrl(png));

    // Assert（B1：URL 已转本地路径并打包成单元素列表）
    EXPECT_EQ(printCallCount, 1);
    ASSERT_EQ(lastPrintPaths.size(), 1);
    EXPECT_EQ(lastPrintPaths.at(0), png);
}

// ═════════════════ showShortcutPanel ═════════════════

TEST_F(FileControlTest, ShowShortcutPanel_CenterPosition_StartsViewerWithJsonAndPos)
{
    // Arrange
    procStartCount = 0;
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODeviceBase::OpenMode)>(&QProcess::start), [this](QProcess *, const QString &program, const QStringList &arguments, QIODeviceBase::OpenMode) {
        ++procStartCount;
        lastProgram = program;
        lastProcArgs = arguments;
    });

    // Act
    obj->showShortcutPanel(100, 200);

    // Assert（B1：参数为 -j=快捷键 JSON 与 -p=坐标）
    EXPECT_EQ(procStartCount, 1);
    EXPECT_EQ(lastProgram, QStringLiteral("deepin-shortcut-viewer"));
    ASSERT_EQ(lastProcArgs.size(), 2);
    EXPECT_TRUE(lastProcArgs.at(0).startsWith(QStringLiteral("-j=")));
    EXPECT_EQ(lastProcArgs.at(1), QStringLiteral("-p=100,200"));
}

// ═════════════════ slotFileReName ═════════════════

TEST_F(FileControlTest, SlotFileReName_WithSuffixTrue_RenamesAndEmitsSignal)
{
    // Arrange：name 自带后缀
    const QString oldFile = makePngFile(tmpDir.path(), "suftrue.png");
    const QString newFile = QDir(tmpDir.path()).filePath("newsuf.png");
    QSignalSpy spy(obj, &FileControl::imageRenamed);
    stub.set_lamda(VADDR(ImageFileWatcher, fileRename),
                   [this](ImageFileWatcher *, const QString &oldPath, const QString &newPath) {
                       ++watcherRenameCount;
                       lastWatcherRenameOld = oldPath;
                       lastWatcherRenameNew = newPath;
                   });

    // Act
    const bool ok = obj->slotFileReName(QStringLiteral("newsuf.png"), localUrl(oldFile), true);

    // Assert（B2+B4：文件移动、信号、watcher 通知三者齐全）
    EXPECT_TRUE(ok);
    EXPECT_TRUE(QFile::exists(newFile));
    EXPECT_FALSE(QFile::exists(oldFile));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toUrl(), QUrl::fromLocalFile(oldFile));
    EXPECT_EQ(spy.at(0).at(1).toUrl(), QUrl::fromLocalFile(newFile));
    EXPECT_EQ(watcherRenameCount, 1);
    EXPECT_EQ(lastWatcherRenameNew, newFile);
}

TEST_F(FileControlTest, SlotFileReName_WithoutSuffix_KeepsOldSuffix)
{
    // Arrange：name 不带后缀 → 自动拼接原后缀
    const QString oldFile = makePngFile(tmpDir.path(), "suffix.png");
    const QString newFile = QDir(tmpDir.path()).filePath("renewed.png");
    QSignalSpy spy(obj, &FileControl::imageRenamed);
    stub.set_lamda(VADDR(ImageFileWatcher, fileRename),
                   [this](ImageFileWatcher *, const QString &, const QString &) {
                       ++watcherRenameCount;
                   });

    // Act
    const bool ok = obj->slotFileReName(QStringLiteral("renewed"), localUrl(oldFile), false);

    // Assert（B3+B4）
    EXPECT_TRUE(ok);
    EXPECT_TRUE(QFile::exists(newFile));
    EXPECT_FALSE(QFile::exists(oldFile));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(watcherRenameCount, 1);
}

TEST_F(FileControlTest, SlotFileReName_MissingFile_ReturnsFalse)
{
    // Arrange
    const QString missing = QDir(tmpDir.path()).filePath("ghost.png");
    QSignalSpy spy(obj, &FileControl::imageRenamed);

    // Act
    const bool ok = obj->slotFileReName(QStringLiteral("newname"), localUrl(missing), false);

    // Assert（B1：失败且无副作用）
    EXPECT_FALSE(ok);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(FileControlTest, SlotFileReName_TargetDirMissing_ReturnsFalse)
{
    // Arrange：目标父目录不存在 → rename 失败
    const QString oldFile = makePngFile(tmpDir.path(), "stay.png");
    QSignalSpy spy(obj, &FileControl::imageRenamed);

    // Act
    const bool ok = obj->slotFileReName(QStringLiteral("no-such-dir/target"), localUrl(oldFile),
                                        true);

    // Assert（B5：失败且原文件保持原位）
    EXPECT_FALSE(ok);
    EXPECT_TRUE(QFile::exists(oldFile));
    EXPECT_EQ(spy.count(), 0);
}

// ═════════════════ slotFileSuffix ═════════════════

namespace {
struct SuffixCase {
    QString pathInput;
    bool withDot;
    QString expected;
};
}  // namespace

class SlotFileSuffixParamTest : public FileControlTest,
                                public ::testing::WithParamInterface<SuffixCase> {
};

TEST_P(SlotFileSuffixParamTest, SlotFileSuffix_ParamSet_ReturnsCompleteSuffix)
{
    // Arrange
    const auto &c = GetParam();
    QString input = c.pathInput;
    if (input == QStringLiteral("@png@")) {
        // 占位符替换为真实存在的多级后缀文件
        input = QDir(tmpDir.path()).filePath(QStringLiteral("archive.tar.gz"));
        QFile f(input);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("x");
        f.close();
    }
    if (!input.isEmpty()) {
        // 统一以 file:// URL 传入：源码 QUrl(path).toLocalFile() 对裸路径返回空串，
        // QFile::exists("") 恒 false（与 isCan* 系列同源的 D7 输入约定）
        input = localUrl(input);
    }

    // Act
    const QString suffix = obj->slotFileSuffix(input, c.withDot);

    // Assert（B1/B2/B3 见参数注释）
    EXPECT_EQ(suffix, c.expected);
    EXPECT_EQ(suffix.isEmpty(), c.expected.isEmpty());
}

INSTANTIATE_TEST_SUITE_P(
        DotAndExistence, SlotFileSuffixParamTest,
        ::testing::Values(
                SuffixCase{QStringLiteral("@png@"), true, QStringLiteral(".tar.gz")},   // B2
                SuffixCase{QStringLiteral("@png@"), false, QStringLiteral("tar.gz")},  // B3
                SuffixCase{QStringLiteral("/nowhere/absent.png"), true, QString()},    // B1 缺失
                SuffixCase{QString(), false, QString()}));                             // B1 空串

// ═════════════════ slotGetFileName / slotGetFileNameSuffix ═════════════════

TEST_F(FileControlTest, SlotGetFileName_PlainPath_ReturnsBaseName)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "照片图.png");
    ASSERT_EQ(png.startsWith(QStringLiteral("file://")), false);

    // Act
    const QString name = obj->slotGetFileName(png);

    // Assert（B2）
    EXPECT_EQ(name, QStringLiteral("照片图"));
    EXPECT_EQ(name, QFileInfo(png).completeBaseName());
}

TEST_F(FileControlTest, SlotGetFileName_FileUrl_ReturnsBaseName)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "urlname.png");
    const QString input = localUrl(png);
    ASSERT_TRUE(input.startsWith(QStringLiteral("file://")));

    // Act
    const QString name = obj->slotGetFileName(input);

    // Assert（B1+B2：URL 先转本地路径再取名）
    EXPECT_EQ(name, QStringLiteral("urlname"));
    EXPECT_EQ(name, QFileInfo(png).completeBaseName());
}

TEST_F(FileControlTest, SlotGetFileNameSuffix_PlainPath_ReturnsFileName)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "fullname.png");
    ASSERT_EQ(png.startsWith(QStringLiteral("file://")), false);

    // Act
    const QString name = obj->slotGetFileNameSuffix(png);

    // Assert（B2）
    EXPECT_EQ(name, QStringLiteral("fullname.png"));
    EXPECT_EQ(name, QFileInfo(png).fileName());
}

TEST_F(FileControlTest, SlotGetFileNameSuffix_FileUrl_ReturnsFileName)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "urlfull.jpeg");
    const QString input = localUrl(png);
    ASSERT_TRUE(input.startsWith(QStringLiteral("file://")));

    // Act
    const QString name = obj->slotGetFileNameSuffix(input);

    // Assert（B1+B2）
    EXPECT_EQ(name, QStringLiteral("urlfull.jpeg"));
    EXPECT_EQ(name, QFileInfo(png).fileName());
}

// ═════════════════ slotGetInfo ═════════════════

TEST_F(FileControlTest, SlotGetInfo_FirstLookup_LoadsMetadataOnce)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "meta.png");
    fakeMetaData = QMap<QString, QString>{ { QStringLiteral("Resolution"),
                                             QStringLiteral("4x4") } };
    metaCallCount = 0;
    stub.set_lamda(&LibUnionImage_NameSpace::getAllMetaData,
                   [this](const QString &) -> QMap<QString, QString> {
                       ++metaCallCount;
                       return fakeMetaData;
                   });

    // Act
    const QString res = obj->slotGetInfo(QStringLiteral("Resolution"), localUrl(png));
    const QString cached = obj->slotGetInfo(QStringLiteral("Resolution"), localUrl(png));

    // Assert（B1 首次加载 + B2 同路径复用缓存不再加载）
    EXPECT_EQ(res, QStringLiteral("4x4"));
    EXPECT_EQ(cached, QStringLiteral("4x4"));
    EXPECT_EQ(metaCallCount, 1);
    EXPECT_EQ(obj->m_currentPath, png);
}

TEST_F(FileControlTest, SlotGetInfo_UnknownKey_ReturnsDash)
{
    // Arrange
    const QString png = makePngFile(tmpDir.path(), "meta2.png");
    fakeMetaData.clear();
    stub.set_lamda(&LibUnionImage_NameSpace::getAllMetaData,
                   [this](const QString &) -> QMap<QString, QString> {
                       ++metaCallCount;
                       return fakeMetaData;
                   });

    // Act
    const QString val = obj->slotGetInfo(QStringLiteral("NotAKey"), localUrl(png));

    // Assert（B3：空值兜底为 "-"）
    EXPECT_EQ(val, QStringLiteral("-"));
    EXPECT_EQ(metaCallCount, 1);
}

// ═════════════════ standardPicturesPath ═════════════════

TEST_F(FileControlTest, StandardPicturesPath_PicturesLocation_MatchesQtValue)
{
    // Arrange
    const QString expect = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    // Act
    const QString got = obj->standardPicturesPath();

    // Assert（B1：与 QStandardPaths 的图片目录一致且非空）
    EXPECT_EQ(got, expect);
    EXPECT_FALSE(got.isEmpty());
}

// ═════════════════ terminateShortcutPanelProcess ═════════════════

TEST_F(FileControlTest, TerminateShortcutPanelProcess_IdleProcess_TerminatesAndWaits)
{
    // Arrange
    stub.set_lamda(static_cast<void (QProcess::*)()>(&QProcess::terminate),
                   [this](QProcess *) { ++procTermCount; });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                   [this](QProcess *, int msecs) -> bool {
                       ++procWaitCount;
                       lastWaitMs = msecs;
                       return false;
                   });

    // Act
    obj->terminateShortcutPanelProcess();

    // Assert（B1：先 terminate 再 waitForFinished(2000)）
    EXPECT_EQ(procTermCount, 1);
    EXPECT_EQ(procWaitCount, 1);
    EXPECT_EQ(lastWaitMs, 2000);
}

TEST_F(FileControlTest, TerminateShortcutPanelProcess_WaitReturnsTrue_CompletesQuickly)
{
    // Arrange
    stub.set_lamda(static_cast<void (QProcess::*)()>(&QProcess::terminate),
                   [this](QProcess *) { ++procTermCount; });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                   [this](QProcess *, int) -> bool {
                       ++procWaitCount;
                       return true;
                   });

    // Act
    obj->terminateShortcutPanelProcess();

    // Assert（B1：waitFor 成功路径，仅一次 terminate）
    EXPECT_EQ(procTermCount, 1);
    EXPECT_EQ(procWaitCount, 1);
}

// ═════════════════ 疑似源码缺陷清单（只标红不修）═════════════════
// D1 setWallpaper: X11 分支 QGuiApplication::primaryScreen()->name() 未判空
//    （filecontrol.cpp setWallpaper 线程体内，headless/无屏环境空指针解引用风险）
// D2 getNamePath: newPath 拼接用 newName 原串而非转换后的 now
//    （file:// 输入时新路径错误，now 转换结果被丢弃）
// D3 isCheckOnly: open 成功后 fd 未 close、锁未释放，且变量名 flock 遮蔽同名函数
//    （fd 泄漏；锁依赖进程退出才释放。注：lockf 为进程级锁，同进程重复加锁仍成功）
// D4 isCanDelete: 局部变量 isAlbum 恒为 false，`|| (isAlbum && isWritable)` 分支永不生效
// D5 slotGetFileName 系列: file:// 前缀判断使用 startsWith("file://") 而非 QUrl 解析，
//    大小写/编码变体（如 "FILE://"）不识别（轻微，行为锁定为当前实现）
// D6 getDirImagePath: QFileInfo(QUrl(path).toLocalFile()).dir().path() 把入参当"文件"取父目录，
//    传目录 URL 时扫描到其父目录（本例 /tmp 下历史图片文件混入结果）——入参约定应为目录内文件路径
// D7 isCanDelete/isCanReadable/isCanRename/isCanSupportOcr/isCanWrite/isSupportSetWallpaper:
//    QUrl(path).toLocalFile() 对无 scheme 裸路径返回空串（无 isImage 的 isLocalFile?x:path 回退），
//    裸路径输入静默判 false，与 isImage 行为不一致（输入约定为 file:// URL，测试按真实行为锁定）
