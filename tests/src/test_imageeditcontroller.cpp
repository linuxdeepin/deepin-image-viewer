// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | ImageEditController(QObject*) | low | - | 1 | 1 |
// | active() | mid | in_degree:4 | 2 | 2 |
// | applyBoxBlur(const QRect&,int)（现名 applyGaussianBlur，经 applyEffect("gaussian") 间接覆盖） | low | - | 1 | 2 |
// | applyEffect(const QString&,const QRectF&,int) | mid | complexity:5 | 2 | 4 (TEST_F) + 3 (TEST_P 实例) = 7 |
// | applyGraffiti(const QRect&,int)（private，经 applyEffect("graffiti") 间接覆盖） | low | - | 1 | 1 + 3 (TEST_P 实例) = 4 |
// | applyMosaic(const QRect&,int)（private，经 applyEffect("mosaic") 间接覆盖） | low | - | 1 | 2 |
// | beginEdit(const QUrl&,int) | low | - | 1 | 6 |
// | canEdit(const QUrl&) | mid | in_degree:3 | 2 | 3 (TEST_F) + 4 (TEST_P 实例) = 7 |
// | canRedo() | low | - | 1 | 3 |
// | canUndo() | low | - | 1 | 3 |
// | commitHistory() | mid | complexity:5 | 2 | 4 |
// | crop(const QRectF&) | mid | in_degree:3 | 2 | 4 |
// | defaultSaveUrl() | mid | name_pattern:defaultSaveUrl | 2 | 2 |
// | discard() | low | - | 1 | 3 |
// | image() | mid | in_degree:5 | 2 | 2 |
// | isEditing(const QUrl&,int) | low | - | 1 | 3 |
// | isOriginalUrl(const QUrl&) | low | - | 1 | 3 |
// | markSaved() | mid | name_pattern:markSaved | 2 | 3 |
// | modified() | mid | in_degree:3 | 2 | 3 |
// | pixelRect(const QRectF&)（private，经 crop/applyEffect 间接覆盖） | low | - | 1 | 3 |
// | redo() | low | - | 1 | 3 |
// | revision() | low | - | 1 | 2 |
// | rotateClockwise() | low | - | 1 | 2 |
// | saveComposite(const QUrl&,const QVariantList&) | high | complexity:17,alloc_in_loop:1 | 4 | 8 |
// | undo() | low | - | 1 | 4 |
// | EditedImageProvider(ImageEditController*) | low | - | 1 | 2 |
// | requestImage(const QString&,QSize*,const QSize&) | low | - | 1 | 3 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x] （26 个 inventory 方法全部有对应用例，private 方法经 public API 间接覆盖）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x] （路径空/不支持后缀/多帧/有效；strength 合法/非法；rect 全图/半图/越界/过小）
// 3. 每个等价类的边界值显式覆盖: [x] （frameIndex 0/越界；historyIndex 0/-1/49/50 上限；2x2 最小矩形）
// 4. 同质 ≥ 3 组用 TEST_P: [x] （不支持后缀 ×4、非法强度 ×3、graffiti 强度 ×3）
// 5. 分支清单 → 用例映射已列出: [x] （见下方各方法分支清单）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x] （源码无 throw 分支，不适用；错误路径用返回值+信号断言）
// 8. 负面场景有专门用例: [x] （空 URL/格式不匹配/不可写目录/损坏文件/越界 frameIndex/无图操作）
// 9. 负面用例验证强异常安全: [x] （失败后 revision/image/canUndo 状态保持不变）
// 10. stub_ext vs gMock 选择正确: [x] （纯数据驱动类，无外部进程/网络/环境依赖，无需 stub/gMock）
//
// 说明：MCP 图谱对该文件的行号锚点是旧版（416 行版），本文件分支清单依据当前版
// （622 行，get_code_snippet 类节点 + search_code 逐行拼合还原）真实源码编写；
// inventory 中 applyBoxBlur 在当前源码中已更名 applyGaussianBlur（仍为 private），
// 经 public 入口 applyEffect("gaussian", rect, strength) 间接覆盖。
//
// 分支清单（来源：ImageEditController::canEdit，imageeditcontroller.cpp:35-46）
// B1: path.isEmpty() → return false
// B2: reader.imageCount() > 1（多帧图，如 GIF）→ return false
// B3: QFileInfo(path).suffix().toLower() ∉ {bmp,jpg,jpeg,png,pgm,ppm,xpm,ico,icns} → return false（∈ → true）
// 用例映射：
// - CanEdit_EmptyUrl_ReturnsFalse                        → B1
// - CanEdit_MultiFrameImage_ReturnsFalse                 → B2
// - CanEdit_UnsupportedSuffix_ReturnsFalse（TEST_P×4）   → B3
// - CanEdit_SupportedPng_ReturnsTrue                     → B3（∈ 集合侧）
//
// 分支清单（来源：ImageEditController::active）
// B1: return !m_image.isNull()（无图 false / 有图 true 两态）
// 用例映射：
// - Active_WithoutImage_ReturnsFalse / Active_AfterBeginEdit_ReturnsTrue → B1 两侧
//
// 分支清单（来源：ImageEditController::canRedo）
// B1: m_historyIndex >= 0 && m_historyIndex + 1 < m_history.size()（短路两翼）
// 用例映射：
// - CanRedo_Initially_ReturnsFalse / CanRedo_AfterUndo_ReturnsTrue / CanRedo_AfterRedoToLatest_ReturnsFalse → B1
//
// 分支清单（来源：ImageEditController::canUndo）
// B1: return m_historyIndex > 0
// 用例映射：
// - CanUndo_Initially_ReturnsFalse / CanUndo_AfterCommit_ReturnsTrue / CanUndo_BackToOldest_ReturnsFalse → B1
//
// 分支清单（来源：ImageEditController::modified）
// B1: return m_historyIndex != m_savedHistoryIndex
// 用例映射：
// - Modified_AfterBeginEdit_ReturnsFalse / Modified_AfterCommit_ReturnsTrue / Modified_AfterMarkSaved_ReturnsFalse → B1
//
// 分支清单（来源：ImageEditController::beginEdit，imageeditcontroller.cpp:84-112）
// B1: !canEdit(source)（内含空路径 B1/多帧 B2/不支持后缀 B3）→ return false
// B2: isEditing(source, frameIndex) 已在编辑 → 提前 return true（不重载、不发信号）
// B3: frameIndex > 0 && !reader.jumpToImage(frameIndex)（单帧图越界帧号）→ return false
// B4: reader.read() 为 null（文件不存在/内容损坏）→ return false
// B5: 读取成功 → m_image 转 ARGB32_Premultiplied，重置 m_source/m_frameIndex/m_history/m_historyIndex/m_savedHistoryIndex
// B6: 成功路径 ++m_revision
// B7: 成功路径 Q_EMIT activeChanged()
// B8: 成功路径 Q_EMIT historyChanged()
// B9: 成功路径 Q_EMIT revisionChanged() + return true
// 用例映射：
// - BeginEdit_UnsupportedSuffix_ReturnsFalseWithoutSignals  → B1(B3 内含)
// - BeginEdit_SameSourceAgain_ReturnsTrueWithoutReload      → B2
// - BeginEdit_FrameIndexOutOfRange_ReturnsFalse             → B3
// - BeginEdit_NonexistentPng_ReturnsFalse / BeginEdit_CorruptPngContent_ReturnsFalse → B4
// - BeginEdit_ValidPng_LoadsImageAndEmitsSignals            → B5/B6/B7/B8/B9
//
// 分支清单（来源：ImageEditController::isOriginalUrl，imageeditcontroller.cpp:122-134）
// B1: destInfo.exists() ? canonicalFilePath : absoluteFilePath（存在→canonical）
// B2: srcInfo.exists() ? canonicalFilePath : absoluteFilePath（存在→canonical）
// B3: destCanonical == srcCanonical → 相等返回 true，否则 false
// 用例映射：
// - IsOriginalUrl_SameExistingPath_ReturnsTrue        → B1/B2/B3
// - IsOriginalUrl_DifferentPath_ReturnsFalse          → B1/B2/B3
// - IsOriginalUrl_NonexistentDestination_ReturnsFalse → B1（absoluteFilePath 侧）
//
// 分支清单（来源：ImageEditController::saveComposite，imageeditcontroller.cpp:136-287）
// B1: m_image.isNull() → return false（无 saveFailed 信号）
// B2: targetPath.isEmpty() → Q_EMIT saveFailed + return false
// B3: QImageReader::imageFormat(targetPath) 为空（目标文件不存在）→ 回退取后缀
// B4: normalizeFormat lambda 内 format == "jpg" || "jpeg" → 归一为 "jpeg"
// B5: normalizeFormat(targetFormat) != normalizedSource → saveFailed + return false
// B6: for annotations 循环（空列表 0 次）
// B7: rawPoints.size() < 2 → continue（跳过该注解）
// B8: rawPoint 内层循环（归一化坐标 → 像素坐标）
// B9: bounds 求 min/max 循环
// B10: type == "rect" → 旋转绘制矩形
// B11: else if type == "ellipse" → 旋转绘制椭圆
// B12: else if type == "text" || type == "number" → 进入文字/数字分支
// B13: fontFamily 非空 → font.setFamily
// B14: type == "number" → 填充圆 + 按亮度选黑/白笔
// B15: luminance > kNumberTextContrastThreshold 三元 → Qt::black / Qt::white
// B16: else（type == "text"）→ 绘制文本
// B17: else（其余类型）→ QPainterPath 折线
// B18: 折线 for 循环 lineTo
// B19: type == "pen" → 旋转绘制路径
// B20: 非 pen → 直接 drawPath
// B21: type == "arrow" → 追加两条箭头翼线
// B22: !file.open(WriteOnly) → saveFailed + return false
// B23: !writer.write(result) → saveFailed + cancelWriting + return false
// B24: !file.commit() → saveFailed + return false；全成功 → return true
// 用例映射：
// - SaveComposite_WithoutActiveImage_ReturnsFalseSilently    → B1
// - SaveComposite_EmptyDestinationUrl_ReturnsFalse           → B2
// - SaveComposite_MatchingPngNoAnnotations_WritesFile       → B3/B6(0 次)/B24
// - SaveComposite_FormatMismatch_ReturnsFalseAndKeepsState  → B4/B5
// - SaveComposite_JpgSourceAndJpegDest_SynonymsAccepted     → B4（两侧归一相等）
// - SaveComposite_AnnotationsWithFewerThanTwoPoints_Skipped → B7
// - SaveComposite_RectPenNumberAnnotations_AllSucceed       → B8-B21（各类型注解）
// - SaveComposite_UnwritableDirectory_ReturnsFalse          → B22
//
// 分支清单（来源：ImageEditController::discard，imageeditcontroller.cpp:289-306）
// B1: m_image.isNull() → 提前 return（不发射任何信号）
// B2: 有图 → 清空 m_image/m_source/m_frameIndex/m_history，index=-1，saved=0，++m_revision
// B3: 清空后 Q_EMIT activeChanged()/historyChanged()/revisionChanged()
// 用例映射：
// - Discard_WithoutImage_EmitsNothing          → B1
// - Discard_WithActiveImage_ClearsAndEmitsAll  → B2/B3
// - Discard_ThenBeginDifferentSource_Succeeds  → B2/B3 后可重新 beginEdit
//
// 分支清单（来源：ImageEditController::applyEffect，imageeditcontroller.cpp:324-353）
// B1: m_image.isNull() → return false
// B2: pixelRect 宽 < 2 || 高 < 2 → return false
// B3: effect == "gaussian" 且 strength ∉ {5,15,30} → return false
// B4: effect == "gaussian" 且强度合法 → applyGaussianBlur + 成功路径
// B5: effect == "mosaic" 且 strength ∉ {8,16,32} → return false
// B6: effect == "mosaic" 且强度合法 → applyMosaic + 成功路径
// B7: effect == "graffiti" 且 strength ∉ {8,16,32} → return false
// B8: effect == "graffiti" 且强度合法 → applyGraffiti + 成功路径
// B9: 其它 effect 名 → return false
// B10: 成功路径 ++m_revision + Q_EMIT revisionChanged() + return true
// 用例映射：
// - ApplyEffect_WithoutImage_ReturnsFalse                     → B1
// - ApplyEffect_TooSmallRegion_ReturnsFalseWithoutRevisionBump → B2
// - ApplyEffect_InvalidStrength_ReturnsFalse（TEST_P×3）      → B3/B5/B7
// - ApplyEffect_GaussianOnSolidImage_PreservesPixels          → B4/B10
// - ApplyEffect_UnknownEffectName_ReturnsFalse                → B9
// - ApplyMosaic_MixedColorsRegion_AveragesToMeanColor         → B6/B10
// - ApplyGraffiti_ViaEffect_PreservesSolidPixels 等           → B8/B10
//
// 分支清单（来源：ImageEditController::crop，imageeditcontroller.cpp:355-369）
// B1: m_image.isNull() → return false
// B2: rect.width() < 2 || rect.height() < 2 || rect == m_image.rect() → return false（全图裁剪拒绝）
// B3: m_image = m_image.copy(rect) + ++m_revision + Q_EMIT revisionChanged() + return true
// 用例映射：
// - Crop_WithoutImage_ReturnsFalse               → B1
// - Crop_FullImageRect_ReturnsFalseWithoutRevisionBump / Crop_TooSmallRect_ReturnsFalse → B2
// - Crop_HalfNormalizedRect_ReturnsTrueAndResizes → B3
//
// 分支清单（来源：ImageEditController::commitHistory，imageeditcontroller.cpp:386-407）
// B1: m_image.isNull() → 提前 return（无 historyChanged 信号）
// B2: m_historyIndex + 1 < m_history.size()（存在 redo 分支需截断）
// B3: m_savedHistoryIndex > m_historyIndex → m_savedHistoryIndex = -1
// B4: m_history.resize(m_historyIndex + 1)（丢弃 redo 分支）
// B5: m_history.push_back(m_image) 追加快照
// B6: while m_history.size() > 50 → removeFirst + index-- + saved>=0 时 saved--
// B7: m_historyIndex = size-1 + Q_EMIT historyChanged()
// 用例映射：
// - CommitHistory_WithoutImage_EmitsNothing            → B1
// - CommitHistory_AppendsSnapshot_AndEmitsHistoryChanged → B5/B7
// - CommitHistory_TruncatesRedoBranch_OnNewCommit     → B2/B4
// - CommitHistory_CapsHistoryAtFiftyEntries           → B6（50 条上限循环边界）
//
// 分支清单（来源：ImageEditController::markSaved，imageeditcontroller.cpp:409-415）
// 说明：本方法源码无 if 分支（赋值 + 出锁 + 发射）；下列为其可观测行为路径（含状态分支）。
// B1: m_savedHistoryIndex = m_historyIndex（未编辑态为 -1 → modified 变 false）
// B2: locker.unlock() 后 Q_EMIT historyChanged()（恰 1 次）
// B3: 提交后调用 → modified 由 true 变 false
// B4: revision/canUndo 等其它状态保持不变
// B5: 未编辑时调用 → saved 置 -1，后续 modified 保持 false
// 用例映射：
// - MarkSaved_AfterCommit_ClearsModifiedFlag    → B1/B3
// - MarkSaved_EmitsHistoryChangedExactlyOnce    → B2/B4
// - MarkSaved_InUneditedState_KeepsModifiedFalse → B5
//
// 分支清单（来源：ImageEditController::undo，imageeditcontroller.cpp:417-434）
// B1: m_historyIndex <= 0（无历史/已在最早）→ return false
// B2: --m_historyIndex 后取 m_history[m_historyIndex] 快照
// B3: target.cacheKey() != m_image.cacheKey()（图像变化）→ m_image = target + ++m_revision
// B4: imageChanged → Q_EMIT revisionChanged()
// B5: 无论是否变化 Q_EMIT historyChanged()
// B6: return true
// 用例映射：
// - Undo_NothingToUndo_ReturnsFalse / Undo_AtOldestEntry_ReturnsFalse → B1
// - Undo_AfterCropCommit_RestoresOriginalImage  → B2/B3/B4/B5/B6
// - Undo_IdenticalSnapshot_DoesNotBumpRevision  → B2 + B3 为 false + B5
//
// 分支清单（来源：ImageEditController::redo，imageeditcontroller.cpp:436-453）
// B1: m_historyIndex < 0 || m_historyIndex + 1 >= m_history.size() → return false
// B2: ++m_historyIndex 后取快照
// B3: imageChanged → m_image = target + ++m_revision
// B4: imageChanged → Q_EMIT revisionChanged()
// B5: Q_EMIT historyChanged()
// B6: return true
// 用例映射：
// - Redo_NoHistory_ReturnsFalse / Redo_AtLatestEntry_ReturnsFalse → B1
// - Redo_AfterUndo_RestoresCroppedImage                          → B2-B6
//
// 分支清单（来源：ImageEditController::applyGaussianBlur（原 applyBoxBlur），imageeditcontroller.cpp:455-469）
// B1: sampleRect = rect.adjusted(-r,-r,r,r) ∩ m_image.rect()（边缘采样越界钳制）
// B2: cv::GaussianBlur kernelSize = r*2+1，BORDER_REPLICATE 边界复制
// B3: QPainter 画回 rect 区域（drawImage 源矩形平移）
// 用例映射：
// - ApplyBoxBlur_ViaGaussianEffect_SolidImageStaysUniform → B1/B2（纯色图模糊后不变）
// - ApplyBoxBlur_EdgeClampedRegion_CompletesWithoutArtifacts → B1（贴边区域）
//
// 分支清单（来源：ImageEditController::applyMosaic，imageeditcontroller.cpp:471-502）
// B1: for y 块行循环（y += blockSize）
// B2: for x 块列循环
// B3: blockWidth/blockHeight = qMin(blockSize, 剩余)（末块边界）
// B4: for row / for column 像素累加循环（red/green/blue/alpha 累加）
// B5: 平均色 qRgba(sums/count)（整除截断）
// B6: for row 末块回填 + QPainter drawImage 画回 m_image
// 用例映射：
// - ApplyMosaic_MixedColorsRegion_AveragesToMeanColor → B1-B6（8x8 双色块均值 127）
// - ApplyMosaic_SolidRegion_PixelsUnchanged           → B1-B6（纯色均值不变）
//
// 分支清单（来源：ImageEditController::applyGraffiti，imageeditcontroller.cpp:504-607）
// B1: brush（:/res/graffiti_mixer_tip.png）isNull → 提前 return
// B2: diameter 三元 strength==8?96:(strength==16?174:256)
// B3: baseMask 画笔 alpha 掩码双层循环（brushY/brushX）
// B4: scales {0.85,1.0,1.15} × angles {±20,±10,0} 生成 stampMasks 双层循环
// B5: 行盖章 y 循环（y < result.height() + rowStep）
// B6: 行内 distance 循环（distance < result.width() + diameter）
// B7: !hasReservoir → 首次采样设 reservoir；否则 0.7/0.3 混合
// B8: 随机选 stampMasks 掩码
// B9: coverage == 0 → continue（跳过透明印点）
// B10: destinationAlpha == 255 → 不透明直混；否则按 alpha 加权合成
// 用例映射：
// - ApplyGraffiti_ViaEffect_PreservesSolidPixels       → B6-B10（纯色图印章后逐像素不变）
// - ApplyGraffiti_AllStrengths_ReturnsTrue（TEST_P×3） → B2（8/16/32 三档直径）

