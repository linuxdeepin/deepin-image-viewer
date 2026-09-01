// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 测试对象：src/src/unionimage/baseutils.cpp 中 Libutils::base 命名空间内的
// 14 个自由函数（字符串/日期工具、剪贴板、回收站、文件管理器联动等）。
// 环境隔离：
//   - stub QStandardPaths::writableLocation -> QTemporaryDir（HomeLocation 回收站路径隔离）
//   - stub Libutils::image::removeThumbnail -> 计数器（trashFile 出向依赖隔离）
//   - stub QDesktopServices::openUrl -> 计数器（真实打开文件管理器隔离）
//   - 所有文件输入均由程序构造并落盘到 QTemporaryDir；错误分支用坏文件/不存在路径
//
// 用例计数声明（min 按 level/factors 推导：low=1, mid=2, high=3；
//               alloc_in_loop/loop 因子 +1；TEST_P 的 actual 计参数组数）
// | method                   | level | factors                        | min | actual |
// |--------------------------|-------|--------------------------------|-----|--------|
// | SpliteText               | high  | complexity:6,recursive,in_degree:3 | 3 | 6    |
// | copyImageToClipboard     | mid   | alloc_in_loop:2                | 3   | 3      |
// | getFileContent           | low   | -                              | 1   | 2      |
// | getNotExistsTrashFileName| mid   | complexity:5                   | 2   | 4      |
// | hash                     | mid   | in_degree:4                    | 2   | 3      |
// | mountDeviceExist         | low   | -                              | 1   | 2      |
// | onMountDevice            | low   | -                              | 1   | 4      |
// | renderSVG                | low   | -                              | 1   | 2      |
// | showInFileManager        | low   | -                              | 1   | 3      |
// | stringHeight             | low   | -                              | 1   | 1      |
// | stringToDateTime         | low   | -                              | 1   | 3      |
// | stringWidth              | low   | -                              | 1   | 1      |
// | timeToString             | low   | -                              | 1   | 2      |
// | trashFile                | high  | complexity:7,lines:81,in_degree:3 | 3 | 4    |
//
// 最小清单（test-types.md §8）：
// [x] 1  每个函数 ≥ 1 用例（14/14）
// [x] 2  等价类划分：空输入/不存在路径/坏文件/正常输入/超长输入
// [x] 3  边界值显式覆盖：明显不超宽（+1000 余量）、首字符越界、后缀 200 字符截断、
//        循环 0/1/N 次（剪贴板路径列表空/单/双）
// [x] 4  同质 ≥3 组输入用 TEST_P（SpliteText/hash/onMountDevice/stringToDateTime 共 4 组）
// [x] 5  分支清单已列出并映射到用例名（见下方各「分支清单」段）
// [x] 6  if/循环/early-return 分支有触发用例（例外：trashFile 的 infoFile.open
//        失败与 rename 失败分支需权限注入，见该段说明）
// [x] 7  无显式 throw，异常路径不适用（Qt 返回值式错误处理）
// [x] 8  负面场景（空路径列表/不存在文件/坏 SVG/非法日期串）有专门用例
// [x] 9  负面用例验证状态：missing 源文件不写 trashinfo、不触发 removeThumbnail
// [x] 10 Qt 类/项目内自由函数用 stub_ext，无 gMock 适用场景
//
// 分支清单（来源：baseutils.cpp 自由函数 SpliteText）
// B1: nTextSize > nLabelSize → 进入切分；否则原样返回（含明显富余的不切分侧）
// B2: for 循环累计字符宽度，nOffset >= nLabelSize → 记录 nPos 并 break
// B3: nPos-1 < 0 → nPos 归 0（首字符即超宽）
// B4: bReturn=true 且 qstrLeftData != "" → 空格替换 \n 后拼接递归（顶层无分隔符，
//     且递归按 3 参调用走 bReturn 默认值=false → 深层仍插 \n）
//     → 无空格长文本无词边界，按宽度硬切在词中插 \n（D4 缺陷：无法整词换行）
// B5: bReturn=false 且 qstrLeftData != "" → left + "\n" + 递归
// B6: qstrLeftData == ""（nPos==0）→ 落到末尾 return text 原样返回
//
// 用例映射：
// - SpliteText_VariousInputs_MatchExpectedSplitContract（TEST_P 6 组）→ B1~B6
//   （Fits→B1 假侧；FirstCharExceeds→B3+B6；NoSpace/Spaces×Return 组合覆盖 B2/B4/B5；
//    断言取不变量：切分只插 \n 不丢字符，期望不依赖测试端字体度量）
//
// 分支清单（来源：baseutils.cpp 自由函数 getNotExistsTrashFileName）
// B1: fileName 含 '/' → 截取末段 basename
// B2: 含 '.' → 提取后缀（含点）
// B3: suffix.size() > 200 → 截断为 200
// B4: name.left(200 - suffix.size()) 主名长度截断
// B5: while 首候选不存在 → 直接返回；存在 → 主名取 Md5 十六进制重试
//
// 用例映射：
// - GetNotExistsTrashFileName_PlainNamesNoConflict_ReturnNamesUnchanged     → B5(直返)/B2(无点)
// - GetNotExistsTrashFileName_PathLikeInput_StripsDirectoryPart            → B1+B2
// - GetNotExistsTrashFileName_ExistingCandidate_FallsBackToMd5Name         → B5(md5 重试)
// - GetNotExistsTrashFileName_OverlongSuffix_TruncatesToTwoHundredChars    → B3+B4
//
// 分支清单（来源：baseutils.cpp 自由函数 trashFile）
// B1: trashFilesPath 不存在 → mkpath；B2: trashInfoPath 不存在 → mkpath
// B3: originalInfo.exists() 为假 → return false（早退）
// B4: while 候选 info/files 路径已存在 → nr++ 生成 baseName.nr[.suffix] 重试
// B5: infoFile.open(WriteOnly) 失败 → return false（需权限注入，未覆盖）
// B6: rename 失败 → return false（需跨设备/权限注入，未覆盖）
// B7: 成功 → 写 trashinfo + rename + removeThumbnail + return true
//
// 用例映射：
// - TrashFile_MissingSource_ReturnsFalseWithoutSideEffects  → B1+B2+B3
// - TrashFile_ValidFile_MovesToTrashAndWritesInfo           → B7
// - TrashFile_NameCollision_RetriesWithNumberedName         → B4（含无后缀变体）
// - 未覆盖：B5/B6（需把 info 目录改为只读/制造 rename 失败，CI 环境无法稳定注入）
//
// 分支清单（来源：baseutils.cpp 自由函数 copyImageToClipboard）
// B1: for 遍历 paths（0/1/N 次循环边界）
// B2: !path.isEmpty() → 拼接 text；空串路径跳过 text 但仍进 dataUrls
// B3: text 以 '\n' 结尾 → 去尾，否则原样
//
// 用例映射：
// - CopyImageToClipboard_TwoPaths_FillsAllMimeFormats       → B1(N=2)/B2/B3
// - CopyImageToClipboard_EmptyList_ProducesEmptyTextAndUrls → B1(0 次)
// - CopyImageToClipboard_SinglePath_TextHasNoTrailingNewline→ B1(1 次)/B3(假)
//
// 分支清单（来源：baseutils.cpp 自由函数 mountDeviceExist）
// B1: path 以 "/media/" 开头 → 提取挂载点
// B2: path 以 "/run/media/" 开头 → 提取挂载点
// B3: 均不命中 → mountPoint 为空串
// B4: QFileInfo(mountPoint).exists() 决定返回值
//
// 用例映射：
// - MountDeviceExist_NonMountPath_ReturnsFalseForEmptyMountPoint → B3/B4(假)
// - MountDeviceExist_MediaPath_QueriesExtractedMountPoint        → B1/B4(桩真)
//
// 分支清单（来源：baseutils.cpp 自由函数 showInFileManager）
// B1: path 为空 → 早退不打开
// B2: !QFile::exists(path) → 早退不打开
// B3: 有效 → QDesktopServices::openUrl（源码连续调用两次，疑似缺陷只标红不修）
//
// 用例映射：
// - ShowInFileManager_EmptyPath_SkipsOpenUrl            → B1
// - ShowInFileManager_NonExistingPath_SkipsOpenUrl      → B2
// - ShowInFileManager_ExistingFile_OpensAbsoluteUrlTwice→ B3
//
// 分支清单（来源：baseutils.cpp 自由函数 renderSVG）
// B1: reader.canRead() → 按 scaledSize*size*ratio 渲染并设 devicePixelRatio
// B2: canRead 假 → pixmap.load 兜底（坏文件 → 空 pixmap）
//
// 用例映射：
// - RenderSVG_ValidPng_ReturnsNonNullScaledPixmap   → B1
// - RenderSVG_UnreadableFile_ReturnsNullPixmap      → B2
//
// 分支清单（来源：baseutils.cpp 自由函数 timeToString）
// B1: normalFormat=true → DATETIME_FORMAT_NORMAL
// B2: normalFormat=false → DATETIME_FORMAT_EXIF
// 映射：TimeToString_FormatFlag_SelectsCorrespondingFormat → B1+B2
//
// 分支清单（来源：baseutils.cpp 自由函数 stringToDateTime）
// B1: EXIF 格式解析成功 → 直接返回
// B2: EXIF 失败 → 回退 DATETIME_FORMAT_NORMAL；两次均失败 → 无效 QDateTime
// 映射：StringToDateTime_VariousInputs_ReturnsParsedDateTime（TEST_P 3 组）→ B1/B2
//
// 分支清单（来源：baseutils.cpp 自由函数 onMountDevice）
// B1/B2: startsWith("/media/") / startsWith("/run/media/") 短路两侧
// 映射：OnMountDevice_VariousPaths_ReturnsExpectedFlag（TEST_P 4 组）→ B1/B2/均假
//
// 分支清单（来源：baseutils.cpp 自由函数 getFileContent）
// B1: open 成功 → 读全部内容；B2: 失败 → 返回空串
// 映射：GetFileContent_ExistingFile_ReturnsExactContent → B1；
//       GetFileContent_MissingAndEmptyFiles_ReturnEmptyString → B2 + B1(空文件)

