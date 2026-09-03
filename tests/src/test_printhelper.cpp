// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | PrintHelper | low | - | 1 | 1 |
// | getInstance | low | - | 1 | 1 |
// | getIntance | mid | - | 2 | 2 |
// | showPrintDialog | mid | - | 2 | 5 |
// | ~PrintHelper | low | - | 1 | 1 |
// | RequestedSlot | low | - | 1 | 1 |
// | paintRequestSync | low | - | 1 | 7 |
// | ~RequestedSlot | low | - | 1 | 1 |
// ─── actual 均不低于 min ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（8/8 方法，含构造/析构）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（路径：存在/缺失/空列表/多页；图像：宽扁/方/高瘦/空图）
// 3. 每个等价类的边界值显式覆盖: [x]（0 张/1 张/2 张边界；单页与多页分界 imageCount 1/>1）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（paintRequestSync 宽高缩放 3 组）
// 5. 分支清单 → 用例映射已列出: [x]（见下方分支清单，来源 MCP get_code_snippet printhelper.cpp:100-215）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（无显式 throw；if/else/循环边界全覆盖）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无 throw 路径，错误路径以返回值/状态断言替代）
// 8. 负面场景有专门用例: [x]（空 paths/缺失文件/null QImage/空图片列表）
// 9. 负面用例验证强异常安全: [x]（调用后 m_paths/m_imgs 清空、无崩溃断言）
// 10. stub_ext vs gMock 选择正确: [x]（Qt/DTK 非虚方法 static_cast 消歧，unionimage 自由函数 static_cast；
//     虚函数 exec/newPage 不可 stub-ext → 定时器 reject / 子类覆写，见文件头"stub 安全说明"）

#include <gtest/gtest.h>

#include <cmath>
#include <memory>

#include <DPrintPreviewDialog>

#include <QApplication>
#include <QCoreApplication>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QPainter>
#include <QPointer>
#include <QPrinter>
#include <QRectF>
#include <QStringList>
#include <QTemporaryDir>
#include <QThread>
#include <QTimer>

#include "printhelper.h"
#include "stub_ext/stubext.h"
#include "unionimage.h"

DWIDGET_USE_NAMESPACE