#include <gtest/gtest.h>

#include <QColor>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>
#include <QVariantList>

#include "stub_ext/stubext.h"
#include "imageeditcontroller.h"

namespace {

// 程序生成纯色图（不依赖磁盘素材）
QImage makeSolidImage(int width, int height, const QColor &color)
{
    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(color);
    return image;
}

// 左右双色图：左半 leftColor，右半 rightColor（用于 mosaic 均值断言）
QImage makeTwoColorImage(int width, int height, const QColor &leftColor, const QColor &rightColor)
{
    QImage image(width, height, QImage::Format_ARGB32);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x)
            image.setPixel(x, y, x < width / 2 ? leftColor.rgba() : rightColor.rgba());
    }
    return image;
}

// 两帧 GIF 字节流（4x4，GIF89a + NETSCAPE 循环扩展 + 2 个图像块）——用于 canEdit
// 的多帧分支。注意：Qt6 的 QGifHandler::imageCount 需逐帧成功解码，帧数据必须
// 是可解码的真实 LZW 流（手工最简帧会被判 0 帧）；以下为验证过的规范字节。
QByteArray twoFrameGifBytes()
{
    return QByteArray::fromHex(
        "47494638396104000400810000ff000000000000000000000021ff0b4e45545343415045"
        "322e30030100000021f904000a0000002c00000000040004000008090001081c48b02080"
        "800021f904010a0001002c00000000040004008100ff0000000000000000000008090001"
        "081c48b0208080003b");
}