#include <gtest/gtest.h>

#include <QByteArray>
#include <QClipboard>
#include <QCryptographicHash>
#include <QDate>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QImage>
#include <QMimeData>
#include <QPixmap>
#include <QSize>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTime>
#include <QUrl>

#include "baseutils.h"
#include "imageutils.h"   // Libutils::image::removeThumbnail（trashFile 出向依赖，SetUp 桩掉）
#include "stub_ext/stubext.h"

namespace lb = Libutils::base;

// baseutils.h 可能未导出以下自由函数声明（定义位于 baseutils.cpp 的
// Libutils::base 命名空间内），此处按源码签名镜像声明，供本 TU 调用（仅声明不定义；
// 与头文件已有声明重复时为合法重复声明）
namespace Libutils {
namespace base {
QString SpliteText(const QString &text, const QFont &font, int nLabelSize, bool bReturn);
void copyImageToClipboard(const QStringList &paths);
QString getFileContent(const QString &file);
QString getNotExistsTrashFileName(const QString &fileName);
QString hash(const QString &str);
bool mountDeviceExist(const QString &path);
bool onMountDevice(const QString &path);
QPixmap renderSVG(const QString &filePath, const QSize &size);
void showInFileManager(const QString &path);
int stringHeight(const QFont &f, const QString &str);
QDateTime stringToDateTime(const QString &time);
int stringWidth(const QFont &f, const QString &str);
QString timeToString(const QDateTime &time, bool normalFormat);
bool trashFile(const QString &file);
}  // namespace base
}  // namespace Libutils