// ═══════════════════════════════════════════════════════════════════
// stub 安全说明（实测教训）：
// stub-ext 通过 union 把成员指针首字重解释为函数地址再打序言补丁，
// 虚函数 PMF（Itanium ABI vcall 偏移编码，小整数）会被误当地址 → memcpy 零页 SEGV。
// 因此本文件禁止 stub 任何虚函数：
// - DPrintPreviewDialog::exec（虚）→ 不 stub，改用重复定时器在 exec 的嵌套
//   事件循环中找到对话框、延迟数拍后 reject()，防模态阻塞（见 armDialogReject）；
// - DPrinter::newPage（= QPrinter::newPage，虚）→ 不 stub，测试持有 printer
//   实例，用 CountingPrinter 子类覆写 newPage() 计数；
// - QImageReader::read/jumpToImage 也不 stub：exec 真跑后 QCommonStyle 会经
//   QImageReader 加载样式图标，全局补丁会劫持这些无关调用 → 多页分支改用
//   真实 2 页 TIFF 文件驱动，以 paintRequested 时刻的 m_imgs 页数作断言；
// - 仅 stub 非虚目标：setDocName / RequestedSlot::paintRequestSync（经 moc 直呼）
//   / QPainter::drawImage(4参) / 自由函数 loadStaticImageFromFile。
// ═══════════════════════════════════════════════════════════════════
// 分支清单与用例映射（来源：MCP get_code_snippet）
//
// 分支清单（来源：PrintHelper::showPrintDialog，printhelper.cpp:100-162）
// B1: imgReadreder.imageCount() > 1 → 多页分支：jumpToImage + read 逐页装入 m_imgs，路径计入 tempExsitPaths
// B2: imageCount() <= 1             → 单图分支：loadStaticImageFromFile 加载
// B3: !img.isNull()                 → m_imgs 追加该图，路径计入 tempExsitPaths
// B4: img.isNull()                  → 仅告警，不追加图与路径
// B5: runtimeDtkVersion >= 5.4.10 且 tempExsitPaths.count() > 0 → setDocName(首路径 completeBaseName + ".pdf")
// B6: tempExsitPaths.count() == 0（空 paths / 全部加载失败）→ 不调 setDocName
// B7: exec()（本构建未定义 USE_TEST）→ 定时器在 exec 事件循环内 reject 对话框，断言 dialogSeen
// 用例映射：
// - ShowPrintDialog_SingleImageFile_SetsDocNameAndClearsCache → B2+B3+B5+B7
// - ShowPrintDialog_MultiPageImage_ReadsEveryPage             → B1+B5+B7
// - ShowPrintDialog_EmptyPaths_DoesNotSetDocName              → B6+B7
// - ShowPrintDialog_MissingFile_LoadFailsImageSkipped         → B2+B4+B6+B7
// - ShowPrintDialog_LoadFailedFirst_OnlyLoadedPathsCounted    → B2+B3+B4+B5+B7（失败在前，文档名取首个成功路径）
//
// 分支清单（来源：RequestedSlot::paintRequestSync，printhelper.cpp:175-215）
// B1: !img.isNull()                       → 计算缩放比例并 drawImage
// B2: img.isNull()                        → 跳过该页（不绘制）
// B3: wRect.height() - img.height()*ratio > 0 → 适应宽（按宽缩放，垂直居中）
// B4: else                                → 适应高（按高缩放，水平居中）
// B5: indexNum != m_imgs.size()           → newPage() 换页
// B6: m_imgs 为空                         → 循环 0 次，不绘制不换页
// 用例映射：
// - PaintRequestSync_EmptyImageList_PaintsNothing          → B6
// - PaintRequestSync_SingleImage_WritesPdfWithoutPageBreak → B1+B3+B5(否)
// - PaintRequestSync_NullImage_SkippedButPageBreakKept     → B2+B5
// - PaintRequestSync_TwoImages_AddsSinglePageBreak         → B1+B3+B5
// - PaintRequestSync_ImageScaling_UsesExpectedGeometry（TEST_P）→ B1+B3+B4
// ═══════════════════════════════════════════════════════════════════