// 不支持后缀参数：路径（不必存在）→ 期望 false
// 见 CanEdit_UnsupportedSuffix_ReturnsFalse

// applyEffect 非法强度参数
struct EffectStrengthCase {
    QString effect;
    int strength;
};

}  // namespace

class ImageEditControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        stub.clear();
        controller = new ImageEditController();
    }

    void TearDown() override {
        delete controller;
        controller = nullptr;
        stub.clear();
    }

    // 在临时目录生成 8x8 纯色 PNG 并 beginEdit（多数用例的标准前置）
    QUrl beginWithSolidPng(const QColor &color, const QString &fileName = QStringLiteral("photo.png"))
    {
        const QString path = dir.filePath(fileName);
        makeSolidImage(8, 8, color).save(path, "PNG");
        const QUrl source = QUrl::fromLocalFile(path);
        controller->beginEdit(source);
        return source;
    }

    stub_ext::StubExt stub;
    ImageEditController *controller = nullptr;
    QTemporaryDir dir;
};

// ═══════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F / TEST_P 包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════

TEST_F(ImageEditControllerTest, ImageEditController_WithParent_InitializesDefaultState)
{
    // Arrange
    QObject parent;

    // Act
    ImageEditController *child = new ImageEditController(&parent);

    // Assert
    EXPECT_EQ(child->parent(), &parent);
    EXPECT_EQ(child->revision(), 0);
    EXPECT_FALSE(child->active());
    EXPECT_TRUE(child->image().isNull());
    EXPECT_FALSE(child->canUndo());
    EXPECT_FALSE(child->canRedo());
    delete child;
}

TEST_F(ImageEditControllerTest, Active_WithoutImage_ReturnsFalse)
{
    // Arrange
    QSignalSpy spy(controller, &ImageEditController::activeChanged);

    // Act
    const bool active = controller->active();

    // Assert
    EXPECT_FALSE(active);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_TRUE(controller->image().isNull());
}

TEST_F(ImageEditControllerTest, Active_AfterBeginEdit_ReturnsTrue)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));

    // Act
    const bool active = controller->active();

    // Assert
    EXPECT_TRUE(active);
    EXPECT_FALSE(controller->image().isNull());
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
}

TEST_F(ImageEditControllerTest, CanEdit_EmptyUrl_ReturnsFalse)
{
    // Arrange
    const QUrl emptyUrl;

    // Act
    const bool editable = controller->canEdit(emptyUrl);

    // Assert
    EXPECT_FALSE(editable);
    EXPECT_EQ(emptyUrl.toLocalFile(), QString());
    EXPECT_EQ(controller->revision(), 0);
}

// 同质多组输入：不支持的后缀（含不存在文件，走后缀判断分支）
class CanEditSuffixParamTest : public ImageEditControllerTest,
                               public ::testing::WithParamInterface<QString> {
};

TEST_P(CanEditSuffixParamTest, CanEdit_UnsupportedSuffix_ReturnsFalse)
{
    // Arrange
    const QUrl source = QUrl::fromLocalFile(dir.filePath(GetParam()));

    // Act
    const bool editable = controller->canEdit(source);

    // Assert
    EXPECT_FALSE(editable);  // 后缀 ∉ 支持列表（imageCount 分支不命中）
    EXPECT_EQ(source.fileName(), GetParam());
}

INSTANTIATE_TEST_SUITE_P(UnsupportedSuffixes, CanEditSuffixParamTest,
                         ::testing::Values(QStringLiteral("clip.gif"),
                                           QStringLiteral("pic.webp"),
                                           QStringLiteral("note.txt"),
                                           QStringLiteral("doc.pdf")));

TEST_F(ImageEditControllerTest, CanEdit_MultiFrameImage_ReturnsFalse)
{
    // Arrange
    const QString path = dir.filePath("anim.gif");
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(twoFrameGifBytes());
    file.close();
    const QUrl source = QUrl::fromLocalFile(path);

    // Act
    const bool editable = controller->canEdit(source);

    // Assert
    EXPECT_FALSE(editable);  // 多帧图：imageCount() > 1 分支（gif 后缀同样不支持）
    EXPECT_GT(QImageReader(path).imageCount(), 1);
}

TEST_F(ImageEditControllerTest, CanEdit_SupportedPng_ReturnsTrue)
{
    // Arrange
    const QString path = dir.filePath("photo.png");
    makeSolidImage(4, 4, QColor(0, 255, 0)).save(path, "PNG");
    const QUrl source = QUrl::fromLocalFile(path);

    // Act
    const bool editable = controller->canEdit(source);

    // Assert
    EXPECT_TRUE(editable);  // 单帧 + png 后缀 ∈ 支持列表
    EXPECT_EQ(QImageReader(path).imageCount(), 1);
}

TEST_F(ImageEditControllerTest, BeginEdit_ValidPng_LoadsImageAndEmitsSignals)
{
    // Arrange
    QSignalSpy activeSpy(controller, &ImageEditController::activeChanged);
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);
    const QString path = dir.filePath("photo.png");
    makeSolidImage(8, 8, QColor(255, 0, 0)).save(path, "PNG");

    // Act
    const bool started = controller->beginEdit(QUrl::fromLocalFile(path));

    // Assert
    EXPECT_TRUE(started);
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
    EXPECT_EQ(controller->image().format(), QImage::Format_ARGB32_Premultiplied);
    EXPECT_EQ(controller->revision(), 1);
    EXPECT_EQ(activeSpy.count(), 1);
    EXPECT_EQ(historySpy.count(), 1);
    EXPECT_EQ(revisionSpy.count(), 1);
}

TEST_F(ImageEditControllerTest, BeginEdit_UnsupportedSuffix_ReturnsFalseWithoutSignals)
{
    // Arrange
    QSignalSpy activeSpy(controller, &ImageEditController::activeChanged);
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);
    const QUrl source = QUrl::fromLocalFile(dir.filePath("note.txt"));

    // Act
    const bool started = controller->beginEdit(source);

    // Assert
    EXPECT_FALSE(started);  // canEdit 预检失败
    EXPECT_EQ(controller->revision(), 0);          // 状态未被破坏
    EXPECT_TRUE(controller->image().isNull());
    EXPECT_EQ(activeSpy.count(), 0);                // 未发射任何信号
    EXPECT_EQ(revisionSpy.count(), 0);
}

TEST_F(ImageEditControllerTest, BeginEdit_NonexistentPng_ReturnsFalse)
{
    // Arrange
    const QUrl source = QUrl::fromLocalFile(dir.filePath("missing.png"));

    // Act
    const bool started = controller->beginEdit(source);

    // Assert
    EXPECT_FALSE(started);  // canEdit 通过（后缀 png），但 read() 为 null
    EXPECT_EQ(controller->revision(), 0);
    EXPECT_FALSE(controller->active());
}

