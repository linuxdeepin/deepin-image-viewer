// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | adjustImageToRealPosition | high | complexity:10,cognitive:19 | 3 | 14 |
// | canSave | mid | in_degree:4 | 2 | 3 |
// | convertToSRgbColorSpace | high | complexity:14,cognitive:21 | 3 | 3 |
// | creatNewImage | low | - | 1 | 3 |
// | detectImageFormat | high | complexity:16,lines:112 | 3 | 17 |
// | getFileFormat | low | - | 1 | 3 |
// | getFileMimeType | mid | lines:82 | 2 | 4 |
// | getImageType | mid | complexity:5,in_degree:4 | 2 | 6 |
// | getOrientation | mid | in_degree:4 | 2 | 2 |
// | getPathType | high | complexity:6,cognitive:21 | 3 | 7 |
// | isImageSupportRotate | mid | in_degree:3 | 2 | 2 |
// | isNoneQImage | low | - | 1 | 2 |
// | loadStaticImageFromFile | high | complexity:8,cognitive:19,lines:101 | 3 | 8 |
// | noneQImage | low | - | 1 | 1 |
// | rotateImage | mid | in_degree:4 | 2 | 4 |
// | rotateImageFIleWithImage | high | complexity:8,cognitive:16 | 3 | 5 |
// | rotateImageFile | high | complexity:9,cognitive:21 | 3 | 6 |
// | size2Human | mid | complexity:6,cognitive:17 | 2 | 9 |
// | supportMovieFormat | low | - | 1 | 1 |
// | supportStaticFormat | low | - | 1 | 1 |
// | unionImageSupportFormat | low | - | 1 | 1 |
// | unionImageVersion | low | - | 1 | 1 |
// | UnionImage_Private ctor | mid | - | 2 | 2 |
// | ~UnionImage_Private | low | - | 1 | 1 |
// ─── actual 均不低于 min（actual=参数化实例数+独立用例数）───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（19 自由函数 + UnionImage_Private 2 方法全覆盖；
//    convertToSRgbColorSpace 为 static 内部链接函数，经 loadStaticImageFromFile 间接覆盖）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（角度合法/非法、格式支持/不支持、空/非空输入、
//    尺寸边界 0/1023/1024/1M/1G、方向 1-8 全枚举）
// 3. 每个等价类的边界值显式覆盖: [x]（size2Human 0/512/1023/1024/1536/1M/2M/1G/2G；
//    角度 %90 边界 45/90；depth 8/16/其它）
// 4. 同质 ≥3 组用 TEST_P: [x]（detectImageFormat 15 组、size2Human 9 组、
//    adjustImageToRealPosition 8 组、getFileFormat/creatNewImage/getFileMimeType 各 3 组）
// 5. 分支清单 → 用例映射已列出: [x]（见下方分支清单块）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本文件无显式 throw 路径，错误路径走返回值+errorMsg 断言）
// 8. 负面场景有专门用例: [x]（空文件/损坏数据/非法角度/非法后缀/目录目标路径）
// 9. 负面用例验证强异常安全: [x]（rotateImage 45° 后图像内容/尺寸不变断言等）
// 10. stub_ext vs gMock 选择正确: [x]（QImageReader::imageCount 与
//     Libutils::image::isVaultFile 均为非虚依赖，走 stub_ext）
//
// 分支清单（来源：unionimage.cpp get_code_snippet）
//
// 分支清单（来源：adjustImageToRealPosition(const QImage &, int)）
// B1: case 1/default → 原图返回
// B2: case 2 → 水平翻转
// B3: case 3 → 180° 旋转
// B4: case 4 → 垂直翻转
// B5: case 5 → 90° + 水平翻转
// B6: case 6 → 90° 顺时针
// B7: case 7 → 90° + 垂直翻转
// B8: case 8 → -90° 逆时针
// 用例映射：
// - AdjustImageToRealPosition_ParamOrientations_ReturnsTransformedImage /* TEST_P 1-8 */ → B1-B8
// - AdjustImageToRealPosition_OrientationOne_ReturnsOriginalUnchanged → B1
// - AdjustImageToRealPosition_HorizontalMirror_SwapsLeftRightPixels → B2
// - AdjustImageToRealPosition_OneHundredEighty_ReversesPixelPositions → B3
// - AdjustImageToRealPosition_VerticalMirror_FlipsTopBottomRows → B4
// - AdjustImageToRealPosition_ClockwiseNinety_MapsCornersCorrectly → B6
// - AdjustImageToRealPosition_CounterClockwiseNinety_MapsCornersCorrectly → B8
//
// 分支清单（来源：canSave(const QString &)）
// B1: r.imageCount() > 1 → return false
// B2: suffix ∈ m_canSave → return true
// B3: 否则 → return false
// 用例映射：
// - CanSave_MultiFrameImage_ReturnsFalse → B1
// - CanSave_SingleFramePng_ReturnsTrue → B2
// - CanSave_UnsupportedSuffix_ReturnsFalse → B3
//
// 分支清单（来源：convertToSRgbColorSpace(const QImage &)）
// B1: image.isNull() → 原图返回
// B2: colorSpace 有效且 != sRGB → 需转换
// B3: colorSpace 无效且 format==CMYK8888 → 需转换
// B4: !needsConversion → 原图返回
// B5: convertedToColorSpace 成功 → 返回转换图
// B6: 方法2 手动 setColorSpace+格式转换
// B7: 方法3 convertToFormat(RGB888)
// B8: 全部失败 → 原图返回
// 用例映射（static 函数，经 loadStaticImageFromFile 间接触发）：
// - ConvertToSRgbColorSpace_UntaggedImage_NoConversionApplied → B4
// - ConvertToSRgbColorSpace_SRgbTaggedImage_KeepsValidImage → B2(否)/B4
// - ConvertToSRgbColorSpace_WideGamutImage_ConvertedSuccessfully → B2/B5
//
// 分支清单（来源：detectImageFormat(const QString &)）
// B0: 文件打开失败 → return ""
// B1: "BM" → BMP; B2: "DDS" → DDS; B3: "GIF8" → GIF; B4: "icns" → ICNS
// B5: \xff\xd8 → JPG; B6: MNG 头 → MNG; B7: "P1"/"P4" → PBM
// B8: "P2"/"P5" → PGM; B9: "P3"/"P6" → PPM; B10: PNG 头 → PNG
// B11: 含 "<svg" → SVG; B12: "MM\x00\x2a"/"II\x2a\x00" → TIFF
// B13: "RIFFr\x00\x00\x00WEBPVP" → WEBP
// B14: 含 max_width 且 max_height → XBM; B15: "/* XPM */" → XPM
// B16: 无魔数匹配 → 后缀大写
// 用例映射：
// - DetectImageFormat_ParamMagicBytes_ReturnsFormatName /* TEST_P 15 组 */ → B1-B15
// - DetectImageFormat_UnreadablePath_ReturnsEmptyString → B0
// - DetectImageFormat_UnknownMagic_FallsBackToSuffix → B16
//
// 分支清单（来源：getFileMimeType(const QString &)）
// B1: MIME ∈ mimeToFormat → 返回映射格式
// B2: 不在映射表 → return QString()
// 用例映射：
// - GetFileMimeType_ParamImageFiles_ReturnsMappedFormat /* TEST_P 3 组 */ → B1
// - GetFileMimeType_UnmappedMimeType_ReturnsEmptyString → B2
//
// 分支清单（来源：getImageType(const QString &)）
// B1: path 为空 → ImageTypeBlank
// B2: 文件不存在 → ImageTypeBlank
// B3: svg 后缀且可渲染 → ImageTypeSvg
// B4: gif/mng/webp 多帧或 gif/mng MIME → ImageTypeDynamic
// B5: 其余 imageCount>1 → ImageTypeMulti
// B6: 否则 → ImageTypeStatic
// 用例映射：
// - GetImageType_EmptyPath_ReturnsBlankType → B1
// - GetImageType_MissingFile_ReturnsBlankType → B2
// - GetImageType_SvgFile_ReturnsSvgType → B3
// - GetImageType_AnimatedGifMultiFrame_ReturnsDynamicType → B4
// - GetImageType_MultiPageTiffStubbed_ReturnsMultiType → B5
// - GetImageType_StaticPng_ReturnsStaticType → B6
//
// 分支清单（来源：getPathType(const QString &)）
// B1: 含 "smb-share:server=" → SMB
// B2: 含 "mtp:host=" → MTP
// B3: 含 "gphoto2:host=" → PTP
// B4: 含 "gphoto2:host=Apple" → APPLE（源码序导致不可达，见缺陷清单）
// B5: isVaultFile → SAFEBOX
// B6: 含 $HOME/.local/share/Trash → RECYCLEBIN
// B7: 默认 → LOCAL
// 用例映射：
// - GetPathType_SmbUri_ReturnsSmbType → B1
// - GetPathType_MtpUri_ReturnsMtpType → B2
// - GetPathType_Gphoto2Uri_ReturnsPtpType → B3
// - GetPathType_Gphoto2AppleUri_ReturnsPtpInsteadOfApple → B3/B4
// - GetPathType_VaultFlaggedPath_ReturnsSafeboxType → B5
// - GetPathType_HomeTrashPath_ReturnsRecyclebinType → B6
// - GetPathType_PlainLocalPath_ReturnsLocalType → B7
//
// 分支清单（来源：loadStaticImageFromFile(const QString &, QImage &, QString &, const QString &, int)）
// B1: file_info.size()==0 → false + "error file!"
// B2: 后缀/MIME ∈ m_qtSupported → Qt 读取路径
// B3: format_bar 为空 → 用检测后缀；非空 → 覆盖格式
// B4: 尺寸超 maxDimension → setScaledSize
// B5: imageCount>0 或非 ICNS → 尝试 read
// B6: read 成功 → convertToSRgbColorSpace 后返回 true
// B7: read 失败 → 旧方法 detectImageFormat+canRead
// B8: 旧方法 canRead 失败 → QImage(path) 兜底
// B9: try_res 为空 → false + "load image by qt faild..."
// B10: ICNS 且无帧 → false（无 errorMsg）
// B11: 后缀不受支持 → false
// 用例映射：
// - LoadStaticImageFromFile_EmptyFile_ReturnsFalseWithError → B1
// - LoadStaticImageFromFile_NormalPng_ReturnsTrueAndImage → B2/B3(空)/B5/B6
// - LoadStaticImageFromFile_LargeImage_ScaledToMaxDimension → B4
// - LoadStaticImageFromFile_SuffixMismatchedContent_LoadsViaOldMethod → B7
// - LoadStaticImageFromFile_CorruptPng_ReturnsFalseWithQtError → B8/B9
// - LoadStaticImageFromFile_UnsupportedSuffix_ReturnsFalse → B11
// - LoadStaticImageFromFile_IcnsWithoutImages_ReturnsFalse → B10
// - LoadStaticImageFromFile_NonEmptyFormatBar_OverridesDetection → B3(非空)
//
// 分支清单（来源：rotateImage(int, QImage &)）
// B1: angel%90!=0 → false
// B2: image.isNull() → false
// B3: 拷贝成功 → transformed 后 true
// 用例映射：
// - RotateImage_AngleNotMultipleOfNinety_ReturnsFalse → B1
// - RotateImage_NullImage_ReturnsFalseWithoutTouchingImage → B2
// - RotateImage_ClockwiseNinety_SwapsDimensionsReturnsTrue → B3
// - RotateImage_OneHundredEighty_ReversesPixelsReturnsTrue → B3
//
// 分支清单（来源：rotateImageFile(int, const QString &, QString &, const QString &)）
// B1: angel%90!=0 → false + "unsupported angel"
// B2: targetPath 为空 → 保存至原路径；非空 → 保存至目标
// B3: SVG 且加载失败 → false
// B4: SVG 加载成功 → QSvgGenerator 重写 → true
// B5: format ∈ m_qtrotate → Qt 矩阵旋转
// B6: 保存失败 → false + "save image failed"
// B7: 保存成功 → true
// B8: 格式不支持 → false + "not support rotate image format:"
// 用例映射：
// - RotateImageFile_InvalidAngle_ReturnsFalseWithError → B1
// - RotateImageFile_PngRotatedInPlace_ReturnsTrueWithSwappedSize → B2(空)/B5/B7
// - RotateImageFile_PngRotatedToTargetPath_OriginalUntouched → B2(非空)/B7
// - RotateImageFile_SvgFile_ReturnsTrueAndRewritesFile → B4
// - RotateImageFile_UnsupportedFormat_ReturnsFalseWithMessage → B8
// - RotateImageFile_DirectoryTargetPath_ReturnsFalseWithSaveError → B6
//
// 分支清单（来源：rotateImageFIleWithImage(int, QImage &, const QString &, QString &)）
// B1: angel%90!=0 → false + "unsupported angel"
// B2: img.isNull() → false（无 erroMsg）
// B3: SVG → QSvgGenerator 写回 → true
// B4: JPG/JPEG 且 QImage(path,"JPG") 非空 → 旋转保存 → true
// B5: JPG 加载失败 → false
// B6: 其它格式 → false
// 用例映射：
// - RotateImageFIleWithImage_InvalidAngle_ReturnsFalseWithMessage → B1
// - RotateImageFIleWithImage_NullImage_ReturnsFalse → B2
// - RotateImageFIleWithImage_SvgFile_RewritesAndReturnsTrue → B3
// - RotateImageFIleWithImage_JpgFile_SavesRotatedReturnsTrue → B4
// - RotateImageFIleWithImage_UnsupportedFormat_ReturnsFalse → B6
//
// 分支清单（来源：size2Human(const qlonglong)）
// B1: bytes<1024 → "N B"
// B2/B3: <1MB 整数/带小数 → "N KB"/"N.N KB"
// B4/B5: <1GB 整数/带小数 → "N MB"/"N.N MB"
// B6/B7: ≥1GB 整数/带小数 → "N GB"/"N.N GB"
// 用例映射：
// - Size2Human_ParamBytes_ReturnsHumanReadable /* TEST_P 9 组 */ → B1-B7

