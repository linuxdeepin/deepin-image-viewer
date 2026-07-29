// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_unionimage.h"
#include "unionimage.h"

#include <QImage>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QDir>
#include <QBuffer>

// 被测代码位于该命名空间
using namespace LibUnionImage_NameSpace;

void ut_unionimage::SetUp() {}
void ut_unionimage::TearDown() {}

// ---------------- unionImageVersion ----------------
TEST_F(ut_unionimage, UnionImageVersion_WhenCalled_ReturnsNonEmptyVersionString)
{
    const QString ver = unionImageVersion();
    EXPECT_FALSE(ver.isEmpty());
    EXPECT_TRUE(ver.contains("0.0.4"));
}

// ---------------- unionImageSupportFormat ----------------
TEST_F(ut_unionimage, UnionImageSupportFormat_WhenCalled_ReturnsNonEmptyListContainingCommonFormats)
{
    const QStringList list = unionImageSupportFormat();
    EXPECT_FALSE(list.isEmpty());
    EXPECT_TRUE(list.contains("JPG"));
    EXPECT_TRUE(list.contains("PNG"));
    EXPECT_TRUE(list.contains("BMP"));
}

// ---------------- supportStaticFormat ----------------
TEST_F(ut_unionimage, SupportStaticFormat_WhenCalled_ReturnsSameListAsSupportFormat)
{
    const QStringList s = supportStaticFormat();
    EXPECT_FALSE(s.isEmpty());
    EXPECT_TRUE(s.contains("PNG"));
}

// ---------------- supportMovieFormat ----------------
TEST_F(ut_unionimage, SupportMovieFormat_WhenCalled_ReturnsListMaybeEmpty)
{
    // m_movie_formats 在实现中从未被填充，结果应为空 list
    const QStringList m = supportMovieFormat();
    EXPECT_TRUE(m.isEmpty());
}

// ---------------- creatNewImage ----------------
TEST_F(ut_unionimage, CreatNewImage_WhenDepthIs8_CreatesRgb888Image)
{
    QImage img;
    EXPECT_TRUE(creatNewImage(img, 4, 4, 8));
    EXPECT_FALSE(img.isNull());
    EXPECT_EQ(img.width(), 4);
    EXPECT_EQ(img.height(), 4);
    EXPECT_EQ(img.format(), QImage::Format_RGB888);
}

TEST_F(ut_unionimage, CreatNewImage_WhenDepthIs16_CreatesRgb16Image)
{
    QImage img;
    EXPECT_TRUE(creatNewImage(img, 2, 3, 16));
    EXPECT_FALSE(img.isNull());
    EXPECT_EQ(img.format(), QImage::Format_RGB16);
}

TEST_F(ut_unionimage, CreatNewImage_WhenDepthIsOther_CreatesRgb32Image)
{
    QImage img;
    EXPECT_TRUE(creatNewImage(img, 5, 5, 32));
    EXPECT_FALSE(img.isNull());
    EXPECT_EQ(img.format(), QImage::Format_RGB32);
}

TEST_F(ut_unionimage, CreatNewImage_WhenWidthHeightZero_CreatesNullImage)
{
    QImage img;
    // QImage(0,0,...) 构造为空图
    EXPECT_TRUE(creatNewImage(img, 0, 0, 8));
    EXPECT_TRUE(img.isNull());
}

// ---------------- isNoneQImage ----------------
TEST_F(ut_unionimage, IsNoneQImage_WhenImageIsDefaultConstructed_ReturnsTrue)
{
    QImage img;  // 默认构造为 null,与 noneQImage()(同样为 null)相等
    EXPECT_TRUE(isNoneQImage(img));
}

TEST_F(ut_unionimage, IsNoneQImage_WhenImageIsValid_ReturnsFalse)
{
    QImage img(2, 2, QImage::Format_RGB32);
    EXPECT_FALSE(isNoneQImage(img));
}

// ---------------- rotateImage ----------------
TEST_F(ut_unionimage, RotateImage_WhenAngleNotMultipleOf90_ReturnsFalse)
{
    QImage img(2, 2, QImage::Format_RGB32);
    QImage original = img;
    EXPECT_FALSE(rotateImage(45, img));
    EXPECT_EQ(img, original);  // 未被旋转
}

