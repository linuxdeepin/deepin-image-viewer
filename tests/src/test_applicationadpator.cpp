// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | ApplicationAdaptorCtor | low | - | 1 | 2 |
// | openImageFile | mid | - | 2 | 8 |
// ─── actual 均不低于 min ───
// 注：openImageFile 8 例 = 1 TEST_F（null 控制器）+ 7 TEST_P 参数化实例。
//
// ⚠️ 环境安全约束（全量运行回归教训）：禁止在主线程 `new ApplicationAdaptor(nullptr)`
// ——Qt 6.8 该路径产生孤儿 QDBusAdaptorConnector + queued polish 毒事件（详见文件内
// 匿名命名空间注释），毒害后续所有 processEvents 驱动用例。null 场景必须走
// NullAdaptorQuarantineThread 隔离（构造用例）或 fileControl 直写（B1 分支用例）。
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（inventory 2 方法全覆盖）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（file:// URL/裸绝对路径/相对路径/空串/远程 URL × 可读性/图像性组合）
// 3. 每个等价类的边界值显式覆盖: [x]（空串输入、isCanReadable 短路、isImage 失败）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（OpenImageFile 7 组同构断言）
// 5. 分支清单 → 用例映射已列出: [x]（见下方分支清单块）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（B1-B5 全覆盖）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类无显式 throw；错误路径以返回值/信号断言覆盖）
// 8. 负面场景有专门用例: [x]（null 控制器、不可读文件、非图像文件、空串输入）
// 9. 负面用例验证强异常安全: [x]（失败路径不发射信号、不触碰图像检查短路）
// 10. stub_ext vs gMock 选择正确: [x]（项目内部 FileControl::isCanReadable/isImage 用 VADDR；
//     PrintHelper 同理；DBus 服务端 slot 直接调用断言，连接层不 stub isValid，无虚接口注入点，不适用 gMock）
//
// ─────────────────────────────────────────────────────────────
// 分支清单（来源：get_code_snippet ApplicationAdaptor::openImageFile applicationadpator.cpp:20-33 真实源码）
// ─────────────────────────────────────────────────────────────
// ApplicationAdaptor::openImageFile(const QString &fileName)
// B1: fileControl == nullptr                          → 跳过全部检查 return false
// B2: inputUrl.isLocalFile() ? fromLocalFile(toLocalFile()) : inputUrl（三元）→ file:// URL
//     解包再封包归一化；其余输入（裸路径/相对路径/空串/远程 URL）原样保留，不折叠为 file: 形态
// B3: !fileControl->isCanReadable(urlPath)（&& 短路）  → return false，isImage 不被调用
// B4: isCanReadable && !isImage                       → return false，不发射信号
// B5: isCanReadable && isImage                        → Q_EMIT openImageFile(urlPath) return true
// 映射： ApplicationAdaptor_ConstructWithController_ParentLinkedToController      → ctor
//        ApplicationAdaptor_ConstructWithNullController_NoParentAttached          → ctor(空)
//        OpenImageFile_NullController_ReturnsFalseWithoutControllerChecks         → B1
//        OpenImageFile_VariousInputs_ReturnsExpected (TEST_P)                     → B2/B3/B4/B5
//   其中参数组：file-url+ok→B2(真)+B5；裸绝对路径→B2(假)+B5（原样透传）；
//              相对路径→B2(假)+B5（原样透传）；空串→B2(假)+B5（空串往返）；
//              远程 URL→B2(假)+B5（不折叠，生产 isCanReadable 会拒绝远程）；
//              不可读→B3(短路)；可读非图像→B4

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QStringList>
#include <QThread>
#include <QUrl>

#include "stub_ext/stubext.h"

#include "applicationadpator.h"
#include "filecontrol.h"