namespace {

// 构造纯色小图并落盘 png，返回绝对路径
QString makeSolidPng(const QString &dir, const QString &name, int w, int h)
{
    QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::red);
    const QString path = QDir(dir).filePath(name);
    img.save(path, "png");
    return path;
}

// 写入原始字节的坏文件，返回绝对路径
QString writeRawFile(const QString &dir, const QString &name, const QByteArray &bytes)
{
    const QString path = QDir(dir).filePath(name);
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(bytes);
    f.close();
    return path;
}

// 写文本文件，返回绝对路径
QString writeTextFile(const QString &dir, const QString &name, const char *content)
{
    return writeRawFile(dir, name, QByteArray(content));
}

struct Md5Case {
    QByteArray input;
    const char *expected;
};

struct MountFlagCase {
    QString path;
    bool expected;
};

struct StrToDtCase {
    enum Kind { ExifFormatted, NormalFormatted, Garbage };
    Kind kind;
};

// INSTANTIATE 参数值在静态初始化期求值（早于 fixture/stub 生效），字体度量相关
// 期望必须在测试体内构造，参数只携带"输入种类"
struct SpliteCase {
    enum Kind { Fits, FirstCharExceeds, NoSpaceReturnFalse, NoSpaceReturnTrue,
                SpacesReturnTrue, SpacesReturnFalse };
    Kind kind;
};

}  // namespace

class FreeBaseUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        m_removeThumbCalls = 0;
        m_removeThumbArg.clear();
        // HomeLocation -> 临时目录，隔离 getNotExistsTrashFileName/trashFile 的回收站路径
        stub.set_lamda(
                static_cast<QString (*)(QStandardPaths::StandardLocation)>(
                        &QStandardPaths::writableLocation),
                [this](QStandardPaths::StandardLocation) -> QString {
                    return m_home.path();
                });
        // trashFile 出向依赖：真实 removeThumbnail 会触碰缩略图缓存，桩为计数器
        stub.set_lamda(&Libutils::image::removeThumbnail,
                       [this](const QString &path) -> void {
                           ++m_removeThumbCalls;
                           m_removeThumbArg = path;
                       });
    }

    void TearDown() override
    {
        // 剪贴板为进程级全局状态，用后清空防止污染其他用例
        QGuiApplication::clipboard()->clear();
        stub.clear();
    }

    QTemporaryDir m_home;     // 充当隔离 HOME（回收站 .local/share/Trash 落在此）
    QTemporaryDir m_work;     // 被测文件的工作目录
    stub_ext::StubExt stub;
    int m_removeThumbCalls = 0;
    QString m_removeThumbArg;
};

// ── TEST_P 子 Fixture（brief 坑 #1：主 Fixture 保持 ::testing::Test）──────────

struct SpliteTextParamTest : public FreeBaseUtilsTest,
                             public ::testing::WithParamInterface<SpliteCase> {};

struct HashParamTest : public FreeBaseUtilsTest,
                       public ::testing::WithParamInterface<Md5Case> {};

struct OnMountParamTest : public FreeBaseUtilsTest,
                          public ::testing::WithParamInterface<MountFlagCase> {};

struct StrToDtParamTest : public FreeBaseUtilsTest,
                          public ::testing::WithParamInterface<StrToDtCase> {};

// ═══════════════════════════════════════════════════════════════════
// ⚠️ 每个 TEST_F 必须包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════════

// ── SpliteText ────────────────────────────────────────────────────

