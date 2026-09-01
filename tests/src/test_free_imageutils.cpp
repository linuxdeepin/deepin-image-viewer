// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

// 测试对象：src/src/unionimage/imageutils.cpp 中 unionimage(Libutils::image) 命名空间内的
// 25 个自由函数（读写图/缩略图/旋转/元信息等图像工具函数）。
// 环境隔离：
//   - stub QProcess::systemEnvironment -> XDG_CACHE_HOME 指向 QTemporaryDir（缩略图缓存隔离）
//   - stub QDir::homePath -> 指向 QTemporaryDir（保险箱/回收站路径判断隔离）
//   - 所有图片输入均由程序构造 QImage 落盘到 QTemporaryDir；错误分支用坏文件/不存在路径
//
// 用例计数声明（min 按 level/factors 推导：low=1, mid=2, high=3；TEST_P 的 actual 计参数组数）
// | method                | level | factors                                   | min | actual |
// |-----------------------|-------|-------------------------------------------|-----|--------|
// | toMd5                 | low   | -                                         | 1   | 3      |
// | size2HumanT           | mid   | complexity:6,cognitive:17                 | 2   | 9      |
// | makeVaultLocalPath    | mid   | in_degree:3                               | 2   | 3      |
// | isVaultFile           | mid   | in_degree:4                               | 2   | 4      |
// | isCanRemove           | mid   | name_pattern:isCanRemove                  | 2   | 3      |
// | imageSupportRead      | mid   | in_degree:3                               | 2   | 6      |
// | imageSupportSave      | mid   | name_pattern:imageSupportSave             | 2   | 2      |
// | imageSupportWallPaper | low   | -                                         | 1   | 2      |
// | rotate                | mid   | in_degree:6                               | 2   | 2      |
// | supportedImageFormats | low   | -                                         | 1   | 1      |
// | getAllMetaData        | low   | -                                         | 1   | 2      |
// | getCreateDateTime     | mid   | complexity:5                              | 2   | 2      |
// | getImagesInfo         | mid   | complexity:5                              | 2   | 3      |
// | getOrientation        | low   | -                                         | 1   | 2      |
// | getRotatedImage       | low   | -                                         | 1   | 3      |
// | cutSquareImage        | mid   | in_degree:3                               | 2   | 6      |
// | cachePixmap           | low   | -                                         | 1   | 2      |
// | thumbnailAttribute    | low   | -                                         | 1   | 2      |
// | thumbnailCachePath    | mid   | in_degree:3                               | 2   | 2      |
// | thumbnailPath         | mid   | complexity:5,in_degree:3                  | 2   | 3      |
// | thumbnailExist        | low   | -                                         | 1   | 1      |
// | removeThumbnail       | mid   | in_degree:3,name_pattern:removeThumbnail  | 2   | 2      |
// | generateThumbnail     | mid   | in_degree:3                               | 2   | 2      |
// | getThumbnail          | low   | -                                         | 1   | 4      |
// | scaleImage            | high  | complexity:7,lines:52,recursive,in_degree:3 | 3 | 4    |
//
// 最小清单（test-types.md §8）：
// [x] 1  每个公开函数 ≥ 1 用例（25/25）
// [x] 2  每个输入维度按等价类划分（有效图片/坏文件/缺失路径/空输入）
// [x] 3  边界值显式覆盖（1023/1024/1048575/1048576 字节档位、目录空/单/嵌套）
// [x] 4  同质 ≥3 组输入用 TEST_P（toMd5/size2HumanT/makeVaultLocalPath/isVaultFile/
//        isCanRemove/imageSupportRead/thumbnailPath 共 7 组）
// [x] 5  分支清单已列出并映射到用例名（见下方各「分支清单」段）
// [x] 6  if/switch/early-return 分支有触发用例（例外见 scaleImage 段未覆盖说明）
// [x] 7  无显式 throw，异常路径不适用（Qt 返回值式错误处理）
// [x] 8  负面场景（空 pixmap/X3F 后缀/不存在路径/坏文件/缓存缺失）有专门用例
// [x] 9  负面用例验证返回值与状态（缩略图文件仍不存在、缓存未被污染）
// [x] 10 Qt 类/全局函数用 stub_ext，无 gMock 适用场景
//
// 分支清单（来源：imageutils.cpp 自由函数 scaleImage）
// B1: !imageSupportRead(path)（X3F 后缀）→ 提前 return QImage()
// B2: !reader.canRead()（路径不存在/坏文件）→ 提前 return QImage()
// B3: !tSize.isValid() → 走 getAllMetaData("Dimension") 回退
// B4: rl.length()==2 → 元数据解析出宽高
// B5: rl.length()!=2 → tSize 保持无效（仅告警）
// B6: tImg.width()>size.width() || tImg.height()>size.height()（格式不响应 ScaledSize）→ 再缩放分支
// B7: 再缩放分支内 tImg.isNull() → return QImage()
// B8: tImg.save(tmp) 成功 → 递归 scaleImage(tmp, size)
// B9: tImg.save(tmp) 失败 → return QImage()
// B10: 短路左侧 tImg.width()>size.width() 单独为真
// B11: 短路右侧 tImg.height()>size.height() 单独为真
// B12: else（尺寸已满足）→ return tImg 正常路径
// B13: tSize.scale(KeepAspectRatio) 等比约束（缩小与放大两侧）
// B14: reader.setAutoTransform(true) 路径（EXIF 自动旋转透传）
//
// 用例映射：
// - ScaleImage_UnsupportedX3FSuffix_ReturnsNullImage               → B1
// - ScaleImage_MissingPath_ReturnsNullImage                        → B2
// - ScaleImage_ValidPng_ReturnsAspectScaledImage                   → B4(经 reader.size 直取)/B12/B13
// - ScaleImage_LargerBoundingBox_ReturnsUpscaledImage              → B13(放大侧)/B12
// - 未覆盖：B3/B5（需 canRead 为真但 reader.size() 无效的格式）、B6~B11（需不响应
//   ScaledSize 的图像格式触发"落盘 PNG 重入"递归）——本运行环境无稳定构造方式，
//   见最终汇报；递归临时文件路径为源码内建 QDir::tempPath 固定名，无法从外部注入
//
// 分支清单（来源：imageutils.cpp 自由函数 size2HumanT）
// B1: bytes < 1024 → "N B"（含 0 与 1023 边界）
// B2: bytes < 1024^2 → KB 档
// B3: KB 值为整数（qCeil==qFloor）→ "N KB"
// B4: KB 值带小数 → "N.N KB"
// B5: bytes < 1024^3 → MB 档
// B6: MB 值为整数 → "N MB"
// B7: MB 值带小数 → "N.N MB"
// B8: else → GB 档
// B9: GB 值为整数 → "N GB"
// B10: GB 值带小数 → "N.N GB"
// B11: 进位边界 1048575B = 1023.999KB 四舍五入为整数 "1024 KB"
// B12: 档位切换边界 1023B→1024B、1048575B→1048576B
// B13: 单位后缀常量（B/KB/MB/GB）
//
// 用例映射：
// - Size2HumanT_SizeBoundaries_ReturnsHumanReadableString（TEST_P 9 组）→ B1~B13
//
// 分支清单（来源：imageutils.cpp 自由函数 generateThumbnail）
// B1: lImg.isNull() 短路左侧 → 失败标记分支
// B2: nImg.isNull() 短路右侧 → 失败标记分支
// B3: 失败分支 for(keys) 写属性循环
// B4: 正常分支 for(keys) 循环写 large/normal 属性
// B5: lImg.save && nImg.save 均成功 → return true
// B6: 任一 save 失败 → return false
// B7: 失败标记 1x1 png img.save(failedP)
//
// 用例映射：
// - GenerateThumbnail_ValidPng_WritesLargeAndNormal      → B4/B5
// - GenerateThumbnail_UnreadablePath_WritesFailMarker    → B1/B3/B7
//
// 分支清单（来源：imageutils.cpp 自由函数 getThumbnail）
// B1: QFileInfo(encodePath).exists() → 返回 large 缓存 QPixmap
// B2: failEncodePath 存在 → return QPixmap()（不再生成）
// B3: !cacheOnly 短路左侧
// B4: generateThumbnail(path) 成功 → 返回生成结果
// B5: cacheOnly 或生成失败 → return QPixmap()
// B6: QMutexLocker 全局互斥（串行化路径）
// B7: 生成后从 encodePath 加载
//
// 用例映射：
// - GetThumbnail_LargeCacheHit_ReturnsCachedPixmap       → B1
// - GetThumbnail_FailMarkerPresent_ReturnsNullPixmap     → B2
// - GetThumbnail_CacheOnlyNoCache_SkipsGeneration        → B3(假)/B5
// - GetThumbnail_NoCacheAllowed_GeneratesAndReturns      → B3(真)/B4/B7
//
// 分支清单（来源：imageutils.cpp 自由函数 getCreateDateTime）
// B1: 初始 !dt.isValid()（恒真，进入元数据回退）
// B2: DateTimeOriginal 为空 → 改读 DateTimeDigitized
// B3: 两者皆空 → 取当前时间字符串
// B4: 元数据解析失败 → 回退 birthTime
// B5: birthTime 无效 → 回退 currentDateTime
// B6: 最终 return dt
//
// 用例映射：
// - GetCreateDateTime_ExistingFile_ReturnsBirthTimeFallback   → B1/B2(假)/B4
// - GetCreateDateTime_MissingFile_ReturnsCurrentTimeFallback  → B2(真)/B3/B5
//
// 分支清单（来源：imageutils.cpp 自由函数 getImagesInfo）
// B1: !recursive → 非递归 entryInfoList(QDir::Files)
// B2: 非递归 for 循环 + imageSupportRead 过滤
// B3: recursive → QDirIterator(Subdirectories) while 循环
// B4: 递归 imageSupportRead 过滤
// B5: 空目录 → 0 次循环
// B6: 非递归提前 return
// B7: 单元素/多元素循环边界
//
// 用例映射：
// - GetImagesInfo_NonRecursiveFilter_ReturnsTopLevelOnly → B1/B2/B6
// - GetImagesInfo_RecursiveTraversal_ReturnsNestedImages → B3/B4/B7
// - GetImagesInfo_EmptyDirectory_ReturnsEmptyList         → B5
//
// 分支清单（来源：imageutils.cpp 自由函数 getRotatedImage）
// B1: detectImageFormat 为空 → 默认 QImageReader
// B2: reader.canRead() → read()
// B3: 格式分支 canRead 假 → tImg = QImage(path) 兜底
// B4: readerF.canRead() 真 → read()
//
// 用例映射：
// - GetRotatedImage_ValidPng_ReturnsLoadedImage           → B4
// - GetRotatedImage_MissingPath_ReturnsNullImage          → B1/B2(假)
// - GetRotatedImage_GarbagePngFile_ReturnsNullImage       → B3
//
// 分支清单（来源：imageutils.cpp 自由函数 imageSupportRead）
// B1: suffix == "icns"（精确小写）→ true
// B2: errorList 命中（X3F 大小写不敏感）→ return false 提前退
// B3: 其余后缀 → true
// B4: 无后缀（空串）→ true
// B5: 早退 return false 路径
//
// 用例映射：
// - ImageSupportRead_VariousSuffixes_ReturnsExpectedFlag（TEST_P 6 组）→ B1~B5
//
// 分支清单（来源：imageutils.cpp 自由函数 thumbnailPath）
// B1: case ThumbNormal → /normal/<md5>.png
// B2: case ThumbLarge → /large/<md5>.png
// B3: case ThumbFail → /fail/<md5>.png
// B4: default → 空串
// B5: md5 基于 FullyEncoded URL 的 toLocal8Bit 拼接
//
// 用例映射：
// - ThumbnailPath_EachThumbnailType_ReturnsTypeSubdirPath（TEST_P 3 组）→ B1/B2/B3/B5
//
// 分支清单（来源：imageutils.cpp 自由函数 thumbnailCachePath）
// B1: 环境变量命中 XDG_CACHE_HOME（el.length()==2）→ 使用该值
// B2: 未命中 → home + "/.cache" 回退
// B3: mkpath 创建 normal/large/fail 三个子目录
//
// 用例映射：
// - ThumbnailCachePath_XdgCacheHomeSet_ReturnsIsolatedDir     → B1/B3
// - ThumbnailCachePath_XdgCacheHomeMissing_FallsBackToHome    → B2/B3
//
// 分支清单（来源：imageutils.cpp 自由函数 thumbnailExist）
// B1: 对应类型缩略图文件存在 → true
// B2: 不存在 → false
// B3: 只检查指定 type（类型隔离，不查其他类型）
//
// 用例映射：
// - ThumbnailExist_MarkerCreated_ReturnsTrueOnlyForType → B1/B2/B3
//
// 分支清单（来源：imageutils.cpp 自由函数 isVaultFile）
// B1: rootPath 以 '/' 结尾 → chop(1)
// B2: path.contains(rootPath) && path.left(6)!="search" → true
// B3: "search" 前缀路径 → 短路右侧为假 → false
//
// 用例映射：
// - IsVaultFile_VariousPaths_ReturnsExpectedFlag（TEST_P 4 组）→ B1/B2/B3
//
// 分支清单（来源：imageutils.cpp 自由函数 isCanRemove）
// B1: isVaultFile(path) || path.contains(trashPath) → false
// B2: 普通路径 → true
//
// 用例映射：
// - IsCanRemove_VariousPaths_ReturnsExpectedFlag（TEST_P 3 组）→ B1(两侧)/B2
//
// 分支清单（来源：imageutils.cpp 自由函数 makeVaultLocalPath）
// B1: base 为空 → 使用 VAULT_DECRYPT_DIR_NAME("vault_unlocked")
// B2: path 以 '/' 开头 → 不追加分隔符
// B3: path 不以 '/' 开头 → 追加 "/"
//
// 用例映射：
// - MakeVaultLocalPath_VariousInputs_ReturnsExpectedVaultPath（TEST_P 3 组）→ B1/B2/B3
//
// 分支清单（来源：imageutils.cpp 自由函数 rotate）
// B1: LibUnionImage_NameSpace::rotateImageFile 返回假 → 记录告警
// B2: 返回真 → 原样透传 true
//
// 用例映射：
// - Rotate_DelegateSucceeds_ReturnsTrue    → B2
// - Rotate_DelegateFails_ReturnsFalse      → B1
//
// 分支清单（来源：imageutils.cpp 自由函数 imageSupportWallPaper）
// B1: reader.imageCount() > 0 → 进入后缀/格式双校验
// B2: 白名单 && 后缀均命中 → true
// B3: 后缀不匹配或 imageCount==0 → false
//
// 用例映射：
// - ImageSupportWallPaper_SupportedPng_ReturnsTrueAndSuffixMismatchFalse → B1/B2/B3
// - ImageSupportWallPaper_GarbageFiles_ReturnsFalse                      → B1(假)/B3
//
// 分支清单（来源：imageutils.cpp 自由函数 thumbnailAttribute）
// B1: url.isLocalFile() → 提取本地文件属性
// B2: reader.canRead() → 写入宽高
// B3: 非 local URL → 返回空 map（TODO 分支）
//
// 用例映射：
// - ThumbnailAttribute_LocalFileUrl_ReturnsThumbKeys  → B1/B2
// - ThumbnailAttribute_RemoteUrl_ReturnsEmptyMap      → B3
//
// 分支清单（来源：imageutils.cpp 自由函数 getAllMetaData）
// B1: admMap.contains("DateTime")（新建空 map，恒假——疑似死代码）→ 走 else
// B2: else → 以 lastModified 填 DateTimeOriginal/DateTimeDigitized
//
// 用例映射：
// - GetAllMetaData_ValidPngFile_ReturnsExpectedFields   → B1(假)/B2
// - GetAllMetaData_MissingFile_ReturnsFallbackFields    → B1(假)/B2
//
// 分支清单（来源：imageutils.cpp 自由函数 cachePixmap）
// B1: QPixmapCache::find 未命中 → 从文件加载并 insert
// B2: 命中 → 直接返回缓存
//
// 用例映射：
// - CachePixmap_CacheHit_ReturnsCachedPixmapWithoutDisk → B2
// - CachePixmap_CacheMiss_LoadsFromDiskAndCaches        → B1/B2
//
// 分支清单（来源：imageutils.cpp 自由函数 supportedImageFormats）
// B1: for 遍历 unionImageSupportFormat → 每项加 "*." 前缀
//
// 用例映射：
// - SupportedImageFormats_StubbedList_ReturnsStarPrefixedEntries → B1

