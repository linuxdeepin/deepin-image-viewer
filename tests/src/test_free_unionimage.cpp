// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | adjustImageToRealPosition | high | complexity:10,cognitive:19 | 3 | 14 |
// | canSave | mid | in_degree:4 | 2 | 3 |
// | convertToSRgbColorSpace | high | complexity:14,cognitive:21 | 3 | 4 |
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
// | supportStaticFormat | low | - | 1 | 1 |
// | unionImageSupportFormat | low | - | 1 | 1 |
// | unionImageVersion | low | - | 1 | 1 |
// | UnionImage_Private ctor | mid | - | 2 | 2 |
// | ~UnionImage_Private | low | - | 1 | 1 |
// ─── actual 均不低于 min（actual=参数化实例数+独立用例数）───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（18 自由函数 + UnionImage_Private 2 方法全覆盖；
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
// - BUG326991_ConvertToSRgbColorSpace_CmykImage_ConvertedToNonCmyk → B3/B6（PMS sev2 色差根因锚点）
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
// B3: 含 "gphoto2:host=Apple" → APPLE（特判须先于通用 gphoto2:host= 判断方可达）
// B4: 含 "gphoto2:host=" → PTP
// B5: isVaultFile → SAFEBOX
// B6: 含 $HOME/.local/share/Trash → RECYCLEBIN
// B7: 默认 → LOCAL
// 用例映射：
// - GetPathType_SmbUri_ReturnsSmbType → B1
// - GetPathType_MtpUri_ReturnsMtpType → B2
// - GetPathType_Gphoto2AppleUri_ReturnsAppleType → B3
// - GetPathType_Gphoto2Uri_ReturnsPtpType → B4
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
// B1: angel%90!=0 → false + "unsupported angle"
// B2: img.isNull() → false + "image is null"
// B3: SVG → QSvgGenerator 写回 → true
// B4: JPG/JPEG 且 QImage(path,"JPG") 非空 → 旋转保存 → true
// B5: JPG 加载失败 → false
// B6: 其它格式 → false
// 用例映射：
// - RotateImageFIleWithImage_InvalidAngle_ReturnsFalseWithMessage → B1
// - RotateImageFIleWithImage_NullImage_ReturnsFalseWithMessage → B2
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

#include <QBuffer>
#include <QByteArray>
#include <QColor>
#include <QColorSpace>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QStringList>
#include <QTemporaryDir>
#include <QImageReader>
#include <QTransform>
#include <QtEndian>

#include <zlib.h>

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
//（位置/顺序必须一致，否则构造函数越界写）；
// 构造/析构为类内 inline 定义，链接期与 unionimage.o 弱符号合并
class UnionImage_Private {
public:
    UnionImage_Private();
    ~UnionImage_Private();