TEST_P(SpliteTextParamTest, SpliteText_VariousInputs_MatchExpectedSplitContract)
{
    const auto &c = GetParam();

    // Arrange（标签宽度由字体度量推导，保证跨字体环境可复现）
    QFont font;
    const QFontMetrics fm(font);
    QString text;
    int labelSize = 0;
    bool bReturn = false;
    switch (c.kind) {
    case SpliteCase::Fits:
        text = QStringLiteral("abc");
        labelSize = fm.horizontalAdvance(text) + 1000;  // B1 假侧：明显富余，任何字体下必然不切分
        bReturn = false;
        break;
    case SpliteCase::FirstCharExceeds:
        text = QStringLiteral("WWWW");
        labelSize = 1;                                // 首字符即超宽 → B3/B6
        bReturn = false;
        break;
    case SpliteCase::NoSpaceReturnFalse:
    case SpliteCase::NoSpaceReturnTrue:
        text = QString(30, QChar('W'));
        labelSize = fm.horizontalAdvance(text) / 3;   // 约三行
        bReturn = (c.kind == SpliteCase::NoSpaceReturnTrue);
        break;
    case SpliteCase::SpacesReturnTrue:
    case SpliteCase::SpacesReturnFalse:
        text = QStringLiteral("WWWW WW WW WWW WW WW WWW");
        labelSize = fm.horizontalAdvance(text) / 3;
        bReturn = (c.kind == SpliteCase::SpacesReturnTrue);
        break;
    }

    // Act
    const QString result = lb::SpliteText(text, font, labelSize, bReturn);

    // Assert（不变量：不超宽原样返回；超宽切分只插 \n 不丢字符（remove('\n') 可还原）；
    //         bReturn=true 空格全部转 \n；无空格长文本被按宽度硬切在词中——D4 缺陷，
    //         期望值不依赖测试端字体度量，任何字体环境不变量恒成立）
    switch (c.kind) {
    case SpliteCase::Fits:
    case SpliteCase::FirstCharExceeds:
        EXPECT_EQ(result, text);                                     // B1 假 / B3+B6 原样返回
        EXPECT_FALSE(result.contains(QLatin1Char('\n')));
        break;
    case SpliteCase::NoSpaceReturnFalse:
        EXPECT_NE(result, text);                                     // B5: 已切分
        EXPECT_EQ(result.split(QLatin1Char('\n')).join(QString()), text);   // 字符不丢失
        for (const QString &line : result.split(QLatin1Char('\n')))
            EXPECT_LE(fm.horizontalAdvance(line), labelSize);        // 每行不再超宽
        break;
    case SpliteCase::NoSpaceReturnTrue:
        EXPECT_NE(result, text);                                     // 超宽输入必被硬切（D4 实际行为）
        EXPECT_EQ(QString(result).remove(QLatin1Char('\n')), text);  // D4: 硬切只插 \n，不丢字符
        for (const QString &seg : result.split(QLatin1Char('\n')))
            EXPECT_LE(fm.horizontalAdvance(seg),
                      labelSize * 2);   // 宽松上界：顶层无分隔符至多合并两段，其余段均 < label
        break;
    case SpliteCase::SpacesReturnTrue:
        EXPECT_FALSE(result.contains(QLatin1Char(' ')));             // 空格全部替换为 \n
        EXPECT_EQ(QString(result).remove(QLatin1Char('\n')),
                  QString(text).remove(QLatin1Char(' ')));           // 词不丢不重排（字符序列一致）
        EXPECT_GE(result.count(QLatin1Char('\n')),
                  text.count(QLatin1Char(' ')));                      // 每个原空格至少产出一个 \n
        break;
    case SpliteCase::SpacesReturnFalse:
        EXPECT_EQ(result.split(QLatin1Char('\n')).join(QString()), text);   // B5: 字符不丢失
        EXPECT_LE(result.count(QLatin1Char('\n')), text.count(QLatin1Char('W')));  // 分行数受字符数约束
        break;
    }
}

INSTANTIATE_TEST_SUITE_P(
        SpliteCases, SpliteTextParamTest,
        ::testing::Values(
                SpliteCase{SpliteCase::Fits},
                SpliteCase{SpliteCase::FirstCharExceeds},
                SpliteCase{SpliteCase::NoSpaceReturnFalse},
                SpliteCase{SpliteCase::NoSpaceReturnTrue},
                SpliteCase{SpliteCase::SpacesReturnTrue},
                SpliteCase{SpliteCase::SpacesReturnFalse}));

// ── copyImageToClipboard ──────────────────────────────────────────

TEST_F(FreeBaseUtilsTest, CopyImageToClipboard_TwoPaths_FillsAllMimeFormats)
{
    // Arrange
    const QString p1 = m_work.filePath("a.png");
    const QString p2 = m_work.filePath("b.jpg");
    writeTextFile(m_work.path(), "a.png", "x");
    writeTextFile(m_work.path(), "b.jpg", "y");
    QList<QUrl> expectedUrls;
    expectedUrls << QUrl::fromLocalFile(p1) << QUrl::fromLocalFile(p2);
    const QByteArray expectedGnome =
            QByteArray("copy\n") + QUrl::fromLocalFile(p1).toEncoded() +
            QByteArray("\n") + QUrl::fromLocalFile(p2).toEncoded();

    // Act
    lb::copyImageToClipboard(QStringList{ p1, p2 });

    // Assert
    const QMimeData *mime = QGuiApplication::clipboard()->mimeData();
    EXPECT_EQ(mime->text(), p1 + QLatin1Char('\n') + p2);            // B3: 去掉末尾换行
    EXPECT_EQ(mime->urls(), expectedUrls);                           // B1/B2: URL 列表完整
    EXPECT_EQ(mime->data("x-special/gnome-copied-files"), expectedGnome);  // gnome 格式
}

TEST_F(FreeBaseUtilsTest, CopyImageToClipboard_EmptyList_ProducesEmptyTextAndUrls)
{
    // Arrange
    const QStringList emptyPaths;

    // Act
    lb::copyImageToClipboard(emptyPaths);

    // Assert
    const QMimeData *mime = QGuiApplication::clipboard()->mimeData();
    EXPECT_TRUE(mime->text().isEmpty());        // B1 0 次循环：text 为空
    EXPECT_TRUE(mime->urls().isEmpty());        // URL 列表为空
    EXPECT_EQ(mime->data("x-special/gnome-copied-files"), QByteArray("copy"));  // 尾 '\n' 被移除
}

TEST_F(FreeBaseUtilsTest, CopyImageToClipboard_SinglePath_TextHasNoTrailingNewline)
{
    // Arrange
    const QString p1 = m_work.filePath("only.png");
    writeTextFile(m_work.path(), "only.png", "x");

    // Act
    lb::copyImageToClipboard(QStringList{ p1 });

    // Assert
    const QMimeData *mime = QGuiApplication::clipboard()->mimeData();
    EXPECT_EQ(mime->text(), p1);                // B1 1 次循环 + B3 假：无尾随换行
    EXPECT_EQ(mime->urls().size(), 1);
}