namespace {

// openImageFile 参数化用例：DBus 入参 + 控制器桩开关 + 期望结果
struct OpenFileCase {
    QString input;             // DBus 收到的 fileName
    bool canRead;              // isCanReadable 桩返回
    bool isImageOk;            // isImage 桩返回
    bool expectedRet;          // 期望返回值
    QString expectedUrl;       // 期望流入 isCanReadable/isImage/信号的归一化 urlPath
    int expectedSignals;       // 期望 openImageFile 信号发射次数
    int expectedIsImageCalls;  // 期望 isImage 被调用次数（B3 短路时为 0）
};

// ── 毒事件隔离（全量运行 SEGV 根因）──────────────────────────────
// Qt 6.8 QDBusAbstractAdaptor(nullptr) 违反其契约（Q_ASSERT_X 非空父对象）：
//   qDBusCreateAdaptorConnector(nullptr) → new QDBusAdaptorConnector(nullptr)
//   —— 无任何父对象的孤儿 connector，进程内无法回收；
//   且 ctor 向其投递 Qt::QueuedConnection 的 polish() 元调用事件。
//   polish() 首次分发即执行 parent()->children()，parent()==nullptr → 读 0x8 偏移
//   → 后续任何 sendPostedEvents/processEvents（FileControl waitDbusCalls、
//   ImageInfo waitUntilStatus 等）分发到该事件即 SEGV（libQt6DBus）。
// 对策：null-parent 真实构造必须在"从不 exec() 的短命 QThread"内完成——
// 毒事件留在该线程事件队列中，随线程数据销毁被丢弃，永不进入主线程队列。
class NullAdaptorQuarantineThread : public QThread {
public:
    QObject *seenParent = (QObject *)quintptr(1);  // 哨兵：非 0/1 区分真实结果
    const QMetaObject *seenMetaObject = nullptr;
    bool constructedOk = false;

protected:
    void run() override {
        ApplicationAdaptor adaptor(nullptr);  // 真实 null-parent 构造路径（含 Qt 内部孤儿 connector）
        seenParent = adaptor.parent();
        seenMetaObject = adaptor.metaObject();
        constructedOk = true;
    }  // adaptor 析构；孤儿 connector 的 queued polish 留在本线程队列，本线程无事件循环
};

}  // namespace

class ApplicationAdaptorTest : public ::testing::Test {
protected:
    void SetUp() override {
        stub.clear();
        m_fileControl = new FileControl();
    }