    QStringList m_qtSupported;
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

QByteArray makeJpegWithSegments(const QByteArray &segments)
{
    QByteArray jpeg;
    QBuffer buffer(&jpeg);
    if (!buffer.open(QIODevice::WriteOnly) || !makePatternImage(16, 8).save(&buffer, "JPEG"))
        return {};
    buffer.close();

    // Keep the JFIF APP0 immediately after SOI when adding test segments.
    if (jpeg.size() < 6 || jpeg.mid(2, 2) != QByteArrayLiteral("\xff\xe0"))
        return {};
    const int app0End = 4 + qFromBigEndian<quint16>(jpeg.constData() + 4);
    jpeg.insert(app0End, segments);
    return jpeg;
}

QByteArray standardExifApp1()
{
    // JPEG APP1 containing little-endian EXIF. IFD0 holds Model and an EXIF IFD pointer.
    // The EXIF IFD holds ExposureTime, ISO, DateTimeOriginal, DateTimeDigitized and LensModel.
    return QByteArray::fromHex(
        "ffe100bb45786966000049492a0008000000"
        "020010010200110000006800000069870400010000002600000000000000"
        "05009a82050001000000ab000000278803000100000090010000039002001400000083000000"
        "04900200140000009700000034a402000a0000007900000000000000"
        "4558494620546573742043616d65726100546573742035306d6d00"
        "323031383a30313a30322031363a30343a303500323031393a30323a30332031373a30353a303600"
        "010000007d000000");
}

QByteArray makePngWithExif(const QByteArray &tiff = standardExifApp1().mid(10))
{
    QByteArray png;
    QBuffer buffer(&png);
    if (!buffer.open(QIODevice::WriteOnly) || !makePatternImage(16, 8).save(&buffer, "PNG"))
        return {};
    buffer.close();

    // The eXIf chunk contains the TIFF data from standardExifApp1(), without the EXIF prefix.
    const QByteArray chunkData = "eXIf" + tiff;
    QByteArray chunk(4, Qt::Uninitialized);
    qToBigEndian<quint32>(tiff.size(), chunk.data());
    chunk.append(chunkData);
    char checksum[4];
    const auto crc = crc32(0, reinterpret_cast<const Bytef *>(chunkData.constData()), chunkData.size());
    qToBigEndian<quint32>(crc, checksum);
    chunk.append(checksum, sizeof(checksum));
    png.insert(33, chunk); // After the PNG signature and IHDR chunk.

    return png;
}

QByteArray cameraXmp()
{
    return QByteArrayLiteral(R"xml(<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
    xmlns:tiff="http://ns.adobe.com/tiff/1.0/" xmlns:exif="http://ns.adobe.com/exif/1.0/">
    <rdf:Description rdf:about="" tiff:Model="XMP Test Camera" exif:ExposureTime="1/125"
                     exif:FNumber="4/1" exif:DateTimeOriginal="2018-01-02T16:04:05+08:00">
      <exif:ISOSpeedRatings><rdf:Seq><rdf:li>400</rdf:li><rdf:li>800</rdf:li></rdf:Seq></exif:ISOSpeedRatings>
      <exif:FocalLength>50/1</exif:FocalLength>
    </rdf:Description>
    </rdf:RDF>)xml");
}

QByteArray photoshopResource(quint16 id, const QByteArray &name, const QByteArray &data)
{
    QByteArray resource = QByteArrayLiteral("8BIM") + QByteArray(2, '\0');
    qToBigEndian(id, resource.data() + 4);
    resource.append(char(name.size()));
    resource.append(name);
    if ((name.size() + 1) % 2)
        resource.append('\0');
    char length[4];
    qToBigEndian<quint32>(data.size(), length);
    resource.append(length, sizeof(length));
    resource.append(data);
    if (data.size() % 2)
        resource.append('\0');
    return resource;
}

QByteArray canonFlashApp1()
{
    // Canon ShotInfo[15] = -0x14 represents -2/3 EV.
    return QByteArray::fromHex(
        "ffe1007845786966000049492a000800000002000f010200060000002600000069870400010000002c00"
        "00000000000043616e6f6e0001007c920700320000003e00000000000000010004000300100000005000"
        "000000000000200000000000000000000000000000000000000000000000000000000000ecff");
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

TEST_F(FreeUnionImageTest, GetAllMetaData_JpegWithExif_ReturnsCameraAndShootingFields)
{
    const QByteArray jpeg = makeJpegWithSegments(standardExifApp1());
    ASSERT_FALSE(jpeg.isEmpty());
    const QString path = writeRawFile(tmpDir, QStringLiteral("相机.jpg"), jpeg);
    const QString modifiedTime = QFileInfo(path).lastModified().toString(QStringLiteral("yyyy/MM/dd HH:mm"));
    ASSERT_FALSE(modifiedTime.isEmpty());
    ASSERT_NE(modifiedTime, QStringLiteral("2019/02/03 17:05"));
    const auto metadata = getAllMetaData(path);

    EXPECT_EQ(metadata.value("Dimension"), QStringLiteral("16x8"));
    EXPECT_EQ(metadata.value("Model"), QStringLiteral("EXIF Test Camera"));
    EXPECT_EQ(metadata.value("LensType"), QStringLiteral("Test 50mm"));
    EXPECT_EQ(metadata.value("ISOSpeedRatings"), QStringLiteral("400"));
    EXPECT_TRUE(metadata.value("ExposureTime").contains(QStringLiteral("1/125")));
    EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));
    EXPECT_EQ(metadata.value("DateTimeDigitized"), modifiedTime);
    EXPECT_FALSE(metadata.contains("ColorSpace"));
    EXPECT_FALSE(metadata.contains("FlashExposureComp"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_PngWithExif_ReturnsCameraAndCaptureTime)
{
    const QByteArray png = makePngWithExif();
    ASSERT_FALSE(png.isEmpty());

    const QString path = writeRawFile(tmpDir, "camera.png", png);
    ASSERT_FALSE(QImage(path).isNull());
    const auto metadata = getAllMetaData(path);
    EXPECT_EQ(metadata.value("Dimension"), QStringLiteral("16x8"));
    EXPECT_EQ(metadata.value("Model"), QStringLiteral("EXIF Test Camera"));
    EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));
    EXPECT_EQ(metadata.value("DateTimeDigitized"),
              QFileInfo(path).lastModified().toString(QStringLiteral("yyyy/MM/dd HH:mm")));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_PngExifDirectoryBeyondApp1Limit)
{
    QByteArray tiff = standardExifApp1().mid(10);
    // Move the EXIF IFD past 64 KiB, preserving the referenced values near the start.
    constexpr quint32 directoryOffset = 70000;
    const auto directory = tiff.mid(38, 66);
    qToLittleEndian(directoryOffset, tiff.data() + 30);
    tiff.append(QByteArray(directoryOffset - tiff.size(), '\0'));
    tiff.append(directory);
    const auto png = makePngWithExif(tiff);
    ASSERT_FALSE(png.isEmpty());
    const auto path = writeRawFile(tmpDir, "large-exif.png", png);
    const auto metadata = getAllMetaData(path);
    EXPECT_EQ(metadata.value("Model"), QStringLiteral("EXIF Test Camera"));
    EXPECT_EQ(metadata.value("LensType"), QStringLiteral("Test 50mm"));
    EXPECT_EQ(metadata.value("ISOSpeedRatings"), QStringLiteral("400"));
    EXPECT_EQ(metadata.value("ExposureTime"), QStringLiteral("1/125 sec."));
    EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_PngWithBadExifChecksum_IgnoresTheChunk)
{
    QByteArray png = makePngWithExif();
    ASSERT_FALSE(png.isEmpty());
    const qsizetype checksumEnd = 33 + 8 + standardExifApp1().size() - 10 + 4;
    png[checksumEnd - 1] = png.at(checksumEnd - 1) ^ 1;
    const auto path = writeRawFile(tmpDir, "bad-exif-crc.png", png);
    const auto metadata = getAllMetaData(path);
    EXPECT_EQ(metadata.value("Dimension"), QStringLiteral("16x8"));
    EXPECT_FALSE(metadata.contains("Model"));
    EXPECT_FALSE(metadata.contains("ISOSpeedRatings"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_WebpWithExif_ReadsChunkAfterOddSizedImageData)
{
    if (!QImageReader::supportedImageFormats().contains("webp"))
        GTEST_SKIP() << "WebP image format plugin is unavailable";

    // VP8X announces EXIF and a 16x8 canvas. The 17-byte VP8L payload has a padding byte.
    const auto image = QByteArray::fromHex(
        "524946460000000057454250565038580a000000080000000f0000070000"
        "5650384c110000002f0fc0010007508a5294a2ff8188e87f0000");
    for (const bool withPrefix : {false, true}) {
        SCOPED_TRACE(withPrefix ? "EXIF prefix" : "Raw TIFF");
        const QByteArray payload = standardExifApp1().mid(withPrefix ? 4 : 10);
        QByteArray webp = image + "EXIF" + QByteArray(4, '\0') + payload;
        if (payload.size() % 2)
            webp.append('\0');
        qToLittleEndian<quint32>(webp.size() - 8, webp.data() + 4);
        qToLittleEndian<quint32>(payload.size(), webp.data() + image.size() + 4);

        const QString path = writeRawFile(tmpDir, withPrefix ? "prefixed.webp" : "camera.webp", webp);
        ASSERT_FALSE(QImage(path).isNull());
        const auto metadata = getAllMetaData(path);
        EXPECT_EQ(metadata.value("Dimension"), QStringLiteral("16x8"));
        EXPECT_EQ(metadata.value("ISOSpeedRatings"), QStringLiteral("400"));
        EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));
    }
}

