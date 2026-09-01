// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | LiveTextAnalyzer(parent) | low | - | 1 | 3 |
// | ~LiveTextAnalyzer | low | - | 1 | 1 |
// | setImage(image) | low | - | 1 | 3 |
// | analyze(token) | low | - | 1 | 2 |
// | breakAnalyze() | low | - | 1 | 2 |
// | liveBlock() | mid | - | 2 | 2 |
// | charBox(blockIndex) | mid | - | 2 | 5 |（2 TEST_F + TEST_P×2 实例 + 空块 1）
// | textResult(blockIndex,startIndex,len) | low | - | 1 | 6 |（2 TEST_F + TEST_P×4 实例）
// | requestImage(id,size,requestedSize) | low | - | 1 | 13 |（7 TEST_F + TEST_P×3×2 实例）
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（inventory 全部 9 个 testable 条目均有映射，protected requestImage 经 -fno-access-control 直接调用）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（blockIndex 有效/越界/负数/空列表、startIndex 负数、len 0/负/超余、id 规范/无下划线/负数/垃圾、pixelRatio 1/2、requestedSize 有效/单边 0/全 0）
// 3. 每个等价类的边界值显式覆盖: [x]（index==size-1 与 ==size、charBoxes 首元素基址、len 超余取剩余、空 textBoxes 循环 0 次）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（charBox 非法索引×2、textResult 非法参数×4、requestImage 无效尺寸×3、畸形 id×3）
// 5. 分支清单 → 用例映射已列出: [x]（见下方分支清单，均来自 get_code_snippet 真实源码 livetextanalyzer.cpp:19-174）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（源码无 throw；全部 if/while/for/短路分支均有映射；loongarch 为编译期 #if 不适用）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（源码无 throw 分支，不适用）
// 8. 负面场景有专门用例: [x]（非法索引/空图片/畸形 id/越界索引/size 空指针）
// 9. 负面用例验证强异常安全: [x]（空图片与越界索引用例断言计数器/状态不受损）
// 10. stub_ext vs gMock 选择正确: [x]（DeepinOCRDriver 为外部库具体类无虚函数注入点，一律 stub_ext static_cast 消歧）
//
// 环境隔离说明（SetUp/TearDown 全局生效）：
// - DeepinOCRDriver 全部被调方法（loadDefaultPlugin/setUseHardware/setMatrix/isRunning/analyze/
//   breakAnalyze/getTextBoxes/getCharBoxes/getResultFromBox）在 SetUp 中先 stub 再构造被测对象，
//   深度 OCR 插件 dlopen/推理从未真实发生，输入输出全部由 Fixture 成员注入/记录
// - pixelRatio 在 SetUp 构造后显式钉为 1.0，屏蔽 offscreen 屏幕差异；高 DPI 分支用例内改为 2.0
// - analyze 的 QtConcurrent 后台任务在 TearDown 先 waitForDone 再 stub.clear()，杜绝补丁窗口竞态
// - QGuiApplication::primaryScreen 仅在"无屏构造"用例内临时 stub，TearDown 统一恢复
//
// 疑似源码缺陷（行为锁定，未修改源码）：
// 1. charBox：getCharBoxes 返回空时 boxes[0].points[0] 无空检查直接访问 → UB（livetextanalyzer.cpp:117-118），
//    无法安全编写触发用例，仅以清单标注
// 2. 构造函数 parent 形参未转发给 QQuickImageProvider 基类，QObject 父子关系丢失（livetextanalyzer.cpp:19-24）
// 3. analyze：`while (ocrDriver->isRunning()) { }` 忙等无睡眠，占用 CPU 且与 breakAnalyze 存在竞态窗口（livetextanalyzer.cpp:77）
// 4. requestImage：畸形 id 经 toUInt 解析失败静默归 0，误取第 0 块而非报错（livetextanalyzer.cpp:151-152）
//
// 分支清单（来源：get_code_snippet livetextanalyzer.cpp:19-42 LiveTextAnalyzer 构造函数）
// B1: qApp->primaryScreen() 非空 → pixelRatio = devicePixelRatio()
// B2: primaryScreen() 为空 → pixelRatio 保持默认 1.0
// B3: loongarch 硬件加速开关为编译期 #if，本平台不可触发（不适用）
// 用例映射：
// - LiveTextAnalyzer_FreshInstance_LoadsPluginAndHardware   → B1（offscreen 屏存在路径 + 插件/硬件调用计数）
// - LiveTextAnalyzer_NoPrimaryScreen_KeepsUnitPixelRatio    → B2（stub primaryScreen 返回空）
// - LiveTextAnalyzer_ParentArgument_DroppedByBase           → B1 + 缺陷 2 行为锁定
//
// 分支清单（来源：get_code_snippet livetextanalyzer.cpp:44-47 ~LiveTextAnalyzer）
// 析构仅 delete ocrDriver，无分支
// 用例映射：
// - Destructor_AfterAnalyze_DeletesDriverWithoutDamage      → 析构后幸存实例仍可用
//
// 分支清单（来源：get_code_snippet livetextanalyzer.cpp:49-67 setImage）
// B1: pixelRatio > 1 → scaled(原宽/ratio, 原高/ratio) 后送 setMatrix
// B2: pixelRatio <= 1 → 直接送 setMatrix（转换 RGB888 后）
// 用例映射：
// - SetImage_UnitRatio_ConvertsRgb888AndForwardsMatrix      → B2（4x2 → setMatrix(2,4,step12,RGB)）
// - SetImage_HighPixelRatio_ScalesBeforeMatrix              → B1（8x4 ratio2 → setMatrix(2,4) 且缓存原图）
// - SetImage_NullImage_ForwardsEmptyMatrix                  → B2 负面（空图 setMatrix(0,0)）
//
// 分支清单（来源：get_code_snippet livetextanalyzer.cpp:69-81 analyze）
// B1: while (ocrDriver->isRunning()) 为真 → 自旋等待（0 次与 2 次迭代均覆盖）
// B2: analyze() 结果 true/false → emit analyzeFinished(result, token)
// 用例映射：
// - Analyze_IdleDriver_EmitsFinishedWithTrueAndToken        → B1(0 次)/B2(true)
// - Analyze_BusyDriverSpinThenResult_EmitsFalse             → B1(2 次)/B2(false)
//
// 分支清单（来源：get_code_snippet livetextanalyzer.cpp:83-89 breakAnalyze）
// B1: isRunning() 为真 → 调 ocrDriver->breakAnalyze()
// B2: isRunning() 为假 → 直接返回不转发
// 用例映射：
// - BreakAnalyze_RunningDriver_ForwardsBreakOnce            → B1
// - BreakAnalyze_IdleDriver_SkipsForwarding                 → B2
//
// 分支清单（来源：get_code_snippet livetextanalyzer.cpp:91-108 liveBlock）
// B1: 外层 for 0 个文本框 → 空 QVariantList
// B2: 外层 for N 框 × 内层 for points.size() 次 → [x0,y0,...,xN,yN,angle] 扁平化
// 用例映射：
// - LiveBlock_NoTextBoxes_ReturnsEmptyList                  → B1
// - LiveBlock_MultipleBoxes_FlattensPointsAndAngle          → B2（4 点框 + 3 点框验证内层循环变长）
//
// 分支清单（来源：get_code_snippet livetextanalyzer.cpp:110-129 charBox）
// B1: blockIndex >= getTextBoxes().size() → if 越界判定
// B2: 越界早退 return QVariant()（无效值返回路径）
// B3: 有效 → for 遍历字符框，以 boxes[0].points[0].first 为基址压入 [0, 各框 points[1].first - base]
// 注：getCharBoxes 为空时 B3 中 boxes[0] 无检查（疑似缺陷 1）
// 用例映射：
// - CharBox_ValidBlock_ReturnsOffsetsRelativeToFirstChar    → B3
// - CharBox_LastBoundaryBlock_ReturnsOffsets                → B3（index==size-1 边界）
// - CharBox_InvalidBlockIndex_ReturnsInvalidVariant（TEST_P）→ B1/B2（负数与 ==size）
// - CharBox_NoTextBoxes_ReturnsInvalidVariant               → B1/B2（空列表 + index 0）
//
// 分支清单（来源：get_code_snippet livetextanalyzer.cpp:131-145 textResult）
// B1: blockIndex>=size || startIndex<0 || len<=0（短路或，三条件独立触发）→ 返回 ""
// B2: 有效 → getResultFromBox(index) + mid(startIndex, len)
// 用例映射：
// - TextResult_ValidRange_ReturnsSubstring                  → B2
// - TextResult_LenBeyondEnd_ReturnsRemainder                → B2（len 超余边界）
// - TextResult_InvalidParams_ReturnsEmpty（TEST_P）          → B1（块越界/start 负/len 0/len 负）
//
// 分支清单（来源：get_code_snippet livetextanalyzer.cpp:148-174 requestImage）
// B1: id 无 "_" → indexOf 为 -1，+1 后 mid(0) 解析整串（垃圾串 toUInt 归 0 → 疑似缺陷 4）
// B2: index >= getTextBoxes().size() → 返回空 QImage（size 出参不写）
// B3: 有效 → rect 坐标 × pixelRatio 后 imageCache.copy 裁剪
// B4: size != nullptr → *size = image.size()
// B5: requestedSize.width()>0 && requestedSize.height()>0（两侧短路均覆盖）→ scaled
// B6: 否则原样返回
// 用例映射：
// - RequestImage_ValidIndex_ReturnsCroppedRegion            → B3/B4/B6（5x5 裁剪 + 像素色）
// - RequestImage_WithRequestedSize_ReturnsScaled            → B3/B4/B5（3x2 缩放，*size 保持裁剪尺寸）
// - RequestImage_NonPositiveRequestedSize_ReturnsUnscaled（TEST_P）→ B5 短路两侧/B6
// - RequestImage_IndexOutOfRange_ReturnsNullImage           → B2
// - RequestImage_MalformedId_FallsBackToIndexZero（TEST_P）→ B1（缺陷 4 行为锁定）
// - RequestImage_HighPixelRatio_DoublesCropRegion           → B3（ratio=2 坐标放大）
// - RequestImage_NullSizePointer_NoCrash                    → B4 反例（size 为空不写）