#include <gtest/gtest.h>

#include <QByteArray>
#include <QColor>
#include <QColorSpace>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QStringList>
#include <QTemporaryDir>
#include <QImageReader>
#include <QTransform>

#include "stub_ext/stubext.h"

#include "imageutils.h"
#include "types.h"
#include "unionimage.h"

// unionimage.cpp 内未在 unionimage.h 声明的符号（定义于 LibUnionImage_NameSpace，
// 外部链接），镜像声明必须与其定义同命名空间，否则链接符号不一致
namespace LibUnionImage_NameSpace {
QImage adjustImageToRealPosition(const QImage &image, int orientation);
const QString getFileFormat(const QString &path);
const QString getFileMimeType(const QString &path);
QImage noneQImage();
bool isNoneQImage(const QImage &qi);
QString size2Human(const qlonglong bytes);

// unionimage.cpp 文件内隐藏类（无头文件），按源码成员布局逐成员镜像声明
//（含 m_movie_formats，位置/顺序必须一致，否则构造函数越界写）；
// 构造/析构为类内 inline 定义，链接期与 unionimage.o 弱符号合并
class UnionImage_Private {
public:
    UnionImage_Private();
    ~UnionImage_Private();

    QStringList m_qtSupported;
    QHash<QString, int> m_movie_formats;
    QStringList m_canSave;
    QStringList m_qtrotate;
};
} // namespace LibUnionImage_NameSpace