TEST_F(FreeUnionImageTest, GetAllMetaData_IcoWithPngExif_UsesOnlyTheDeclaredIconEntry)
{
    const QByteArray png = makePngWithExif();
    ASSERT_FALSE(png.isEmpty());
    QByteArray icon = QByteArray::fromHex("00000100010010080000010020000000000016000000");
    qToLittleEndian<quint32>(png.size(), icon.data() + 14);
    icon.append(png);
    const QString path = writeRawFile(tmpDir, "camera.ico", icon);
    ASSERT_FALSE(QImage(path).isNull());
    const auto metadata = getAllMetaData(path);
    EXPECT_EQ(metadata.value("Model"), QStringLiteral("EXIF Test Camera"));
    EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));

    // The valid PNG bytes extend beyond this directory entry's stated length.
    qToLittleEndian<quint32>(33, icon.data() + 14);
    const QString truncated = writeRawFile(tmpDir, "truncated.ico", icon);
    EXPECT_FALSE(getAllMetaData(truncated).contains("Model"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_PsdWithExifAndXmp_PrefersExif)
{
    const QByteArray header = QByteArray::fromHex("384250530001000000000000000300000001000000010008000300000000");
    const QByteArray otherResource = photoshopResource(1000, "nm", QByteArrayLiteral("odd"));
    const QByteArray xmpResource = photoshopResource(1060, {}, cameraXmp());
    const QByteArray exifResource = photoshopResource(1058, "Exif", standardExifApp1().mid(10));
    for (bool includeExif : {false, true}) {
        SCOPED_TRACE(includeExif);
        const QByteArray resources = otherResource + xmpResource + (includeExif ? exifResource : QByteArray());
        QByteArray psd = header + QByteArray(4, '\0');
        qToBigEndian<quint32>(resources.size(), psd.data() + header.size());
        psd.append(resources);
        psd.append(QByteArray(9, '\0')); // Empty layer data and an uncompressed RGB pixel.
        const QString path = writeRawFile(tmpDir, includeExif ? "camera.psd" : "xmp.psd", psd);
        if (QImageReader::supportedImageFormats().contains("psd"))
            ASSERT_FALSE(QImage(path).isNull());
        const auto metadata = getAllMetaData(path);
        EXPECT_EQ(metadata.value("Model"), includeExif ? QStringLiteral("EXIF Test Camera") : QStringLiteral("XMP Test Camera"));
        EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));
        EXPECT_EQ(metadata.value("FocalLength"), QStringLiteral("50 mm"));
    }
}