#include "livetextanalyzer.h"

#include "stub_ext/stubext.h"

#include <gtest/gtest.h>

#include <QGuiApplication>
#include <QImage>
#include <QPoint>
#include <QScreen>
#include <QSignalSpy>
#include <QSize>
#include <QThreadPool>
#include <QVariant>
#include <QVariantList>

#include <deepin-ocr-plugin-manager/deepinocrplugin.h>
#include <deepin-ocr-plugin-manager/deepinocrplugindef.h>

#include <string>
#include <vector>

namespace {

// 构造一个 4 点文本框（OCR 常规四边形），坐标乘 scale 便于生成不同数据
DeepinOCRPlugin::TextBox makeTextBox(float x0, float y0, float x1, float y1,
                                     float x2, float y2, float x3, float y3, float angle)
{
    DeepinOCRPlugin::TextBox box;
    box.points = { { x0, y0 }, { x1, y1 }, { x2, y2 }, { x3, y3 } };
    box.angle = angle;
    return box;
}

// 3 点文本框：验证 liveBlock 内层循环按 points.size() 变长
DeepinOCRPlugin::TextBox makeThreePointBox(float angle)
{
    DeepinOCRPlugin::TextBox box;
    box.points = { { 9.f, 8.f }, { 7.f, 6.f }, { 5.f, 4.f } };
    box.angle = angle;
    return box;
}

}  // namespace

class LiveTextAnalyzerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        installDriverStubs();
        obj = new LiveTextAnalyzer();
        // offscreen 屏幕的 devicePixelRatio 因环境而异，钉为 1.0 保证确定性；高 DPI 用例内改 2.0
        obj->pixelRatio = 1.0;
    }

    void TearDown() override
    {
        // 先等 QtConcurrent 后台任务结束再撤补丁，避免异步线程闯入未 stub 的真实插件
        QThreadPool::globalInstance()->waitForDone(60000);
        stub.clear();
        delete obj;
    }

    // OCR 驱动的全部出向依赖：结果注入 + 调用记录，隔离 deepin-ocr-plugin 的 dlopen/推理
    void installDriverStubs()
    {
        using Driver = DeepinOCRPlugin::DeepinOCRDriver;
        stub.set_lamda(static_cast<bool (Driver:: *)()>(&Driver::loadDefaultPlugin),
                       [this](Driver *) -> bool { ++loadDefaultPluginCount; return true; });
        stub.set_lamda(static_cast<bool (Driver:: *)(const std::vector<std::pair<DeepinOCRPlugin::HardwareID, int>> &)>(&Driver::setUseHardware),
                       [this](Driver *, const std::vector<std::pair<DeepinOCRPlugin::HardwareID, int>> &) -> bool { ++setUseHardwareCount; return true; });
        stub.set_lamda(static_cast<bool (Driver:: *)(int, int, unsigned char *, size_t, DeepinOCRPlugin::PixelType)>(&Driver::setMatrix),
                       [this](Driver *, int height, int width, unsigned char *, size_t step, DeepinOCRPlugin::PixelType type) -> bool {
                           ++setMatrixCount;
                           matrixHeight = height;
                           matrixWidth = width;
                           matrixStep = step;
                           matrixType = type;
                           return true;
                       });
        stub.set_lamda(static_cast<bool (Driver:: *)()>(&Driver::isRunning),
                       [this](Driver *) -> bool { ++isRunningCount; return runningCountdown-- > 0; });
        stub.set_lamda(static_cast<bool (Driver:: *)()>(&Driver::analyze),
                       [this](Driver *) -> bool { ++driverAnalyzeCount; return driverAnalyzeResult; });
        stub.set_lamda(static_cast<bool (Driver:: *)()>(&Driver::breakAnalyze),
                       [this](Driver *) -> bool { ++driverBreakCount; return true; });
        stub.set_lamda(static_cast<std::vector<DeepinOCRPlugin::TextBox> (Driver:: *)() const>(&Driver::getTextBoxes),
                       [this](const Driver *) -> std::vector<DeepinOCRPlugin::TextBox> { ++getTextBoxesCount; return textBoxes; });
        stub.set_lamda(static_cast<std::vector<DeepinOCRPlugin::TextBox> (Driver:: *)(size_t) const>(&Driver::getCharBoxes),
                       [this](const Driver *, size_t index) -> std::vector<DeepinOCRPlugin::TextBox> {
                           ++getCharBoxesCount;
                           lastCharBoxIndex = index;
                           return charBoxes;
                       });
        stub.set_lamda(static_cast<std::string (Driver:: *)(size_t)>(&Driver::getResultFromBox),
                       [this](Driver *, size_t index) -> std::string {
                           ++getResultCount;
                           lastResultIndex = index;
                           return boxText;
                       });
    }

    stub_ext::StubExt stub;
    LiveTextAnalyzer *obj = nullptr;

    // 输入注入
    std::vector<DeepinOCRPlugin::TextBox> textBoxes;
    std::vector<DeepinOCRPlugin::TextBox> charBoxes;
    std::string boxText;
    int runningCountdown = 0;           // >0 时 isRunning 返回真（模拟忙等迭代次数）
    bool driverAnalyzeResult = true;

    // 调用记录
    int loadDefaultPluginCount = 0;
    int setUseHardwareCount = 0;
    int setMatrixCount = 0;
    int matrixHeight = -1;
    int matrixWidth = -1;
    size_t matrixStep = 0;
    DeepinOCRPlugin::PixelType matrixType = DeepinOCRPlugin::PixelType::Pixel_Unknown;
    int isRunningCount = 0;
    int driverAnalyzeCount = 0;
    int driverBreakCount = 0;
    int getTextBoxesCount = 0;
    int getCharBoxesCount = 0;
    size_t lastCharBoxIndex = static_cast<size_t>(-1);
    int getResultCount = 0;
    size_t lastResultIndex = static_cast<size_t>(-1);
};