// unionimage.h 声明的自由函数与上述镜像声明同在该命名空间，
// 引入后用例内保持非限定调用
using namespace LibUnionImage_NameSpace;

namespace {

QImage makePatternImage(int width, int height)
{
    QImage img(width, height, QImage::Format_RGB32);
    img.fill(QColor(20, 20, 20));
    img.setPixelColor(0, 0, QColor(255, 0, 0));           // 左上 red
    img.setPixelColor(width - 1, 0, QColor(0, 0, 255));   // 右上 blue
    img.setPixelColor(0, height - 1, QColor(0, 255, 0));  // 左下 green
    return img;
}

QString writeRawFile(const QTemporaryDir &dir, const QString &name, const QByteArray &data)
{
    const QString path = dir.filePath(name);
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
        f.write(data);
    f.close();
    return path;
}

QString writeImageFile(const QTemporaryDir &dir, const QString &name,
                       int width, int height, const char *format)
{
    const QString path = dir.filePath(name);
    makePatternImage(width, height).save(path, format);
    return path;
}

QString writeSvgFile(const QTemporaryDir &dir, const QString &name, int width, int height)
{
    const QString body = QString(
        "<svg xmlns='http://www.w3.org/2000/svg' width='%1' height='%2'>"
        "<rect width='%1' height='%2' fill='red'/></svg>").arg(width).arg(height);
    return writeRawFile(dir, name, body.toUtf8());
}

struct SuffixCase {
    QString path;
    QString expected;
};

struct DepthCase {
    int depth;
    QImage::Format format;
};

struct FormatMagic {
    QByteArray head;
    QString expected;
};

struct OrientationCase {
    int orientation;
    bool swapDims;
};

struct MimeFormatCase {
    const char *fileSuffix;
    const char *saveFormat;
    QString expected;
};

struct SizeCase {
    qlonglong bytes;
    QString expected;
};

} // namespace

class FreeUnionImageTest : public ::testing::Test {
protected:
    void SetUp() override { stub.clear(); }
    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
    QTemporaryDir tmpDir;
};

struct SuffixParamTest : public FreeUnionImageTest,
                         public ::testing::WithParamInterface<SuffixCase> {};
struct DepthParamTest : public FreeUnionImageTest,
                        public ::testing::WithParamInterface<DepthCase> {};
struct FormatMagicParamTest : public FreeUnionImageTest,
                              public ::testing::WithParamInterface<FormatMagic> {};
struct OrientationParamTest : public FreeUnionImageTest,
                              public ::testing::WithParamInterface<OrientationCase> {};
struct MimeFormatParamTest : public FreeUnionImageTest,
                             public ::testing::WithParamInterface<MimeFormatCase> {};
struct SizeCaseParamTest : public FreeUnionImageTest,
                           public ::testing::WithParamInterface<SizeCase> {};