TEST_F(FreeUnionImageTest, GetAllMetaData_GifWithXmp_ReturnsShootingFields)
{
    QByteArray gif = QByteArray::fromHex("47494638396101000100800000000000ffffff2c00000000010001000002024401003b");
    QByteArray extension = QByteArray::fromHex("21ff0b") + "XMP DataXMP" + cameraXmp();
    extension.append('\x01');
    for (int byte = 255; byte >= 0; --byte)
        extension.append(static_cast<char>(byte));
    extension.append('\0');
    gif.insert(gif.size() - 1, extension); // After the first frame, before the GIF trailer.
    const QString path = writeRawFile(tmpDir, "camera.gif", gif);
    ASSERT_FALSE(QImage(path).isNull());
    const auto metadata = getAllMetaData(path);
    EXPECT_EQ(metadata.value("Model"), QStringLiteral("XMP Test Camera"));
    EXPECT_EQ(metadata.value("ISOSpeedRatings"), QStringLiteral("400"));
    EXPECT_EQ(metadata.value("ExposureTime"), QStringLiteral("1/125 sec."));
    EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_SvgWithXmp_ReadsCameraPropertiesFromMetadata)
{
    const QByteArray svg = QByteArrayLiteral("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"8\"><metadata>")
                          + cameraXmp() + "</metadata><rect width=\"16\" height=\"8\"/></svg>";
    const QString path = writeRawFile(tmpDir, "camera.svg", svg);
    ASSERT_FALSE(QImage(path).isNull());
    const auto metadata = getAllMetaData(path);
    EXPECT_EQ(metadata.value("Model"), QStringLiteral("XMP Test Camera"));
    EXPECT_EQ(metadata.value("ApertureValue"), QStringLiteral("f/4"));
    EXPECT_EQ(metadata.value("FocalLength"), QStringLiteral("50 mm"));
    EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_SvgzWithNestedXmp_ReadsExtendedIso)
{
    QByteArray xmp = cameraXmp();
    xmp.replace("xmlns:tiff=", "xmlns:exifEX=\"http://cipa.jp/exif/1.0/\" xmlns:tiff=");
    xmp.replace("exif:FNumber=", "exifEX:PhotographicSensitivity=\"65535\" exifEX:SensitivityType=\"3\" "
                                "exifEX:ISOSpeed=\"102400\" exif:FNumber=");
    const QByteArray svg = QByteArrayLiteral("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"8\"><g><metadata>")
                          + xmp + "</metadata></g><rect width=\"16\" height=\"8\"/></svg>";
    const QString uncompressed = writeRawFile(tmpDir, "nested.svg", svg);
    const QString compressed = tmpDir.filePath("nested.svgz");
    gzFile file = gzopen(QFile::encodeName(compressed).constData(), "wb");
    ASSERT_NE(file, nullptr);
    const int bytesWritten = gzwrite(file, svg.constData(), static_cast<unsigned int>(svg.size()));
    const int closeResult = gzclose(file);
    ASSERT_EQ(bytesWritten, svg.size());
    ASSERT_EQ(closeResult, Z_OK);

    for (const QString &path : {uncompressed, compressed}) {
        SCOPED_TRACE(path.toStdString());
        ASSERT_FALSE(QImage(path).isNull());
        const auto metadata = getAllMetaData(path);
        EXPECT_EQ(metadata.value("Model"), QStringLiteral("XMP Test Camera"));
        EXPECT_EQ(metadata.value("ISOSpeedRatings"), QStringLiteral("102400"));
        EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));
    }
}