#include <gtest/gtest.h>

#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QMap>
#include <QPixmap>
#include <QPixmapCache>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>
#include <QtGlobal>
#include <QtMath>

#include "imageutils.h"
#include "unionimage.h"
#include "stub_ext/stubext.h"

namespace liu = Libutils::image;

// imageutils.h 未导出以下三个自由函数的声明（定义位于 imageutils.cpp 的
// Libutils::image 命名空间内），此处按源码签名镜像声明，供本 TU 调用（仅声明不定义）
namespace Libutils {
namespace image {
const QString toMd5(const QByteArray &data);
QString size2HumanT(const qlonglong bytes);
QMap<QString, QString> thumbnailAttribute(const QUrl &url);
}  // namespace image
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

struct Md5Case {
    QByteArray input;
    const char *expected;
};

struct HumanSizeCase {
    qlonglong bytes;
    QString expected;
    QString unit;
};

struct VaultPathCase {
    QString path;
    QString base;
    QString expectedSuffix;
};

struct BoolPathCase {
    QString path;
    bool expected;
};

// INSTANTIATE 的参数值在静态初始化期求值（早于 fixture 成员与 stub 生效），
// 路径必须在测试体内构造，故参数只携带"路径种类"
struct VaultFlagCase {
    enum Kind { VaultFile, PlainFile, SearchPrefixed, EmptyPath };
    Kind kind;
    bool expected;
};

struct CanRemoveCase {
    enum Kind { TrashFile, VaultFile, PlainFile };
    Kind kind;
    bool expected;
};

struct ThumbTypeCase {
    liu::ThumbnailType type;
    QString subdir;
};

}  // namespace

class FreeImageUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // XDG_CACHE_HOME -> 临时目录，隔离 thumbnailCachePath()/generateThumbnail() 等的落盘
        stub.set_lamda(static_cast<QStringList (*)()>(&QProcess::systemEnvironment),
                       [this]() -> QStringList {
                           QStringList env;
                           env << QStringLiteral("XDG_CACHE_HOME=") + m_cacheDir.path();
                           return env;
                       });
        // HOME -> 临时目录，隔离 VAULT_BASE_PATH / 回收站路径判断（已重定向到临时目录）
        stub.set_lamda(static_cast<QString (*)()>(&QDir::homePath),
                       [this]() -> QString { return m_homeDir.path(); });
    }

    void TearDown() override
    {
        for (const QString &key : std::as_const(m_pixmapCacheKeys))
            QPixmapCache::remove(key);
        m_pixmapCacheKeys.clear();
        stub.clear();
    }

    QTemporaryDir m_homeDir;
    QTemporaryDir m_cacheDir;
    QTemporaryDir m_workDir;
    stub_ext::StubExt stub;
    QStringList m_pixmapCacheKeys;
};

// ── TEST_P 子 Fixture（brief 坑 #1：主 Fixture 保持 ::testing::Test）──────────

struct ToMd5ParamTest : public FreeImageUtilsTest,
                        public ::testing::WithParamInterface<Md5Case> {};

struct Size2HumanTParamTest : public FreeImageUtilsTest,
                              public ::testing::WithParamInterface<HumanSizeCase> {};

struct MakeVaultLocalPathParamTest : public FreeImageUtilsTest,
                                     public ::testing::WithParamInterface<VaultPathCase> {};

struct IsVaultFileParamTest : public FreeImageUtilsTest,
                              public ::testing::WithParamInterface<VaultFlagCase> {};

struct IsCanRemoveParamTest : public FreeImageUtilsTest,
                              public ::testing::WithParamInterface<CanRemoveCase> {};

struct ImageSupportReadParamTest : public FreeImageUtilsTest,
                                   public ::testing::WithParamInterface<BoolPathCase> {};

struct ThumbnailPathParamTest : public FreeImageUtilsTest,
                                public ::testing::WithParamInterface<ThumbTypeCase> {};

// ── toMd5 ─────────────────────────────────────────────────────────────────────

TEST_P(ToMd5ParamTest, ToMd5_KnownInputs_ReturnsExpectedDigest)
{
    const auto &c = GetParam();

    // Arrange
    const QByteArray input = c.input;

    // Act
    const QString digest = liu::toMd5(input);

    // Assert
    EXPECT_EQ(digest.size(), 32);
    EXPECT_EQ(digest, QString::fromLatin1(c.expected));
}

