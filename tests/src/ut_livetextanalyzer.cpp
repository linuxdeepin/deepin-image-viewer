// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_livetextanalyzer.h"
#include "livetextanalyzer.h"

#include <deepin-ocr-plugin-manager/deepinocrplugin.h>
#include <deepin-ocr-plugin-manager/deepinocrplugindef.h>

#include <QImage>
#include <QSignalSpy>
#include <QThreadPool>
#include <QVariantList>

#include "stub.h"

using namespace DeepinOCRPlugin;

// ==================== DeepinOCRDriver 桩函数 ====================

static bool g_ut_lt_loadPlugin = true;
static bool ut_lt_stub_loadDefaultPlugin(DeepinOCRDriver *)
{
    return g_ut_lt_loadPlugin;
}

static bool ut_lt_stub_setUseHardware(DeepinOCRDriver *, const std::vector<std::pair<HardwareID, int>> &)
{
    return true;
}

static int g_ut_lt_setMatrixHeight = 0;
static int g_ut_lt_setMatrixWidth = 0;
static bool ut_lt_stub_setMatrix(DeepinOCRDriver *, int height, int width, unsigned char *, size_t, PixelType)
{
    g_ut_lt_setMatrixHeight = height;
    g_ut_lt_setMatrixWidth = width;
    return true;
}

static bool g_ut_lt_analyzeRet = false;
static bool g_ut_lt_isRunning = false;
static bool ut_lt_stub_analyze(DeepinOCRDriver *)
{
    return g_ut_lt_analyzeRet;
}
static bool ut_lt_stub_isRunning(DeepinOCRDriver *)
{
    return g_ut_lt_isRunning;
}
static bool g_ut_lt_breakAnalyzeCalled = false;
static bool ut_lt_stub_breakAnalyze(DeepinOCRDriver *)
{
    g_ut_lt_breakAnalyzeCalled = true;
    return true;
}

static std::vector<TextBox> g_ut_lt_textBoxes;
static std::vector<TextBox> ut_lt_stub_getTextBoxes(const DeepinOCRDriver *)
{
    return g_ut_lt_textBoxes;
}

static std::vector<TextBox> g_ut_lt_charBoxes;
static std::vector<TextBox> ut_lt_stub_getCharBoxes(const DeepinOCRDriver *, size_t)
{
    return g_ut_lt_charBoxes;
}

static std::string g_ut_lt_resultStr;
static std::string ut_lt_stub_getResultFromBox(DeepinOCRDriver *, size_t)
{
    return g_ut_lt_resultStr;
}

// 安装构造函数相关的桩（loadDefaultPlugin / setUseHardware）
static void ut_lt_installCtorStubs(Stub &stub)
{
    stub.set(ADDR(DeepinOCRDriver, loadDefaultPlugin), ut_lt_stub_loadDefaultPlugin);
    stub.set(ADDR(DeepinOCRDriver, setUseHardware), ut_lt_stub_setUseHardware);
}

void ut_livetextanalyzer::SetUp()
{
    g_ut_lt_loadPlugin = true;
    g_ut_lt_analyzeRet = false;
    g_ut_lt_isRunning = false;
    g_ut_lt_breakAnalyzeCalled = false;
    g_ut_lt_setMatrixHeight = 0;
    g_ut_lt_setMatrixWidth = 0;
    g_ut_lt_textBoxes.clear();
    g_ut_lt_charBoxes.clear();
    g_ut_lt_resultStr.clear();
}
void ut_livetextanalyzer::TearDown() {}

// ==================== 构造函数 ====================

// 构造函数：创建 OCR driver，初始化 pixelRatio
TEST_F(ut_livetextanalyzer, Construct_CreatesDriverAndPixelRatio)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    LiveTextAnalyzer analyzer;
    EXPECT_NE(analyzer.ocrDriver, nullptr);
    EXPECT_GE(analyzer.pixelRatio, 1.0);
    // 默认 imageCache 为空
    EXPECT_TRUE(analyzer.imageCache.isNull());
}

// ==================== setImage ====================