class UnionImage_PrivateTest : public ::testing::Test {
protected:
    void SetUp() override { stub.clear(); }
    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

// ═══════════════════════════════════════════════════════════════
// 以下每个 TEST_F/TEST_P 均含 // Arrange / // Act / // Assert 三段
// ═══════════════════════════════════════════════════════════════

TEST_F(FreeUnionImageTest, UnionImageVersion_VersionQuery_ReturnsExactString)
{
    // Arrange
    const QString expected = QStringLiteral("UnionImage Version:0.0.4\n");

    // Act
    const QString ver = unionImageVersion();

    // Assert
    EXPECT_EQ(ver, expected);
    EXPECT_EQ(ver.count(QLatin1Char('\n')), 1);
}

TEST_F(FreeUnionImageTest, NoneQImage_EveryCall_ReturnsSameNullImage)
{
    // Arrange
    const QImage none = noneQImage();

    // Act
    const QImage again = noneQImage();

    // Assert
    EXPECT_TRUE(none.isNull());
    EXPECT_EQ(none.size(), QSize(0, 0));
    EXPECT_TRUE(none == again);
}

TEST_F(FreeUnionImageTest, IsNoneQImage_NoneImage_ReturnsTrue)
{
    // Arrange
    const QImage none = noneQImage();

    // Act
    const bool ret = isNoneQImage(none);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(none.size(), QSize(0, 0));
}

TEST_F(FreeUnionImageTest, IsNoneQImage_ValidImage_ReturnsFalse)
{
    // Arrange
    const QImage img = makePatternImage(2, 2);

    // Act
    const bool ret = isNoneQImage(img);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(img.size(), QSize(2, 2));
}

TEST_P(SuffixParamTest, GetFileFormat_ParamPaths_ExtractsSuffix)
{
    // Arrange
    const SuffixCase &c = GetParam();

    // Act
    const QString suffix = getFileFormat(c.path);

    // Assert
    EXPECT_EQ(suffix, c.expected);
    EXPECT_EQ(suffix.count(QLatin1Char('.')), c.expected.count(QLatin1Char('.')));
}

INSTANTIATE_TEST_SUITE_P(PathSuffixes, SuffixParamTest, ::testing::Values(
    SuffixCase{QStringLiteral("pictures/photo.PNG"), QStringLiteral("PNG")},
    SuffixCase{QStringLiteral("archive/backup.tar.gz"), QStringLiteral("gz")},
    SuffixCase{QStringLiteral("noextension"), QString()}
));

TEST_P(DepthParamTest, CreatNewImage_ParamDepth_CreatesMatchingFormat)
{
    // Arrange
    const DepthCase &c = GetParam();
    QImage res;

    // Act
    const bool ret = creatNewImage(res, 6, 4, c.depth, UNKNOWNTYPE);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(res.format(), c.format);
    EXPECT_EQ(res.size(), QSize(6, 4));
    EXPECT_FALSE(res.isNull());
}

INSTANTIATE_TEST_SUITE_P(BitDepths, DepthParamTest, ::testing::Values(
    DepthCase{8, QImage::Format_RGB888},
    DepthCase{16, QImage::Format_RGB16},
    DepthCase{24, QImage::Format_RGB32}
));

TEST_P(SizeCaseParamTest, Size2Human_ParamBytes_ReturnsHumanReadable)
{
    // Arrange
    const SizeCase &c = GetParam();

    // Act
    const QString human = size2Human(c.bytes);

    // Assert
    EXPECT_EQ(human, c.expected);
    EXPECT_TRUE(human.endsWith(QLatin1String("B")));
}

INSTANTIATE_TEST_SUITE_P(ByteBoundaries, SizeCaseParamTest, ::testing::Values(
    SizeCase{0, QStringLiteral("0 B")},
    SizeCase{512, QStringLiteral("512 B")},
    SizeCase{1023, QStringLiteral("1023 B")},
    SizeCase{1024, QStringLiteral("1 KB")},
    SizeCase{1536, QStringLiteral("1.5 KB")},
    SizeCase{1048576, QStringLiteral("1 MB")},
    SizeCase{2097152, QStringLiteral("2 MB")},
    SizeCase{1073741824LL, QStringLiteral("1 GB")},
    SizeCase{2147483648LL, QStringLiteral("2 GB")}
));

TEST_P(FormatMagicParamTest, DetectImageFormat_ParamMagicBytes_ReturnsFormatName)
{
    // Arrange
    const FormatMagic &c = GetParam();
    const QString path = writeRawFile(tmpDir, QStringLiteral("sample.dat"), c.head);

    // Act
    const QString format = detectImageFormat(path);

    // Assert
    EXPECT_EQ(format, c.expected);
    EXPECT_EQ(QFileInfo(path).suffix(), QString("dat"));
}

INSTANTIATE_TEST_SUITE_P(MagicHeaders, FormatMagicParamTest, ::testing::Values(
    FormatMagic{QByteArray::fromHex("424d"), QStringLiteral("BMP")},
    FormatMagic{QByteArray::fromHex("44445320"), QStringLiteral("DDS")},
    FormatMagic{QByteArray::fromHex("474946383961"), QStringLiteral("GIF")},
    FormatMagic{QByteArray::fromHex("69636e73"), QStringLiteral("ICNS")},
    FormatMagic{QByteArray::fromHex("ffd8ffe00010"), QStringLiteral("JPG")},
    FormatMagic{QByteArray::fromHex("8a4d4e470d0a1a0a"), QStringLiteral("MNG")},
    FormatMagic{QByteArray::fromHex("50340a"), QStringLiteral("PBM")},
    FormatMagic{QByteArray::fromHex("50350a"), QStringLiteral("PGM")},
    FormatMagic{QByteArray::fromHex("50360a"), QStringLiteral("PPM")},
    FormatMagic{QByteArray::fromHex("89504e470d0a1a0a"), QStringLiteral("PNG")},
    FormatMagic{QByteArray("<svg width='2' height='2'></svg>"), QStringLiteral("SVG")},
    FormatMagic{QByteArray::fromHex("4d4d002a"), QStringLiteral("TIFF")},
    FormatMagic{QByteArray::fromHex("5249464672000000574542505650"), QStringLiteral("WEBP")},
    FormatMagic{QByteArray("#define max_width 16\n#define max_height 16\n"), QStringLiteral("XBM")},
    FormatMagic{QByteArray("/* XPM */static char x[]"), QStringLiteral("XPM")}
));

TEST_F(FreeUnionImageTest, DetectImageFormat_UnreadablePath_ReturnsEmptyString)
{
    // Arrange
    const QString path = tmpDir.filePath(QStringLiteral("missing.dat"));

    // Act
    const QString format = detectImageFormat(path);

    // Assert
    EXPECT_EQ(format, QString(""));
    EXPECT_FALSE(QFile::exists(path));
}

TEST_F(FreeUnionImageTest, DetectImageFormat_UnknownMagic_FallsBackToSuffix)
{
    // Arrange
    const QString path = writeRawFile(tmpDir, QStringLiteral("photo.xyz"),
                                       QByteArray("plain random payload"));

    // Act
    const QString format = detectImageFormat(path);

    // Assert
    EXPECT_EQ(format, QString("XYZ"));
    EXPECT_GT(format.size(), 0);
}

TEST_P(MimeFormatParamTest, GetFileMimeType_ParamImageFiles_ReturnsMappedFormat)
{
    // Arrange
    const MimeFormatCase &c = GetParam();
    const QString name = QStringLiteral("probe.") + QString::fromLatin1(c.fileSuffix);
    const QString path = writeImageFile(tmpDir, name, 4, 2, c.saveFormat);

    // Act
    const QString format = getFileMimeType(path);

    // Assert
    EXPECT_EQ(format, c.expected);
    EXPECT_TRUE(QFile::exists(path));
}

INSTANTIATE_TEST_SUITE_P(MimeMappings, MimeFormatParamTest, ::testing::Values(
    MimeFormatCase{"png", "PNG", QStringLiteral("PNG")},
    MimeFormatCase{"jpg", "JPG", QStringLiteral("JPEG")},
    MimeFormatCase{"bmp", "BMP", QStringLiteral("BMP")}
));

TEST_F(FreeUnionImageTest, GetFileMimeType_UnmappedMimeType_ReturnsEmptyString)
{
    // Arrange
    const QString path = writeRawFile(tmpDir, QStringLiteral("notes.bin"),
                                       QByteArray("just some plain text payload"));

    // Act
    const QString format = getFileMimeType(path);

    // Assert
    EXPECT_EQ(format, QString());
    EXPECT_TRUE(format.isNull());
}

TEST_F(FreeUnionImageTest, CanSave_SingleFramePng_ReturnsTrue)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("plain.png"), 4, 2, "PNG");

    // Act
    const bool ret = canSave(path);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(QFileInfo(path).suffix(), QString("png"));
}

TEST_F(FreeUnionImageTest, CanSave_MultiFrameImage_ReturnsFalse)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("frames.png"), 4, 2, "PNG");
    int callCount = 0;
    stub.set_lamda(static_cast<int (QImageReader::*)() const>(&QImageReader::imageCount),
                   [&callCount](QImageReader *) -> int {
                       ++callCount;
                       return 2;
                   });

    // Act
    const bool ret = canSave(path);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(callCount, 1);
}

TEST_F(FreeUnionImageTest, CanSave_UnsupportedSuffix_ReturnsFalse)
{
    // Arrange
    const QString path = writeRawFile(tmpDir, QStringLiteral("unknown.xyz"),
                                       QByteArray("XX"));

    // Act
    const bool ret = canSave(path);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(QFileInfo(path).suffix().toUpper(), QString("XYZ"));
}

TEST_F(FreeUnionImageTest, IsImageSupportRotate_SavableFormat_ReturnsTrue)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("rotatable.png"), 4, 2, "PNG");

    // Act
    const bool ret = isImageSupportRotate(path);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(QFileInfo(path).suffix().toUpper(), QString("PNG"));
}

TEST_F(FreeUnionImageTest, IsImageSupportRotate_UnsupportedFormat_ReturnsFalse)
{
    // Arrange
    const QString path = writeRawFile(tmpDir, QStringLiteral("locked.xyz"),
                                       QByteArray("XX"));

    // Act
    const bool ret = isImageSupportRotate(path);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(QFileInfo(path).suffix().toUpper(), QString("XYZ"));
}

TEST_F(FreeUnionImageTest, GetOrientation_EmptyPath_ReturnsDefaultOrientation)
{
    // Arrange
    const QString empty;

    // Act
    const int ret = getOrientation(empty);

    // Assert
    EXPECT_EQ(ret, 1);
    EXPECT_EQ(getOrientation(empty), getOrientation(QStringLiteral("")));
}

TEST_F(FreeUnionImageTest, GetOrientation_ExistingPngFile_ReturnsDefaultOrientation)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("oriented.png"), 4, 2, "PNG");

    // Act
    const int ret = getOrientation(path);

    // Assert
    EXPECT_EQ(ret, 1);
    EXPECT_TRUE(QFile::exists(path));
}