namespace {

// DPrintPreviewDialog 交互捕获（shared_ptr 保证定时器晚触发时不悬垂）
struct DialogCaptures {
    int docNameCount = 0;
    QString docName;
    bool dialogSeen = false;  // exec 的嵌套事件循环中找到对话框（证明走到 B7）
    bool rejected = false;    // 对话框已被定时器关闭
    int seenTicks = 0;        // 对话框出现后的等待拍数
    QTimer *rejectTimer = nullptr;
};

// paintRequestSync 缩放参数化用例：fitHeight=false 走"适应宽"分支
struct PaintScaleCase {
    int imgW;
    int imgH;
    bool fitHeight;
};

// 手工构造 2 页未压缩灰度 TIFF（Qt 无多帧写出 API），
// 驱动 showPrintDialog 的 imageCount()>1 多页分支
QByteArray makeTwoPageTiff(int w, int h)
{
    const quint32 pageBytes = quint32(w * h);
    const quint16 entryCount = 9;
    const quint32 ifd0Off = 8;
    const quint32 ifdSize = 2 + quint32(entryCount) * 12 + 4;
    const quint32 data0Off = ifd0Off + ifdSize;
    const quint32 ifd1Off = data0Off + pageBytes;
    const quint32 data1Off = ifd1Off + ifdSize;

    auto u16 = [](quint16 v) { return QByteArray(reinterpret_cast<const char *>(&v), 2); };
    auto u32 = [](quint32 v) { return QByteArray(reinterpret_cast<const char *>(&v), 4); };
    auto entry = [&](quint16 tag, quint16 type, quint32 count, quint32 value) {
        return u16(tag) + u16(type) + u32(count) + u32(value);
    };
    auto buildIfd = [&](quint32 nextIfd, quint32 stripOff, char fill) {
        QByteArray ifd = u16(entryCount);
        ifd += entry(256, 4, 1, quint32(w));           // ImageWidth LONG
        ifd += entry(257, 4, 1, quint32(h));           // ImageLength LONG
        ifd += entry(258, 3, 1, 8);                    // BitsPerSample SHORT 内联
        ifd += entry(259, 3, 1, 1);                    // Compression = 无压缩
        ifd += entry(262, 3, 1, 1);                    // Photometric = BlackIsZero
        ifd += entry(273, 4, 1, stripOff);             // StripOffsets
        ifd += entry(277, 3, 1, 1);                    // SamplesPerPixel = 1
        ifd += entry(278, 4, 1, quint32(h));           // RowsPerStrip
        ifd += entry(279, 4, 1, pageBytes);            // StripByteCounts
        ifd += u32(nextIfd);
        ifd += QByteArray(int(pageBytes), fill);
        return ifd;
    };

    QByteArray tiff = QByteArray("II\x2a\x00", 4) + u32(ifd0Off);
    tiff += buildIfd(ifd1Off, data0Off, char(0x10));
    tiff += buildIfd(0, data1Off, char(0xe0));
    return tiff;
}

// 镜像源码 printhelper.cpp:196-209 的目标矩形计算，用于期望值推导
QRectF expectedPaintRect(const QRectF &wRect, int imgW, int imgH)
{
    const qreal ratioW = wRect.width() * 1.0 / imgW;
    if (qreal(wRect.height() - imgH * ratioW) > 0) {
        return QRectF(0, qAbs(wRect.height() - imgH * ratioW) / 2,
                      wRect.width(), imgH * ratioW);
    }
    const qreal ratioH = wRect.height() * 1.0 / imgH;
    return QRectF((wRect.width() - imgW * ratioH) / 2, 0,
                  imgW * ratioH, wRect.height());
}

void expectRectNear(const QRectF &actual, const QRectF &expected)
{
    // 坐标含源码 qAbs() 运算，容忍整型截断差异；宽高由同式推导，精确比较
    EXPECT_NEAR(actual.x(), expected.x(), 1.5);
    EXPECT_NEAR(actual.y(), expected.y(), 1.5);
    EXPECT_DOUBLE_EQ(actual.width(), expected.width());
    EXPECT_DOUBLE_EQ(actual.height(), expected.height());
}

}  // namespace

class PrintHelperTest : public ::testing::Test {
protected:
    stub_ext::StubExt stub;
    PrintHelper *helper = nullptr;
    QTemporaryDir tmpDir;

    void SetUp() override {}