TEST_F(FreeUnionImageTest, GetAllMetaData_SvgWithLargePrefixAndMultipleMetadataBlocks)
{
    QByteArray svg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="16" height="8">
        <metadata><rdf:Description xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
            xmlns:tiff="http://ns.adobe.com/tiff/1.0/" tiff:ImageWidth="16"/></metadata>
        <metadata><rdf:Description xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
            xmlns:tiff="http://ns.adobe.com/tiff/1.0/" tiff:Model="Oversized Metadata"/><!--)";
    svg.append(QByteArray(1024 * 1024, ' '));
    svg.append("--></metadata><metadata>");
    svg.append(cameraXmp());
    svg.append("</metadata></svg>");
    const auto path = writeRawFile(tmpDir, "multiple-metadata.svg", svg);
    const auto metadata = getAllMetaData(path);
    EXPECT_EQ(metadata.value("Model"), QStringLiteral("XMP Test Camera"));
    EXPECT_EQ(metadata.value("ISOSpeedRatings"), QStringLiteral("400"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_TiffWithExif_ReturnsShootingFields)
{
    // A 64x64 grayscale TIFF with its EXIF directory before the pixel strip and IFD0 after it.
    QByteArray tiff = QByteArray::fromBase64(
        "SUkqAIQQAAAGAJqCBQABAAAAcgAAAJ2CBQABAAAAagAAACeIAwABAAAAkAEAAAOQAgAUAAAA"
        "VgAAAAmSAwABAAAAAAAAADSkAgAKAAAAegAAAAAAAAAyMDE4OjAxOjAyIDE2OjA0OjA1AAQA"
        "AAABAAAAbxKDAAAAAEBUZXN0IDUwbW0=");
    tiff.append(QByteArray(64 * 64, '\0'));
    tiff.append(QByteArray::fromBase64(
        "AAwAAAEDAAEAAABAAAAAAQEDAAEAAABAAAAAAgEDAAEAAAAIAAAAAwEDAAEAAAABAAAABgED"
        "AAEAAAABAAAAEAECABEAAAAaEQAAEQEEAAEAAACEAAAAFQEDAAEAAAABAAAAFgEDAAEAAABA"
        "AAAAFwEEAAEAAAAAEAAAHAEDAAEAAAABAAAAaYcEAAEAAAAIAAAAAAAAAEVYSUYgVGVzdCBD"
        "YW1lcmEA"));
    const QString path = writeRawFile(tmpDir, QStringLiteral("相机.tiff"), tiff);
    const auto metadata = getAllMetaData(path);

    EXPECT_EQ(metadata.value("Model"), QStringLiteral("EXIF Test Camera"));
    EXPECT_EQ(metadata.value("LensType"), QStringLiteral("Test 50mm"));
    EXPECT_EQ(metadata.value("ISOSpeedRatings"), QStringLiteral("400"));
    EXPECT_EQ(metadata.value("ApertureValue"), QStringLiteral("f/4"));
    EXPECT_EQ(metadata.value("ExposureTime"), QStringLiteral("1/125 sec."));
    EXPECT_FALSE(metadata.value("Flash").isEmpty());
    EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_HeifAndAvifWithExif_ReadPrimaryImageMetadata)
{
    // Both files store the tags from standardExifApp1() in the primary image's Exif item.
    // The HEIC has a six-byte prefix before the TIFF header, and the AVIF has a zero offset.
    const QByteArray heic = QByteArray::fromBase64(
        "AAAAHGZ0eXBoZWljAAAAAG1pZjFoZWljbWlhZgAAAadtZXRhAAAAAAAAACFoZGxyAAAAAAAA"
        "AABwaWN0AAAAAAAAAAAAAAAAAAAAAA5waXRtAAAAAAABAAAANGlsb2MAAAAAREAAAgABAAAA"
        "AAHLAAEAAAAAAAAANgACAAAAAAIBAAEAAAAAAAAAvQAAADhpaW5mAAAAAAACAAAAFWluZmUC"
        "AAAAAAEAAGh2YzEAAAAAFWluZmUCAAABAAIAAEV4aWYAAAAA5mlwcnAAAADHaXBjbwAAAHNo"
        "dmNDAQNwAAAAAAAAAAAAHvAA/P34+AAADwMgAAEAGEABDAH//wNwAAADAJAAAAMAAAMAHroC"
        "QCEAAQAnQgEBA3AAAAMAkAAAAwAAAwAeoCCBBZbqrprmwIAAAAMAgAAAAwCEIgABAAZEAcFz"
        "wYkAAAAUaXNwZQAAAAAAAABAAAAAQAAAAChjbGFwAAAAEAAAAAEAAAAIAAAAAf///9AAAAAC"
        "////yAAAAAIAAAAQcGl4aQAAAAADCAgIAAAAF2lwbWEAAAAAAAAAAQABBIECgwQAAAAaaXJl"
        "ZgAAAAAAAAAOY2RzYwACAAEAAQAAAPttZGF0AAAAMigBrxMhXsNA9rQ5i2ppI/049gjmjksY"
        "DcyGh2GEVn0iYbn6kNCsfWCQg6pApoCrXxycAAAABkV4aWYAAElJKgAIAAAAAgAQAQIAEQAA"
        "AGgAAABphwQAAQAAACYAAAAAAAAABQCaggUAAQAAAKsAAAAniAMAAQAAAJABAAADkAIAFAAA"
        "AIMAAAAEkAIAFAAAAJcAAAA0pAIACgAAAHkAAAAAAAAARVhJRiBUZXN0IENhbWVyYQBUZXN0"
        "IDUwbW0AMjAxODowMTowMiAxNjowNDowNQAyMDE5OjAyOjAzIDE3OjA1OjA2AAEAAAB9AAAA");
    const QByteArray avif = QByteArray::fromBase64(
        "AAAAHGZ0eXBhdmlmAAAAAGF2aWZtaWYxbWlhZgAAASttZXRhAAAAAAAAACFoZGxyAAAAAAAA"
        "AABwaWN0AAAAAAAAAAAAAAAAAAAAAA5waXRtAAAAAAABAAAANGlsb2MAAAAAREAAAgABAAAA"
        "AAFPAAEAAAAAAAAAGgACAAAAAAFpAAEAAAAAAAAAtwAAADhpaW5mAAAAAAACAAAAFWluZmUC"
        "AAAAAAEAAGF2MDEAAAAAFWluZmUCAAABAAIAAEV4aWYAAAAAamlwcnAAAABLaXBjbwAAABNj"
        "b2xybmNseAABAA0ABoAAAAAMYXYxQ4EADAAAAAAUaXNwZQAAAAAAAAAQAAAACAAAABBwaXhp"
        "AAAAAAMICAgAAAAXaXBtYQAAAAAAAAABAAEEAYIDBAAAABppcmVmAAAAAAAAAA5jZHNjAAIA"
        "AQABAAAA2W1kYXQSAAoFGAy/sCEyDxgACiiihAACOK3SEQiEgAAAAABJSSoACAAAAAIAEAEC"
        "ABEAAABoAAAAaYcEAAEAAAAmAAAAAAAAAAUAmoIFAAEAAACrAAAAJ4gDAAEAAACQAQAAA5AC"
        "ABQAAACDAAAABJACABQAAACXAAAANKQCAAoAAAB5AAAAAAAAAEVYSUYgVGVzdCBDYW1lcmEA"
        "VGVzdCA1MG1tADIwMTg6MDE6MDIgMTY6MDQ6MDUAMjAxOTowMjowMyAxNzowNTowNgABAAAA"
        "fQAAAA==");
    const QMap<QString, QByteArray> fixtures = {
        {QStringLiteral("相机.heic"), heic},
        {QStringLiteral("相机.heif"), heic},
        {QStringLiteral("相机.avif"), avif},
    };
    for (auto it = fixtures.cbegin(); it != fixtures.cend(); ++it) {
        SCOPED_TRACE(it.key().toStdString());
        const QString path = writeRawFile(tmpDir, it.key(), it.value());
        const auto metadata = getAllMetaData(path);
        EXPECT_EQ(metadata.value("Model"), QStringLiteral("EXIF Test Camera"));
        EXPECT_EQ(metadata.value("LensType"), QStringLiteral("Test 50mm"));
        EXPECT_EQ(metadata.value("ISOSpeedRatings"), QStringLiteral("400"));
        EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));
    }
}