// ── getFileContent ────────────────────────────────────────────────

TEST_F(FreeBaseUtilsTest, GetFileContent_ExistingFile_ReturnsExactContent)
{
    // Arrange
    const QString path = writeTextFile(m_work.path(), "note.txt", "hello baseutils");

    // Act
    const QString content = lb::getFileContent(path);

    // Assert
    EXPECT_EQ(content, QStringLiteral("hello baseutils"));   // B1: 全量读出
    EXPECT_EQ(content.size(), 15);                           // 字符数精确
}

TEST_F(FreeBaseUtilsTest, GetFileContent_MissingAndEmptyFiles_ReturnEmptyString)
{
    // Arrange
    const QString missing = m_work.filePath("nope.txt");
    const QString empty = writeTextFile(m_work.path(), "empty.txt", "");

    // Act
    const QString gotMissing = lb::getFileContent(missing);
    const QString gotEmpty = lb::getFileContent(empty);

    // Assert
    EXPECT_EQ(gotMissing, QString());      // B2: 打开失败返回空串
    EXPECT_EQ(gotEmpty, QString());        // B1: 空文件内容为空
}

// ── getNotExistsTrashFileName ─────────────────────────────────────

TEST_F(FreeBaseUtilsTest, GetNotExistsTrashFileName_PlainNamesNoConflict_ReturnNamesUnchanged)
{
    // Arrange（隔离 HOME 下 Trash 无任何已有文件）
    const QString trashProbe = m_home.path() + "/.local/share/Trashphoto.jpg";
    ASSERT_FALSE(QFile::exists(trashProbe));   // 前置：候选名未被占用

    // Act
    const QString withSuffix = lb::getNotExistsTrashFileName(QStringLiteral("photo.jpg"));
    const QString noSuffix = lb::getNotExistsTrashFileName(QStringLiteral("README"));

    // Assert
    EXPECT_EQ(withSuffix, QStringLiteral("photo.jpg"));   // B5 直返 + B2 提取 .jpg
    EXPECT_EQ(noSuffix, QStringLiteral("README"));        // 无点 → 后缀为空，原样
}

TEST_F(FreeBaseUtilsTest, GetNotExistsTrashFileName_PathLikeInput_StripsDirectoryPart)
{
    // Arrange
    const QString input = QStringLiteral("/x/y/photo.jpg");

    // Act
    const QString result = lb::getNotExistsTrashFileName(input);

    // Assert
    EXPECT_EQ(result, QStringLiteral("photo.jpg"));       // B1: 截掉 '/x/y/' 目录段
    EXPECT_TRUE(result.endsWith(QStringLiteral(".jpg"))); // B2: 后缀保留
}

TEST_F(FreeBaseUtilsTest, GetNotExistsTrashFileName_ExistingCandidate_FallsBackToMd5Name)
{
    // Arrange：源码探测路径为 trashPath + name + suffix（缺少路径分隔符，见缺陷清单），
    // 在该拼接连通处预置同名文件以触发 md5 重试
    ASSERT_TRUE(QDir(m_home.path() + "/.local/share").mkpath(QStringLiteral(".")));
    writeTextFile(m_home.path() + "/.local/share", "Trashphoto.jpg", "occupied");
    const QString expectedMd5 =
            QString(QCryptographicHash::hash("photo", QCryptographicHash::Md5).toHex());

    // Act
    const QString result = lb::getNotExistsTrashFileName(QStringLiteral("photo.jpg"));

    // Assert
    EXPECT_EQ(result, expectedMd5 + QStringLiteral(".jpg"));   // B5: 主名换 md5、后缀保留
    EXPECT_NE(result, QStringLiteral("photo.jpg"));            // 冲突名不再返回
}

TEST_F(FreeBaseUtilsTest, GetNotExistsTrashFileName_OverlongSuffix_TruncatesToTwoHundredChars)
{
    // Arrange：'ab' + '.' + 250 个 'x' → 后缀超 200 触发截断，主名被截为空
    const QString input = QStringLiteral("ab.") + QString(250, QChar('x'));

    // Act
    const QString result = lb::getNotExistsTrashFileName(input);

    // Assert
    EXPECT_EQ(result.size(), 200);                        // B3+B4: 总长收敛到 200
    EXPECT_TRUE(result.startsWith(QLatin1Char('.')));     // 后缀保留点号开头
    EXPECT_EQ(result, QStringLiteral(".") + QString(199, QChar('x')));  // 精确内容
}

// ── hash ──────────────────────────────────────────────────────────

TEST_P(HashParamTest, Hash_KnownInputs_ReturnsExpectedMd5Hex)
{
    const auto &c = GetParam();

    // Arrange
    const QString input = QString::fromUtf8(c.input);

    // Act
    const QString digest = lb::hash(input);

    // Assert
    EXPECT_EQ(digest.size(), 32);                                  // md5 十六进制定长
    EXPECT_EQ(digest, QString::fromLatin1(c.expected));            // 已知向量精确匹配
}

INSTANTIATE_TEST_SUITE_P(
        HashKnownInputs, HashParamTest,
        ::testing::Values(
                Md5Case{ "", "d41d8cd98f00b204e9800998ecf8427e" },
                Md5Case{ "abc", "900150983cd24fb0d6963f7d28e17f72" },
                Md5Case{ "hello", "5d41402abc4b2a76b9719d911017c592" }));