INSTANTIATE_TEST_SUITE_P(
        ToMd5KnownInputs, ToMd5ParamTest,
        ::testing::Values(
                Md5Case{"", "d41d8cd98f00b204e9800998ecf8427e"},
                Md5Case{"abc", "900150983cd24fb0d6963f7d28e17f72"},
                Md5Case{"hello", "5d41402abc4b2a76b9719d911017c592"}));

// ── size2HumanT ───────────────────────────────────────────────────────────────

TEST_P(Size2HumanTParamTest, Size2HumanT_SizeBoundaries_ReturnsHumanReadableString)
{
    const auto &c = GetParam();

    // Arrange
    const qlonglong bytes = c.bytes;

    // Act
    const QString got = liu::size2HumanT(bytes);

    // Assert
    EXPECT_EQ(got, c.expected);
    EXPECT_EQ(got.section(QLatin1Char(' '), 1), c.unit);
}

INSTANTIATE_TEST_SUITE_P(
        Size2HumanTCases, Size2HumanTParamTest,
        ::testing::Values(
                HumanSizeCase{512, "512 B", "B"},
                HumanSizeCase{1023, "1023 B", "B"},
                HumanSizeCase{1024, "1 KB", "KB"},
                HumanSizeCase{1536, "1.5 KB", "KB"},
                HumanSizeCase{1048575, "1024 KB", "KB"},
                HumanSizeCase{1048576, "1 MB", "MB"},
                HumanSizeCase{1572864, "1.5 MB", "MB"},
                HumanSizeCase{1073741824, "1 GB", "GB"},
                HumanSizeCase{1610612736, "1.5 GB", "GB"}));

// ── makeVaultLocalPath ────────────────────────────────────────────────────────

TEST_P(MakeVaultLocalPathParamTest, MakeVaultLocalPath_VariousInputs_ReturnsExpectedVaultPath)
{
    const auto &c = GetParam();

    // Arrange（homePath 已被 stub 到 m_homeDir，期望值随之确定）
    const QString expected = m_homeDir.path() + "/.local/share/applications" + c.expectedSuffix;

    // Act
    const QString got = liu::makeVaultLocalPath(c.path, c.base);

    // Assert
    EXPECT_EQ(got, expected);
    EXPECT_FALSE(got.contains("//"));
}

INSTANTIATE_TEST_SUITE_P(
        MakeVaultLocalPathCases, MakeVaultLocalPathParamTest,
        ::testing::Values(
                VaultPathCase{"a.png", "", "/vault_unlocked/a.png"},
                VaultPathCase{"/b/c.png", "", "/vault_unlocked/b/c.png"},
                VaultPathCase{"x/y.png", "files", "/files/x/y.png"}));

// ── isVaultFile ───────────────────────────────────────────────────────────────

TEST_P(IsVaultFileParamTest, IsVaultFile_VariousPaths_ReturnsExpectedFlag)
{
    const auto &c = GetParam();

    // Arrange
    QString path;
    switch (c.kind) {
    case VaultFlagCase::VaultFile:
        path = liu::makeVaultLocalPath("img.png", "");
        break;
    case VaultFlagCase::PlainFile:
        path = QDir(m_workDir.path()).filePath("plain.png");
        break;
    case VaultFlagCase::SearchPrefixed:
        path = QString("search") + liu::makeVaultLocalPath("", "");
        break;
    case VaultFlagCase::EmptyPath:
        break;
    }

    // Act
    const bool got = liu::isVaultFile(path);
    const bool gotTrailingSlash = liu::isVaultFile(path + "/");

    // Assert
    EXPECT_EQ(got, c.expected);
    EXPECT_EQ(gotTrailingSlash, c.expected);
}

INSTANTIATE_TEST_SUITE_P(
        IsVaultFileCases, IsVaultFileParamTest,
        ::testing::Values(
                VaultFlagCase{VaultFlagCase::VaultFile, true},
                VaultFlagCase{VaultFlagCase::PlainFile, false},
                VaultFlagCase{VaultFlagCase::SearchPrefixed, false},
                VaultFlagCase{VaultFlagCase::EmptyPath, false}));

// ── isCanRemove ───────────────────────────────────────────────────────────────

TEST_P(IsCanRemoveParamTest, IsCanRemove_VariousPaths_ReturnsExpectedFlag)
{
    const auto &c = GetParam();

    // Arrange
    QString path;
    switch (c.kind) {
    case CanRemoveCase::TrashFile:
        // 回收站路径（homePath 已重定向到临时目录，仅做字符串包含判断，不落盘）
        path = QDir(m_homeDir.path()).filePath(".local/share/Trash/junk.png");
        break;
    case CanRemoveCase::VaultFile:
        path = liu::makeVaultLocalPath("v.png", "");
        break;
    case CanRemoveCase::PlainFile:
        path = QDir(m_workDir.path()).filePath("normal.png");
        break;
    }

    // Act
    const bool got = liu::isCanRemove(path);
    const bool gotTrailingSlash = liu::isCanRemove(path + "/");

    // Assert
    EXPECT_EQ(got, c.expected);
    EXPECT_EQ(gotTrailingSlash, c.expected);
}

INSTANTIATE_TEST_SUITE_P(
        IsCanRemoveCases, IsCanRemoveParamTest,
        ::testing::Values(
                CanRemoveCase{CanRemoveCase::TrashFile, false},
                CanRemoveCase{CanRemoveCase::VaultFile, false},
                CanRemoveCase{CanRemoveCase::PlainFile, true}));

// ── imageSupportRead ──────────────────────────────────────────────────────────

TEST_P(ImageSupportReadParamTest, ImageSupportRead_VariousSuffixes_ReturnsExpectedFlag)
{
    const auto &c = GetParam();

    // Arrange
    const QString path = c.path;

    // Act
    const bool got = liu::imageSupportRead(path);

    // Assert
    EXPECT_EQ(got, c.expected);
    EXPECT_EQ(c.expected, QFileInfo(path).suffix().toUpper() != QString("X3F"));
}

INSTANTIATE_TEST_SUITE_P(
        ImageSupportReadCases, ImageSupportReadParamTest,
        ::testing::Values(
                BoolPathCase{"a.icns", true},
                BoolPathCase{"dir/b.png", true},
                BoolPathCase{"c.X3F", false},
                BoolPathCase{"d.x3f", false},
                BoolPathCase{"/path/no_suffix", true},
                BoolPathCase{"e.ICNS", true}));

// ── imageSupportSave（委托 LibUnionImage_NameSpace::canSave）──────────────────

TEST_F(FreeImageUtilsTest, ImageSupportSave_DelegateTrue_ReturnsTrueAndForwards)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "save.png", 4, 4);
    int calls = 0;
    QString seenPath;
    stub.set_lamda(&LibUnionImage_NameSpace::canSave,
                   [&](const QString &p) -> bool {
                       ++calls;
                       seenPath = p;
                       return true;
                   });

    // Act
    const bool got = liu::imageSupportSave(path);

    // Assert
    EXPECT_TRUE(got);  // branch: canSave 返回 true 透传
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(seenPath, path);
}