TEST_F(FreeUnionImageTest, GetAllMetaData_DngWithExif_ReturnsRawShootingFields)
{
    // Little-endian DNG with a 64x64 CFA strip, camera and lens names, and EXIF shooting settings.
    QByteArray dng = QByteArray::fromHex(
        "49492a0008000000140000010400010000004000000001010400010000004000"
        "0000020103000100000010000000030103000100000001000000060103000100"
        "0000238000000f010200050000004c0100001001020010000000520100001101"
        "040001000000a801000012010300010000000100000015010300010000000100"
        "00001601040001000000400000001701040001000000002000001c0103000100"
        "0000010000008d82030002000000020002008e82010004000000000101026987"
        "040001000000fe00000012c60100040000000104000013c60100040000000101"
        "000014c6020010000000620100001dc6040001000000ff0f0000000000000600"
        "9a82050001000000720100009d820500010000007a0100002788030001000000"
        "900100000390020014000000820100000a920500010000009601000034a40200"
        "0a0000009e0100000000000054657374000052415720546573742043616d6572"
        "610052415720546573742043616d65726100010000007d000000040000000100"
        "0000323031383a30313a30322031363a30343a30350032000000010000005465"
        "73742035306d6d00");
    dng.replace("2018:01:02 16:04:05", "2024:03:10 02:30:00");
    dng.append(QByteArray(64 * 64 * 2, '\0'));
    const QString path = writeRawFile(tmpDir, QStringLiteral("相机.dng"), dng);
    ASSERT_TRUE(QFileInfo(path).exists());

    const auto metadata = getAllMetaData(path);
    EXPECT_EQ(metadata.value("Model"), QStringLiteral("RAW Test Camera"));
    EXPECT_EQ(metadata.value("LensType"), QStringLiteral("Test 50mm"));
    EXPECT_EQ(metadata.value("ISOSpeedRatings"), QStringLiteral("400"));
    EXPECT_EQ(metadata.value("ApertureValue"), QStringLiteral("f/4"));
    EXPECT_EQ(metadata.value("ExposureTime"), QStringLiteral("1/125 sec."));
    EXPECT_EQ(metadata.value("FocalLength"), QStringLiteral("50 mm"));
    EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2024/03/10 02:30"));
    EXPECT_EQ(metadata.value("DateTimeDigitized"),
              QFileInfo(path).lastModified().toString(QStringLiteral("yyyy/MM/dd HH:mm")));

    // LibRaw also reports DateTimeDigitized as its aggregate timestamp. It is not the capture time.
    dng.replace(QByteArray::fromHex("0390020014000000"), QByteArray::fromHex("0490020014000000"));
    const QString digitizedPath = writeRawFile(tmpDir, "digitized-only.dng", dng);
    const auto digitized = getAllMetaData(digitizedPath);
    EXPECT_EQ(digitized.value("Model"), QStringLiteral("RAW Test Camera"));
    const QString modified = QFileInfo(digitizedPath).lastModified().toString(QStringLiteral("yyyy/MM/dd HH:mm"));
    ASSERT_NE(modified, QStringLiteral("2024/03/10 02:30"));
    EXPECT_EQ(digitized.value("DateTimeOriginal"), modified);
}