TEST_F(ImageEditControllerTest, BeginEdit_CorruptPngContent_ReturnsFalse)
{
    // Arrange
    const QString path = dir.filePath("bad.png");
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write("this is not a real image payload");
    file.close();

    // Act
    const bool started = controller->beginEdit(QUrl::fromLocalFile(path));

    // Assert
    EXPECT_FALSE(started);  // 后缀合法但内容损坏：read() null 分支
    EXPECT_TRUE(controller->image().isNull());
    EXPECT_EQ(controller->revision(), 0);
}

TEST_F(ImageEditControllerTest, BeginEdit_FrameIndexOutOfRange_ReturnsFalse)
{
    // Arrange
    const QUrl source = beginWithSolidPng(QColor(0, 0, 255));  // 单帧图

    // Act
    const bool started = controller->beginEdit(source, 3);

    // Assert
    EXPECT_FALSE(started);  // 单帧图 jumpToImage(3) 失败分支
    EXPECT_EQ(controller->revision(), 1);  // 前置 beginEdit 的版本号保留（强安全）
}

TEST_F(ImageEditControllerTest, BeginEdit_SameSourceAgain_ReturnsTrueWithoutReload)
{
    // Arrange
    const QUrl source = beginWithSolidPng(QColor(255, 0, 0));
    QSignalSpy activeSpy(controller, &ImageEditController::activeChanged);
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool started = controller->beginEdit(source);

    // Assert
    EXPECT_TRUE(started);                  // isEditing 短路：直接成功
    EXPECT_EQ(controller->revision(), 1);  // 未重新加载（版本号不递增）
    EXPECT_EQ(activeSpy.count(), 0);       // 不重复发信号
    EXPECT_EQ(historySpy.count(), 0);
    EXPECT_EQ(revisionSpy.count(), 0);
}

TEST_F(ImageEditControllerTest, CanUndo_Initially_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));  // historyIndex = 0（无可撤销项）

    // Act
    const bool canUndo = controller->canUndo();

    // Assert
    EXPECT_FALSE(canUndo);
    EXPECT_EQ(controller->revision(), 1);
}

TEST_F(ImageEditControllerTest, CanUndo_AfterCommit_ReturnsTrue)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    controller->commitHistory();  // historyIndex = 1

    // Act
    const bool canUndo = controller->canUndo();

    // Assert
    EXPECT_TRUE(canUndo);
    EXPECT_TRUE(controller->modified());
    EXPECT_EQ(controller->revision(), 1);  // commitHistory 不递增版本号
}

TEST_F(ImageEditControllerTest, CanUndo_BackToOldest_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    controller->commitHistory();
    ASSERT_TRUE(controller->undo());  // 回到 index 0

    // Act
    const bool canUndo = controller->canUndo();

    // Assert
    EXPECT_FALSE(canUndo);  // historyIndex = 0，不可再撤销
    EXPECT_TRUE(controller->canRedo());   // 前向快照仍在
    EXPECT_EQ(controller->revision(), 1);  // undo 相同快照未递增版本
}

TEST_F(ImageEditControllerTest, CanRedo_Initially_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));

    // Act
    const bool canRedo = controller->canRedo();

    // Assert
    EXPECT_FALSE(canRedo);  // 已在最新快照
    EXPECT_EQ(controller->revision(), 1);  // 仅 beginEdit 一次版本变更
    EXPECT_FALSE(controller->canUndo());
}

TEST_F(ImageEditControllerTest, CanRedo_AfterUndo_ReturnsTrue)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));              // A（8x8）
    controller->crop(QRectF(0, 0, 0.5, 1.0));          // B（4x8）
    controller->commitHistory();                       // history = [A, B]
    controller->crop(QRectF(0, 0, 1.0, 0.5));          // C（4x4）
    controller->commitHistory();                       // history = [A, B, C]，index = 2
    ASSERT_TRUE(controller->undo());                   // index = 1，图像回到 B（历史中部）

    // Act
    const bool canRedo = controller->canRedo();

    // Assert
    EXPECT_TRUE(canRedo);   // 撤销一次后存在前向快照 C
    EXPECT_TRUE(controller->canUndo());                // 中部：仍有更早快照 A 可撤销
    EXPECT_EQ(controller->image().size(), QSize(4, 8));  // undo 恢复的是 B
    EXPECT_EQ(controller->revision(), 4);              // begin+crop+crop+undo
}

TEST_F(ImageEditControllerTest, CanRedo_AfterRedoToLatest_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    controller->crop(QRectF(0, 0, 0.5, 1.0));
    controller->commitHistory();
    ASSERT_TRUE(controller->undo());
    ASSERT_TRUE(controller->redo());

    // Act
    const bool canRedo = controller->canRedo();

    // Assert
    EXPECT_FALSE(canRedo);  // 已回到最新
    EXPECT_EQ(controller->image().size(), QSize(4, 8));
}

TEST_F(ImageEditControllerTest, Revision_Initially_ReturnsZero)
{
    // Arrange
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    const int revision = controller->revision();

    // Assert
    EXPECT_EQ(revision, 0);
    EXPECT_EQ(revisionSpy.count(), 0);
}

TEST_F(ImageEditControllerTest, Revision_AfterBeginEditAndCrop_EqualsTwo)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));  // revision 1

    // Act
    controller->crop(QRectF(0, 0, 0.5, 1.0));  // revision 2
    controller->commitHistory();               // 不递增

    // Assert
    EXPECT_EQ(controller->revision(), 2);
    EXPECT_TRUE(controller->canUndo());
}

TEST_F(ImageEditControllerTest, Image_Initially_ReturnsNullImage)
{
    // Arrange
    QSignalSpy activeSpy(controller, &ImageEditController::activeChanged);

    // Act
    const QImage image = controller->image();

    // Assert
    EXPECT_TRUE(image.isNull());
    EXPECT_EQ(image.size(), QSize(0, 0));
    EXPECT_EQ(activeSpy.count(), 0);
}

TEST_F(ImageEditControllerTest, Image_AfterBeginEdit_ReturnsConvertedImage)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));

    // Act
    const QImage image = controller->image();

    // Assert
    EXPECT_EQ(image.size(), QSize(8, 8));
    EXPECT_EQ(image.format(), QImage::Format_ARGB32_Premultiplied);
    EXPECT_EQ(image.pixelColor(3, 3), QColor(255, 0, 0));
}

TEST_F(ImageEditControllerTest, IsEditing_ActiveSourceAndFrame_ReturnsTrue)
{
    // Arrange
    const QUrl source = beginWithSolidPng(QColor(255, 0, 0));

    // Act
    const bool editing = controller->isEditing(source, 0);

    // Assert
    EXPECT_TRUE(editing);
    EXPECT_TRUE(controller->active());
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
}

TEST_F(ImageEditControllerTest, IsEditing_DifferentSource_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    const QUrl other = QUrl::fromLocalFile(dir.filePath("other.png"));

    // Act
    const bool editing = controller->isEditing(other, 0);

    // Assert
    EXPECT_FALSE(editing);
    EXPECT_NE(other, controller->defaultSaveUrl());  // 源不同
}

TEST_F(ImageEditControllerTest, IsEditing_DifferentFrameIndex_ReturnsFalse)
{
    // Arrange
    const QUrl source = beginWithSolidPng(QColor(255, 0, 0));  // m_frameIndex = 0

    // Act
    const bool editing = controller->isEditing(source, 1);

    // Assert
    EXPECT_FALSE(editing);  // 帧号不匹配
    EXPECT_TRUE(controller->isEditing(source, 0));
    EXPECT_EQ(controller->revision(), 1);
}

TEST_F(ImageEditControllerTest, DefaultSaveUrl_PngSource_ReturnsEditedCopyName)
{
    // Arrange
    const QString path = dir.filePath("photo.png");
    makeSolidImage(4, 4, QColor(255, 0, 0)).save(path, "PNG");
    controller->beginEdit(QUrl::fromLocalFile(path));

    // Act
    const QUrl saveUrl = controller->defaultSaveUrl();

    // Assert
    EXPECT_EQ(saveUrl.toLocalFile(), dir.filePath(QStringLiteral("photo-编辑.png")));
    EXPECT_TRUE(saveUrl.isLocalFile());
}

TEST_F(ImageEditControllerTest, DefaultSaveUrl_JpegSource_PreservesSuffix)
{
    // Arrange
    const QString path = dir.filePath("pic.jpg");
    makeSolidImage(4, 4, QColor(0, 255, 0)).save(path, "JPG");
    ASSERT_TRUE(controller->beginEdit(QUrl::fromLocalFile(path)));

    // Act
    const QUrl saveUrl = controller->defaultSaveUrl();

    // Assert
    EXPECT_EQ(saveUrl.toLocalFile(), dir.filePath(QStringLiteral("pic-编辑.jpg")));
    EXPECT_TRUE(saveUrl.toLocalFile().endsWith(QStringLiteral(".jpg")));
}

TEST_F(ImageEditControllerTest, IsOriginalUrl_SameExistingPath_ReturnsTrue)
{
    // Arrange
    const QUrl source = beginWithSolidPng(QColor(255, 0, 0));

    // Act
    const bool original = controller->isOriginalUrl(source);

    // Assert
    EXPECT_TRUE(original);  // canonicalFilePath 相等
    EXPECT_TRUE(QFile::exists(source.toLocalFile()));
    EXPECT_EQ(QFileInfo(controller->defaultSaveUrl().toLocalFile()).absolutePath(),
              QFileInfo(source.toLocalFile()).absolutePath());  // 同一文件（目录一致）
}