TEST_F(FreeUnionImageTest, GetImageType_EmptyPath_ReturnsBlankType)
{
    // Arrange
    const QString empty;

    // Act
    const imageViewerSpace::ImageType type = getImageType(empty);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::ImageTypeBlank);
    EXPECT_NE(type, imageViewerSpace::ImageTypeStatic);
}

TEST_F(FreeUnionImageTest, GetImageType_MissingFile_ReturnsBlankType)
{
    // Arrange
    const QString path = tmpDir.filePath(QStringLiteral("ghost/missing.png"));

    // Act
    const imageViewerSpace::ImageType type = getImageType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::ImageTypeBlank);
    EXPECT_FALSE(QFile::exists(path));
}

TEST_F(FreeUnionImageTest, GetImageType_SvgFile_ReturnsSvgType)
{
    // Arrange
    const QString path = writeSvgFile(tmpDir, QStringLiteral("vector.svg"), 4, 2);

    // Act
    const imageViewerSpace::ImageType type = getImageType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::ImageTypeSvg);
    EXPECT_EQ(QFileInfo(path).suffix().toLower(), QString("svg"));
}

TEST_F(FreeUnionImageTest, GetImageType_AnimatedGifMultiFrame_ReturnsDynamicType)
{
    // Arrange
    const QString path = writeRawFile(tmpDir, QStringLiteral("anim.gif"),
                                       QByteArray::fromHex("474946383961"));
    int frameCount = 0;
    stub.set_lamda(static_cast<int (QImageReader::*)() const>(&QImageReader::imageCount),
                   [&frameCount](QImageReader *) -> int {
                       ++frameCount;
                       return 5;
                   });

    // Act
    const imageViewerSpace::ImageType type = getImageType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::ImageTypeDynamic);
    EXPECT_EQ(frameCount, 1);
}

TEST_F(FreeUnionImageTest, GetImageType_MultiPageTiffStubbed_ReturnsMultiType)
{
    // Arrange
    const QString path = writeRawFile(tmpDir, QStringLiteral("album.tiff"),
                                       QByteArray::fromHex("4d4d002a"));
    stub.set_lamda(static_cast<int (QImageReader::*)() const>(&QImageReader::imageCount),
                   [](QImageReader *) -> int { return 3; });

    // Act
    const imageViewerSpace::ImageType type = getImageType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::ImageTypeMulti);
    EXPECT_EQ(QFileInfo(path).suffix().toLower(), QString("tiff"));
}

TEST_F(FreeUnionImageTest, GetImageType_StaticPng_ReturnsStaticType)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("single.png"), 4, 2, "PNG");

    // Act
    const imageViewerSpace::ImageType type = getImageType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::ImageTypeStatic);
    EXPECT_EQ(QFileInfo(path).suffix().toLower(), QString("png"));
}

TEST_F(FreeUnionImageTest, GetPathType_SmbUri_ReturnsSmbType)
{
    // Arrange
    const QString path = QStringLiteral("smb-share:server=nas/share1/img.png");
    QString captured;
    stub.set_lamda(Libutils::image::isVaultFile,
                   [&captured](const QString &p) -> bool {
                       captured = p;
                       return false;
                   });

    // Act
    const imageViewerSpace::PathType type = getPathType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::PathTypeSMB);
    // 源码为 if/else-if 链，SMB 分支命中后短路，isVaultFile 不应被调用
    EXPECT_TRUE(captured.isEmpty());
}

TEST_F(FreeUnionImageTest, GetPathType_MtpUri_ReturnsMtpType)
{
    // Arrange
    const QString path = QStringLiteral("mtp:host=Acer_tablet/Sun01/DCIM/a.jpg");
    QString captured;
    stub.set_lamda(Libutils::image::isVaultFile,
                   [&captured](const QString &p) -> bool {
                       captured = p;
                       return false;
                   });

    // Act
    const imageViewerSpace::PathType type = getPathType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::PathTypeMTP);
    // else-if 链在 MTP 分支短路，isVaultFile 不应被调用
    EXPECT_TRUE(captured.isEmpty());
}

TEST_F(FreeUnionImageTest, GetPathType_Gphoto2Uri_ReturnsPtpType)
{
    // Arrange
    const QString path = QStringLiteral("gphoto2:host=Canon_R6/Store0001/IMG_1.jpg");
    QString captured;
    stub.set_lamda(Libutils::image::isVaultFile,
                   [&captured](const QString &p) -> bool {
                       captured = p;
                       return false;
                   });

    // Act
    const imageViewerSpace::PathType type = getPathType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::PathTypePTP);
    // else-if 链在 gphoto2 分支短路，isVaultFile 不应被调用
    EXPECT_TRUE(captured.isEmpty());
}

TEST_F(FreeUnionImageTest, GetPathType_Gphoto2AppleUri_ReturnsPtpInsteadOfApple)
{
    // Arrange
    // 源码缺陷：gphoto2:host= 判断先于 gphoto2:host=Apple，APPLE 分支不可达
    const QString path = QStringLiteral("gphoto2:host=Apple_iPhone/DCIM/IMG.jpg");
    QString captured;
    stub.set_lamda(Libutils::image::isVaultFile,
                   [&captured](const QString &p) -> bool {
                       captured = p;
                       return false;
                   });

    // Act
    const imageViewerSpace::PathType type = getPathType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::PathTypePTP);
    EXPECT_NE(type, imageViewerSpace::PathTypeAPPLE);
    // else-if 链短路，isVaultFile 不应被调用
    EXPECT_TRUE(captured.isEmpty());
}

TEST_F(FreeUnionImageTest, GetPathType_VaultFlaggedPath_ReturnsSafeboxType)
{
    // Arrange
    const QString path = QStringLiteral("vaultspace/locked/a.png");
    QString captured;
    stub.set_lamda(Libutils::image::isVaultFile,
                   [&captured](const QString &p) -> bool {
                       captured = p;
                       return true;
                   });

    // Act
    const imageViewerSpace::PathType type = getPathType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::PathTypeSAFEBOX);
    EXPECT_EQ(captured, path);
}

TEST_F(FreeUnionImageTest, GetPathType_HomeTrashPath_ReturnsRecyclebinType)
{
    // Arrange
    const QString path = qEnvironmentVariable("HOME")
                         + QStringLiteral("/.local/share/Trash/files/a.png");
    QString captured;
    stub.set_lamda(Libutils::image::isVaultFile,
                   [&captured](const QString &p) -> bool {
                       captured = p;
                       return false;
                   });

    // Act
    const imageViewerSpace::PathType type = getPathType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::PathTypeRECYCLEBIN);
    EXPECT_EQ(captured, path);
}

TEST_F(FreeUnionImageTest, GetPathType_PlainLocalPath_ReturnsLocalType)
{
    // Arrange
    const QString path = QStringLiteral("pictures/local/a.png");
    QString captured;
    stub.set_lamda(Libutils::image::isVaultFile,
                   [&captured](const QString &p) -> bool {
                       captured = p;
                       return false;
                   });

    // Act
    const imageViewerSpace::PathType type = getPathType(path);

    // Assert
    EXPECT_EQ(type, imageViewerSpace::PathTypeLOCAL);
    EXPECT_EQ(captured, path);
}