TEST_F(FreeImageUtilsTest, ImageSupportSave_DelegateFalse_ReturnsFalse)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "nosave.png", 4, 4);
    int calls = 0;
    stub.set_lamda(&LibUnionImage_NameSpace::canSave,
                   [&](const QString &) -> bool {
                       ++calls;
                       return false;
                   });

    // Act
    const bool got = liu::imageSupportSave(path);

    // Assert
    EXPECT_FALSE(got);  // branch: canSave 返回 false 透传
    EXPECT_EQ(calls, 1);
}

// ── imageSupportWallPaper ─────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, ImageSupportWallPaper_SupportedPng_ReturnsTrueAndSuffixMismatchFalse)
{
    // Arrange
    const QString png = makeSolidPng(m_workDir.path(), "wall.png", 10, 10);
    const QString wrongSuffix = m_workDir.filePath("wall_as.txt");
    QFile::copy(png, wrongSuffix);  // png 字节 + txt 后缀

    // Act
    const bool okSuffix = liu::imageSupportWallPaper(png);
    const bool badSuffix = liu::imageSupportWallPaper(wrongSuffix);

    // Assert
    EXPECT_EQ(QImageReader(png).imageCount(), 1);  // 前置交叉验证：可解码
    EXPECT_TRUE(okSuffix);   // branch: 后缀与格式均在白名单
    EXPECT_FALSE(badSuffix); // branch: 内容为 png 但后缀不在白名单
}

TEST_F(FreeImageUtilsTest, ImageSupportWallPaper_GarbageFiles_ReturnsFalse)
{
    // Arrange
    const QString junkPng = writeRawFile(m_workDir.path(), "junk.png", QByteArray("not an image at all"));
    const QString junkTxt = writeRawFile(m_workDir.path(), "junk2.txt", QByteArray("\x89PNGoops"));

    // Act
    const bool gotPng = liu::imageSupportWallPaper(junkPng);
    const bool gotTxt = liu::imageSupportWallPaper(junkTxt);

    // Assert（canRead 失败时 imageCount() 按文档返回 -1，仍满足源码 imageCount()>0 为假）
    EXPECT_EQ(QImageReader(junkPng).imageCount(), -1);  // 前置交叉验证：不可解码
    EXPECT_FALSE(gotPng);  // branch: imageCount()==0
    EXPECT_FALSE(gotTxt);
}

// ── rotate（委托 LibUnionImage_NameSpace::rotateImageFile）────────────────────

TEST_F(FreeImageUtilsTest, Rotate_DelegateSucceeds_ReturnsTrue)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "rot.png", 8, 4);
    int calls = 0;
    int seenDegree = 0;
    QString seenPath;
    stub.set_lamda(&LibUnionImage_NameSpace::rotateImageFile,
                   [&](int angel, const QString &p, QString &erroMsg, const QString &) -> bool {
                       ++calls;
                       seenDegree = angel;
                       seenPath = p;
                       erroMsg = "ok";
                       return true;
                   });

    // Act
    const bool got = liu::rotate(path, 90);

    // Assert
    EXPECT_TRUE(got);  // branch: 委托成功透传 true
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(seenDegree, 90);
    EXPECT_EQ(seenPath, path);
}

TEST_F(FreeImageUtilsTest, Rotate_DelegateFails_ReturnsFalse)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "rotfail.png", 8, 4);
    int calls = 0;
    stub.set_lamda(&LibUnionImage_NameSpace::rotateImageFile,
                   [&](int, const QString &, QString &erroMsg, const QString &) -> bool {
                       ++calls;
                       erroMsg = "rotate failed";
                       return false;
                   });

    // Act
    const bool got = liu::rotate(path, 45);

    // Assert
    EXPECT_FALSE(got);  // branch: 委托失败走告警分支并返回 false
    EXPECT_EQ(calls, 1);
}

// ── supportedImageFormats ─────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, SupportedImageFormats_StubbedList_ReturnsStarPrefixedEntries)
{
    // Arrange
    stub.set_lamda(&LibUnionImage_NameSpace::unionImageSupportFormat,
                   []() -> const QStringList {
                       return QStringList{"PNG", "JPG", "ICO"};
                   });

    // Act
    const QStringList got = liu::supportedImageFormats();

    // Assert
    EXPECT_EQ(got, (QStringList{"*.PNG", "*.JPG", "*.ICO"}));
    EXPECT_EQ(got.size(), 3);
    EXPECT_TRUE(got.first().startsWith("*."));
}

// ── getAllMetaData ────────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, GetAllMetaData_ValidPngFile_ReturnsExpectedFields)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "meta.png", 8, 4);

    // Act
    const QMap<QString, QString> got = liu::getAllMetaData(path);

    // Assert
    EXPECT_EQ(got.value("Dimension"), QString("8x4"));
    EXPECT_EQ(got.value("FileName"), QString("meta.png"));
    EXPECT_EQ(got.value("FileFormat"), QString("png"));
    EXPECT_EQ(got.value("FileSize"), liu::size2HumanT(QFileInfo(path).size()));
    EXPECT_TRUE(got.contains("DateTimeOriginal"));
}

TEST_F(FreeImageUtilsTest, GetAllMetaData_MissingFile_ReturnsFallbackFields)
{
    // Arrange
    const QString path = m_workDir.filePath("missing.png");

    // Act
    const QMap<QString, QString> got = liu::getAllMetaData(path);

    // Assert
    EXPECT_EQ(got.value("Dimension"), QString("-1x-1"));  // reader.size() 无效 → -1x-1
    EXPECT_EQ(got.value("FileSize"), QString("0 B"));
    // QFileInfo::fileName() 是纯路径解析，文件不存在仍返回末段文件名
    EXPECT_EQ(got.value("FileName"), QString("missing.png"));
}

// ── getCreateDateTime ─────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, GetCreateDateTime_ExistingFile_ReturnsBirthTimeFallback)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "born.png", 4, 4);
    const QDateTime birthTime = QFileInfo(path).birthTime();

    // Act
    const QDateTime got = liu::getCreateDateTime(path);

    // Assert（元数据格式不匹配解析失败 → 回退文件 birthTime）
    EXPECT_TRUE(got.isValid());
    EXPECT_GT(got.toSecsSinceEpoch(), qint64(1577836800));  // 晚于 2020-01-01
    if (birthTime.isValid())
        EXPECT_EQ(got, birthTime);
}

TEST_F(FreeImageUtilsTest, GetCreateDateTime_MissingFile_ReturnsCurrentTimeFallback)
{
    // Arrange
    const QString path = m_workDir.filePath("no_such_file.png");

    // Act
    const QDateTime got = liu::getCreateDateTime(path);

    // Assert（元数据与 birthTime 均无效 → 回退当前时间）
    EXPECT_TRUE(got.isValid());
    EXPECT_GT(got.toSecsSinceEpoch(), qint64(1577836800));  // 晚于 2020-01-01
}

// ── getImagesInfo ─────────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, GetImagesInfo_NonRecursiveFilter_ReturnsTopLevelOnly)
{
    // Arrange
    makeSolidPng(m_workDir.path(), "a.png", 4, 4);
    writeRawFile(m_workDir.path(), "b.X3F", QByteArray("x"));
    QDir().mkpath(m_workDir.filePath("sub"));
    const QString nested = makeSolidPng(m_workDir.filePath("sub"), "c.png", 4, 4);
    ASSERT_TRUE(QFileInfo::exists(nested));

    // Act
    const QFileInfoList got = liu::getImagesInfo(m_workDir.path(), false);

    // Assert（X3F 被过滤、子目录不进入）
    EXPECT_EQ(got.size(), 1);
    EXPECT_EQ(got.first().fileName(), QString("a.png"));
}