TEST_F(ImageEditControllerTest, IsOriginalUrl_DifferentPath_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    const QString otherPath = dir.filePath("copy.png");
    makeSolidImage(4, 4, QColor(255, 0, 0)).save(otherPath, "PNG");

    // Act
    const bool original = controller->isOriginalUrl(QUrl::fromLocalFile(otherPath));

    // Assert
    EXPECT_FALSE(original);  // 两个不同存在的文件
    EXPECT_TRUE(QFile::exists(otherPath));
    EXPECT_EQ(controller->revision(), 1);  // 查询不改变编辑状态
}

TEST_F(ImageEditControllerTest, IsOriginalUrl_NonexistentDestination_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    const QUrl missing = QUrl::fromLocalFile(dir.filePath("never-created.png"));

    // Act
    const bool original = controller->isOriginalUrl(missing);

    // Assert
    EXPECT_FALSE(original);  // absoluteFilePath 回退侧：路径与源 canonical 不等
    EXPECT_FALSE(QFile::exists(missing.toLocalFile()));
    EXPECT_EQ(controller->revision(), 1);
}

TEST_F(ImageEditControllerTest, Discard_WithActiveImage_ClearsAndEmitsAll)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    QSignalSpy activeSpy(controller, &ImageEditController::activeChanged);
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    controller->discard();

    // Assert
    EXPECT_FALSE(controller->active());
    EXPECT_TRUE(controller->image().isNull());
    EXPECT_EQ(controller->revision(), 2);  // beginEdit(1) + discard(1)
    EXPECT_EQ(activeSpy.count(), 1);
    EXPECT_EQ(historySpy.count(), 1);
    EXPECT_EQ(revisionSpy.count(), 1);
}

TEST_F(ImageEditControllerTest, Discard_WithoutImage_EmitsNothing)
{
    // Arrange
    QSignalSpy activeSpy(controller, &ImageEditController::activeChanged);
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    controller->discard();

    // Assert
    EXPECT_EQ(controller->revision(), 0);  // 无图早退：版本号不变
    EXPECT_EQ(activeSpy.count(), 0);       // 不发射任何信号
    EXPECT_EQ(historySpy.count(), 0);
    EXPECT_EQ(revisionSpy.count(), 0);
}

TEST_F(ImageEditControllerTest, Discard_ThenBeginDifferentSource_Succeeds)
{
    // Arrange
    const QUrl first = beginWithSolidPng(QColor(255, 0, 0), QStringLiteral("a.png"));
    controller->discard();
    const QString secondPath = dir.filePath("b.png");
    makeSolidImage(4, 4, QColor(0, 0, 255)).save(secondPath, "PNG");
    const QUrl second = QUrl::fromLocalFile(secondPath);

    // Act
    const bool started = controller->beginEdit(second);

    // Assert
    EXPECT_TRUE(started);
    EXPECT_EQ(controller->revision(), 3);  // begin + discard + begin
    EXPECT_FALSE(controller->isEditing(first, 0));
    EXPECT_TRUE(controller->isEditing(second, 0));
}

TEST_F(ImageEditControllerTest, CommitHistory_WithoutImage_EmitsNothing)
{
    // Arrange
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);

    // Act
    controller->commitHistory();

    // Assert
    EXPECT_EQ(historySpy.count(), 0);   // 无图早退分支
    EXPECT_FALSE(controller->canUndo());
    EXPECT_EQ(controller->revision(), 0);
}

TEST_F(ImageEditControllerTest, CommitHistory_AppendsSnapshot_AndEmitsHistoryChanged)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);

    // Act
    controller->commitHistory();

    // Assert
    EXPECT_EQ(historySpy.count(), 1);
    EXPECT_TRUE(controller->canUndo());     // 快照入历史
    EXPECT_FALSE(controller->canRedo());    // 已在最新
    EXPECT_TRUE(controller->modified());    // index(1) != saved(0)
}

TEST_F(ImageEditControllerTest, CommitHistory_TruncatesRedoBranch_OnNewCommit)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));              // A
    controller->crop(QRectF(0, 0, 0.5, 1.0));          // B（4x8）
    controller->commitHistory();                       // history = [A, B]
    ASSERT_TRUE(controller->undo());                   // 回到 A，B 进入 redo 分支
    controller->crop(QRectF(0.5, 0, 0.5, 1.0));        // C（4x8）

    // Act
    controller->commitHistory();                       // 截断 B，history = [A, C]

    // Assert
    EXPECT_FALSE(controller->canRedo());  // B 已被丢弃
    EXPECT_TRUE(controller->canUndo());
    EXPECT_EQ(controller->image().size(), QSize(4, 8));  // 当前为 C
}

TEST_F(ImageEditControllerTest, CommitHistory_ManyCommits_HistoryCappedAtFifty)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    for (int i = 0; i < 52; ++i)
        controller->commitHistory();  // 历史上限 50（while removeFirst 边界）

    // Act
    int undoCount = 0;
    while (controller->undo())
        ++undoCount;

    // Assert
    EXPECT_EQ(undoCount, 49);            // 50 条历史：index 49 → 0 共 49 次 undo
    EXPECT_FALSE(controller->canUndo()); // 最早一条不可再撤销
    EXPECT_TRUE(controller->canRedo());
}

TEST_F(ImageEditControllerTest, MarkSaved_AfterCommit_ClearsModifiedFlag)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    controller->crop(QRectF(0, 0, 0.5, 1.0));
    controller->commitHistory();
    ASSERT_TRUE(controller->modified());

    // Act
    controller->markSaved();

    // Assert
    EXPECT_FALSE(controller->modified());  // savedHistoryIndex 对齐
    EXPECT_TRUE(controller->canUndo());    // 历史状态不受影响
    EXPECT_FALSE(controller->canRedo());   // 仍在最新快照
    EXPECT_EQ(controller->image().size(), QSize(4, 8));  // 裁剪结果保留
}

TEST_F(ImageEditControllerTest, MarkSaved_AfterEdit_EmitsHistoryChangedOnce)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);
    const int revisionBefore = controller->revision();

    // Act
    controller->markSaved();

    // Assert
    EXPECT_EQ(historySpy.count(), 1);               // 恰好一次
    EXPECT_EQ(revisionSpy.count(), 0);              // 不动版本号
    EXPECT_EQ(controller->revision(), revisionBefore);
}

TEST_F(ImageEditControllerTest, MarkSaved_InUneditedState_KeepsModifiedFalse)
{
    // Arrange
    // 初始态 historyIndex=-1 / savedHistoryIndex=0 → modified()==true（源码现状）
    ASSERT_TRUE(controller->modified());

    // Act
    controller->markSaved();  // saved := -1

    // Assert
    EXPECT_FALSE(controller->modified());  // -1 == -1
    EXPECT_FALSE(controller->canUndo());
    EXPECT_FALSE(controller->active());
    EXPECT_EQ(controller->revision(), 0);
}

TEST_F(ImageEditControllerTest, Modified_AfterBeginEdit_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));  // index=0, saved=0

    // Act
    const bool modified = controller->modified();

    // Assert
    EXPECT_FALSE(modified);
    EXPECT_EQ(controller->revision(), 1);
}

TEST_F(ImageEditControllerTest, Modified_AfterCommit_ReturnsTrue)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));

    // Act
    controller->commitHistory();  // index=1, saved=0

    // Assert
    EXPECT_TRUE(controller->modified());
    EXPECT_TRUE(controller->canUndo());
    EXPECT_EQ(controller->revision(), 1);  // commitHistory 不递增版本
}

TEST_F(ImageEditControllerTest, Modified_AfterMarkSaved_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    controller->commitHistory();
    ASSERT_TRUE(controller->modified());

    // Act
    controller->markSaved();

    // Assert
    EXPECT_FALSE(controller->modified());
    EXPECT_TRUE(controller->canUndo());  // 历史仍在，仅保存点前移
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
}

TEST_F(ImageEditControllerTest, Undo_NothingToUndo_ReturnsFalse)
{
    // Arrange
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);

    // Act
    const bool undone = controller->undo();

    // Assert
    EXPECT_FALSE(undone);  // 无历史（index=-1 <= 0）
    EXPECT_EQ(historySpy.count(), 0);
    EXPECT_EQ(controller->revision(), 0);
}

TEST_F(ImageEditControllerTest, Undo_AfterCropCommit_RestoresOriginalImage)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));       // 8x8
    controller->crop(QRectF(0, 0, 0.5, 1.0));   // 4x8
    controller->commitHistory();
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);

    // Act
    const bool undone = controller->undo();

    // Assert
    EXPECT_TRUE(undone);
    EXPECT_EQ(controller->image().size(), QSize(8, 8));  // 恢复裁剪前
    EXPECT_EQ(controller->revision(), 3);                // begin+crop+undo
    EXPECT_EQ(revisionSpy.count(), 1);
    EXPECT_EQ(historySpy.count(), 1);
}

TEST_F(ImageEditControllerTest, Undo_IdenticalSnapshot_DoesNotBumpRevision)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    controller->commitHistory();  // 未做修改：快照与当前 cacheKey 相同
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);

    // Act
    const bool undone = controller->undo();

    // Assert
    EXPECT_TRUE(undone);                     // 指针回退成功
    EXPECT_EQ(controller->revision(), 1);    // 图像未变化：不递增
    EXPECT_EQ(revisionSpy.count(), 0);       // 不发 revisionChanged
    EXPECT_EQ(historySpy.count(), 1);        // 但历史指针变化仍发信号
}