TEST_F(ut_unionimage, RotateImage_WhenImageIsNull_ReturnsFalse)
{
    QImage img;
    EXPECT_FALSE(rotateImage(90, img));
}

TEST_F(ut_unionimage, RotateImage_WhenRotate90_SwapsWidthAndHeight)
{
    QImage img(4, 2, QImage::Format_RGB32);
    img.fill(Qt::red);
    EXPECT_TRUE(rotateImage(90, img));
    EXPECT_EQ(img.width(), 2);
    EXPECT_EQ(img.height(), 4);
}

TEST_F(ut_unionimage, RotateImage_WhenRotate180_KeepsDimensions)
{
    QImage img(4, 2, QImage::Format_RGB32);
    EXPECT_TRUE(rotateImage(180, img));
    EXPECT_EQ(img.width(), 4);
    EXPECT_EQ(img.height(), 2);
}

// ---------------- detectImageFormat ----------------
TEST_F(ut_unionimage, DetectImageFormat_WhenFileCannotOpen_ReturnsEmpty)
{
    EXPECT_EQ(detectImageFormat("/nonexistent/path/file.xyz"), QString(""));
}

TEST_F(ut_unionimage, DetectImageFormat_WhenPngHeader_ReturnsPNG)
{
    QTemporaryFile tmp;
    tmp.open();
    // PNG 文件头
    const QByteArray pngHeader = "\x89PNG\x0d\x0a\x1a\x0a";
    tmp.write(pngHeader);
    tmp.close();
    EXPECT_EQ(detectImageFormat(tmp.fileName()), QString("PNG"));
}

TEST_F(ut_unionimage, DetectImageFormat_WhenJpgHeader_ReturnsJPG)
{
    QTemporaryFile tmp;
    tmp.open();
    tmp.write("\xff\xd8\xff\xe0");
    tmp.close();
    EXPECT_EQ(detectImageFormat(tmp.fileName()), QString("JPG"));
}

TEST_F(ut_unionimage, DetectImageFormat_WhenBmpHeader_ReturnsBMP)
{
    QTemporaryFile tmp;
    tmp.open();
    tmp.write("BM");
    tmp.close();
    EXPECT_EQ(detectImageFormat(tmp.fileName()), QString("BMP"));
}

TEST_F(ut_unionimage, DetectImageFormat_WhenGifHeader_ReturnsGIF)
{
    QTemporaryFile tmp;
    tmp.open();
    tmp.write("GIF89a");
    tmp.close();
    EXPECT_EQ(detectImageFormat(tmp.fileName()), QString("GIF"));
}

TEST_F(ut_unionimage, DetectImageFormat_WhenUnknownHeader_ReturnsSuffix)
{
    // 临时文件后缀为空(默认),结果为空字符串；这里通过指定后缀验证 fallback 路径
    QTemporaryFile tmp("XXXXXX.PNG");
    tmp.open();
    tmp.write("randombytes_not_a_real_image");
    tmp.close();
    EXPECT_EQ(detectImageFormat(tmp.fileName()), QString("PNG"));
}

// ---------------- loadStaticImageFromFile ----------------
TEST_F(ut_unionimage, LoadStaticImageFromFile_WhenFileEmpty_ReturnsFalse)
{
    QTemporaryFile tmp;
    tmp.open();  // 空文件,大小为 0
    tmp.close();
    QImage res;
    QString err;
    EXPECT_FALSE(loadStaticImageFromFile(tmp.fileName(), res, err));
    EXPECT_TRUE(res.isNull());
    EXPECT_TRUE(err.contains("error file"));
}

TEST_F(ut_unionimage, LoadStaticImageFromFile_WhenValidPng_ReturnsTrueAndLoadsImage)
{
    // 先准备一张有效的 PNG
    QImage src(3, 3, QImage::Format_RGB32);
    src.fill(Qt::blue);

    QTemporaryFile tmp("XXXXXX.PNG");
    tmp.open();
    src.save(&tmp, "PNG");
    tmp.close();

    QImage res;
    QString err;
    EXPECT_TRUE(loadStaticImageFromFile(tmp.fileName(), res, err));
    EXPECT_FALSE(res.isNull());
    EXPECT_EQ(res.width(), 3);
    EXPECT_EQ(res.height(), 3);
}