TEST_F(FreeImageUtilsTest, GetImagesInfo_RecursiveTraversal_ReturnsNestedImages)
{
    // Arrange
    makeSolidPng(m_workDir.path(), "a.png", 4, 4);
    QDir().mkpath(m_workDir.filePath("sub"));
    makeSolidPng(m_workDir.filePath("sub"), "c.png", 4, 4);

    // Act
    const QFileInfoList got = liu::getImagesInfo(m_workDir.path(), true);

    // Assert
    EXPECT_EQ(got.size(), 2);
    QStringList names;
    for (const QFileInfo &info : got)
        names << info.fileName();
    EXPECT_TRUE(names.contains("a.png"));
    EXPECT_TRUE(names.contains("c.png"));
}

TEST_F(FreeImageUtilsTest, GetImagesInfo_EmptyDirectory_ReturnsEmptyList)
{
    // Arrange
    const QString emptyDir = m_workDir.filePath("empty");
    QDir().mkpath(emptyDir);

    // Act
    const QFileInfoList flat = liu::getImagesInfo(emptyDir, false);
    const QFileInfoList deep = liu::getImagesInfo(emptyDir, true);

    // Assert（0 次循环边界）
    EXPECT_TRUE(flat.isEmpty());
    EXPECT_EQ(deep.size(), 0);
}

// ── getOrientation ────────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, GetOrientation_StubbedBackend_ReturnsDelegatedValue)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "ori.png", 4, 4);
    int calls = 0;
    QString seenPath;
    stub.set_lamda(&LibUnionImage_NameSpace::getOrientation,
                   [&](const QString &p) -> int {
                       ++calls;
                       seenPath = p;
                       return 6;
                   });

    // Act
    const int got = liu::getOrientation(path);

    // Assert
    EXPECT_EQ(got, 6);
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(seenPath, path);
}

TEST_F(FreeImageUtilsTest, GetOrientation_RealPng_ReturnsDefaultOrientation)
{
    // Arrange（真实实现为常量返回 1）
    const QString png = makeSolidPng(m_workDir.path(), "ori_real.png", 4, 4);
    const QString missing = m_workDir.filePath("ori_missing.png");

    // Act
    const int gotPng = liu::getOrientation(png);
    const int gotMissing = liu::getOrientation(missing);

    // Assert
    EXPECT_EQ(gotPng, 1);
    EXPECT_EQ(gotMissing, 1);
}

// ── getRotatedImage ───────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, GetRotatedImage_ValidPng_ReturnsLoadedImage)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "plain.png", 8, 4);

    // Act
    const QImage got = liu::getRotatedImage(path);

    // Assert
    EXPECT_FALSE(got.isNull());
    EXPECT_EQ(got.size(), QSize(8, 4));
}

TEST_F(FreeImageUtilsTest, GetRotatedImage_MissingPath_ReturnsNullImage)
{
    // Arrange
    const QString path = m_workDir.filePath("ghost.png");

    // Act
    const QImage got = liu::getRotatedImage(path);

    // Assert
    EXPECT_TRUE(got.isNull());
    EXPECT_EQ(got.width(), 0);
}

TEST_F(FreeImageUtilsTest, GetRotatedImage_GarbagePngFile_ReturnsNullImage)
{
    // Arrange
    const QString path = writeRawFile(m_workDir.path(), "broken.png",
                                      QByteArray("this is not really a png payload"));

    // Act
    const QImage got = liu::getRotatedImage(path);

    // Assert
    EXPECT_TRUE(got.isNull());
    EXPECT_EQ(got.height(), 0);
}

// ── cutSquareImage ────────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, CutSquareImage_WidePixmap_ReturnsCenteredSquare)
{
    // Arrange
    QPixmap src(30, 10);
    src.fill(Qt::blue);

    // Act
    const QPixmap got = liu::cutSquareImage(src, QSize(10, 10));

    // Assert（KeepAspectRatioByExpanding + 中心裁剪 → 10x10）
    EXPECT_FALSE(got.isNull());
    EXPECT_EQ(got.width(), 10);
    EXPECT_EQ(got.height(), 10);
}

TEST_F(FreeImageUtilsTest, CutSquareImage_SquarePixmap_ReturnsRequestedSize)
{
    // Arrange
    QPixmap src(40, 40);
    src.fill(Qt::green);

    // Act
    const QPixmap got = liu::cutSquareImage(src, QSize(20, 20));

    // Assert
    EXPECT_FALSE(got.isNull());
    EXPECT_EQ(got.size(), QSize(20, 20));
}

TEST_F(FreeImageUtilsTest, CutSquareImage_NullPixmap_ReturnsNullResult)
{
    // Arrange
    QPixmap src;

    // Act
    const QPixmap got = liu::cutSquareImage(src, QSize(10, 10));

    // Assert
    EXPECT_TRUE(got.isNull());
    EXPECT_EQ(got.width(), 0);
}

// ── cutSquareImage 单参重载（补测：lcov FNDA:0，直接调用真实函数）───────────────
// 实现（imageutils.cpp:170-174）：cutSquareImage(pixmap) → cutSquareImage(pixmap, pixmap.size())
// 缺陷注记（只标红不修改）：单参重载名承诺“方形”，实际透传原始（可能非方的）尺寸。

TEST_F(FreeImageUtilsTest, CutSquareImage_SingleArgSquarePixmap_ReturnsSameSquareSize)
{
    // Arrange：24x24 方图（单参重载以 pixmap.size() 为目标）
    QPixmap src(24, 24);
    src.fill(Qt::cyan);

    // Act
    const QPixmap got = liu::cutSquareImage(src);

    // Assert：方图输入输出等尺寸正方形
    EXPECT_FALSE(got.isNull());
    EXPECT_EQ(got.size(), QSize(24, 24));
}

TEST_F(FreeImageUtilsTest, CutSquareImage_SingleArgWidePixmap_ReturnsFullInputSize)
{
    // Arrange：30x12 宽图——单参重载透传 pixmap.size()（30x12，非正方形）
    QPixmap src(30, 12);
    src.fill(Qt::magenta);

    // Act
    const QPixmap got = liu::cutSquareImage(src);

    // Assert：真实行为固化——输出与输入同尺寸，未被裁为 min(w,h) 正方形（见缺陷注记）
    EXPECT_FALSE(got.isNull());
    EXPECT_EQ(got.size(), QSize(30, 12));
}

TEST_F(FreeImageUtilsTest, CutSquareImage_SingleArgNullPixmap_ReturnsNull)
{
    // Arrange：空图（负面输入）
    QPixmap src;

    // Act
    const QPixmap got = liu::cutSquareImage(src);

    // Assert
    EXPECT_TRUE(got.isNull());
    EXPECT_EQ(got.width(), 0);
}