// ── mountDeviceExist ──────────────────────────────────────────────

TEST_F(FreeBaseUtilsTest, MountDeviceExist_NonMountPath_ReturnsFalseForEmptyMountPoint)
{
    // Arrange（工作目录路径不以 /media/ 或 /run/media/ 开头）
    const QString plainDir = m_work.path();

    // Act
    const bool got = lb::mountDeviceExist(plainDir);

    // Assert
    EXPECT_EQ(got, false);                                                // B3/B4: 挂载点为空串 → 不存在
    EXPECT_EQ(lb::mountDeviceExist(QStringLiteral("media/u/dev/a.jpg")),
              false);                                                     // 相对前缀不命中
}

TEST_F(FreeBaseUtilsTest, MountDeviceExist_MediaPath_QueriesExtractedMountPoint)
{
    // Arrange：桩 QFileInfo::exists 记录探测路径并返回真
    QString probedPath;
    stub.set_lamda(static_cast<bool (QFileInfo::*)() const>(&QFileInfo::exists),
                   [&probedPath](QFileInfo *self) -> bool {
                       probedPath = self->filePath();
                       return true;
                   });

    // Act
    const bool got = lb::mountDeviceExist(QStringLiteral("/media/u/dev/img.jpg"));

    // Assert
    EXPECT_TRUE(got);                                       // B1/B4: 挂载点存在 → 真
    EXPECT_EQ(probedPath, QStringLiteral("/media/u/dev/"));  // 探测的正是提取出的挂载点
}

// ── onMountDevice ─────────────────────────────────────────────────

TEST_P(OnMountParamTest, OnMountDevice_VariousPaths_ReturnsExpectedFlag)
{
    const auto &c = GetParam();

    // Arrange
    const QString path = c.path;

    // Act
    const bool got = lb::onMountDevice(path);

    // Assert
    EXPECT_EQ(got, c.expected);                    // B1/B2/均假 三类输入
    EXPECT_EQ(got, path.startsWith(QLatin1String("/media/"))
                        || path.startsWith(QLatin1String("/run/media/")));  // 与谓词语义一致
}

INSTANTIATE_TEST_SUITE_P(
        OnMountPaths, OnMountParamTest,
        ::testing::Values(
                MountFlagCase{ QStringLiteral("/media/u/dev/a.jpg"), true },
                MountFlagCase{ QStringLiteral("/run/media/u/dev/a.jpg"), true },
                MountFlagCase{ QStringLiteral("/data/u/a.jpg"), false },
                MountFlagCase{ QStringLiteral("media/u/a.jpg"), false }));

// ── renderSVG ─────────────────────────────────────────────────────

TEST_F(FreeBaseUtilsTest, RenderSVG_ValidPng_ReturnsNonNullScaledPixmap)
{
    // Arrange
    const QString png = makeSolidPng(m_work.path(), "solid.png", 32, 32);
    const qreal ratio = qApp->devicePixelRatio();

    // Act
    const QPixmap pm = lb::renderSVG(png, QSize(8, 8));

    // Assert
    EXPECT_FALSE(pm.isNull());                                  // B1: 可读走缩放渲染
    EXPECT_EQ(pm.width(), qRound(qreal(8) * ratio));            // 尺寸 = 目标 * devicePixelRatio
    EXPECT_EQ(pm.height(), qRound(qreal(8) * ratio));
    EXPECT_DOUBLE_EQ(pm.devicePixelRatio(), ratio);             // B1: ratio 已写回
}

TEST_F(FreeBaseUtilsTest, RenderSVG_UnreadableFile_ReturnsNullPixmap)
{
    // Arrange：垃圾字节的 .svg 文件，canRead 与兜底 load 均失败
    const QString broken = writeRawFile(m_work.path(), "broken.svg",
                                        QByteArray("this is not svg at all"));

    // Act
    const QPixmap pm = lb::renderSVG(broken, QSize(8, 8));

    // Assert
    EXPECT_TRUE(pm.isNull());     // B2: 兜底 pixmap.load 失败 → 空 pixmap
    EXPECT_EQ(pm.width(), 0);
}

// ── showInFileManager ─────────────────────────────────────────────

TEST_F(FreeBaseUtilsTest, ShowInFileManager_EmptyPath_SkipsOpenUrl)
{
    // Arrange
    int openCalls = 0;
    QUrl openedUrl;
    stub.set_lamda(static_cast<bool (*)(const QUrl &)>(&QDesktopServices::openUrl),
                   [&openCalls, &openedUrl](const QUrl &url) -> bool {
                       ++openCalls;
                       openedUrl = url;
                       return true;
                   });

    // Act
    lb::showInFileManager(QString());

    // Assert
    EXPECT_EQ(openCalls, 0);                     // B1: 空路径早退
    EXPECT_TRUE(openedUrl.isEmpty());            // 未捕获任何 URL
}

TEST_F(FreeBaseUtilsTest, ShowInFileManager_NonExistingPath_SkipsOpenUrl)
{
    // Arrange
    int openCalls = 0;
    stub.set_lamda(static_cast<bool (*)(const QUrl &)>(&QDesktopServices::openUrl),
                   [&openCalls](const QUrl &) -> bool {
                       ++openCalls;
                       return true;
                   });
    const QString missing = m_work.filePath("missing.jpg");

    // Act
    lb::showInFileManager(missing);

    // Assert
    EXPECT_EQ(openCalls, 0);     // B2: 不存在路径早退，不打开文件管理器
    EXPECT_FALSE(QFile::exists(missing));
}