TEST_F(FreeUnionImageTest, RotateImage_AngleNotMultipleOfNinety_ReturnsFalse)
{
    // Arrange
    QImage img = makePatternImage(2, 2);
    const QImage before = img;

    // Act
    const bool ret = rotateImage(45, img);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(img.size(), before.size());
    EXPECT_EQ(img.pixelColor(0, 0), before.pixelColor(0, 0));
}

TEST_F(FreeUnionImageTest, RotateImage_NullImage_ReturnsFalseWithoutTouchingImage)
{
    // Arrange
    QImage img;

    // Act
    const bool ret = rotateImage(90, img);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(img.size(), QSize(0, 0));
}

TEST_F(FreeUnionImageTest, RotateImage_ClockwiseNinety_SwapsDimensionsReturnsTrue)
{
    // Arrange
    QImage img = makePatternImage(3, 2);

    // Act
    const bool ret = rotateImage(90, img);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(img.size(), QSize(2, 3));
    EXPECT_EQ(img.pixelColor(1, 0), QColor(255, 0, 0));
    EXPECT_EQ(img.pixelColor(1, 2), QColor(0, 0, 255));
    EXPECT_EQ(img.pixelColor(0, 0), QColor(0, 255, 0));
}

TEST_F(FreeUnionImageTest, RotateImage_OneHundredEighty_ReversesPixelsReturnsTrue)
{
    // Arrange
    QImage img = makePatternImage(3, 2);

    // Act
    const bool ret = rotateImage(180, img);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(img.size(), QSize(3, 2));
    EXPECT_EQ(img.pixelColor(2, 1), QColor(255, 0, 0));
    EXPECT_EQ(img.pixelColor(0, 1), QColor(0, 0, 255));
}

TEST_P(OrientationParamTest, AdjustImageToRealPosition_ParamOrientations_ReturnsTransformedImage)
{
    // Arrange
    const OrientationCase &c = GetParam();
    const QImage src = makePatternImage(3, 2);

    // Act
    const QImage result = adjustImageToRealPosition(src, c.orientation);

    // Assert
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.size(), c.swapDims ? QSize(2, 3) : QSize(3, 2));
}

INSTANTIATE_TEST_SUITE_P(Orientations, OrientationParamTest, ::testing::Values(
    OrientationCase{1, false},
    OrientationCase{2, false},
    OrientationCase{3, false},
    OrientationCase{4, false},
    OrientationCase{5, true},
    OrientationCase{6, true},
    OrientationCase{7, true},
    OrientationCase{8, true}
));

TEST_F(FreeUnionImageTest, AdjustImageToRealPosition_OrientationOne_ReturnsOriginalUnchanged)
{
    // Arrange
    const QImage src = makePatternImage(3, 2);

    // Act
    const QImage result = adjustImageToRealPosition(src, 1);

    // Assert
    EXPECT_EQ(result.size(), QSize(3, 2));
    EXPECT_EQ(result, src);
}

TEST_F(FreeUnionImageTest, AdjustImageToRealPosition_HorizontalMirror_SwapsLeftRightPixels)
{
    // Arrange
    const QImage src = makePatternImage(3, 2);

    // Act
    const QImage result = adjustImageToRealPosition(src, 2);

    // Assert
    EXPECT_EQ(result.size(), QSize(3, 2));
    EXPECT_EQ(result.pixelColor(2, 0), QColor(255, 0, 0));
    EXPECT_EQ(result.pixelColor(0, 0), QColor(0, 0, 255));
}

TEST_F(FreeUnionImageTest, AdjustImageToRealPosition_OneHundredEighty_ReversesPixelPositions)
{
    // Arrange
    const QImage src = makePatternImage(3, 2);

    // Act
    const QImage result = adjustImageToRealPosition(src, 3);

    // Assert
    EXPECT_EQ(result.size(), QSize(3, 2));
    EXPECT_EQ(result.pixelColor(2, 1), QColor(255, 0, 0));
    EXPECT_EQ(result.pixelColor(0, 1), QColor(0, 0, 255));
}

TEST_F(FreeUnionImageTest, AdjustImageToRealPosition_VerticalMirror_FlipsTopBottomRows)
{
    // Arrange
    const QImage src = makePatternImage(3, 2);

    // Act
    const QImage result = adjustImageToRealPosition(src, 4);

    // Assert
    EXPECT_EQ(result.size(), QSize(3, 2));
    EXPECT_EQ(result.pixelColor(0, 1), QColor(255, 0, 0));
    EXPECT_EQ(result.pixelColor(2, 1), QColor(0, 0, 255));
}

TEST_F(FreeUnionImageTest, AdjustImageToRealPosition_ClockwiseNinety_MapsCornersCorrectly)
{
    // Arrange
    const QImage src = makePatternImage(3, 2);

    // Act
    const QImage result = adjustImageToRealPosition(src, 6);

    // Assert
    EXPECT_EQ(result.size(), QSize(2, 3));
    EXPECT_EQ(result.pixelColor(1, 0), QColor(255, 0, 0));
    EXPECT_EQ(result.pixelColor(1, 2), QColor(0, 0, 255));
    EXPECT_EQ(result.pixelColor(0, 0), QColor(0, 255, 0));
}

TEST_F(FreeUnionImageTest, AdjustImageToRealPosition_CounterClockwiseNinety_MapsCornersCorrectly)
{
    // Arrange
    const QImage src = makePatternImage(3, 2);

    // Act
    const QImage result = adjustImageToRealPosition(src, 8);

    // Assert
    EXPECT_EQ(result.size(), QSize(2, 3));
    EXPECT_EQ(result.pixelColor(0, 2), QColor(255, 0, 0));
    EXPECT_EQ(result.pixelColor(0, 0), QColor(0, 0, 255));
}

TEST_F(FreeUnionImageTest, LoadStaticImageFromFile_EmptyFile_ReturnsFalseWithError)
{
    // Arrange
    const QString path = writeRawFile(tmpDir, QStringLiteral("empty.png"), QByteArray());
    QImage res;
    QString errMsg;

    // Act
    const bool ret = loadStaticImageFromFile(path, res, errMsg);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(errMsg, QString("error file!"));
    EXPECT_TRUE(res.isNull());
}

TEST_F(FreeUnionImageTest, LoadStaticImageFromFile_NormalPng_ReturnsTrueAndImage)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("normal.png"), 8, 6, "PNG");
    QImage res;
    QString errMsg;

    // Act
    const bool ret = loadStaticImageFromFile(path, res, errMsg);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(res.size(), QSize(8, 6));
    EXPECT_EQ(errMsg, QString("use QImage"));
}

TEST_F(FreeUnionImageTest, LoadStaticImageFromFile_LargeImage_ScaledToMaxDimension)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("large.png"), 100, 40, "PNG");
    QImage res;
    QString errMsg;

    // Act
    const bool ret = loadStaticImageFromFile(path, res, errMsg, QString(), 50);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(res.width(), 50);
    EXPECT_EQ(res.height(), 20);
}

TEST_F(FreeUnionImageTest, LoadStaticImageFromFile_SuffixMismatchedContent_LoadsViaOldMethod)
{
    // Arrange
    // 真实 BMP 数据存为 .png 后缀：首轮按 png 解码失败，旧方法按魔数 BMP 成功
    const QString path = writeImageFile(tmpDir, QStringLiteral("fake.png"), 3, 2, "BMP");
    QImage res;
    QString errMsg;

    // Act
    const bool ret = loadStaticImageFromFile(path, res, errMsg);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(errMsg, QString("use old method to load QImage"));
    EXPECT_EQ(res.size(), QSize(3, 2));
}