// ── cachePixmap ───────────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, CachePixmap_CacheHit_ReturnsCachedPixmapWithoutDisk)
{
    // Arrange（键对应的文件并不存在，命中缓存即证明未读盘）
    const QString key = m_workDir.filePath("never_on_disk.png");
    QPixmap cached(5, 5);
    cached.fill(Qt::red);
    ASSERT_TRUE(QPixmapCache::insert(key, cached));
    m_pixmapCacheKeys << key;

    // Act
    const QPixmap got = liu::cachePixmap(key);

    // Assert
    EXPECT_EQ(got.width(), 5);
    EXPECT_EQ(got.height(), 5);
}

TEST_F(FreeImageUtilsTest, CachePixmap_CacheMiss_LoadsFromDiskAndCaches)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "loadme.png", 7, 3);
    QPixmapCache::remove(path);
    m_pixmapCacheKeys << path;

    // Act
    const QPixmap first = liu::cachePixmap(path);
    const QPixmap second = liu::cachePixmap(path);  // 第二次走缓存命中

    // Assert
    EXPECT_EQ(first.width(), 7);
    EXPECT_EQ(first.height(), 3);
    EXPECT_EQ(second.width(), 7);
    EXPECT_EQ(second.height(), 3);
}

// ── thumbnailCachePath ────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, ThumbnailCachePath_XdgCacheHomeSet_ReturnsIsolatedDir)
{
    // Arrange（SetUp 已 stub XDG_CACHE_HOME -> m_cacheDir）
    const QString cacheBase = m_cacheDir.path();

    // Act
    const QString got = liu::thumbnailCachePath();

    // Assert
    EXPECT_EQ(got, cacheBase + "/thumbnails");
    EXPECT_TRUE(QDir(m_cacheDir.filePath("thumbnails/large")).exists());
    EXPECT_TRUE(QDir(m_cacheDir.filePath("thumbnails/normal")).exists());
    EXPECT_TRUE(QDir(m_cacheDir.filePath("thumbnails/fail")).exists());
}

TEST_F(FreeImageUtilsTest, ThumbnailCachePath_XdgCacheHomeMissing_FallsBackToHome)
{
    // Arrange（局部覆盖 systemEnvironment stub：无 XDG_CACHE_HOME）
    stub_ext::StubExt localStub;
    localStub.set_lamda(static_cast<QStringList (*)()>(&QProcess::systemEnvironment),
                        []() -> QStringList { return QStringList(); });

    // Act
    const QString got = liu::thumbnailCachePath();

    // Assert（homePath 已重定向到 m_homeDir → 回退到 m_homeDir/.cache）
    EXPECT_EQ(got, m_homeDir.filePath(".cache/thumbnails"));
    EXPECT_TRUE(QDir(m_homeDir.filePath(".cache/thumbnails/large")).exists());
    localStub.clear();
}

// ── thumbnailAttribute ────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, ThumbnailAttribute_LocalFileUrl_ReturnsThumbKeys)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "attr.png", 8, 4);
    const QUrl url = QUrl::fromLocalFile(path);

    // Act
    const QMap<QString, QString> got = liu::thumbnailAttribute(url);

    // Assert
    EXPECT_EQ(got.value("Software"), QString("Deepin Image Viewer"));
    EXPECT_EQ(got.value("Thumb::Image::Width"), QString("8"));
    EXPECT_EQ(got.value("Thumb::Image::Height"), QString("4"));
    EXPECT_EQ(got.value("Thumb::Mimetype"), QString("image/png"));
    EXPECT_EQ(got.value("Thumb::Size"), QString::number(QFileInfo(path).size()));
    EXPECT_EQ(got.value("Thumb::MTime"),
              QString::number(QFileInfo(path).lastModified().toSecsSinceEpoch()));
    EXPECT_EQ(got.value("Thumb::URI"), url.toString());
}

TEST_F(FreeImageUtilsTest, ThumbnailAttribute_RemoteUrl_ReturnsEmptyMap)
{
    // Arrange
    const QUrl url(QStringLiteral("https://example.com/a.png"));

    // Act
    const QMap<QString, QString> got = liu::thumbnailAttribute(url);

    // Assert
    EXPECT_TRUE(got.isEmpty());
    EXPECT_EQ(got.size(), 0);
}

// ── thumbnailPath ─────────────────────────────────────────────────────────────

TEST_P(ThumbnailPathParamTest, ThumbnailPath_EachThumbnailType_ReturnsTypeSubdirPath)
{
    const auto &c = GetParam();

    // Arrange
    const QString img = makeSolidPng(m_workDir.path(), "tp.png", 4, 4);
    const QString md5 = liu::toMd5(
            QUrl::fromLocalFile(img).toString(QUrl::FullyEncoded).toLocal8Bit());
    const QString base = liu::thumbnailCachePath();

    // Act
    const QString got = liu::thumbnailPath(img, c.type);

    // Assert
    EXPECT_EQ(got, base + "/" + c.subdir + "/" + md5 + ".png");
    EXPECT_EQ(QFileInfo(got).dir().dirName(), c.subdir);
}

INSTANTIATE_TEST_SUITE_P(
        ThumbnailPathCases, ThumbnailPathParamTest,
        ::testing::Values(
                ThumbTypeCase{liu::ThumbNormal, "normal"},
                ThumbTypeCase{liu::ThumbLarge, "large"},
                ThumbTypeCase{liu::ThumbFail, "fail"}));

// ── thumbnailExist ────────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, ThumbnailExist_MarkerCreated_ReturnsTrueOnlyForType)
{
    // Arrange
    const QString img = makeSolidPng(m_workDir.path(), "exists.png", 4, 4);
    liu::thumbnailCachePath();  // 确保 normal/large/fail 子目录存在
    const QString largePath = liu::thumbnailPath(img, liu::ThumbLarge);
    QFile::copy(img, largePath);

    // Act
    const bool before = liu::thumbnailExist(img, liu::ThumbNormal);
    const bool after = liu::thumbnailExist(img, liu::ThumbLarge);

    // Assert（只检查指定类型）
    EXPECT_FALSE(before);
    EXPECT_NE(before, after);
    EXPECT_EQ(QFileInfo(largePath).dir().dirName(), QString("large"));
    EXPECT_TRUE(after);
}

// ── removeThumbnail ───────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, RemoveThumbnail_MarkersExist_RemovesAllTypeFiles)
{
    // Arrange
    const QString img = makeSolidPng(m_workDir.path(), "rm.png", 4, 4);
    liu::thumbnailCachePath();
    const QString largePath = liu::thumbnailPath(img, liu::ThumbLarge);
    const QString normalPath = liu::thumbnailPath(img, liu::ThumbNormal);
    const QString failPath = liu::thumbnailPath(img, liu::ThumbFail);
    QFile::copy(img, largePath);
    QFile::copy(img, normalPath);
    QFile::copy(img, failPath);
    EXPECT_TRUE(QFileInfo::exists(largePath));

    // Act
    liu::removeThumbnail(img);

    // Assert
    int existingAfter = 0;
    for (const QString &p : {largePath, normalPath, failPath}) {
        if (QFileInfo::exists(p))
            ++existingAfter;
    }
    EXPECT_EQ(existingAfter, 0);
    EXPECT_FALSE(QFileInfo::exists(largePath));
}