TEST_F(ImageEditControllerTest, Undo_AtOldestEntry_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));  // index=0
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);

    // Act
    const bool undone = controller->undo();

    // Assert
    EXPECT_FALSE(undone);  // historyIndex <= 0 分支
    EXPECT_EQ(historySpy.count(), 0);
    EXPECT_TRUE(controller->active());  // 图像不受影响
}

TEST_F(ImageEditControllerTest, Redo_NoHistory_ReturnsFalse)
{
    // Arrange
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);

    // Act
    const bool redone = controller->redo();

    // Assert
    EXPECT_FALSE(redone);  // index=-1 分支
    EXPECT_EQ(historySpy.count(), 0);
    EXPECT_EQ(controller->revision(), 0);
}

TEST_F(ImageEditControllerTest, Redo_AfterUndo_RestoresCroppedImage)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    controller->crop(QRectF(0, 0, 0.5, 1.0));   // 4x8
    controller->commitHistory();
    ASSERT_TRUE(controller->undo());            // 回 8x8
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool redone = controller->redo();

    // Assert
    EXPECT_TRUE(redone);
    EXPECT_EQ(controller->image().size(), QSize(4, 8));  // 恢复裁剪后
    EXPECT_EQ(controller->revision(), 4);                // begin+crop+undo+redo
    EXPECT_EQ(revisionSpy.count(), 1);
    EXPECT_FALSE(controller->canRedo());                 // 已在最新
}

TEST_F(ImageEditControllerTest, Redo_AtLatestEntry_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    controller->crop(QRectF(0, 0, 0.5, 1.0));
    controller->commitHistory();
    QSignalSpy historySpy(controller, &ImageEditController::historyChanged);

    // Act
    const bool redone = controller->redo();

    // Assert
    EXPECT_FALSE(redone);  // index+1 >= size 分支
    EXPECT_EQ(historySpy.count(), 0);
    EXPECT_EQ(controller->image().size(), QSize(4, 8));  // 状态未破坏
}

TEST_F(ImageEditControllerTest, Crop_HalfNormalizedRect_ReturnsTrueAndResizes)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool cropped = controller->crop(QRectF(0, 0, 0.5, 1.0));

    // Assert
    EXPECT_TRUE(cropped);
    EXPECT_EQ(controller->image().size(), QSize(4, 8));  // pixelRect: ceil(0.5*8)=4
    EXPECT_EQ(controller->revision(), 2);
    EXPECT_EQ(revisionSpy.count(), 1);
}

TEST_F(ImageEditControllerTest, Crop_FullImageRect_ReturnsFalseWithoutRevisionBump)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool cropped = controller->crop(QRectF(0, 0, 1.0, 1.0));  // == m_image.rect()

    // Assert
    EXPECT_FALSE(cropped);                 // 全图裁剪被拒绝
    EXPECT_EQ(controller->revision(), 1);  // 版本号不变（强安全）
    EXPECT_EQ(revisionSpy.count(), 0);
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
}

TEST_F(ImageEditControllerTest, Crop_TooSmallRect_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));

    // Act
    const bool cropped = controller->crop(QRectF(0, 0, 0.1, 0.1));  // 1x1 < 2x2

    // Assert
    EXPECT_FALSE(cropped);  // 宽/高 < 2 分支
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
    EXPECT_EQ(controller->revision(), 1);
}

TEST_F(ImageEditControllerTest, Crop_WithoutImage_ReturnsFalse)
{
    // Arrange
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool cropped = controller->crop(QRectF(0, 0, 0.5, 0.5));

    // Assert
    EXPECT_FALSE(cropped);  // isNull 早退分支
    EXPECT_TRUE(controller->image().isNull());
    EXPECT_EQ(controller->revision(), 0);
    EXPECT_EQ(revisionSpy.count(), 0);
}

TEST_F(ImageEditControllerTest, PixelRect_HalfWidthNormalizedRect_MapsToHalfPixelWidth)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));  // 8x8

    // Act
    const bool cropped = controller->crop(QRectF(0.25, 0.0, 0.5, 1.0));  // floor(2) .. ceil(6)

    // Assert
    EXPECT_TRUE(cropped);
    EXPECT_EQ(controller->image().size(), QSize(4, 8));  // 6-2=4，验证 floor/ceil 映射
}

TEST_F(ImageEditControllerTest, PixelRect_OutOfRangeRect_ClampsToImageBounds)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));  // 8x8

    // Act
    const bool cropped = controller->crop(QRectF(0.25, 0.25, 1.25, 1.25));  // ∩ [0,1]

    // Assert
    EXPECT_TRUE(cropped);
    EXPECT_EQ(controller->image().size(), QSize(6, 6));  // (0.25,0.25)-(1,1) → 2..8
}

TEST_F(ImageEditControllerTest, PixelRect_TinyNormalizedRect_RejectedByEffect)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool applied = controller->applyEffect(QStringLiteral("mosaic"),
                                                 QRectF(0, 0, 0.1, 0.1), 8);  // 1x1 区域

    // Assert
    EXPECT_FALSE(applied);                 // < 2x2 分支
    EXPECT_EQ(controller->revision(), 1);  // 状态保持
    EXPECT_EQ(revisionSpy.count(), 0);
}

TEST_F(ImageEditControllerTest, ApplyEffect_GaussianOnSolidImage_PreservesPixels)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool applied = controller->applyEffect(QStringLiteral("gaussian"),
                                                 QRectF(0, 0, 1.0, 1.0), 5);

    // Assert
    EXPECT_TRUE(applied);  // 纯色图高斯模糊后逐像素不变
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
    EXPECT_EQ(controller->image().pixelColor(0, 0), QColor(255, 0, 0));
    EXPECT_EQ(controller->image().pixelColor(7, 7), QColor(255, 0, 0));
    EXPECT_EQ(controller->image().pixelColor(4, 4), QColor(255, 0, 0));
    EXPECT_EQ(controller->revision(), 2);
    EXPECT_EQ(revisionSpy.count(), 1);
}

TEST_F(ImageEditControllerTest, ApplyEffect_UnknownEffectName_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool applied = controller->applyEffect(QStringLiteral("sharpen"),
                                                 QRectF(0, 0, 1.0, 1.0), 5);

    // Assert
    EXPECT_FALSE(applied);                 // 未知 effect：else 分支
    EXPECT_EQ(controller->revision(), 1);  // 强安全：版本号/图像不变
    EXPECT_EQ(revisionSpy.count(), 0);
}

TEST_F(ImageEditControllerTest, ApplyEffect_WithoutImage_ReturnsFalse)
{
    // Arrange
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool applied = controller->applyEffect(QStringLiteral("gaussian"),
                                                 QRectF(0, 0, 1.0, 1.0), 5);

    // Assert
    EXPECT_FALSE(applied);  // isNull 早退
    EXPECT_EQ(controller->revision(), 0);
    EXPECT_TRUE(controller->image().isNull());
    EXPECT_EQ(revisionSpy.count(), 0);
}

TEST_F(ImageEditControllerTest, ApplyEffect_TooSmallRegion_ReturnsFalseWithoutRevisionBump)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));

    // Act
    const bool applied = controller->applyEffect(QStringLiteral("graffiti"),
                                                 QRectF(0.9, 0.9, 0.1, 0.1), 8);

    // Assert
    EXPECT_FALSE(applied);                 // 区域 1x1 < 2x2
    EXPECT_EQ(controller->revision(), 1);
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
}

// 同质多组输入：各 effect 的非法强度档位
class ApplyEffectStrengthParamTest : public ImageEditControllerTest,
                                     public ::testing::WithParamInterface<EffectStrengthCase> {
};

TEST_P(ApplyEffectStrengthParamTest, ApplyEffect_InvalidStrength_ReturnsFalse)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    const EffectStrengthCase param = GetParam();
    QSignalSpy revisionSpy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool applied = controller->applyEffect(param.effect, QRectF(0, 0, 1.0, 1.0),
                                                 param.strength);

    // Assert
    EXPECT_FALSE(applied);                 // 强度 ∉ 该 effect 允许档位
    EXPECT_EQ(controller->revision(), 1);  // 强安全
    EXPECT_EQ(revisionSpy.count(), 0);
}

INSTANTIATE_TEST_SUITE_P(InvalidStrengths, ApplyEffectStrengthParamTest,
                         ::testing::Values(
                             EffectStrengthCase{ QStringLiteral("gaussian"), 7 },
                             EffectStrengthCase{ QStringLiteral("mosaic"), 5 },
                             EffectStrengthCase{ QStringLiteral("graffiti"), 99 }));

TEST_F(ImageEditControllerTest, ApplyBoxBlur_ViaGaussianEffect_SolidImageStaysUniform)
{
    // Arrange
    beginWithSolidPng(QColor(0, 128, 0));

    // Act
    const bool applied = controller->applyEffect(QStringLiteral("gaussian"),
                                                 QRectF(0, 0, 1.0, 1.0), 15);

    // Assert
    EXPECT_TRUE(applied);  // applyGaussianBlur（原 applyBoxBlur）：纯色模糊不变
    EXPECT_EQ(controller->image().pixelColor(0, 0), QColor(0, 128, 0));
    EXPECT_EQ(controller->image().pixelColor(7, 7), QColor(0, 128, 0));
    EXPECT_EQ(controller->revision(), 2);
}