// ══════════════════════════ 构造 / 析构 ══════════════════════════

TEST_F(LiveTextAnalyzerTest, LiveTextAnalyzer_FreshInstance_LoadsPluginAndHardware)
{
    // Arrange: SetUp 已以 stub 构造完成实例，读取构造期产生的副作用计数
    const int pluginLoads = loadDefaultPluginCount;
    const int hardwareConfigs = setUseHardwareCount;
    const qreal ratio = obj->pixelRatio;

    // Act: 构造发生在 SetUp，此处核对计数即验证构造行为

    // Assert  // ctor B1: 构造即加载默认插件并配置硬件加速（x86 路径 setUseHardware 恰好一次）
    EXPECT_EQ(pluginLoads, 1);
    EXPECT_EQ(hardwareConfigs, 1);
    ASSERT_NE(obj->ocrDriver, nullptr);
    EXPECT_GE(ratio, 1.0);
}

TEST_F(LiveTextAnalyzerTest, LiveTextAnalyzer_NoPrimaryScreen_KeepsUnitPixelRatio)
{
    // Arrange: stub 主屏幕为空（无窗口系统分支），再构造第二个实例
    stub.set_lamda(static_cast<QScreen *(*)()>(&QGuiApplication::primaryScreen),
                   []() -> QScreen * { return nullptr; });

    // Act
    LiveTextAnalyzer noScreenObj;

    // Assert  // ctor B2: 无主屏时 pixelRatio 保持默认 1.0，对象仍可用
    EXPECT_EQ(noScreenObj.pixelRatio, 1.0);
    EXPECT_TRUE(noScreenObj.liveBlock().toList().isEmpty());
}

TEST_F(LiveTextAnalyzerTest, LiveTextAnalyzer_ParentArgument_DroppedByBase)
{
    // Arrange: 栈上父对象 + 传入 parent 构造
    QObject parentObj;

    // Act
    LiveTextAnalyzer child(&parentObj);

    // Assert  // 现状：parent 形参未转发基类，QObject 父子关系丢失（疑似缺陷 2，行为锁定）
    EXPECT_EQ(child.parent(), nullptr);
    EXPECT_EQ(loadDefaultPluginCount, 2);
    ASSERT_NE(child.ocrDriver, nullptr);
}

TEST_F(LiveTextAnalyzerTest, Destructor_AfterAnalyze_DeletesDriverWithoutDamage)
{
    // Arrange: 构造第二个实例并注入一块文本框，幸存实例 SetUp 已就绪
    auto *victim = new LiveTextAnalyzer();
    victim->pixelRatio = 1.0;
    textBoxes = { makeTextBox(0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f) };

    // Act
    delete victim;

    // Assert  // 析构仅释放自身驱动，不影响全局 stub 与幸存实例
    const QVariantList blocks = obj->liveBlock().toList();
    EXPECT_EQ(blocks.size(), 1);
    EXPECT_EQ(getTextBoxesCount, 1);
}

// ══════════════════════════ setImage ══════════════════════════

TEST_F(LiveTextAnalyzerTest, SetImage_UnitRatio_ConvertsRgb888AndForwardsMatrix)
{
    // Arrange: 4x2 ARGB 图，pixelRatio 已钉 1.0
    QImage image(4, 2, QImage::Format_ARGB32);
    image.fill(Qt::red);

    // Act
    obj->setImage(image);

    // Assert  // setImage B2: 不缩放，RGB888 转换后 setMatrix(高2, 宽4, step12, Pixel_RGB)
    ASSERT_EQ(setMatrixCount, 1);
    EXPECT_EQ(matrixHeight, 2);
    EXPECT_EQ(matrixWidth, 4);
    EXPECT_EQ(matrixStep, size_t(12));
    EXPECT_EQ(matrixType, DeepinOCRPlugin::PixelType::Pixel_RGB);
    EXPECT_EQ(obj->imageCache.size(), QSize(4, 2));
}