TEST_F(FreeBaseUtilsTest, ShowInFileManager_ExistingFile_OpensAbsoluteUrlTwice)
{
    // Arrange
    int openCalls = 0;
    QUrl openedUrl;
    stub.set_lamda(static_cast<bool (*)(const QUrl &)>(&QDesktopServices::openUrl),
                   [&openCalls, &openedUrl](const QUrl &url) -> bool {
                       ++openCalls;
                       openedUrl = url;
                       return true;
                   });
    const QString path = writeTextFile(m_work.path(), "view.txt", "v");

    // Act
    lb::showInFileManager(path);

    // Assert
    EXPECT_EQ(openCalls, 2);    // B3: 源码 #if 1 分支与其后各调用一次 openUrl（疑似缺陷，只标红）
    EXPECT_EQ(openedUrl, QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath()));
}

// ── stringHeight / stringWidth ────────────────────────────────────

TEST_F(FreeBaseUtilsTest, StringWidth_TextAndEmpty_MatchFontMetricsWidth)
{
    // Arrange
    QFont font;
    const QFontMetrics fm(font);
    const QString text = QStringLiteral("Deepin Viewer 123");

    // Act
    const int got = lb::stringWidth(font, text);
    const int gotEmpty = lb::stringWidth(font, QString());

    // Assert
    EXPECT_EQ(got, fm.boundingRect(text).width());     // 与 QFontMetrics 契约一致
    EXPECT_GT(got, 0);                                 // 非空文本宽度为正
    EXPECT_EQ(gotEmpty, fm.boundingRect(QString()).width());  // 空串边界
}

TEST_F(FreeBaseUtilsTest, StringHeight_TextAndEmpty_MatchFontMetricsHeight)
{
    // Arrange
    QFont font;
    const QFontMetrics fm(font);
    const QString text = QStringLiteral("Deepin Viewer 123");

    // Act
    const int got = lb::stringHeight(font, text);
    const int gotEmpty = lb::stringHeight(font, QString());

    // Assert
    EXPECT_EQ(got, fm.boundingRect(text).height());           // 与 QFontMetrics 契约一致
    EXPECT_GT(got, 0);                                        // 行高为正
    EXPECT_EQ(gotEmpty, fm.boundingRect(QString()).height()); // 空串边界
}

// ── timeToString ──────────────────────────────────────────────────

TEST_F(FreeBaseUtilsTest, TimeToString_FormatFlag_SelectsCorrespondingFormat)
{
    // Arrange
    const QDateTime dt(QDate(2023, 5, 7), QTime(8, 6, 5));

    // Act
    const QString normal = lb::timeToString(dt, true);
    const QString exif = lb::timeToString(dt, false);

    // Assert
    // DATETIME_FORMAT_* 为 unionimage.cpp:122-123 文件内宏（未导出），行为实证锁定：
    // EXIF 分支输出 "yyyy:MM:dd hh:mm:ss"（冒号形态，源码 unionimage.cpp:123 实证）；
    // normal 分支输出 "yyyy.MM.dd"（仅日期、点分隔，stringToDateTime 调试日志实证）；
    // 两分支可区分；EXIF 经 stringToDateTime 可完整解析回同一时刻，normal 仅保留日期
    EXPECT_EQ(exif, dt.toString(QStringLiteral("yyyy:MM:dd hh:mm:ss")));   // B2: EXIF 冒号格式
    EXPECT_EQ(normal, dt.toString(QStringLiteral("yyyy.MM.dd")));          // B1: normal 点分日期
    EXPECT_NE(normal, exif);                                               // 两分支产出可区分
    EXPECT_EQ(lb::stringToDateTime(exif), dt);                             // B2: exif 可逆
    EXPECT_EQ(lb::stringToDateTime(normal).date(), dt.date());             // B1: normal 仅日期可逆
}

TEST_F(FreeBaseUtilsTest, TimeToString_InvalidDateTime_ReturnsEmptyStringBothFormats)
{
    // Arrange
    const QDateTime invalid;

    // Act
    const QString normal = lb::timeToString(invalid, true);
    const QString exif = lb::timeToString(invalid, false);

    // Assert
    EXPECT_EQ(normal, QString());   // B1: 无效时间格式化结果为空
    EXPECT_EQ(exif, QString());     // B2: 同上
}

// ── stringToDateTime ──────────────────────────────────────────────

TEST_P(StrToDtParamTest, StringToDateTime_VariousInputs_ReturnsParsedDateTime)
{
    const auto &c = GetParam();

    // Arrange
    const QDateTime dt(QDate(2023, 5, 7), QTime(8, 6, 5));
    QString input;
    QDateTime expected;
    switch (c.kind) {
    case StrToDtCase::ExifFormatted:
        // DATETIME_FORMAT_EXIF 实证为 "yyyy:MM:dd hh:mm:ss"（unionimage.cpp:123）
        input = dt.toString(QStringLiteral("yyyy:MM:dd hh:mm:ss"));
        expected = dt;
        break;
    case StrToDtCase::NormalFormatted:
        // normal 形态经 timeToString(true) 生成（宏未导出，走模块自身序列化），走 B2 回退解析
        input = lb::timeToString(dt, true);
        expected = QDateTime(dt.date(), QTime(0, 0));  // normal 格式仅保留日期（yyyy.MM.dd 实证）
        break;
    case StrToDtCase::Garbage:
        input = QStringLiteral("not a datetime");
        expected = QDateTime();   // 两种格式均解析失败 → 无效
        break;
    }

    // Act
    const QDateTime result = lb::stringToDateTime(input);

    // Assert
    EXPECT_EQ(result.isValid(), expected.isValid());                    // 有效性一致
    EXPECT_EQ(result.toString(Qt::ISODate), expected.toString(Qt::ISODate));  // 值精确一致
}