TEST_F(ImageEditControllerTest, ApplyBoxBlur_EdgeClampedRegion_CompletesWithoutArtifacts)
{
    // Arrange
    beginWithSolidPng(QColor(255, 255, 0));  // 8x8 纯色

    // Act
    const bool applied = controller->applyEffect(QStringLiteral("gaussian"),
                                                 QRectF(0.75, 0.75, 0.25, 0.25), 5);  // 右下 2x2 贴边

    // Assert
    EXPECT_TRUE(applied);  // 采样矩形越界被 intersected 钳制（BORDER_REPLICATE）
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
    EXPECT_EQ(controller->image().pixelColor(0, 0), QColor(255, 255, 0));  // 区域外不变
    EXPECT_EQ(controller->image().pixelColor(7, 7), QColor(255, 255, 0));  // 区域内纯色仍不变
}

TEST_F(ImageEditControllerTest, ApplyMosaic_MixedColorsRegion_AveragesToMeanColor)
{
    // Arrange
    const QString path = dir.filePath("twocolor.png");
    makeTwoColorImage(8, 8, QColor(255, 0, 0), QColor(0, 0, 255)).save(path, "PNG");
    ASSERT_TRUE(controller->beginEdit(QUrl::fromLocalFile(path)));  // 左半红/右半蓝

    // Act
    const bool applied = controller->applyEffect(QStringLiteral("mosaic"),
                                                 QRectF(0, 0, 1.0, 1.0), 8);  // 单块覆盖 8x8

    // Assert
    EXPECT_TRUE(applied);  // blockSize=8 → 整图一块：均值 (255*32/64, 0, 255*32/64) = (127,0,127)
    EXPECT_EQ(controller->image().pixelColor(0, 0), QColor(127, 0, 127));
    EXPECT_EQ(controller->image().pixelColor(7, 7), QColor(127, 0, 127));
    EXPECT_EQ(controller->image().pixelColor(4, 4), QColor(127, 0, 127));
}

TEST_F(ImageEditControllerTest, ApplyMosaic_SolidRegion_PixelsUnchanged)
{
    // Arrange
    beginWithSolidPng(QColor(10, 20, 30));

    // Act
    const bool applied = controller->applyEffect(QStringLiteral("mosaic"),
                                                 QRectF(0, 0, 1.0, 1.0), 32);

    // Assert
    EXPECT_TRUE(applied);  // 纯色块均值等于自身
    EXPECT_EQ(controller->image().pixelColor(3, 3), QColor(10, 20, 30));
    EXPECT_EQ(controller->revision(), 2);
}

TEST_F(ImageEditControllerTest, ApplyGraffiti_ViaEffect_PreservesSolidPixels)
{
    // Arrange
    beginWithSolidPng(QColor(200, 30, 40));

    // Act
    const bool applied = controller->applyEffect(QStringLiteral("graffiti"),
                                                 QRectF(0, 0, 1.0, 1.0), 8);

    // Assert
    EXPECT_TRUE(applied);  // 印章混合 reservoir=纯色 → 目标与源同色，逐像素不变
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
    EXPECT_EQ(controller->image().pixelColor(0, 0), QColor(200, 30, 40));
    EXPECT_EQ(controller->image().pixelColor(7, 7), QColor(200, 30, 40));
    EXPECT_EQ(controller->revision(), 2);
}

// 同质多组输入：graffiti 三档合法强度
class ApplyGraffitiStrengthParamTest : public ImageEditControllerTest,
                                       public ::testing::WithParamInterface<int> {
};

TEST_P(ApplyGraffitiStrengthParamTest, ApplyGraffiti_AllStrengths_ReturnsTrue)
{
    // Arrange
    beginWithSolidPng(QColor(0, 100, 200));
    const int strength = GetParam();

    // Act
    const bool applied = controller->applyEffect(QStringLiteral("graffiti"),
                                                 QRectF(0, 0, 1.0, 1.0), strength);

    // Assert
    EXPECT_TRUE(applied);  // 8/16/32 → 直径 96/174/256 三档均可盖章
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
    EXPECT_EQ(controller->revision(), 2);
}

INSTANTIATE_TEST_SUITE_P(AllStrengths, ApplyGraffitiStrengthParamTest,
                         ::testing::Values(8, 16, 32));

TEST_F(ImageEditControllerTest, SaveComposite_MatchingPngNoAnnotations_WritesFile)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    const QString destPath = dir.filePath("out.png");
    QSignalSpy failedSpy(controller, &ImageEditController::saveFailed);

    // Act
    const bool saved = controller->saveComposite(QUrl::fromLocalFile(destPath), QVariantList());

    // Assert
    EXPECT_TRUE(saved);
    EXPECT_EQ(failedSpy.count(), 0);
    EXPECT_TRUE(QFile::exists(destPath));
    QImage reloaded(destPath);
    EXPECT_EQ(reloaded.size(), QSize(8, 8));
    EXPECT_EQ(reloaded.pixelColor(4, 4), QColor(255, 0, 0));
    EXPECT_EQ(controller->revision(), 1);  // 保存不改变编辑状态
}

TEST_F(ImageEditControllerTest, SaveComposite_FormatMismatch_ReturnsFalseAndKeepsState)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));  // 源格式 png
    const QString destPath = dir.filePath("out.jpg");
    QSignalSpy failedSpy(controller, &ImageEditController::saveFailed);

    // Act
    const bool saved = controller->saveComposite(QUrl::fromLocalFile(destPath), QVariantList());

    // Assert
    EXPECT_FALSE(saved);                              // png → jpg 格式不匹配
    EXPECT_EQ(failedSpy.count(), 1);                  // saveFailed 恰一次
    EXPECT_FALSE(failedSpy.at(0).at(0).toString().isEmpty());
    EXPECT_FALSE(QFile::exists(destPath));            // 未产生文件
    EXPECT_EQ(controller->revision(), 1);             // 强安全：状态未破坏
    EXPECT_EQ(controller->image().size(), QSize(8, 8));
}

TEST_F(ImageEditControllerTest, SaveComposite_EmptyDestinationUrl_ReturnsFalseAndEmits)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    QSignalSpy failedSpy(controller, &ImageEditController::saveFailed);

    // Act
    const bool saved = controller->saveComposite(QUrl(), QVariantList());

    // Assert
    EXPECT_FALSE(saved);               // targetPath 为空分支
    EXPECT_EQ(failedSpy.count(), 1);
    EXPECT_EQ(controller->revision(), 1);
}

TEST_F(ImageEditControllerTest, SaveComposite_WithoutActiveImage_ReturnsFalseSilently)
{
    // Arrange
    const QString destPath = dir.filePath("out.png");
    QSignalSpy failedSpy(controller, &ImageEditController::saveFailed);

    // Act
    const bool saved = controller->saveComposite(QUrl::fromLocalFile(destPath), QVariantList());

    // Assert
    EXPECT_FALSE(saved);           // isNull 早退
    EXPECT_EQ(failedSpy.count(), 0);  // 该分支不发 saveFailed
    EXPECT_FALSE(QFile::exists(destPath));
}

TEST_F(ImageEditControllerTest, SaveComposite_UnwritableDirectory_ReturnsFalseAndEmits)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    const QString destPath = dir.filePath(QStringLiteral("no-such-dir/out.png"));
    QSignalSpy failedSpy(controller, &ImageEditController::saveFailed);

    // Act
    const bool saved = controller->saveComposite(QUrl::fromLocalFile(destPath), QVariantList());

    // Assert
    EXPECT_FALSE(saved);           // QSaveFile::open 失败（父目录不存在）
    EXPECT_EQ(failedSpy.count(), 1);
    EXPECT_FALSE(QFile::exists(destPath));
    EXPECT_EQ(controller->revision(), 1);
}

TEST_F(ImageEditControllerTest, SaveComposite_JpgSourceAndJpegDest_SynonymsAccepted)
{
    // Arrange
    const QString sourcePath = dir.filePath("cam.jpg");
    makeSolidImage(8, 8, QColor(90, 90, 90)).save(sourcePath, "JPG");
    ASSERT_TRUE(controller->beginEdit(QUrl::fromLocalFile(sourcePath)));
    const QString destPath = dir.filePath("edited.jpeg");  // jpg/jpeg 归一后均为 jpeg
    QSignalSpy failedSpy(controller, &ImageEditController::saveFailed);

    // Act
    const bool saved = controller->saveComposite(QUrl::fromLocalFile(destPath), QVariantList());

    // Assert
    EXPECT_TRUE(saved);  // normalizeFormat("jpg") == normalizeFormat("jpeg")
    EXPECT_EQ(failedSpy.count(), 0);
    EXPECT_TRUE(QFile::exists(destPath));
    QImage reloaded(destPath);
    EXPECT_EQ(reloaded.size(), QSize(8, 8));
}