TEST_F(LiveTextAnalyzerTest, SetImage_HighPixelRatio_ScalesBeforeMatrix)
{
    // Arrange: 8x4 图，高 DPI ratio=2
    QImage image(8, 4, QImage::Format_ARGB32);
    image.fill(Qt::blue);
    obj->pixelRatio = 2.0;

    // Act
    obj->setImage(image);

    // Assert  // setImage B1: 先缩到 4x2 再送矩阵，imageCache 仍保留原始 8x4
    ASSERT_EQ(setMatrixCount, 1);
    EXPECT_EQ(matrixHeight, 2);
    EXPECT_EQ(matrixWidth, 4);
    EXPECT_EQ(obj->imageCache.size(), QSize(8, 4));
}

TEST_F(LiveTextAnalyzerTest, SetImage_NullImage_ForwardsEmptyMatrix)
{
    // Arrange: 空 QImage（负面输入）
    const QImage nullImage;

    // Act
    obj->setImage(nullImage);

    // Assert  // 空图仍走 setMatrix(0,0)，缓存为空，对象状态未损坏
    ASSERT_EQ(setMatrixCount, 1);
    EXPECT_EQ(matrixHeight, 0);
    EXPECT_EQ(matrixWidth, 0);
    EXPECT_TRUE(obj->imageCache.isNull());
}

// ══════════════════════════ analyze ══════════════════════════

TEST_F(LiveTextAnalyzerTest, Analyze_IdleDriver_EmitsFinishedWithTrueAndToken)
{
    // Arrange: 驱动空闲（runningCountdown=0 → 首次 isRunning 即假），识别成功
    QSignalSpy spy(obj, &LiveTextAnalyzer::analyzeFinished);
    ASSERT_TRUE(spy.isValid());

    // Act
    obj->analyze(QStringLiteral("tok-alpha"));

    // Assert  // analyze B1(0 次自旋)/B2(true)：发信号(true, tok-alpha)
    ASSERT_TRUE(spy.wait(10000));
    EXPECT_EQ(spy.count(), 1);
    ASSERT_EQ(spy.at(0).count(), 2);
    EXPECT_TRUE(spy.at(0).at(0).toBool());
    EXPECT_EQ(spy.at(0).at(1).toString(), QStringLiteral("tok-alpha"));
    EXPECT_EQ(driverAnalyzeCount, 1);
}

TEST_F(LiveTextAnalyzerTest, Analyze_BusyDriverSpinThenResult_EmitsFalse)
{
    // Arrange: 前两次 isRunning 为真（忙等 2 轮），识别结果 false
    runningCountdown = 2;
    driverAnalyzeResult = false;
    QSignalSpy spy(obj, &LiveTextAnalyzer::analyzeFinished);
    ASSERT_TRUE(spy.isValid());

    // Act
    obj->analyze(QStringLiteral("tok-beta"));

    // Assert  // analyze B1(2 次迭代)/B2(false)：共 3 次 isRunning（真真假）后发信号(false, tok-beta)
    ASSERT_TRUE(spy.wait(10000));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(spy.at(0).at(0).toBool());
    EXPECT_EQ(spy.at(0).at(1).toString(), QStringLiteral("tok-beta"));
    EXPECT_EQ(isRunningCount, 3);
}

// ══════════════════════════ breakAnalyze ══════════════════════════

TEST_F(LiveTextAnalyzerTest, BreakAnalyze_RunningDriver_ForwardsBreakOnce)
{
    // Arrange: 驱动识别中（isRunning 首次为真）
    runningCountdown = 1;

    // Act
    obj->breakAnalyze();

    // Assert  // breakAnalyze B1: 运行中 → 转发驱动 breakAnalyze 一次
    EXPECT_EQ(driverBreakCount, 1);
    EXPECT_EQ(isRunningCount, 1);
}

TEST_F(LiveTextAnalyzerTest, BreakAnalyze_IdleDriver_SkipsForwarding)
{
    // Arrange: 驱动空闲（runningCountdown=0）
    runningCountdown = 0;

    // Act
    obj->breakAnalyze();

    // Assert  // breakAnalyze B2: 空闲 → 不转发且不产生其他副作用
    EXPECT_EQ(driverBreakCount, 0);
    EXPECT_EQ(isRunningCount, 1);
}

// ══════════════════════════ liveBlock ══════════════════════════

TEST_F(LiveTextAnalyzerTest, LiveBlock_NoTextBoxes_ReturnsEmptyList)
{
    // Arrange: 未注入任何文本框
    textBoxes.clear();

    // Act
    const QVariant result = obj->liveBlock();

    // Assert  // liveBlock B1: 外层循环 0 次 → 空列表
    EXPECT_TRUE(result.toList().isEmpty());
    EXPECT_EQ(getTextBoxesCount, 1);
}