TEST_F(FreeImageUtilsTest, RemoveThumbnail_NoMarkers_KeepsAbsentState)
{
    // Arrange
    const QString img = makeSolidPng(m_workDir.path(), "rm_none.png", 4, 4);
    liu::thumbnailCachePath();

    // Act
    liu::removeThumbnail(img);

    // Assert（无文件时移除为空操作，不产生副作用）
    EXPECT_EQ(liu::thumbnailExist(img, liu::ThumbLarge), false);
    EXPECT_EQ(liu::thumbnailExist(img, liu::ThumbNormal), false);
}

// ── generateThumbnail ─────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, GenerateThumbnail_ValidPng_WritesLargeAndNormal)
{
    // Arrange
    const QString img = makeSolidPng(m_workDir.path(), "gen.png", 100, 60);

    // Act
    const bool got = liu::generateThumbnail(img);

    // Assert
    const QString largePath = liu::thumbnailPath(img, liu::ThumbLarge);
    const QString normalPath = liu::thumbnailPath(img, liu::ThumbNormal);
    EXPECT_TRUE(got);
    EXPECT_TRUE(QFileInfo::exists(largePath));
    EXPECT_TRUE(QFileInfo::exists(normalPath));
    const QImage large(largePath);
    EXPECT_EQ(large.width(), liu::THUMBNAIL_MAX_SIZE);
    EXPECT_LE(qAbs(large.width() * 60 - large.height() * 100), 100);  // 保持 100:60 宽高比
    EXPECT_EQ(large.text("Software"), QString("Deepin Image Viewer"));
    const QImage normal(normalPath);
    EXPECT_EQ(normal.width(), liu::THUMBNAIL_NORMAL_SIZE);
}

TEST_F(FreeImageUtilsTest, GenerateThumbnail_UnreadablePath_WritesFailMarker)
{
    // Arrange
    const QString missing = m_workDir.filePath("gen_missing.png");

    // Act
    const bool got = liu::generateThumbnail(missing);

    // Assert（scaleImage 读失败 → 1x1 失败标记）
    EXPECT_FALSE(got);
    EXPECT_EQ(QFileInfo::exists(liu::thumbnailPath(missing, liu::ThumbFail)), true);
    EXPECT_EQ(QFileInfo::exists(liu::thumbnailPath(missing, liu::ThumbLarge)), false);
}

// ── getThumbnail ──────────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, GetThumbnail_LargeCacheHit_ReturnsCachedPixmap)
{
    // Arrange
    const QString img = makeSolidPng(m_workDir.path(), "hit.png", 4, 4);
    liu::thumbnailCachePath();
    QImage thumb(10, 5, QImage::Format_ARGB32_Premultiplied);
    thumb.fill(Qt::cyan);
    thumb.save(liu::thumbnailPath(img, liu::ThumbLarge), "png");

    // Act
    const QPixmap got = liu::getThumbnail(img, false);

    // Assert
    EXPECT_FALSE(got.isNull());
    EXPECT_EQ(got.width(), 10);
    EXPECT_EQ(got.height(), 5);
}

TEST_F(FreeImageUtilsTest, GetThumbnail_FailMarkerPresent_ReturnsNullPixmap)
{
    // Arrange
    const QString img = makeSolidPng(m_workDir.path(), "failed.png", 4, 4);
    liu::thumbnailCachePath();
    QFile f(liu::thumbnailPath(img, liu::ThumbFail));
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();
    EXPECT_FALSE(liu::thumbnailExist(img, liu::ThumbLarge));

    // Act
    const QPixmap got = liu::getThumbnail(img, false);

    // Assert（失败标记存在则不再重新生成）
    EXPECT_TRUE(got.isNull());
    EXPECT_EQ(got.width(), 0);
}

TEST_F(FreeImageUtilsTest, GetThumbnail_CacheOnlyNoCache_SkipsGeneration)
{
    // Arrange
    const QString img = makeSolidPng(m_workDir.path(), "cacheonly.png", 4, 4);
    EXPECT_FALSE(liu::thumbnailExist(img, liu::ThumbLarge));

    // Act
    const QPixmap got = liu::getThumbnail(img, true);

    // Assert（cacheOnly 且无缓存 → 空 pixmap，且不触发生成）
    EXPECT_TRUE(got.isNull());
    EXPECT_EQ(liu::thumbnailExist(img, liu::ThumbLarge), false);
    EXPECT_EQ(liu::thumbnailExist(img, liu::ThumbFail), false);
}

TEST_F(FreeImageUtilsTest, GetThumbnail_NoCacheAllowed_GeneratesAndReturns)
{
    // Arrange
    const QString img = makeSolidPng(m_workDir.path(), "autogen.png", 100, 60);

    // Act
    const QPixmap got = liu::getThumbnail(img, false);

    // Assert（现场生成 large 缩略图并返回）
    EXPECT_FALSE(got.isNull());
    EXPECT_EQ(got.width(), liu::THUMBNAIL_MAX_SIZE);
    EXPECT_TRUE(liu::thumbnailExist(img, liu::ThumbLarge));
}

// ── scaleImage ────────────────────────────────────────────────────────────────

TEST_F(FreeImageUtilsTest, ScaleImage_UnsupportedX3FSuffix_ReturnsNullImage)
{
    // Arrange
    const QString path = writeRawFile(m_workDir.path(), "unsupport.X3F", QByteArray("raw"));

    // Act
    const QImage got = liu::scaleImage(path, QSize(50, 50));

    // Assert
    EXPECT_TRUE(got.isNull());
    EXPECT_EQ(got.width(), 0);
}

TEST_F(FreeImageUtilsTest, ScaleImage_MissingPath_ReturnsNullImage)
{
    // Arrange
    const QString path = m_workDir.filePath("scale_missing.png");

    // Act
    const QImage got = liu::scaleImage(path, QSize(50, 50));

    // Assert
    EXPECT_TRUE(got.isNull());
    EXPECT_EQ(got.width(), 0);
}

TEST_F(FreeImageUtilsTest, ScaleImage_ValidPng_ReturnsAspectScaledImage)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "scale.png", 100, 60);

    // Act
    const QImage got = liu::scaleImage(path, QSize(50, 50));

    // Assert（KeepAspectRatio：100x60 → 50x30）
    EXPECT_FALSE(got.isNull());
    EXPECT_EQ(got.size(), QSize(50, 30));
}

TEST_F(FreeImageUtilsTest, ScaleImage_LargerBoundingBox_ReturnsUpscaledImage)
{
    // Arrange
    const QString path = makeSolidPng(m_workDir.path(), "upscale.png", 100, 60);

    // Act
    const QImage got = liu::scaleImage(path, QSize(200, 200));

    // Assert（tSize.scale 放大到边界盒：100x60 → 200x120）
    EXPECT_FALSE(got.isNull());
    EXPECT_EQ(got.size(), QSize(200, 120));
}