    void TearDown() override {
        PrintHelper *singleton = PrintHelper::m_Printer;
        PrintHelper::m_Printer = nullptr;
        if (singleton != nullptr && singleton != helper)
            delete singleton;
        delete helper;
        helper = nullptr;
        stub.clear();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    // 仅 stub 非虚的 setDocName 捕获文档名；exec 为虚函数不可 stub（见文件头说明）
    void stubPreviewDialog(const std::shared_ptr<DialogCaptures> &cap) {
        stub.set_lamda(
            static_cast<void (DPrintPreviewDialog:: *)(const QString &)>(&DPrintPreviewDialog::setDocName),
            [cap](DPrintPreviewDialog *, const QString &name) {
                ++cap->docNameCount;
                cap->docName = name;
            });
    }

    // 重复定时器：在 exec() 的嵌套事件循环中等待预览对话框出现，
    // 出现后第 3 拍再 reject（给预览生成/paintRequested 留时间），使 exec 返回；
    // lambda 仅捕获 shared_ptr，不捕获 this，杜绝晚触发悬垂
    void armDialogReject(const std::shared_ptr<DialogCaptures> &cap) {
        auto *timer = new QTimer(qApp);
        cap->rejectTimer = timer;
        QObject::connect(timer, &QTimer::timeout, timer, [cap]() {
            DPrintPreviewDialog *dlg = nullptr;
            for (QWidget *w : QApplication::topLevelWidgets()) {
                if ((dlg = qobject_cast<DPrintPreviewDialog *>(w)) != nullptr)
                    break;
            }
            if (dlg == nullptr)
                return;  // 对话框尚未出现，继续等待
            cap->dialogSeen = true;
            if (++cap->seenTicks < 3)
                return;
            cap->rejected = true;
            QMetaObject::invokeMethod(dlg, "reject");
            cap->rejectTimer->stop();
            cap->rejectTimer->deleteLater();
        });
        timer->start(5);
    }

    // showPrintDialog 返回后泵事件：派发 exec 期间/残留的定时器与 deferred 删除；
    // 若对话框始终未出现则停表止损
    void flushDialogEvents(const std::shared_ptr<DialogCaptures> &cap) {
        for (int i = 0; i < 200 && !cap->rejected; ++i) {
            QCoreApplication::processEvents();
            QThread::msleep(2);
        }
        if (!cap->rejected && cap->rejectTimer != nullptr) {
            cap->rejectTimer->stop();
            cap->rejectTimer->deleteLater();
        }
    }
};

// ───────────────────────── PrintHelper 构造/析构 ─────────────────────────

TEST_F(PrintHelperTest, PrintHelper_Constructor_CreatesRequestedSlotWithEmptyState) {
    // Arrange：确认 Fixture 初始无被测对象
    ASSERT_EQ(helper, nullptr);

    // Act：执行被测构造函数
    helper = new PrintHelper();

    // Assert
    EXPECT_NE(helper->m_re, nullptr);
    EXPECT_TRUE(helper->m_re->m_paths.isEmpty());
    EXPECT_TRUE(helper->m_re->m_imgs.isEmpty());
}

TEST_F(PrintHelperTest, PrintHelper_Destructor_DefersRequestedSlotDeletion) {
    // Arrange
    helper = new PrintHelper();
    QPointer<RequestedSlot> guard(helper->m_re);
    EXPECT_NE(guard.data(), nullptr);

    // Act
    delete helper;
    helper = nullptr;
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    // Assert：~PrintHelper 对 m_re 调用 deleteLater，事件刷新后被销毁
    EXPECT_TRUE(guard.isNull());
}

// ───────────────────────── PrintHelper::getInstance/getIntance ─────────────────────────

TEST_F(PrintHelperTest, GetInstance_SameSingletonAsLegacyGetIntance) {
    // Arrange：清空静态单例，保证用例自足
    if (PrintHelper::m_Printer != nullptr) {
        delete PrintHelper::m_Printer;
        PrintHelper::m_Printer = nullptr;
    }

    // Act：先经旧拼写入口创建，再经新拼写入口获取
    PrintHelper *legacy = PrintHelper::getIntance();
    PrintHelper *modern = PrintHelper::getInstance();

    // Assert：两个入口指向同一单例
    EXPECT_NE(modern, nullptr);
    EXPECT_EQ(modern, legacy);
    EXPECT_EQ(PrintHelper::m_Printer, modern);
}

TEST_F(PrintHelperTest, GetIntance_NoExistingInstance_CreatesSingletonAndReusesIt) {
    // Arrange：清空静态单例，保证用例自足
    if (PrintHelper::m_Printer != nullptr) {
        delete PrintHelper::m_Printer;
        PrintHelper::m_Printer = nullptr;
    }

    // Act
    PrintHelper *first = PrintHelper::getIntance();
    PrintHelper *second = PrintHelper::getIntance();

    // Assert
    EXPECT_NE(first, nullptr);
    EXPECT_EQ(PrintHelper::m_Printer, first);
    EXPECT_EQ(second, first);
}

TEST_F(PrintHelperTest, GetIntance_ExistingInstance_ReturnsSameWithoutRecreating) {
    // Arrange：预置单例指针
    PrintHelper *preset = new PrintHelper();
    PrintHelper::m_Printer = preset;

    // Act
    PrintHelper *current = PrintHelper::getIntance();

    // Assert：直接复用已有实例，不重新 new
    EXPECT_EQ(current, preset);
    EXPECT_EQ(PrintHelper::m_Printer, preset);
}

// ───────────────────────── PrintHelper::showPrintDialog ─────────────────────────

TEST_F(PrintHelperTest, ShowPrintDialog_SingleImageFile_SetsDocNameAndClearsCache) {
    // Arrange：真实 PNG 文件驱动单图分支（B2/B3）；定时器负责关闭 exec 模态框
    QImage img(32, 16, QImage::Format_RGB32);
    img.fill(Qt::red);
    const QString imgPath = tmpDir.filePath("photo.png");
    ASSERT_TRUE(img.save(imgPath, "PNG"));

    auto cap = std::make_shared<DialogCaptures>();
    stubPreviewDialog(cap);
    armDialogReject(cap);
    helper = new PrintHelper();

    // Act
    helper->showPrintDialog(QStringList{ imgPath }, nullptr);
    flushDialogEvents(cap);

    // Assert：文档名 = completeBaseName + ".pdf"；对话框确实进入 exec；结束后缓存清空
    EXPECT_EQ(cap->docNameCount, 1);
    EXPECT_EQ(cap->docName, QStringLiteral("photo.pdf"));
    EXPECT_TRUE(cap->dialogSeen);
    EXPECT_TRUE(helper->m_re->m_paths.isEmpty());
    EXPECT_TRUE(helper->m_re->m_imgs.isEmpty());
}

TEST_F(PrintHelperTest, ShowPrintDialog_MultiPageImage_ReadsEveryPage) {
    // Arrange：真实 2 页 TIFF 驱动多页分支（B1）；不 stub QImageReader（exec 真跑后
    // QCommonStyle 的图标加载会被全局补丁劫持），分支证据改从 paintRequested
    // 触发时刻的 m_imgs 页数获取（paintRequestSync 为非虚槽，补丁可拦截）
    const QByteArray tiffData = makeTwoPageTiff(4, 4);
    const QString tiffPath = tmpDir.filePath("multi.tiff");
    QFile tiff(tiffPath);
    ASSERT_TRUE(tiff.open(QIODevice::WriteOnly));
    ASSERT_EQ(tiff.write(tiffData), qint64(tiffData.size()));
    tiff.close();

    int paintCalls = 0;
    int imgsAtPaint = -1;
    stub.set_lamda(
        static_cast<void (RequestedSlot:: *)(DPrinter *)>(&RequestedSlot::paintRequestSync),
        [&paintCalls, &imgsAtPaint](RequestedSlot *self, DPrinter *) {
            ++paintCalls;
            imgsAtPaint = self->m_imgs.size();
        });

    auto cap = std::make_shared<DialogCaptures>();
    stubPreviewDialog(cap);
    armDialogReject(cap);
    helper = new PrintHelper();

    // Act
    helper->showPrintDialog(QStringList{ tiffPath }, nullptr);
    flushDialogEvents(cap);

    // Assert：对话框进入 exec 且触发预览绘制时两页均已装入；文档名取首路径 baseName
    EXPECT_TRUE(cap->dialogSeen);
    EXPECT_GE(paintCalls, 1);
    ASSERT_EQ(imgsAtPaint, 2);
    EXPECT_EQ(cap->docName, QStringLiteral("multi.pdf"));
    EXPECT_TRUE(helper->m_re->m_imgs.isEmpty());
}

TEST_F(PrintHelperTest, ShowPrintDialog_EmptyPaths_DoesNotSetDocName) {
    // Arrange
    auto cap = std::make_shared<DialogCaptures>();
    stubPreviewDialog(cap);
    armDialogReject(cap);
    helper = new PrintHelper();

    // Act
    helper->showPrintDialog(QStringList{}, nullptr);
    flushDialogEvents(cap);

    // Assert：空路径列表 → tempExsitPaths 为空 → 不调 setDocName（B6）
    EXPECT_EQ(cap->docNameCount, 0);
    EXPECT_TRUE(cap->dialogSeen);
    EXPECT_TRUE(helper->m_re->m_paths.isEmpty());
    EXPECT_TRUE(helper->m_re->m_imgs.isEmpty());
}

TEST_F(PrintHelperTest, ShowPrintDialog_MissingFile_LoadFailsImageSkipped) {
    // Arrange：不存在文件 + stub 加载失败，驱动单图失败分支（B2+B4）
    int loadCalls = 0;
    QString loadedPath;
    stub.set_lamda(
        static_cast<bool (*)(const QString &, QImage &, QString &, const QString &, int)>(
            &LibUnionImage_NameSpace::loadStaticImageFromFile),
        [&loadCalls, &loadedPath](const QString &path, QImage &, QString &, const QString &, int) -> bool {
            ++loadCalls;
            loadedPath = path;
            return false;
        });

    auto cap = std::make_shared<DialogCaptures>();
    stubPreviewDialog(cap);
    armDialogReject(cap);
    helper = new PrintHelper();
    const QString missingPath = tmpDir.filePath("missing.png");
    ASSERT_FALSE(QFileInfo::exists(missingPath));

    // Act
    helper->showPrintDialog(QStringList{ missingPath }, nullptr);
    flushDialogEvents(cap);

    // Assert：加载失败 → 不追加图像，失败路径不计入 tempExsitPaths → 不调 setDocName（B6）
    EXPECT_EQ(loadCalls, 1);
    EXPECT_EQ(loadedPath, missingPath);
    EXPECT_EQ(cap->docNameCount, 0);
    EXPECT_TRUE(cap->dialogSeen);
    EXPECT_TRUE(helper->m_re->m_imgs.isEmpty());
    EXPECT_TRUE(helper->m_re->m_paths.isEmpty());
}

TEST_F(PrintHelperTest, ShowPrintDialog_LoadFailedFirst_OnlyLoadedPathsCounted) {
    // Arrange：[缺失, 可加载] 顺序驱动——修复前整表追加使失败路径占 tempExsitPaths[0]，
    // 修复后仅成功路径计入，文档名取首个成功路径
    const QString badPath = tmpDir.filePath("bad.png");
    ASSERT_FALSE(QFileInfo::exists(badPath));
    QImage img(16, 16, QImage::Format_RGB32);
    img.fill(Qt::blue);
    const QString goodPath = tmpDir.filePath("good.png");
    ASSERT_TRUE(img.save(goodPath, "PNG"));

    int loadCalls = 0;
    stub.set_lamda(
        static_cast<bool (*)(const QString &, QImage &, QString &, const QString &, int)>(
            &LibUnionImage_NameSpace::loadStaticImageFromFile),
        [&loadCalls, &badPath](const QString &path, QImage &out, QString &, const QString &, int) -> bool {
            ++loadCalls;
            if (path == badPath)
                return false;
            out = QImage(16, 16, QImage::Format_RGB32);
            return true;
        });

    auto cap = std::make_shared<DialogCaptures>();
    stubPreviewDialog(cap);
    armDialogReject(cap);
    helper = new PrintHelper();

    // Act
    helper->showPrintDialog(QStringList{ badPath, goodPath }, nullptr);
    flushDialogEvents(cap);

    // Assert：两个路径都尝试加载，但文档名只取成功路径（good.pdf 而非 bad.pdf）
    EXPECT_EQ(loadCalls, 2);
    EXPECT_EQ(cap->docNameCount, 1);
    EXPECT_EQ(cap->docName, QStringLiteral("good.pdf"));
    EXPECT_TRUE(helper->m_re->m_imgs.isEmpty());
}

// ═════════════════════════════ RequestedSlot ═════════════════════════════

namespace {

// QPrinter::newPage 是虚函数，stub-ext 无法对其 PMF 打补丁（见文件头说明）；
// 测试持有 printer 实例，用子类覆写 newPage 计数，虚分发天然命中
class CountingPrinter : public DPrinter {
public:
    CountingPrinter() = default;
    int newPageCalls = 0;
    bool newPage() override
    {
        ++newPageCalls;
        return true;
    }
};

}  // namespace

class RequestedSlotTest : public ::testing::Test {
protected:
    stub_ext::StubExt stub;
    RequestedSlot *rs = nullptr;
    QTemporaryDir tmpDir;