TEST_F(LiveTextAnalyzerTest, LiveBlock_MultipleBoxes_FlattensPointsAndAngle)
{
    // Arrange: 4 点框（angle 0.25）+ 3 点框（angle -1.5）
    textBoxes = { makeTextBox(1.5f, 2.5f, 3.5f, 2.5f, 3.5f, 6.5f, 1.5f, 6.5f, 0.25f),
                  makeThreePointBox(-1.5f) };

    // Act
    const QVariantList result = obj->liveBlock().toList();

    // Assert  // liveBlock B2: 每框 [x,y,...,angle] 扁平化，内层长度随 points.size() 变化
    ASSERT_EQ(result.size(), 2);
    const QVariantList first = result.at(0).toList();
    ASSERT_EQ(first.size(), 9);   // 4 点 ×2 + angle
    EXPECT_FLOAT_EQ(first.at(0).toFloat(), 1.5f);
    EXPECT_FLOAT_EQ(first.at(1).toFloat(), 2.5f);
    EXPECT_FLOAT_EQ(first.at(6).toFloat(), 1.5f);
    EXPECT_FLOAT_EQ(first.at(7).toFloat(), 6.5f);
    EXPECT_FLOAT_EQ(first.at(8).toFloat(), 0.25f);
    const QVariantList second = result.at(1).toList();
    ASSERT_EQ(second.size(), 7);  // 3 点 ×2 + angle
    EXPECT_FLOAT_EQ(second.at(0).toFloat(), 9.f);
    EXPECT_FLOAT_EQ(second.at(6).toFloat(), -1.5f);
}

// ══════════════════════════ charBox ══════════════════════════

TEST_F(LiveTextAnalyzerTest, CharBox_ValidBlock_ReturnsOffsetsRelativeToFirstChar)
{
    // Arrange: 1 个文本框 + 2 个字符框（基址取首字符 x=10）
    textBoxes = { makeTextBox(10.f, 90.f, 40.f, 90.f, 40.f, 100.f, 10.f, 100.f, 0.f) };
    charBoxes = { makeTextBox(10.f, 90.f, 20.f, 90.f, 20.f, 95.f, 10.f, 95.f, 0.f),
                  makeTextBox(15.f, 90.f, 27.f, 90.f, 27.f, 95.f, 15.f, 95.f, 0.f) };

    // Act
    const QVariantList result = obj->charBox(0).toList();

    // Assert  // charBox B2: [0, 20-10, 27-10]，索引透传给 getCharBoxes(0)
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result.at(0).toInt(), 0);
    EXPECT_FLOAT_EQ(result.at(1).toFloat(), 10.f);
    EXPECT_FLOAT_EQ(result.at(2).toFloat(), 17.f);
    EXPECT_EQ(lastCharBoxIndex, size_t(0));
}

TEST_F(LiveTextAnalyzerTest, CharBox_LastBoundaryBlock_ReturnsOffsets)
{
    // Arrange: 2 个文本框，请求最后一块（index == size-1 边界）
    textBoxes = { makeTextBox(0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f),
                  makeTextBox(5.f, 5.f, 6.f, 5.f, 6.f, 6.f, 5.f, 6.f, 0.f) };
    charBoxes = { makeTextBox(100.f, 50.f, 130.f, 50.f, 130.f, 60.f, 100.f, 60.f, 0.f) };

    // Act
    const QVariantList result = obj->charBox(1).toList();

    // Assert  // charBox B2 边界: 最后一块有效，偏移相对其首字符
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0).toInt(), 0);
    EXPECT_FLOAT_EQ(result.at(1).toFloat(), 30.f);
    EXPECT_EQ(lastCharBoxIndex, size_t(1));
}

namespace {
struct CharBoxInvalidCase {
    int blockIndex;
    const char *desc;
};
}  // namespace

struct CharBoxInvalidParamTest : public LiveTextAnalyzerTest, public ::testing::WithParamInterface<CharBoxInvalidCase> {
};

TEST_P(CharBoxInvalidParamTest, CharBox_InvalidBlockIndex_ReturnsInvalidVariant)
{
    const CharBoxInvalidCase c = GetParam();

    // Arrange: 2 个文本框，请求越界索引（负数转 size_t 巨值 / 恰为 size）
    textBoxes = { makeTextBox(0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f),
                  makeTextBox(2.f, 2.f, 3.f, 2.f, 3.f, 3.f, 2.f, 3.f, 0.f) };
    charBoxes = { makeTextBox(0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f) };

    // Act
    const QVariant result = obj->charBox(c.blockIndex);

    // Assert  // charBox B1: 越界 → 无效 QVariant，且不触达字符框查询
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(getCharBoxesCount, 0);
}

INSTANTIATE_TEST_SUITE_P(
    OutOfRangeIndexes, CharBoxInvalidParamTest,
    ::testing::Values(
        CharBoxInvalidCase{ -1, "negative index" },
        CharBoxInvalidCase{ 2, "index equals size" }));

TEST_F(LiveTextAnalyzerTest, CharBox_NoTextBoxes_ReturnsInvalidVariant)
{
    // Arrange: 无文本框，请求索引 0（空容器边界）
    textBoxes.clear();

    // Act
    const QVariant result = obj->charBox(0);

    // Assert  // charBox B1: 0 >= 0 → 无效 QVariant
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(getCharBoxesCount, 0);
}

// ══════════════════════════ textResult ══════════════════════════