INSTANTIATE_TEST_SUITE_P(
        StrToDtCases, StrToDtParamTest,
        ::testing::Values(
                StrToDtCase{ StrToDtCase::ExifFormatted },
                StrToDtCase{ StrToDtCase::NormalFormatted },
                StrToDtCase{ StrToDtCase::Garbage }));

// ── trashFile ─────────────────────────────────────────────────────

TEST_F(FreeBaseUtilsTest, TrashFile_MissingSource_ReturnsFalseWithoutSideEffects)
{
    // Arrange
    const QString missing = m_work.filePath("ghost.jpg");

    // Act
    const bool moved = lb::trashFile(missing);

    // Assert
    EXPECT_FALSE(moved);                                        // B3: 源不存在早退
    EXPECT_EQ(m_removeThumbCalls, 0);                           // 副作用：未触发缩略图移除
    EXPECT_TRUE(QDir(m_home.path() + "/.local/share/Trash/info")
                        .entryList(QDir::Files).isEmpty());     // 状态：未写任何 trashinfo
}

TEST_F(FreeBaseUtilsTest, TrashFile_ValidFile_MovesToTrashAndWritesInfo)
{
    // Arrange
    const QString src = writeTextFile(m_work.path(), "ut-a.jpg", "aaa");
    const QString filesPath = m_home.path() + "/.local/share/Trash/files/ut-a.jpg";
    const QString infoPath = m_home.path() + "/.local/share/Trash/info/ut-a.jpg.trashinfo";

    // Act
    const bool moved = lb::trashFile(src);

    // Assert
    EXPECT_TRUE(moved);                                   // B7: 成功入回收站
    EXPECT_FALSE(QFile::exists(src));                     // 源文件已被移走
    EXPECT_TRUE(QFile::exists(filesPath));                // 落位 files/
    EXPECT_TRUE(QFile::exists(infoPath));                 // 写入 trashinfo
    const QString infoContent = lb::getFileContent(infoPath);
    EXPECT_TRUE(infoContent.startsWith(QStringLiteral("[Trash Info]\nPath=")));  // info 头部
    EXPECT_TRUE(infoContent.contains(QFileInfo(src).absoluteFilePath()));        // Path 记录原路径
    EXPECT_TRUE(infoContent.contains(QStringLiteral("DeletionDate=")));          // 删除时间字段
    EXPECT_EQ(m_removeThumbCalls, 1);                     // 副作用：缩略图移除恰好一次
    EXPECT_EQ(m_removeThumbArg, src);                     // 参数：传入的是原路径
}

TEST_F(FreeBaseUtilsTest, TrashFile_NameCollision_RetriesWithNumberedName)
{
    // Arrange：同名文件两次入回收站，第二次应走 nr++ 重试生成 baseName.2.suffix
    const QString first = writeTextFile(m_work.path(), "ut-b.jpg", "first");
    const QString secondDir = m_work.path() + "/second";
    ASSERT_TRUE(QDir().mkpath(secondDir));
    const QString second = writeTextFile(secondDir, "ut-b.jpg", "second");

    // Act
    const bool firstMoved = lb::trashFile(first);
    const bool secondMoved = lb::trashFile(second);

    // Assert
    EXPECT_TRUE(firstMoved);    // B7: 第一次直接成功
    EXPECT_TRUE(secondMoved);   // B4: 冲突重试后成功
    EXPECT_TRUE(QFile::exists(m_home.path() + "/.local/share/Trash/files/ut-b.jpg"));
    EXPECT_TRUE(QFile::exists(m_home.path() + "/.local/share/Trash/files/ut-b.2.jpg"));   // B4 编号名
    EXPECT_TRUE(QFile::exists(m_home.path() + "/.local/share/Trash/info/ut-b.2.jpg.trashinfo"));
    EXPECT_EQ(m_removeThumbCalls, 2);   // 每次成功入站各移除一次缩略图
}

TEST_F(FreeBaseUtilsTest, TrashFile_EmptySuffixFile_KeepsNameWithoutSuffixAddition)
{
    // Arrange：无后缀文件两次入回收站，重试名应为 README.2（无后缀拼接段）
    const QString first = writeTextFile(m_work.path(), "README", "r1");
    const QString thirdDir = m_work.path() + "/third";
    ASSERT_TRUE(QDir().mkpath(thirdDir));
    const QString second = writeTextFile(thirdDir, "README", "r2");

    // Act
    const bool firstMoved = lb::trashFile(first);
    const bool secondMoved = lb::trashFile(second);

    // Assert
    EXPECT_TRUE(firstMoved);
    EXPECT_TRUE(secondMoved);
    EXPECT_TRUE(QFile::exists(m_home.path() + "/.local/share/Trash/files/README"));       // 首次原名
    EXPECT_EQ(QFile::exists(m_home.path() + "/.local/share/Trash/files/README.2"),
              true);                                                                      // B4: 无后缀编号名
    EXPECT_EQ(QFile::exists(m_home.path() + "/.local/share/Trash/files/README.2."),
              false);                                                                     // 不产生尾随点
    EXPECT_EQ(m_removeThumbCalls, 2);                                                     // 两次均触发缩略图移除
}