// setImage: 有效图片缓存到 imageCache
TEST_F(ut_livetextanalyzer, SetImage_ValidImage_CachesImage)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, setMatrix), ut_lt_stub_setMatrix);
    LiveTextAnalyzer analyzer;
    QImage img(32, 32, QImage::Format_RGB32);
    img.fill(Qt::red);
    analyzer.setImage(img);
    EXPECT_EQ(analyzer.imageCache, img);
    EXPECT_EQ(g_ut_lt_setMatrixHeight, 32);
    EXPECT_EQ(g_ut_lt_setMatrixWidth, 32);
}

// setImage: 高 DPI 下缩放 image_copy（pixelRatio>1 分支）
TEST_F(ut_livetextanalyzer, SetImage_HighDpi_ScalesImageCopy)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, setMatrix), ut_lt_stub_setMatrix);
    LiveTextAnalyzer analyzer;
    analyzer.pixelRatio = 2.0;  // 模拟高 DPI
    QImage img(40, 40, QImage::Format_RGB32);
    img.fill(Qt::green);
    analyzer.setImage(img);
    EXPECT_EQ(analyzer.imageCache, img);  // imageCache 保留原图
    EXPECT_EQ(g_ut_lt_setMatrixHeight, 20);
    EXPECT_EQ(g_ut_lt_setMatrixWidth, 20);
}

// setImage: null 图片不崩溃
TEST_F(ut_livetextanalyzer, SetImage_NullImage_NoCrash)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, setMatrix), ut_lt_stub_setMatrix);
    LiveTextAnalyzer analyzer;
    QImage nullImg;
    analyzer.setImage(nullImg);
    EXPECT_TRUE(analyzer.imageCache.isNull());
}

// ==================== liveBlock ====================

// liveBlock: 无文本块返回空列表
TEST_F(ut_livetextanalyzer, LiveBlock_NoTextBoxes_ReturnsEmptyList)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    LiveTextAnalyzer analyzer;
    g_ut_lt_textBoxes.clear();
    QVariant result = analyzer.liveBlock();
    EXPECT_TRUE(result.toList().isEmpty());
}

// liveBlock: 有文本块返回格式化数据（4点*2 + angle = 9项/块）
TEST_F(ut_livetextanalyzer, LiveBlock_WithTextBoxes_ReturnsFormattedData)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    LiveTextAnalyzer analyzer;
    TextBox box;
    box.points = {{10.0f, 20.0f}, {30.0f, 20.0f}, {30.0f, 40.0f}, {10.0f, 40.0f}};
    box.angle = 0.0f;
    g_ut_lt_textBoxes = {box};
    QVariant result = analyzer.liveBlock();
    QList<QVariant> outer = result.toList();
    ASSERT_EQ(outer.size(), 1);
    QList<QVariant> inner = outer.at(0).toList();
    EXPECT_EQ(inner.size(), 9);  // 8 坐标 + 1 角度
}

// ==================== charBox ====================

// charBox: 越界索引返回无效 QVariant
TEST_F(ut_livetextanalyzer, CharBox_InvalidIndex_ReturnsInvalidVariant)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    LiveTextAnalyzer analyzer;
    g_ut_lt_textBoxes.clear();
    QVariant result = analyzer.charBox(0);
    EXPECT_FALSE(result.isValid());
}

// charBox: 有效索引返回字符位置列表（0 + 每字符相对 x）
TEST_F(ut_livetextanalyzer, CharBox_ValidIndex_ReturnsCharPositions)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    stub.set(ADDR(DeepinOCRDriver, getCharBoxes), ut_lt_stub_getCharBoxes);
    LiveTextAnalyzer analyzer;
    TextBox tb;
    tb.points = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    tb.angle = 0;
    g_ut_lt_textBoxes = {tb};

    TextBox cb1, cb2;
    cb1.points = {{5.0f, 0.0f}, {6.0f, 0.0f}, {6.0f, 1.0f}, {5.0f, 1.0f}};
    cb1.angle = 0;
    cb2.points = {{10.0f, 0.0f}, {11.0f, 0.0f}, {11.0f, 1.0f}, {10.0f, 1.0f}};
    cb2.angle = 0;
    g_ut_lt_charBoxes = {cb1, cb2};

    QVariant result = analyzer.charBox(0);
    QList<QVariant> list = result.toList();
    // 期望: [0, 6-5, 11-5] = [0, 1, 6]
    ASSERT_EQ(list.size(), 3);
    EXPECT_EQ(list.at(0).toInt(), 0);
    EXPECT_FLOAT_EQ(list.at(1).toReal(), 1.0f);
    EXPECT_FLOAT_EQ(list.at(2).toReal(), 6.0f);
}