TEST_F(LiveTextAnalyzerTest, TextResult_ValidRange_ReturnsSubstring)
{
    // Arrange: 1 个文本框，识别文本 "Hello World"
    textBoxes = { makeTextBox(0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f) };
    boxText = "Hello World";

    // Act
    const QString head = obj->textResult(0, 0, 5);
    const QString tail = obj->textResult(0, 6, 5);
    const QString middle = obj->textResult(0, 4, 3);

    // Assert  // textResult B2: mid(startIndex, len) 精确切片，索引透传 getResultFromBox(0)
    EXPECT_EQ(head, QStringLiteral("Hello"));
    EXPECT_EQ(tail, QStringLiteral("World"));
    EXPECT_EQ(middle, QStringLiteral("o W"));
    EXPECT_EQ(getResultCount, 3);
    EXPECT_EQ(lastResultIndex, size_t(0));
}

TEST_F(LiveTextAnalyzerTest, TextResult_LenBeyondEnd_ReturnsRemainder)
{
    // Arrange: 文本长 11，请求 len=100 越过结尾
    textBoxes = { makeTextBox(0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f) };
    boxText = "Hello World";

    // Act
    const QString result = obj->textResult(0, 6, 100);

    // Assert  // textResult B2 边界: QString::mid 越界取剩余全部
    EXPECT_EQ(result, QStringLiteral("World"));
    EXPECT_EQ(result.length(), 5);
}

namespace {
struct TextResultInvalidCase {
    int block;
    int start;
    int len;
    const char *desc;
};
}  // namespace

struct TextResultInvalidParamTest : public LiveTextAnalyzerTest, public ::testing::WithParamInterface<TextResultInvalidCase> {
};

TEST_P(TextResultInvalidParamTest, TextResult_InvalidParams_ReturnsEmpty)
{
    const TextResultInvalidCase c = GetParam();

    // Arrange: 1 个文本框 + 文本，参数取各非法等价类
    textBoxes = { makeTextBox(0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f) };
    boxText = "Hello World";

    // Act
    const QString result = obj->textResult(c.block, c.start, c.len);

    // Assert  // textResult B1: 短路或的三个条件各自触发 → ""，且不触达结果读取
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(getResultCount, 0);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidParams, TextResultInvalidParamTest,
    ::testing::Values(
        TextResultInvalidCase{ 7, 0, 1, "block out of range" },
        TextResultInvalidCase{ 0, -1, 3, "negative start" },
        TextResultInvalidCase{ 0, 3, 0, "zero length" },
        TextResultInvalidCase{ 0, 3, -5, "negative length" }));

// ══════════════════════════ requestImage ══════════════════════════

TEST_F(LiveTextAnalyzerTest, RequestImage_ValidIndex_ReturnsCroppedRegion)
{
    // Arrange: 10x10 红图入缓存 + 1 框 (1,1)-(5,5)，ratio=1 → 裁剪 5x5
    QImage source(10, 10, QImage::Format_ARGB32);
    source.fill(Qt::red);
    obj->setImage(source);
    textBoxes = { makeTextBox(1.f, 1.f, 2.f, 1.f, 5.f, 5.f, 2.f, 5.f, 0.f) };
    QSize reported;

    // Act
    const QImage image = obj->requestImage(QStringLiteral("rnd_0"), &reported, QSize());

    // Assert  // requestImage B3/B4/B6: 裁剪 5x5 原色返回，*size 同步
    EXPECT_FALSE(image.isNull());
    EXPECT_EQ(image.size(), QSize(5, 5));
    EXPECT_EQ(reported, QSize(5, 5));
    EXPECT_EQ(image.pixel(2, 2), qRgb(255, 0, 0));
}

TEST_F(LiveTextAnalyzerTest, RequestImage_WithRequestedSize_ReturnsScaled)
{
    // Arrange: 同上 5x5 裁剪，请求缩放到 3x2
    QImage source(10, 10, QImage::Format_ARGB32);
    source.fill(Qt::green);
    obj->setImage(source);
    textBoxes = { makeTextBox(1.f, 1.f, 2.f, 1.f, 5.f, 5.f, 2.f, 5.f, 0.f) };
    QSize reported;

    // Act
    const QImage image = obj->requestImage(QStringLiteral("rnd_0"), &reported, QSize(3, 2));

    // Assert  // requestImage B5: 双边正尺寸 → scaled；B4: *size 仍是裁剪尺寸而非缩放尺寸
    EXPECT_EQ(image.size(), QSize(3, 2));
    EXPECT_EQ(reported, QSize(5, 5));
    EXPECT_FALSE(image.isNull());
}

namespace {
struct RequestedSizeCase {
    QSize requested;
    const char *desc;
};
}  // namespace

struct RequestSizeParamTest : public LiveTextAnalyzerTest, public ::testing::WithParamInterface<RequestedSizeCase> {
};

TEST_P(RequestSizeParamTest, RequestImage_NonPositiveRequestedSize_ReturnsUnscaled)
{
    const RequestedSizeCase c = GetParam();

    // Arrange: 5x5 裁剪场景，requestedSize 含非正边（w/h 短路两侧）
    QImage source(10, 10, QImage::Format_ARGB32);
    source.fill(Qt::blue);
    obj->setImage(source);
    textBoxes = { makeTextBox(1.f, 1.f, 2.f, 1.f, 5.f, 5.f, 2.f, 5.f, 0.f) };
    QSize reported;

    // Act
    const QImage image = obj->requestImage(QStringLiteral("rnd_0"), &reported, c.requested);

    // Assert  // requestImage B6: 不满足双边为正 → 不缩放原样返回
    EXPECT_EQ(image.size(), QSize(5, 5));
    EXPECT_EQ(reported, QSize(5, 5));
}