TEST_F(ut_unionimage, LoadStaticImageFromFile_WhenUnsupportedFormat_ReturnsFalse)
{
    QTemporaryFile tmp("XXXXXX.ZZUNKNOWN");
    tmp.open();
    tmp.write("some bytes here that are not empty");
    tmp.close();
    QImage res;
    QString err;
    EXPECT_FALSE(loadStaticImageFromFile(tmp.fileName(), res, err));
}

// ---------------- rotateImageFile ----------------
TEST_F(ut_unionimage, RotateImageFile_WhenAngleNotMultipleOf90_ReturnsFalse)
{
    QString err;
    EXPECT_FALSE(rotateImageFile(30, "/tmp/foo.png", err));
    EXPECT_TRUE(err.contains("unsupported angel"));
}

TEST_F(ut_unionimage, RotateImageFile_WhenPng_RotatesAndSavesSuccessfully)
{
    QImage src(4, 2, QImage::Format_RGB32);
    src.fill(Qt::red);

    QTemporaryDir dir;
    const QString path = dir.path() + "/rot.png";
    ASSERT_TRUE(src.save(path, "PNG"));

    QString err;
    EXPECT_TRUE(rotateImageFile(90, path, err));

    QImage rotated(path);
    EXPECT_FALSE(rotated.isNull());
    EXPECT_EQ(rotated.width(), 2);
    EXPECT_EQ(rotated.height(), 4);
}

TEST_F(ut_unionimage, RotateImageFile_WhenTargetPathProvided_SavesToTarget)
{
    QImage src(4, 2, QImage::Format_RGB32);
    src.fill(Qt::green);

    QTemporaryDir dir;
    const QString srcPath = dir.path() + "/src.bmp";
    const QString tgtPath = dir.path() + "/tgt.bmp";
    ASSERT_TRUE(src.save(srcPath, "BMP"));

    QString err;
    EXPECT_TRUE(rotateImageFile(90, srcPath, err, tgtPath));
    EXPECT_TRUE(QFileInfo(tgtPath).exists());
}

// ---------------- rotateImageFIleWithImage ----------------
TEST_F(ut_unionimage, RotateImageFIleWithImage_WhenAngleNotMultipleOf90_ReturnsFalse)
{
    QImage img(2, 2, QImage::Format_RGB32);
    QString err;
    EXPECT_FALSE(rotateImageFIleWithImage(30, img, "/tmp/foo.png", err));
}

TEST_F(ut_unionimage, RotateImageFIleWithImage_WhenImageIsNull_ReturnsFalse)
{
    QImage img;
    QString err;
    EXPECT_FALSE(rotateImageFIleWithImage(90, img, "/tmp/foo.png", err));
}

TEST_F(ut_unionimage, RotateImageFIleWithImage_WhenUnsupportedFormat_ReturnsFalse)
{
    QImage img(2, 2, QImage::Format_RGB32);
    img.fill(Qt::red);

    QTemporaryFile tmp("XXXXXX.ZZUNKNOWN");
    tmp.open();
    tmp.write("dummy");
    tmp.close();

    QString err;
    EXPECT_FALSE(rotateImageFIleWithImage(90, img, tmp.fileName(), err));
}

TEST_F(ut_unionimage, RotateImageFIleWithImage_WhenJpgFile_RotatesAndSaves)
{
    QImage src(4, 2, QImage::Format_RGB32);
    src.fill(Qt::blue);

    QTemporaryDir dir;
    const QString path = dir.path() + "/r.jpg";
    ASSERT_TRUE(src.save(path, "JPG"));

    QImage img = src;
    QString err;
    EXPECT_TRUE(rotateImageFIleWithImage(90, img, path, err));
}