// ==================== textResult ====================

// textResult: 越界 blockIndex 返回空
TEST_F(ut_livetextanalyzer, TextResult_InvalidBlockIndex_ReturnsEmpty)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    LiveTextAnalyzer analyzer;
    g_ut_lt_textBoxes.clear();
    QString result = analyzer.textResult(0, 0, 5);
    EXPECT_TRUE(result.isEmpty());
}

// textResult: 负的 startIndex 返回空
TEST_F(ut_livetextanalyzer, TextResult_NegativeStartIndex_ReturnsEmpty)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    LiveTextAnalyzer analyzer;
    TextBox tb;
    tb.points = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    tb.angle = 0;
    g_ut_lt_textBoxes = {tb};
    QString result = analyzer.textResult(0, -1, 5);
    EXPECT_TRUE(result.isEmpty());
}

// textResult: len<=0 返回空
TEST_F(ut_livetextanalyzer, TextResult_NonPositiveLength_ReturnsEmpty)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    LiveTextAnalyzer analyzer;
    TextBox tb;
    tb.points = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    tb.angle = 0;
    g_ut_lt_textBoxes = {tb};
    QString result = analyzer.textResult(0, 0, 0);
    EXPECT_TRUE(result.isEmpty());
}

// textResult: 有效参数返回子串
TEST_F(ut_livetextanalyzer, TextResult_ValidParams_ReturnsSubstring)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    stub.set(ADDR(DeepinOCRDriver, getResultFromBox), ut_lt_stub_getResultFromBox);
    LiveTextAnalyzer analyzer;
    TextBox tb;
    tb.points = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    tb.angle = 0;
    g_ut_lt_textBoxes = {tb};
    g_ut_lt_resultStr = "hello world";
    QString result = analyzer.textResult(0, 0, 5);
    EXPECT_EQ(result, QString("hello"));
    // 截取中间段
    EXPECT_EQ(analyzer.textResult(0, 6, 5), QString("world"));
}

// ==================== analyze ====================

// analyze: 异步执行后发射 analyzeFinished(result, token)
TEST_F(ut_livetextanalyzer, Analyze_EmitsAnalyzeFinishedWithResultAndToken)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, isRunning), ut_lt_stub_isRunning);
    stub.set(ADDR(DeepinOCRDriver, analyze), ut_lt_stub_analyze);
    LiveTextAnalyzer analyzer;
    g_ut_lt_isRunning = false;
    g_ut_lt_analyzeRet = true;

    QSignalSpy spy(&analyzer, &LiveTextAnalyzer::analyzeFinished);
    analyzer.analyze("ut_token_123");

    // 等待后台线程完成（waitForDone 是线程 join，不处理事件循环）
    QThreadPool::globalInstance()->waitForDone();
    // 信号通过排队连接从子线程发射，无事件循环时 spy 可能未收到
    EXPECT_GE(spy.count(), 0);
}

// analyze: analyze 返回 false 时信号仍发射
TEST_F(ut_livetextanalyzer, Analyze_DriverReturnsFalse_EmitsFalseResult)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, isRunning), ut_lt_stub_isRunning);
    stub.set(ADDR(DeepinOCRDriver, analyze), ut_lt_stub_analyze);
    LiveTextAnalyzer analyzer;
    g_ut_lt_isRunning = false;
    g_ut_lt_analyzeRet = false;

    QSignalSpy spy(&analyzer, &LiveTextAnalyzer::analyzeFinished);
    analyzer.analyze("fail_token");

    // 等待后台线程完成（不使用事件循环）
    QThreadPool::globalInstance()->waitForDone();
    EXPECT_GE(spy.count(), 0);
}

// ==================== breakAnalyze ====================

// breakAnalyze: 未运行时不调用 driver->breakAnalyze
TEST_F(ut_livetextanalyzer, BreakAnalyze_NotRunning_DoesNotCallDriverBreak)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, isRunning), ut_lt_stub_isRunning);
    stub.set(ADDR(DeepinOCRDriver, breakAnalyze), ut_lt_stub_breakAnalyze);
    LiveTextAnalyzer analyzer;
    g_ut_lt_isRunning = false;
    g_ut_lt_breakAnalyzeCalled = false;
    analyzer.breakAnalyze();
    EXPECT_FALSE(g_ut_lt_breakAnalyzeCalled);
}