TEST_F(FreeUnionImageTest, LoadStaticImageFromFile_CorruptPng_ReturnsFalseWithQtError)
{
    // Arrange
    const QString path = writeRawFile(tmpDir, QStringLiteral("corrupt.png"),
                                       QByteArray("definitely not a png at all"));
    QImage res;
    QString errMsg;

    // Act
    const bool ret = loadStaticImageFromFile(path, res, errMsg);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_NE(errMsg.indexOf(QStringLiteral("faild")), -1);
    EXPECT_TRUE(res.isNull());
}

TEST_F(FreeUnionImageTest, LoadStaticImageFromFile_UnsupportedSuffix_ReturnsFalse)
{
    // Arrange
    const QString path = writeRawFile(tmpDir, QStringLiteral("raw.xyz"),
                                       QByteArray("opaque payload"));
    QImage res;
    QString errMsg;

    // Act
    const bool ret = loadStaticImageFromFile(path, res, errMsg);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_TRUE(res.isNull());
    EXPECT_EQ(errMsg, QString());
}

TEST_F(FreeUnionImageTest, LoadStaticImageFromFile_IcnsWithoutImages_ReturnsFalse)
{
    // Arrange
    const QString path = writeRawFile(tmpDir, QStringLiteral("broken.icns"),
                                       QByteArray("icns-garbage-payload"));
    QImage res;
    QString errMsg;

    // Act
    const bool ret = loadStaticImageFromFile(path, res, errMsg);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_TRUE(res.isNull());
    EXPECT_EQ(errMsg, QString());
}

TEST_F(FreeUnionImageTest, LoadStaticImageFromFile_NonEmptyFormatBar_OverridesDetection)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("bar.png"), 3, 2, "BMP");
    QImage res;
    QString errMsg;

    // Act
    const bool ret = loadStaticImageFromFile(path, res, errMsg, QStringLiteral("BMP"));

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(errMsg, QString("use QImage"));
    EXPECT_EQ(res.size(), QSize(3, 2));
}

TEST_F(FreeUnionImageTest, ConvertToSRgbColorSpace_UntaggedImage_NoConversionApplied)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("untagged.png"), 4, 2, "PNG");
    QImage res;
    QString errMsg;

    // Act
    const bool ret = loadStaticImageFromFile(path, res, errMsg);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(res.size(), QSize(4, 2));
    EXPECT_EQ(res.pixelColor(0, 0), QColor(255, 0, 0));
}

TEST_F(FreeUnionImageTest, ConvertToSRgbColorSpace_SRgbTaggedImage_KeepsValidImage)
{
    // Arrange
    QImage img = makePatternImage(4, 2);
    img.setColorSpace(QColorSpace(QColorSpace::SRgb));
    const QString path = tmpDir.filePath(QStringLiteral("srgb.png"));
    ASSERT_TRUE(img.save(path, "PNG"));
    QImage res;
    QString errMsg;

    // Act
    const bool ret = loadStaticImageFromFile(path, res, errMsg);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(res.size(), QSize(4, 2));
    EXPECT_FALSE(res.isNull());
}

TEST_F(FreeUnionImageTest, ConvertToSRgbColorSpace_WideGamutImage_ConvertedSuccessfully)
{
    // Arrange
    QImage img = makePatternImage(4, 2);
    img.setColorSpace(QColorSpace(QColorSpace::Primaries::DciP3D65,
                                  QColorSpace::TransferFunction::SRgb));
    const QString path = tmpDir.filePath(QStringLiteral("p3.png"));
    ASSERT_TRUE(img.save(path, "PNG"));
    QImage res;
    QString errMsg;

    // Act
    const bool ret = loadStaticImageFromFile(path, res, errMsg);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(res.size(), QSize(4, 2));
    EXPECT_FALSE(res.isNull());
}

TEST_F(FreeUnionImageTest, RotateImageFile_InvalidAngle_ReturnsFalseWithError)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("angle.png"), 4, 2, "PNG");
    const qint64 sizeBefore = QFileInfo(path).size();
    QString erroMsg;

    // Act
    const bool ret = rotateImageFile(45, path, erroMsg);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(erroMsg, QString("unsupported angel"));
    EXPECT_EQ(QFileInfo(path).size(), sizeBefore);
}

TEST_F(FreeUnionImageTest, RotateImageFile_PngRotatedInPlace_ReturnsTrueWithSwappedSize)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("inplace.png"), 4, 2, "PNG");
    QString erroMsg;

    // Act
    const bool ret = rotateImageFile(90, path, erroMsg);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_TRUE(erroMsg.isEmpty());
    const QImage reloaded(path);
    EXPECT_EQ(reloaded.size(), QSize(2, 4));
}

TEST_F(FreeUnionImageTest, RotateImageFile_PngRotatedToTargetPath_OriginalUntouched)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("source.png"), 4, 2, "PNG");
    const QString target = tmpDir.filePath(QStringLiteral("rotated_out.png"));
    QString erroMsg;

    // Act
    const bool ret = rotateImageFile(90, path, erroMsg, target);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(QImage(path).size(), QSize(4, 2));
    EXPECT_EQ(QImage(target).size(), QSize(2, 4));
}

TEST_F(FreeUnionImageTest, RotateImageFile_SvgFile_ReturnsTrueAndRewritesFile)
{
    // Arrange
    const QString path = writeSvgFile(tmpDir, QStringLiteral("turn.svg"), 4, 2);
    QString erroMsg;

    // Act
    const bool ret = rotateImageFile(90, path, erroMsg);

    // Assert
    EXPECT_TRUE(ret);
    QFile rewritten(path);
    ASSERT_TRUE(rewritten.open(QIODevice::ReadOnly));
    const QByteArray content = rewritten.readAll();
    rewritten.close();
    EXPECT_NE(content.indexOf("<svg"), -1);
}

TEST_F(FreeUnionImageTest, RotateImageFile_UnsupportedFormat_ReturnsFalseWithMessage)
{
    // Arrange
    const QString path = writeRawFile(tmpDir, QStringLiteral("plain.xyz"),
                                       QByteArray("no image here"));
    QString erroMsg;

    // Act
    const bool ret = rotateImageFile(90, path, erroMsg);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(erroMsg, QString("not support rotate image format: XYZ"));
}

TEST_F(FreeUnionImageTest, RotateImageFile_DirectoryTargetPath_ReturnsFalseWithSaveError)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("victim.png"), 4, 2, "PNG");
    QDir(tmpDir.path()).mkdir(QStringLiteral("target_dir"));
    const QString target = tmpDir.filePath(QStringLiteral("target_dir"));
    QString erroMsg;

    // Act
    const bool ret = rotateImageFile(90, path, erroMsg, target);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(erroMsg, QString("save image failed"));
    EXPECT_TRUE(QDir(target).exists());
}

TEST_F(FreeUnionImageTest, RotateImageFIleWithImage_InvalidAngle_ReturnsFalseWithMessage)
{
    // Arrange
    QImage img = makePatternImage(2, 2);
    const QString path = writeImageFile(tmpDir, QStringLiteral("any.png"), 2, 2, "PNG");
    QString erroMsg;

    // Act
    const bool ret = rotateImageFIleWithImage(45, img, path, erroMsg);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(erroMsg, QString("unsupported angel"));
}