TEST_F(ImageEditControllerTest, SaveComposite_AnnotationsWithFewerThanTwoPoints_Skipped)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    const QString destPath = dir.filePath("skip.png");
    QVariantMap annotation;
    annotation.insert(QStringLiteral("type"), QStringLiteral("rect"));
    annotation.insert(QStringLiteral("color"), QColor(0, 255, 0).name());
    annotation.insert(QStringLiteral("width"), 0.1);
    annotation.insert(QStringLiteral("points"),
                      QVariantList { QPointF(0.5, 0.5) });  // 仅 1 个点 → 跳过

    // Act
    const bool saved = controller->saveComposite(QUrl::fromLocalFile(destPath),
                                                 QVariantList { annotation });

    // Assert
    EXPECT_TRUE(saved);                        // 无效注解被 continue 跳过，保存仍成功
    EXPECT_TRUE(QFile::exists(destPath));
    QImage reloaded(destPath);
    EXPECT_EQ(reloaded.pixelColor(4, 4), QColor(255, 0, 0));  // 画面未被涂改
    EXPECT_EQ(reloaded.size(), QSize(8, 8));
}

TEST_F(ImageEditControllerTest, SaveComposite_RectPenNumberAnnotations_AllSucceed)
{
    // Arrange
    beginWithSolidPng(QColor(255, 255, 255));  // 白底
    const QString destPath = dir.filePath("annotated.png");
    const auto mkPoints = [](const QList<QPointF> &points) {
        QVariantList list;
        for (const QPointF &point : points)
            list.append(point);
        return list;
    };
    QVariantMap pen;
    pen.insert(QStringLiteral("type"), QStringLiteral("pen"));
    pen.insert(QStringLiteral("color"), QColor(255, 0, 0).name());
    pen.insert(QStringLiteral("width"), 0.5);
    pen.insert(QStringLiteral("points"),
               mkPoints({ QPointF(0, 0), QPointF(1, 1) }));  // 对角粗笔迹
    QVariantMap number;
    number.insert(QStringLiteral("type"), QStringLiteral("number"));
    number.insert(QStringLiteral("color"), QColor(0, 0, 0).name());
    number.insert(QStringLiteral("width"), 0.1);
    number.insert(QStringLiteral("number"), 7);
    number.insert(QStringLiteral("points"),
                  mkPoints({ QPointF(0.25, 0.25), QPointF(0.75, 0.75) }));
    QVariantMap arrow;
    arrow.insert(QStringLiteral("type"), QStringLiteral("arrow"));
    arrow.insert(QStringLiteral("color"), QColor(0, 0, 255).name());
    arrow.insert(QStringLiteral("width"), 0.1);
    arrow.insert(QStringLiteral("points"),
                 mkPoints({ QPointF(0, 1), QPointF(1, 0) }));
    QVariantMap ellipse;
    ellipse.insert(QStringLiteral("type"), QStringLiteral("ellipse"));
    ellipse.insert(QStringLiteral("color"), QColor(0, 255, 0).name());
    ellipse.insert(QStringLiteral("width"), 0.1);
    ellipse.insert(QStringLiteral("points"),
                   mkPoints({ QPointF(0.4, 0.4), QPointF(0.6, 0.6) }));
    QVariantMap text;
    text.insert(QStringLiteral("type"), QStringLiteral("text"));
    text.insert(QStringLiteral("color"), QColor(0, 0, 0).name());
    text.insert(QStringLiteral("width"), 0.1);
    text.insert(QStringLiteral("text"), QStringLiteral("UT"));
    text.insert(QStringLiteral("points"),
                mkPoints({ QPointF(0.1, 0.1), QPointF(0.3, 0.3) }));
    QVariantMap unknown;
    unknown.insert(QStringLiteral("type"), QStringLiteral("marker"));  // 未知类型 → 折线
    unknown.insert(QStringLiteral("color"), QColor(255, 0, 255).name());
    unknown.insert(QStringLiteral("width"), 0.1);
    unknown.insert(QStringLiteral("points"),
                   mkPoints({ QPointF(0.8, 0.1), QPointF(0.9, 0.2) }));
    const QVariantList annotations { pen, number, arrow, ellipse, text, unknown };
    QSignalSpy failedSpy(controller, &ImageEditController::saveFailed);

    // Act
    const bool saved = controller->saveComposite(QUrl::fromLocalFile(destPath), annotations);

    // Assert
    EXPECT_TRUE(saved);  // rect/ellipse/text/number/pen/arrow/未知 各类型均不中断保存
    EXPECT_EQ(failedSpy.count(), 0);
    EXPECT_TRUE(QFile::exists(destPath));
    QImage reloaded(destPath);
    EXPECT_EQ(reloaded.size(), QSize(8, 8));
    EXPECT_NE(reloaded, controller->image().convertToFormat(QImage::Format_ARGB32));  // 画面被注解涂改
}

TEST_F(ImageEditControllerTest, EditedImageProvider_WithController_ReturnsControllerImage)
{
    // Arrange
    beginWithSolidPng(QColor(255, 0, 0));
    EditedImageProvider provider(controller);
    QSize reportedSize;

    // Act
    const QImage image = provider.requestImage(QStringLiteral("edited"), &reportedSize, QSize());

    // Assert
    EXPECT_FALSE(image.isNull());                    // 构造时持有 controller 引用
    EXPECT_EQ(image.size(), QSize(8, 8));
    EXPECT_EQ(reportedSize, QSize(8, 8));
}

TEST_F(ImageEditControllerTest, EditedImageProvider_NullController_ReturnsNullImage)
{
    // Arrange
    EditedImageProvider provider(nullptr);
    QSize reportedSize(123, 456);  // 哨兵值，验证出参被写为空尺寸

    // Act
    const QImage image = provider.requestImage(QStringLiteral("edited"), &reportedSize, QSize());

    // Assert
    EXPECT_TRUE(image.isNull());            // m_controller 为空 → 空图
    EXPECT_EQ(reportedSize, QSize(0, 0));   // 出参仍被写为 result.size()
}

TEST_F(ImageEditControllerTest, RequestImage_SetsSizeOutParam_ToImageSize)
{
    // Arrange
    beginWithSolidPng(QColor(0, 255, 0));
    EditedImageProvider provider(controller);
    QSize reportedSize;

    // Act
    const QImage image = provider.requestImage(QStringLiteral("edited"), &reportedSize, QSize());

    // Assert
    EXPECT_EQ(reportedSize, QSize(8, 8));   // requestedSize 无效 → 不缩放
    EXPECT_EQ(image.size(), QSize(8, 8));
}

TEST_F(ImageEditControllerTest, RequestImage_ValidRequestedSize_ScalesKeepAspectRatio)
{
    // Arrange
    beginWithSolidPng(QColor(0, 0, 255));  // 8x8 方图
    EditedImageProvider provider(controller);
    QSize reportedSize;

    // Act
    const QImage image = provider.requestImage(QStringLiteral("edited"), &reportedSize, QSize(4, 2));

    // Assert
    EXPECT_EQ(image.size(), QSize(2, 2));  // KeepAspectRatio：8x8 适配 4x2 → 2x2
    EXPECT_EQ(reportedSize, QSize(8, 8));  // 出参报告原始尺寸
}

TEST_F(ImageEditControllerTest, RequestImage_EmptyImageWithValidSize_ReturnsNullUnscaled)
{
    // Arrange
    EditedImageProvider provider(nullptr);  // 无控制器 → 空图
    QSize reportedSize;

    // Act
    const QImage image = provider.requestImage(QStringLiteral("edited"), &reportedSize, QSize(64, 64));

    // Assert
    EXPECT_TRUE(image.isNull());           // result.isNull → 不走缩放分支
    EXPECT_EQ(reportedSize, QSize(0, 0));
}

// ─── rotateClockwise（补测：inventory 缺失，按 lcov FNDA 补直接调用用例）───
// 分支（来源：imageeditcontroller.cpp:371-384）：
// B1: m_image.isNull() → return false
// B2: 非空 → QTransform 旋转 90° + ++m_revision + emit revisionChanged → return true
// 映射： RotateClockwise_ActiveImage_ReturnsTrueSwapsSizeAndBumpsRevision → B2
//        RotateClockwise_WithoutImage_ReturnsFalseWithoutSignals          → B1

TEST_F(ImageEditControllerTest, RotateClockwise_ActiveImage_ReturnsTrueSwapsSizeAndBumpsRevision)
{
    // Arrange：12x6 非方图进入编辑会话；记录版本与 revisionChanged 信号
    const QString path = dir.filePath(QStringLiteral("wide.png"));
    makeSolidImage(12, 6, QColor(20, 160, 240)).save(path, "PNG");
    ASSERT_TRUE(controller->beginEdit(QUrl::fromLocalFile(path)));
    const int revisionBefore = controller->revision();
    QSignalSpy spy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool ret = controller->rotateClockwise();

    // Assert：非空图旋转 90 度成功——宽高互换、版本 +1、发一次 revisionChanged
    EXPECT_TRUE(ret);  // branch: m_image 非空
    EXPECT_EQ(controller->image().size(), QSize(6, 12));
    EXPECT_EQ(controller->revision(), revisionBefore + 1);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ImageEditControllerTest, RotateClockwise_WithoutImage_ReturnsFalseWithoutSignals)
{
    // Arrange：未 beginEdit，控制器无图
    ASSERT_TRUE(controller->image().isNull());
    QSignalSpy spy(controller, &ImageEditController::revisionChanged);

    // Act
    const bool ret = controller->rotateClockwise();

    // Assert：m_image.isNull() 早退——返回 false、版本不变、无信号（强异常安全）
    EXPECT_FALSE(ret);  // branch: m_image.isNull()
    EXPECT_EQ(controller->revision(), 0);
    EXPECT_EQ(spy.count(), 0);
}