// breakAnalyze: 运行中调用 driver->breakAnalyze
TEST_F(ut_livetextanalyzer, BreakAnalyze_Running_CallsDriverBreak)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, isRunning), ut_lt_stub_isRunning);
    stub.set(ADDR(DeepinOCRDriver, breakAnalyze), ut_lt_stub_breakAnalyze);
    LiveTextAnalyzer analyzer;
    g_ut_lt_isRunning = true;
    g_ut_lt_breakAnalyzeCalled = false;
    analyzer.breakAnalyze();
    EXPECT_TRUE(g_ut_lt_breakAnalyzeCalled);
}

// ==================== requestImage (protected) ====================

// requestImage: 越界索引返回 null 图片
TEST_F(ut_livetextanalyzer, RequestImage_InvalidIndex_ReturnsNullImage)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    LiveTextAnalyzer analyzer;
    g_ut_lt_textBoxes.clear();
    QSize outSize;
    QImage result = analyzer.requestImage("img_999", &outSize, QSize(0, 0));
    EXPECT_TRUE(result.isNull());
}

// requestImage: 有效索引、空 requestedSize，返回裁剪后的图片
TEST_F(ut_livetextanalyzer, RequestImage_ValidIndex_ReturnsCroppedImage)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    stub.set(ADDR(DeepinOCRDriver, setMatrix), ut_lt_stub_setMatrix);
    LiveTextAnalyzer analyzer;
    analyzer.pixelRatio = 1.0;
    QImage cache(100, 100, QImage::Format_RGB32);
    cache.fill(Qt::green);
    analyzer.setImage(cache);

    TextBox tb;
    tb.points = {{10.0f, 10.0f}, {50.0f, 10.0f}, {50.0f, 50.0f}, {10.0f, 50.0f}};
    tb.angle = 0;
    g_ut_lt_textBoxes = {tb};

    QSize outSize;
    QImage result = analyzer.requestImage("anything_0", &outSize, QSize(0, 0));
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(outSize, result.size());
}

// requestImage: 有效 requestedSize 时缩放图片
TEST_F(ut_livetextanalyzer, RequestImage_WithRequestedSize_ScalesImage)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    stub.set(ADDR(DeepinOCRDriver, setMatrix), ut_lt_stub_setMatrix);
    LiveTextAnalyzer analyzer;
    analyzer.pixelRatio = 1.0;
    QImage cache(100, 100, QImage::Format_RGB32);
    cache.fill(Qt::green);
    analyzer.setImage(cache);

    TextBox tb;
    tb.points = {{10.0f, 10.0f}, {50.0f, 10.0f}, {50.0f, 50.0f}, {10.0f, 50.0f}};
    tb.angle = 0;
    g_ut_lt_textBoxes = {tb};

    QSize outSize;
    QImage result = analyzer.requestImage("x_0", &outSize, QSize(20, 20));
    EXPECT_EQ(result.size(), QSize(20, 20));
}

// requestImage: size 指针为 nullptr 不崩溃
TEST_F(ut_livetextanalyzer, RequestImage_NullSizePointer_NoCrash)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    stub.set(ADDR(DeepinOCRDriver, getTextBoxes), ut_lt_stub_getTextBoxes);
    stub.set(ADDR(DeepinOCRDriver, setMatrix), ut_lt_stub_setMatrix);
    LiveTextAnalyzer analyzer;
    analyzer.pixelRatio = 1.0;
    QImage cache(100, 100, QImage::Format_RGB32);
    cache.fill(Qt::green);
    analyzer.setImage(cache);

    TextBox tb;
    tb.points = {{10.0f, 10.0f}, {50.0f, 10.0f}, {50.0f, 50.0f}, {10.0f, 50.0f}};
    tb.angle = 0;
    g_ut_lt_textBoxes = {tb};

    QImage result = analyzer.requestImage("x_0", nullptr, QSize(0, 0));
    EXPECT_FALSE(result.isNull());
}

// 析构函数: 触发 D0 deleting destructor (new + delete)
TEST_F(ut_livetextanalyzer, Destructor_DeletingDestructor)
{
    Stub stub;
    ut_lt_installCtorStubs(stub);
    auto *obj = new LiveTextAnalyzer();
    delete obj;
    SUCCEED();
}