    void TearDown() override {
        // adaptor 是 fileControl 的子对象，必须先删 adaptor 再删父对象，避免二次释放
        if (m_adaptor) {
            delete m_adaptor;
            m_adaptor = nullptr;
        }
        delete m_fileControl;
        m_fileControl = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    FileControl *m_fileControl = nullptr;
    ApplicationAdaptor *m_adaptor = nullptr;
};

// ═══════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F 包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════

// ─── 构造函数 ───

TEST_F(ApplicationAdaptorTest, ApplicationAdaptor_ConstructWithController_ParentLinkedToController)
{
    // Arrange：SetUp 已构造 FileControl，作为期望父对象
    QObject *expectedParent = m_fileControl;

    // Act
    m_adaptor = new ApplicationAdaptor(m_fileControl);

    // Assert：适配器以控制器为父对象，类型元对象正确
    ASSERT_NE(m_adaptor, nullptr);
    EXPECT_EQ(m_adaptor->parent(), expectedParent);
    EXPECT_STREQ(m_adaptor->metaObject()->className(), "ApplicationAdaptor");
}

TEST_F(ApplicationAdaptorTest, ApplicationAdaptor_ConstructWithNullController_NoParentAttached)
{
    // Arrange：null-parent 构造在主线程会遗留毒 polish 事件（见匿名命名空间注释），
    // 安排在无事件循环的短命工作线程内隔离完成
    NullAdaptorQuarantineThread quarantine;

    // Act
    quarantine.start();
    ASSERT_TRUE(quarantine.wait(30000));

    // Assert：无父对象挂载，继承链正确，构造不崩溃
    EXPECT_EQ(quarantine.seenParent, nullptr);
    ASSERT_NE(quarantine.seenMetaObject, nullptr);
    EXPECT_STREQ(quarantine.seenMetaObject->superClass()->className(), "QDBusAbstractAdaptor");
    EXPECT_TRUE(quarantine.constructedOk);
}

// ─── openImageFile ───

TEST_F(ApplicationAdaptorTest, OpenImageFile_NullController_ReturnsFalseWithoutControllerChecks)
{
    // Arrange：B1 分支——控制器未挂接。Qt 禁止 null-parent 构造 QDBusAbstractAdaptor
    // （主线程构造会遗留毒 polish 事件，见匿名命名空间注释），故用合法父对象构造后
    // 直写私有成员 fileControl（-fno-access-control）模拟空控制器，分支语义不变
    int readableCalls = 0;
    int imageCalls = 0;
    stub.set_lamda(VADDR(FileControl, isCanReadable),
                   [&readableCalls](FileControl *, const QString &) -> bool {
                       ++readableCalls;
                       return true;
                   });
    stub.set_lamda(VADDR(FileControl, isImage),
                   [&imageCalls](FileControl *, const QString &) -> bool {
                       ++imageCalls;
                       return true;
                   });
    QSignalSpy spy(m_fileControl, SIGNAL(openImageFile(QString)));
    m_adaptor = new ApplicationAdaptor(m_fileControl);
    m_adaptor->fileControl = nullptr;

    // Act
    const bool ret = m_adaptor->openImageFile("file:///virtual/pic.png");

    // Assert：直接返回 false，控制器检查与信号均未发生
    EXPECT_FALSE(ret);
    EXPECT_EQ(readableCalls, 0);
    EXPECT_EQ(imageCalls, 0);
    EXPECT_EQ(spy.count(), 0);
}

struct ApplicationAdaptorOpenFileTest : public ApplicationAdaptorTest,
                                        public ::testing::WithParamInterface<OpenFileCase> {};

TEST_P(ApplicationAdaptorOpenFileTest, OpenImageFile_VariousInputs_ReturnsExpected)
{
    // Arrange
    const OpenFileCase &c = GetParam();
    int readableCalls = 0;
    int imageCalls = 0;
    QString readableUrl;
    QString imageUrl;
    stub.set_lamda(VADDR(FileControl, isCanReadable),
                   [&](FileControl *, const QString &path) -> bool {
                       ++readableCalls;
                       readableUrl = path;
                       return c.canRead;
                   });
    stub.set_lamda(VADDR(FileControl, isImage),
                   [&](FileControl *, const QString &path) -> bool {
                       ++imageCalls;
                       imageUrl = path;
                       return c.isImageOk;
                   });
    QSignalSpy spy(m_fileControl, SIGNAL(openImageFile(QString)));
    ASSERT_TRUE(spy.isValid());
    m_adaptor = new ApplicationAdaptor(m_fileControl);

    // Act
    const bool ret = m_adaptor->openImageFile(c.input);

    // Assert：返回值 / url 归一化 / 调用次数 / 信号次数与内容全部对账
    EXPECT_EQ(ret, c.expectedRet);
    EXPECT_EQ(readableCalls, 1);
    EXPECT_EQ(readableUrl, c.expectedUrl);
    EXPECT_EQ(imageCalls, c.expectedIsImageCalls);
    if (c.expectedIsImageCalls == 1) {
        EXPECT_EQ(imageUrl, c.expectedUrl);
    }
    EXPECT_EQ(spy.count(), c.expectedSignals);
    if (c.expectedSignals == 1) {
        EXPECT_EQ(spy.takeFirst().at(0).toString(), c.expectedUrl);
    }
}

INSTANTIATE_TEST_SUITE_P(
    OpenFileVariants, ApplicationAdaptorOpenFileTest,
    ::testing::Values(
        // B2(真) + B5：file:// URL 解包再封包，url 原样往返
        OpenFileCase{"file:///virtual/pic.png", true, true, true, "file:///virtual/pic.png", 1, 1},
        // B2(假) + B5：裸绝对路径原样透传（QUrl(裸路径).isLocalFile()==false，Qt6 实测）
        OpenFileCase{"/virtual/pic.png", true, true, true, "/virtual/pic.png", 1, 1},
        // B2(假) + B5：相对路径原样透传
        OpenFileCase{"pic.png", true, true, true, "pic.png", 1, 1},
        // B2(假) + B5：空串边界——QUrl("") 往返仍为空串
        OpenFileCase{"", true, true, true, "", 1, 1},
        // B2(假) + B5：远程 URL 不再折叠为 file: 伪 URL，原样透传
        //（生产 isCanReadable 对非本地 URL 的 toLocalFile 为空，会拒绝远程）
        OpenFileCase{"http://media.host/pic.png", true, true, true, "http://media.host/pic.png", 1, 1},
        // B3：isCanReadable 短路失败，isImage 不被调用、不发信号
        OpenFileCase{"file:///virtual/pic.png", false, false, false, "file:///virtual/pic.png", 0, 0},
        // B4：可读但非图像，不发信号
        OpenFileCase{"file:///virtual/pic.png", true, false, false, "file:///virtual/pic.png", 0, 1}));