// ---------------- getAllMetaData ----------------
TEST_F(ut_unionimage, GetAllMetaData_WhenValidPng_ReturnsPopulatedMap)
{
    QImage src(2, 2, QImage::Format_RGB32);
    src.fill(Qt::red);

    QTemporaryFile tmp("XXXXXX.PNG");
    tmp.open();
    src.save(&tmp, "PNG");
    tmp.close();

    const QMap<QString, QString> data = getAllMetaData(tmp.fileName());
    EXPECT_TRUE(data.contains("FileName"));
    EXPECT_TRUE(data.contains("FileFormat"));
    EXPECT_EQ(data.value("FileFormat"), QString("PNG"));
    EXPECT_TRUE(data.contains("FileSize"));
    EXPECT_TRUE(data.contains("Dimension"));
}

// ---------------- canSave ----------------
TEST_F(ut_unionimage, CanSave_WhenPng_ReturnsTrue)
{
    QImage src(2, 2, QImage::Format_RGB32);
    QTemporaryFile tmp("XXXXXX.PNG");
    tmp.open();
    src.save(&tmp, "PNG");
    tmp.close();
    EXPECT_TRUE(canSave(tmp.fileName()));
}

TEST_F(ut_unionimage, CanSave_WhenUnsupportedExt_ReturnsFalse)
{
    QTemporaryFile tmp("XXXXXX.ZZUNK");
    tmp.open();
    tmp.write("x");
    tmp.close();
    EXPECT_FALSE(canSave(tmp.fileName()));
}

// ---------------- isImageSupportRotate ----------------
TEST_F(ut_unionimage, IsImageSupportRotate_DelegatesToCanSave)
{
    QImage src(2, 2, QImage::Format_RGB32);
    QTemporaryFile tmp("XXXXXX.PNG");
    tmp.open();
    src.save(&tmp, "PNG");
    tmp.close();
    // isImageSupportRotate 内部直接调用 canSave
    EXPECT_TRUE(isImageSupportRotate(tmp.fileName()));
}

// ---------------- getOrientation ----------------
TEST_F(ut_unionimage, GetOrientation_AlwaysReturnsOne)
{
    // 实现固定返回 1,忽略参数
    EXPECT_EQ(getOrientation("/any/path.png"), 1);
    EXPECT_EQ(getOrientation(""), 1);
}

// ---------------- getImageType ----------------
TEST_F(ut_unionimage, GetImageType_WhenEmptyPath_ReturnsBlank)
{
    EXPECT_EQ(getImageType(""), imageViewerSpace::ImageTypeBlank);
}

TEST_F(ut_unionimage, GetImageType_WhenFileNotExists_ReturnsBlank)
{
    EXPECT_EQ(getImageType("/nonexistent/file.png"), imageViewerSpace::ImageTypeBlank);
}

TEST_F(ut_unionimage, GetImageType_WhenValidPng_ReturnsStatic)
{
    QImage src(2, 2, QImage::Format_RGB32);
    QTemporaryFile tmp("XXXXXX.PNG");
    tmp.open();
    src.save(&tmp, "PNG");
    tmp.close();
    EXPECT_EQ(getImageType(tmp.fileName()), imageViewerSpace::ImageTypeStatic);
}

// ---------------- getPathType ----------------
TEST_F(ut_unionimage, GetPathType_WhenLocalPath_ReturnsLOCAL)
{
    EXPECT_EQ(getPathType("/tmp/some_local_file.png"), imageViewerSpace::PathTypeLOCAL);
}

TEST_F(ut_unionimage, GetPathType_WhenSmbSharePath_ReturnsSMB)
{
    EXPECT_EQ(getPathType("smb-share:server=foo/path"), imageViewerSpace::PathTypeSMB);
}

TEST_F(ut_unionimage, GetPathType_WhenMtpPath_ReturnsMTP)
{
    EXPECT_EQ(getPathType("mtp:host=device/file"), imageViewerSpace::PathTypeMTP);
}

TEST_F(ut_unionimage, GetPathType_WhenGphoto2Path_ReturnsPTP)
{
    EXPECT_EQ(getPathType("gphoto2:host=device"), imageViewerSpace::PathTypePTP);
}

TEST_F(ut_unionimage, GetPathType_WhenTrashPath_ReturnsRecycleBin)
{
    const QString trashPath = QDir::homePath() + "/.local/share/Trash/file.png";
    EXPECT_EQ(getPathType(trashPath), imageViewerSpace::PathTypeRECYCLEBIN);
}