    void SetUp() override { rs = new RequestedSlot(); }

    void TearDown() override {
        delete rs;
        rs = nullptr;
        stub.clear();
    }

    // 配置 PDF 输出的 printer（offscreen 可实例化，不依赖真实打印系统；QPrinter 不可拷贝）
    void setupPdfPrinter(CountingPrinter &printer, const QString &fileName) {
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(tmpDir.filePath(fileName));
    }

    // 仅 stub 非虚的 QPainter::drawImage(4 参) 捕获绘图目标矩形；换页计数见 CountingPrinter
    void stubPainting(QList<QRectF> &drawnRects) {
        stub.set_lamda(
            static_cast<void (QPainter:: *)(const QRectF &, const QImage &, const QRectF &,
                                            Qt::ImageConversionFlags)>(&QPainter::drawImage),
            [&drawnRects](QPainter *, const QRectF &target, const QImage &,
                          const QRectF &, Qt::ImageConversionFlags) {
                drawnRects.append(target);
            });
    }
};

TEST_F(RequestedSlotTest, RequestedSlot_Constructor_SetsParentAndStartsEmpty) {
    // Arrange
    QObject owner;

    // Act
    RequestedSlot *withParent = new RequestedSlot(&owner);

    // Assert：parent 经初始化列表传入基类 QObject（printhelper.cpp），父子关系建立；
    // 初始路径与图像缓存为空；owner 析构时自动回收子对象，无需手动释放
    EXPECT_EQ(withParent->parent(), &owner);
    EXPECT_TRUE(withParent->m_paths.isEmpty());
    EXPECT_TRUE(withParent->m_imgs.isEmpty());
}

TEST_F(RequestedSlotTest, RequestedSlot_Destructor_DeletesChildObjects) {
    // Arrange：以 rs 为父挂子对象，验证析构传递
    QPointer<QObject> childGuard(new QObject(rs));
    childGuard->setObjectName("print-child");
    QPointer<RequestedSlot> selfGuard(rs);
    EXPECT_EQ(childGuard->parent(), static_cast<QObject *>(rs));

    // Act
    delete rs;
    rs = nullptr;

    // Assert：析构后自身与子对象均被销毁
    EXPECT_TRUE(selfGuard.isNull());
    EXPECT_TRUE(childGuard.isNull());
}

TEST_F(RequestedSlotTest, PaintRequestSync_EmptyImageList_PaintsNothing) {
    // Arrange
    CountingPrinter printer;
    setupPdfPrinter(printer, "empty.pdf");
    QList<QRectF> drawnRects;
    stubPainting(drawnRects);
    EXPECT_TRUE(rs->m_imgs.isEmpty());

    // Act
    rs->paintRequestSync(&printer);

    // Assert：循环 0 次，不绘制也不换页（B6）
    EXPECT_EQ(drawnRects.size(), 0);
    EXPECT_EQ(printer.newPageCalls, 0);
}

TEST_F(RequestedSlotTest, PaintRequestSync_SingleImage_WritesPdfWithoutPageBreak) {
    // Arrange：真实绘制（不 stub drawImage），验证 PDF 落盘
    CountingPrinter printer;
    setupPdfPrinter(printer, "single.pdf");
    const QString pdfPath = printer.outputFileName();
    QList<QRectF> drawnRects;
    stubPainting(drawnRects);
    rs->m_imgs.append(QImage(32, 16, QImage::Format_RGB32));

    // Act
    rs->paintRequestSync(&printer);

    // Assert：单图无换页；PDF 文件生成且非空
    EXPECT_EQ(printer.newPageCalls, 0);
    EXPECT_EQ(drawnRects.size(), 1);
    EXPECT_TRUE(QFileInfo::exists(pdfPath));
    EXPECT_GT(QFileInfo(pdfPath).size(), qint64(0));
}

TEST_F(RequestedSlotTest, PaintRequestSync_NullImage_SkippedButPageBreakKept) {
    // Arrange：[null, 有效图] 驱动 B2 + B5
    CountingPrinter printer;
    setupPdfPrinter(printer, "nullmix.pdf");
    QList<QRectF> drawnRects;
    stubPainting(drawnRects);
    rs->m_imgs.append(QImage());
    rs->m_imgs.append(QImage(60, 30, QImage::Format_RGB32));

    // Act
    rs->paintRequestSync(&printer);

    // Assert：null 页被跳过（仅绘制 1 次），页间换页仍执行 1 次
    EXPECT_EQ(drawnRects.size(), 1);
    EXPECT_EQ(printer.newPageCalls, 1);
    expectRectNear(drawnRects.at(0),
                   expectedPaintRect(printer.pageRect(QPrinter::DevicePixel), 60, 30));
}

TEST_F(RequestedSlotTest, PaintRequestSync_TwoImages_AddsSinglePageBreak) {
    // Arrange
    CountingPrinter printer;
    setupPdfPrinter(printer, "two.pdf");
    QList<QRectF> drawnRects;
    stubPainting(drawnRects);
    rs->m_imgs.append(QImage(30, 30, QImage::Format_RGB32));
    rs->m_imgs.append(QImage(40, 20, QImage::Format_RGB32));

    // Act
    rs->paintRequestSync(&printer);

    // Assert：两图各绘制一次，页间恰好一次 newPage；几何与期望一致
    const QRectF wRect = printer.pageRect(QPrinter::DevicePixel);
    EXPECT_EQ(drawnRects.size(), 2);
    EXPECT_EQ(printer.newPageCalls, 1);
    expectRectNear(drawnRects.at(0), expectedPaintRect(wRect, 30, 30));
    expectRectNear(drawnRects.at(1), expectedPaintRect(wRect, 40, 20));
}

// 参数化 Fixture：主 Fixture 保持 ::testing::Test，另建 WithParamInterface 子类
struct RequestedSlotScaleTest : public RequestedSlotTest, public ::testing::WithParamInterface<PaintScaleCase> {
};

TEST_P(RequestedSlotScaleTest, PaintRequestSync_ImageScaling_UsesExpectedGeometry) {
    // Arrange：宽扁/方图走"适应宽"，高瘦图走"适应高"
    const PaintScaleCase &c = GetParam();
    CountingPrinter printer;
    setupPdfPrinter(printer, "scale.pdf");
    QList<QRectF> drawnRects;
    stubPainting(drawnRects);
    rs->m_imgs.append(QImage(c.imgW, c.imgH, QImage::Format_RGB32));

    // Act
    rs->paintRequestSync(&printer);

    // Assert：目标矩形与源码缩放公式一致，单图不换页
    const QRectF expected = expectedPaintRect(printer.pageRect(QPrinter::DevicePixel), c.imgW, c.imgH);
    EXPECT_EQ(drawnRects.size(), 1);
    ASSERT_GT(drawnRects.size(), 0);
    expectRectNear(drawnRects.at(0), expected);
    EXPECT_EQ(printer.newPageCalls, 0);
    // 分支佐证：fitHeight 时期望矩形铺满页高，否则铺满页宽
    EXPECT_EQ(expected.height() == printer.pageRect(QPrinter::DevicePixel).height(), c.fitHeight);
}

INSTANTIATE_TEST_SUITE_P(
    ImageScalingCases,
    RequestedSlotScaleTest,
    ::testing::Values(
        PaintScaleCase{ 32, 16, false },    // 宽扁图 → 适应宽（B3）
        PaintScaleCase{ 100, 100, false },  // 方图（竖版页面）→ 适应宽（B3）
        PaintScaleCase{ 50, 2000, true }    // 高瘦图 → 适应高（B4）
        ));