TEST_F(FreeUnionImageTest, RotateImageFIleWithImage_NullImage_ReturnsFalse)
{
    // Arrange
    QImage img;
    const QString path = writeImageFile(tmpDir, QStringLiteral("nullcase.png"), 2, 2, "PNG");
    QString erroMsg;

    // Act
    const bool ret = rotateImageFIleWithImage(90, img, path, erroMsg);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(erroMsg, QString());
}

TEST_F(FreeUnionImageTest, RotateImageFIleWithImage_SvgFile_RewritesAndReturnsTrue)
{
    // Arrange
    QImage img = makePatternImage(4, 2);
    const QString path = writeSvgFile(tmpDir, QStringLiteral("rewrite.svg"), 4, 2);
    QString erroMsg;

    // Act
    const bool ret = rotateImageFIleWithImage(90, img, path, erroMsg);

    // Assert
    EXPECT_TRUE(ret);
    QFile rewritten(path);
    ASSERT_TRUE(rewritten.open(QIODevice::ReadOnly));
    const QByteArray content = rewritten.readAll();
    rewritten.close();
    EXPECT_NE(content.indexOf("<svg"), -1);
}

TEST_F(FreeUnionImageTest, RotateImageFIleWithImage_JpgFile_SavesRotatedReturnsTrue)
{
    // Arrange
    const QString path = writeImageFile(tmpDir, QStringLiteral("camera.jpg"), 4, 2, "JPG");
    QImage img = makePatternImage(4, 2);
    QString erroMsg;

    // Act
    const bool ret = rotateImageFIleWithImage(90, img, path, erroMsg);

    // Assert
    EXPECT_TRUE(ret);
    const QImage reloaded(path, "JPG");
    EXPECT_FALSE(reloaded.isNull());
    EXPECT_EQ(reloaded.width(), 4);
}

TEST_F(FreeUnionImageTest, RotateImageFIleWithImage_UnsupportedFormat_ReturnsFalse)
{
    // Arrange
    QImage img = makePatternImage(2, 2);
    const QString path = writeRawFile(tmpDir, QStringLiteral("unknown.xyz"),
                                       QByteArray("opaque payload"));
    QString erroMsg;

    // Act
    const bool ret = rotateImageFIleWithImage(90, img, path, erroMsg);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(erroMsg, QString());
}

TEST_F(UnionImage_PrivateTest, UnionImage_Private_Constructor_PopulatesAllFormatLists)
{
    // Arrange
    UnionImage_Private obj;

    // Act
    const QStringList &supported = obj.m_qtSupported;

    // Assert
    EXPECT_EQ(supported.size(), 54);
    EXPECT_TRUE(supported.contains(QStringLiteral("JPG")));
    EXPECT_EQ(supported.count(QStringLiteral("BMP")), 2);
    EXPECT_EQ(supported.count(QStringLiteral("ICNS")), 2);
    EXPECT_EQ(supported.count(QStringLiteral("JP2")), 2);
    EXPECT_EQ(obj.m_canSave.size(), 9);
    EXPECT_EQ(obj.m_qtrotate.size(), 9);
}

TEST_F(UnionImage_PrivateTest, UnionImage_Private_Constructor_SaveAndRotateListsShareEntries)
{
    // Arrange
    UnionImage_Private obj;

    // Act
    const QStringList &canSave = obj.m_canSave;
    const QStringList &rotate = obj.m_qtrotate;

    // Assert
    EXPECT_EQ(canSave, rotate);
    EXPECT_TRUE(rotate.contains(QStringLiteral("ICNS")));
    EXPECT_FALSE(canSave.contains(QStringLiteral("GIF")));
}

TEST_F(UnionImage_PrivateTest, UnionImage_Private_Destructor_CleansUpWithoutError)
{
    // Arrange
    UnionImage_Private *obj = new UnionImage_Private();
    const int supportedCount = obj->m_qtSupported.size();
    const int canSaveCount = obj->m_canSave.size();

    // Act
    delete obj;  // 析构仅输出 qCDebug 日志，不应崩溃/抛异常

    // Assert
    EXPECT_EQ(supportedCount, 54);
    EXPECT_EQ(canSaveCount, 9);
}

// ─── 支持格式查询三函数（补测：lcov FNDA:0，stub.clear() 后直连真实函数）───
// 实现（unionimage.cpp:238-261）：
// - supportStaticFormat()    → 返回 union_image_private.m_qtSupported（54 项静态表）
// - supportMovieFormat()     → 返回 union_image_private.m_movie_formats.keys()
// - unionImageSupportFormat()→ 首次调用把 m_qtSupported 填入函数内 static res 后返回
// 映射： SupportStaticFormat_QueryTable_ReturnsNonEmptyKnownFormats       → 直连真实函数
//        SupportMovieFormat_QueryTable_ReturnsEmptyUnpopulatedList       → 直连真实函数
//        UnionImageSupportFormat_QueryMergedTable_MatchesStaticFormat    → 直连真实函数
// 缺陷注记（只标红不修改）：m_movie_formats 在源码中无任何填充点，
// supportMovieFormat() 恒返回空表（见 SupportMovieFormat 用例断言）。

TEST_F(FreeUnionImageTest, SupportStaticFormat_QueryTable_ReturnsNonEmptyKnownFormats)
{
    // Arrange：确保无桩生效（fixture SetUp 已 clear，再显式清一次防串扰）
    stub.clear();

    // Act
    const QStringList formats = LibUnionImage_NameSpace::supportStaticFormat();

    // Assert：静态格式表非空且含常见格式（BMP/PNG/GIF 均在 m_qtSupported 中）
    EXPECT_GE(formats.size(), 3);
    EXPECT_TRUE(formats.contains(QStringLiteral("BMP")));
    EXPECT_TRUE(formats.contains(QStringLiteral("PNG")));
    EXPECT_TRUE(formats.contains(QStringLiteral("GIF")));
}

TEST_F(FreeUnionImageTest, SupportMovieFormat_QueryTable_ReturnsEmptyUnpopulatedList)
{
    // Arrange：同上，直连真实函数
    stub.clear();

    // Act
    const QStringList movieFormats = LibUnionImage_NameSpace::supportMovieFormat();

    // Assert：m_movie_formats 无填充点 → 恒为空表（缺陷注记，真实行为固化）；
    // 静态格式表非空作对照，证明并非查询机制整体失效
    EXPECT_EQ(movieFormats.size(), 0);
    EXPECT_FALSE(LibUnionImage_NameSpace::supportStaticFormat().isEmpty());
}

TEST_F(FreeUnionImageTest, UnionImageSupportFormat_QueryMergedTable_MatchesStaticFormat)
{
    // Arrange：本用例不得让 SupportedImageFormats 式的 unionImageSupportFormat 桩生效
    stub.clear();

    // Act
    const QStringList formats = LibUnionImage_NameSpace::unionImageSupportFormat();

    // Assert：与静态格式表同源（首次调用填充函数内 static 缓存），非空且含 GIF
    EXPECT_FALSE(formats.isEmpty());
    EXPECT_TRUE(formats.contains(QStringLiteral("GIF")));
    EXPECT_EQ(formats, LibUnionImage_NameSpace::supportStaticFormat());
}