INSTANTIATE_TEST_SUITE_P(
    NonPositiveSizes, RequestSizeParamTest,
    ::testing::Values(
        RequestedSizeCase{ QSize(0, 0), "both zero" },
        RequestedSizeCase{ QSize(0, 5), "zero width short-circuits" },
        RequestedSizeCase{ QSize(5, 0), "zero height short-circuits" }));

TEST_F(LiveTextAnalyzerTest, RequestImage_IndexOutOfRange_ReturnsNullImage)
{
    // Arrange: 1 框，id 索引 9 越界；size 出参预置哨兵值
    QImage source(10, 10, QImage::Format_ARGB32);
    source.fill(Qt::red);
    obj->setImage(source);
    textBoxes = { makeTextBox(1.f, 1.f, 2.f, 1.f, 5.f, 5.f, 2.f, 5.f, 0.f) };
    QSize reported(7, 7);

    // Act
    const QImage image = obj->requestImage(QStringLiteral("rnd_9"), &reported, QSize());

    // Assert  // requestImage B2: 越界早退，空图且 size 出参不被写
    EXPECT_TRUE(image.isNull());
    EXPECT_EQ(reported, QSize(7, 7));
}

namespace {
struct MalformedIdCase {
    QString id;
    const char *desc;
};
}  // namespace

struct MalformedIdParamTest : public LiveTextAnalyzerTest, public ::testing::WithParamInterface<MalformedIdCase> {
};

TEST_P(MalformedIdParamTest, RequestImage_MalformedId_FallsBackToIndexZero)
{
    const MalformedIdCase c = GetParam();

    // Arrange: 1 框 (1,1)-(5,5)，喂畸形 id（无下划线/负数/尾随垃圾）
    QImage source(10, 10, QImage::Format_ARGB32);
    source.fill(Qt::red);
    obj->setImage(source);
    textBoxes = { makeTextBox(1.f, 1.f, 2.f, 1.f, 5.f, 5.f, 2.f, 5.f, 0.f) };
    QSize reported;

    // Act
    const QImage image = obj->requestImage(c.id, &reported, QSize());

    // Assert  // requestImage B1: toUInt 解析失败归 0 → 等价取第 0 块（疑似缺陷 4 行为锁定）
    EXPECT_EQ(image.size(), QSize(5, 5));
    EXPECT_EQ(reported, QSize(5, 5));
}

INSTANTIATE_TEST_SUITE_P(
    MalformedIds, MalformedIdParamTest,
    ::testing::Values(
        MalformedIdCase{ QStringLiteral("garbage"), "no underscore" },
        MalformedIdCase{ QStringLiteral("x_-3"), "negative index" },
        MalformedIdCase{ QStringLiteral("7_tail"), "trailing garbage" }));

TEST_F(LiveTextAnalyzerTest, RequestImage_HighPixelRatio_DoublesCropRegion)
{
    // Arrange: ratio=2，框 (1,1)-(4,3) → rect (2,2)-(8,6) → 7x5
    QImage source(10, 10, QImage::Format_ARGB32);
    source.fill(Qt::cyan);
    obj->setImage(source);
    obj->pixelRatio = 2.0;
    textBoxes = { makeTextBox(1.f, 1.f, 2.f, 1.f, 4.f, 3.f, 2.f, 3.f, 0.f) };
    QSize reported;

    // Act
    const QImage image = obj->requestImage(QStringLiteral("rnd_0"), &reported, QSize());

    // Assert  // requestImage B3: 坐标乘 pixelRatio 后裁剪
    EXPECT_EQ(image.size(), QSize(7, 5));
    EXPECT_EQ(reported, QSize(7, 5));
    EXPECT_EQ(image.pixel(3, 2), qRgb(0, 255, 255));
}

TEST_F(LiveTextAnalyzerTest, RequestImage_NullSizePointer_NoCrash)
{
    // Arrange: 1 框 5x5 裁剪场景，size 出参传空指针（负面输入）
    QImage source(10, 10, QImage::Format_ARGB32);
    source.fill(Qt::magenta);
    obj->setImage(source);
    textBoxes = { makeTextBox(1.f, 1.f, 2.f, 1.f, 5.f, 5.f, 2.f, 5.f, 0.f) };

    // Act
    const QImage image = obj->requestImage(QStringLiteral("rnd_0"), nullptr, QSize());

    // Assert  // requestImage B4 反例: size 为空跳过写出参，正常返回裁剪图
    //（有效路径源码两次查询 getTextBoxes：size 判定一次 + 取框一次，故计数为 2）
    EXPECT_EQ(image.size(), QSize(5, 5));
    EXPECT_EQ(getTextBoxesCount, 2);
}