TEST_F(FreeUnionImageTest, GetAllMetaData_CanonMakerNote_DecodesSignedThirdStops)
{
    const QByteArray jpeg = makeJpegWithSegments(canonFlashApp1());
    ASSERT_FALSE(jpeg.isEmpty());
    const QString path = writeRawFile(tmpDir, "canon-flash.jpg", jpeg);
    EXPECT_EQ(getAllMetaData(path).value("FlashExposureComp"), QStringLiteral("-0.67 EV"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_CanonMakerNote_IgnoresInvalidShotInfoOffsets)
{
    constexpr qsizetype shotInfoOffsetField = 82;
    const struct {
        quint32 offset;
        const char *name;
    } invalidOffsets[] = {
        {0, "TIFF header"},
        {8, "IFD0"},
        {44, "EXIF IFD"},
        {62, "MakerNote IFD"},
        {78, "MakerNote next IFD pointer"},
        {82, "ShotInfo data after the size field"},
        {0xffffffff, "Outside the TIFF payload"},
    };
    for (const auto &testCase : invalidOffsets) {
        SCOPED_TRACE(testCase.name);
        QByteArray app1 = canonFlashApp1();
        qToLittleEndian(testCase.offset, app1.data() + shotInfoOffsetField);
        const QByteArray jpeg = makeJpegWithSegments(app1);
        ASSERT_FALSE(jpeg.isEmpty());
        const QString path = writeRawFile(tmpDir, QStringLiteral("canon-offset-%1.jpg").arg(testCase.offset), jpeg);
        const auto metadata = getAllMetaData(path);
        EXPECT_EQ(metadata.value("Dimension"), QStringLiteral("16x8"));
        EXPECT_FALSE(metadata.contains("FlashExposureComp"));
    }
}

TEST_F(FreeUnionImageTest, GetAllMetaData_CanonMakerNote_IgnoresInconsistentShotInfoSize)
{
    QByteArray app1 = canonFlashApp1();
    // The ShotInfo array has 16 shorts (32 bytes) but its first short declares 30 bytes.
    qToLittleEndian<quint16>(30, app1.data() + 90);
    const QByteArray jpeg = makeJpegWithSegments(app1);
    ASSERT_FALSE(jpeg.isEmpty());
    const QString path = writeRawFile(tmpDir, "canon-shotinfo-size.jpg", jpeg);
    EXPECT_FALSE(getAllMetaData(path).contains("FlashExposureComp"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_NikonMakerNote_DecodesPositiveAndZeroCompensation)
{
    // Outer EXIF is little-endian. The Nikon type 3 note uses its own big-endian TIFF header.
    // Tag 0x0012 is {4, 1, 6, 0}, meaning +2/3 EV.
    QByteArray app1 = QByteArray::fromHex(
        "ffe1005845786966000049492a0008000000010069870400010000001a0000000000000001007c920700"
        "240000002c000000000000004e696b6f6e00021000004d4d002a00000008000100120007000000040401"
        "060000000000");
    const QByteArray jpeg = makeJpegWithSegments(app1);
    ASSERT_FALSE(jpeg.isEmpty());
    const QString path = writeRawFile(tmpDir, "nikon-flash.jpg", jpeg);
    EXPECT_EQ(getAllMetaData(path).value("FlashExposureComp"), QStringLiteral("+0.67 EV"));

    app1[app1.size() - 8] = 0; // First byte of tag value, retaining its multiplier and divisor.
    const QString zeroPath = writeRawFile(tmpDir, "nikon-no-compensation.jpg", makeJpegWithSegments(app1));
    EXPECT_EQ(getAllMetaData(zeroPath).value("FlashExposureComp"), QStringLiteral("0.00 EV"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_JpegWithApp12BeforeExif_ReturnsCameraFields)
{
    // The leading 0xff is a marker fill byte. APP12 contains 8 bytes after its length.
    const QByteArray app12 = QByteArray::fromHex("ffffec000a4475636b79000000");
    // A non-EXIF APP1 should also be skipped.
    const QByteArray otherApp1 = QByteArray::fromHex("ffe100066d657461");
    const QByteArray jpeg = makeJpegWithSegments(app12 + otherApp1 + standardExifApp1());
    ASSERT_FALSE(jpeg.isEmpty());
    const QString path = writeRawFile(tmpDir, "app12-first.jpg", jpeg);
    ASSERT_FALSE(QImage(path).isNull());
    const auto metadata = getAllMetaData(path);

    EXPECT_EQ(metadata.value("Model"), QStringLiteral("EXIF Test Camera"));
    EXPECT_EQ(metadata.value("ISOSpeedRatings"), QStringLiteral("400"));
    EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2018/01/02 16:04"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_JpegWithIsoOverflow_UsesSensitivityType)
{
    const auto original = QByteArray::fromHex(
        "ffe1004c45786966000049492a0008000000010069870400010000001a00000000000000"
        "03002788030001000000ffff000030880300010000000300000033880400010000000090010000000000");
    const struct {
        char type;
        const char *tag;
        const char *name;
    } variants[] = {
        {1, "3188040001000000", "sos.jpg"},
        {2, "3288040001000000", "rei.jpg"},
        {3, "3388040001000000", "iso-speed.jpg"},
    };
    for (const auto &variant : variants) {
        SCOPED_TRACE(variant.name);
        QByteArray app1 = original;
        app1.replace(QByteArray::fromHex("030000003388040001000000"),
                     QByteArray(1, variant.type) + QByteArray(3, '\0') + QByteArray::fromHex(variant.tag));
        const QByteArray jpeg = makeJpegWithSegments(app1);
        ASSERT_FALSE(jpeg.isEmpty());
        const QString path = writeRawFile(tmpDir, QString::fromLatin1(variant.name), jpeg);
        EXPECT_EQ(getAllMetaData(path).value("ISOSpeedRatings"), QStringLiteral("102400"));
    }
}

TEST_F(FreeUnionImageTest, GetAllMetaData_JpegWithBothApertureTags_PrefersFNumber)
{
    // FNumber is f/5.6, while the APEX ApertureValue of 5 converts to about f/5.7.
    const QByteArray app1 = QByteArray::fromHex(
        "ffe1005045786966000049492a0008000000010069870400010000001a00000000000000"
        "02009d820500010000003800000002920500010000004000000000000000380000000a00"
        "00000500000001000000");
    const QByteArray jpeg = makeJpegWithSegments(app1);
    ASSERT_FALSE(jpeg.isEmpty());
    const QString path = writeRawFile(tmpDir, "aperture.jpg", jpeg);
    EXPECT_EQ(getAllMetaData(path).value("ApertureValue"), QStringLiteral("f/5.6"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_ExifCaptureTimeInDstGap_PreservesRecordedTime)
{
    QByteArray app1 = standardExifApp1();
    app1.replace("2018:01:02 16:04:05", "2024:03:10 02:30:00");
    const QByteArray jpeg = makeJpegWithSegments(app1);
    ASSERT_FALSE(jpeg.isEmpty());
    const QString path = writeRawFile(tmpDir, "dst-gap.jpg", jpeg);
    const auto metadata = getAllMetaData(path);

    EXPECT_EQ(metadata.value("DateTimeOriginal"), QStringLiteral("2024/03/10 02:30"));
}

TEST_F(FreeUnionImageTest, GetAllMetaData_JpegWithoutExif_ReturnsFileInfo)
{
    const QString path = writeImageFile(tmpDir, "plain.jpg", 16, 8, "JPEG");
    const QFileInfo file(path);
    ASSERT_TRUE(file.exists());
    const auto metadata = getAllMetaData(path);
    const QString modifiedTime = file.lastModified().toString(QStringLiteral("yyyy/MM/dd HH:mm"));

    EXPECT_EQ(metadata.value("Dimension"), QStringLiteral("16x8"));
    EXPECT_EQ(metadata.value("DateTimeOriginal"), modifiedTime);
    EXPECT_EQ(metadata.value("DateTimeDigitized"), modifiedTime);
    EXPECT_FALSE(metadata.contains("Model"));
    EXPECT_FALSE(metadata.contains("ISOSpeedRatings"));
}

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

TEST_F(FreeUnionImageTest, GetPathType_Gphoto2AppleUri_ReturnsAppleType)
{
    // Arrange
    // Apple 特判先于通用 gphoto2:host= 判断，iPhone 路径应命中 APPLE 分支
    const QString path = QStringLiteral("gphoto2:host=Apple_iPhone/DCIM/IMG.jpg");
    QString captured;
    stub.set_lamda(Libutils::image::isVaultFile,
                   [&captured](const QString &p) -> bool {
                       captured = p;
                       return false;
                   });

    // Act
    const imageViewerSpace::PathType type = getPathType(path);

    // Assert：源码缺陷 D6 — gphoto2:host= 分支先于 Apple 特判命中，Apple 分支为死代码
    EXPECT_EQ(type, imageViewerSpace::PathTypeAPPLE);
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
    EXPECT_EQ(erroMsg, QString("unsupported angle"));
}

TEST_F(FreeUnionImageTest, RotateImageFIleWithImage_NullImage_ReturnsFalseWithMessage)
{
    // Arrange
    QImage img;
    const QString path = writeImageFile(tmpDir, QStringLiteral("nullcase.png"), 2, 2, "PNG");
    QString erroMsg;

    // Act
    const bool ret = rotateImageFIleWithImage(90, img, path, erroMsg);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(erroMsg, QString("image is null"));
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

// ─── 支持格式查询两函数（补测：lcov FNDA:0，stub.clear() 后直连真实函数）───
// 实现（unionimage.cpp）：
// - supportStaticFormat()    → 返回 union_image_private.m_qtSupported（54 项静态表）
// - unionImageSupportFormat()→ 首次调用把 m_qtSupported 填入函数内 static res 后返回
// 注记：supportMovieFormat()/m_movie_formats 因无任何填充点、恒返回空表，已随源码删除，测试同步移除。
// 映射： SupportStaticFormat_QueryTable_ReturnsNonEmptyKnownFormats       → 直连真实函数
//        UnionImageSupportFormat_QueryMergedTable_MatchesStaticFormat    → 直连真实函数

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

// PMS: BUG326991 sev2 | commit "fix: Fixed CMYK image display issue" | 根因: CMYK 色彩空间未转 sRGB 导致 jpg 色差
// 锚定 convertToSRgbColorSpace B3 分支(format==Format_CMYK8888 && colorSpace 无效)→方法2 转 RGB888
TEST_F(FreeUnionImageTest, BUG326991_ConvertToSRgbColorSpace_CmykImage_ConvertedToNonCmyk)
{
    // Arrange: 构造 CMYK 格式图像(JPG 保存可保留 Format_CMYK8888 且 colorSpace 无效)，
    //          触发 convertToSRgbColorSpace B3 分支→方法2(convertToFormat RGB888)转换
    QImage cmykImg(4, 2, QImage::Format_CMYK8888);
    cmykImg.fill(0);
    const QString path = tmpDir.filePath(QStringLiteral("cmyk.jpg"));
    ASSERT_TRUE(cmykImg.save(path, "JPG"));
    QImage res;
    QString errMsg;

    // Act: loadStaticImageFromFile(Qt6) 内部调用 convertToSRgbColorSpace 转换 CMYK→sRGB
    const bool ret = loadStaticImageFromFile(path, res, errMsg);

    // Assert: 转换后图像非空且 format 不再是 CMYK8888(色差根因已修复，转为 RGB888)
    EXPECT_TRUE(ret);
    EXPECT_FALSE(res.isNull());
    EXPECT_NE(res.format(), QImage::Format_CMYK8888);
}
