// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_imageutils.h"
#include "imageutils.h"
#include "unionimage.h"

#include <QImage>
#include <QPixmap>
#include <QPixmapCache>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDateTime>

using namespace Libutils::image;
using LibUnionImage_NameSpace::rotateImageFile;

void ut_imageutils::SetUp() {}
void ut_imageutils::TearDown() {}

// 辅助:创建一张临时 PNG 并返回路径
static QString makeTempPng(QTemporaryDir &dir, const QString &name, int w = 4, int h = 4)
{
    const QString path = dir.path() + "/" + name;
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(Qt::red);
    img.save(path, "PNG");
    return path;
}

// ---------------- imageSupportRead ----------------
TEST_F(ut_imageutils, ImageSupportRead_WhenCommonExt_ReturnsTrue)
{
    EXPECT_TRUE(imageSupportRead("/tmp/foo.png"));
    EXPECT_TRUE(imageSupportRead("/tmp/foo.jpg"));
}

TEST_F(ut_imageutils, ImageSupportRead_WhenIcnsExt_ReturnsTrue)
{
    EXPECT_TRUE(imageSupportRead("/tmp/foo.icns"));
}

TEST_F(ut_imageutils, ImageSupportRead_WhenX3FExt_ReturnsFalse)
{
    EXPECT_FALSE(imageSupportRead("/tmp/foo.x3f"));
}

// ---------------- imageSupportSave ----------------
TEST_F(ut_imageutils, ImageSupportSave_DelegatesToUnionImageCanSave)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "s.png");
    EXPECT_TRUE(imageSupportSave(path));
}

TEST_F(ut_imageutils, ImageSupportSave_WhenUnsupportedExt_ReturnsFalse)
{
    EXPECT_FALSE(imageSupportSave("/tmp/foo.zzzunknown"));
}

// ---------------- rotate ----------------
TEST_F(ut_imageutils, Rotate_WhenAngleInvalid_ReturnsFalse)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "r.png");
    EXPECT_FALSE(rotate(path, 30));
}

TEST_F(ut_imageutils, Rotate_WhenPngAndAngle90_ReturnsTrue)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "r.png", 4, 2);
    EXPECT_TRUE(rotate(path, 90));
}

// ---------------- cutSquareImage (overload 1) ----------------
TEST_F(ut_imageutils, CutSquareImage_WhenSingleArg_DelegatesToTwoArgOverload)
{
    QPixmap src(10, 6);
    src.fill(Qt::blue);
    const QPixmap out = cutSquareImage(src);
    EXPECT_FALSE(out.isNull());
}

// ---------------- cutSquareImage (overload 2) ----------------
TEST_F(ut_imageutils, CutSquareImage_WhenSizeProvided_ReturnsNonNullPixmap)
{
    QPixmap src(20, 20);
    src.fill(Qt::green);
    const QPixmap out = cutSquareImage(src, QSize(8, 8));
    EXPECT_FALSE(out.isNull());
}

// ---------------- getImagesInfo ----------------
TEST_F(ut_imageutils, GetImagesInfo_WhenNonRecursive_ReturnsFilesInDir)
{
    QTemporaryDir dir;
    makeTempPng(dir, "a.png");
    makeTempPng(dir, "b.png");
    const QFileInfoList infos = getImagesInfo(dir.path(), false);
    EXPECT_EQ(infos.size(), 2);
}

TEST_F(ut_imageutils, GetImagesInfo_WhenRecursive_ReturnsAllFiles)
{
    QTemporaryDir dir;
    makeTempPng(dir, "a.png");
    QDir().mkdir(dir.path() + "/sub");
    QImage img(2, 2, QImage::Format_RGB32);
    img.save(dir.path() + "/sub/c.png", "PNG");
    const QFileInfoList infos = getImagesInfo(dir.path(), true);
    EXPECT_EQ(infos.size(), 2);
}

// ---------------- getOrientation ----------------
TEST_F(ut_imageutils, GetOrientation_AlwaysReturnsOne)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "o.png");
    EXPECT_EQ(getOrientation(path), 1);
}

// ---------------- getRotatedImage ----------------
TEST_F(ut_imageutils, GetRotatedImage_WhenValidPng_ReturnsNonEmptyImage)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "g.png");
    const QImage img = getRotatedImage(path);
    EXPECT_FALSE(img.isNull());
}

TEST_F(ut_imageutils, GetRotatedImage_WhenFileNotExists_ReturnsNullImage)
{
    const QImage img = getRotatedImage("/nonexistent/file.png");
    EXPECT_TRUE(img.isNull());
}

// ---------------- scaleImage ----------------
TEST_F(ut_imageutils, ScaleImage_WhenValidImage_ReturnsScaledImage)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "sc.png", 100, 100);
    const QImage out = scaleImage(path, QSize(20, 20));
    EXPECT_FALSE(out.isNull());
}

TEST_F(ut_imageutils, ScaleImage_WhenUnsupportedFormat_ReturnsNullImage)
{
    const QImage out = scaleImage("/tmp/file.zzzunknown", QSize(20, 20));
    EXPECT_TRUE(out.isNull());
}

// ---------------- getCreateDateTime ----------------
TEST_F(ut_imageutils, GetCreateDateTime_WhenFileExists_ReturnsValidDateTime)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "d.png");
    const QDateTime dt = getCreateDateTime(path);
    EXPECT_TRUE(dt.isValid());
}

// ---------------- getAllMetaData ----------------
TEST_F(ut_imageutils, GetAllMetaData_WhenValidPng_ReturnsPopulatedMap)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "m.png");
    const QMap<QString, QString> data = getAllMetaData(path);
    EXPECT_TRUE(data.contains("FileName"));
    EXPECT_TRUE(data.contains("FileFormat"));
    EXPECT_EQ(data.value("FileFormat"), QString("png"));
}

// ---------------- cachePixmap ----------------
TEST_F(ut_imageutils, CachePixmap_WhenFileExists_ReturnsNonEmptyPixmap)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "c.png");
    QPixmapCache::clear();
    const QPixmap pix = cachePixmap(path);
    EXPECT_FALSE(pix.isNull());
}

TEST_F(ut_imageutils, CachePixmap_WhenCalledTwice_SecondFromCache)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "c2.png");
    QPixmapCache::clear();
    const QPixmap p1 = cachePixmap(path);
    const QPixmap p2 = cachePixmap(path);
    EXPECT_FALSE(p1.isNull());
    EXPECT_FALSE(p2.isNull());
}

// ---------------- thumbnailCachePath ----------------
TEST_F(ut_imageutils, ThumbnailCachePath_ReturnsPathEndingWithThumbnails)
{
    const QString p = thumbnailCachePath();
    EXPECT_TRUE(p.endsWith("/thumbnails"));
    EXPECT_TRUE(QFileInfo(p).exists());
}

// ---------------- thumbnailPath ----------------
TEST_F(ut_imageutils, ThumbnailPath_WhenThumbNormal_ReturnsNormalSubdir)
{
    const QString p = thumbnailPath("/tmp/foo.png", ThumbNormal);
    EXPECT_TRUE(p.contains("/normal/"));
    EXPECT_TRUE(p.endsWith(".png"));
}

TEST_F(ut_imageutils, ThumbnailPath_WhenThumbLarge_ReturnsLargeSubdir)
{
    const QString p = thumbnailPath("/tmp/foo.png", ThumbLarge);
    EXPECT_TRUE(p.contains("/large/"));
}

TEST_F(ut_imageutils, ThumbnailPath_WhenThumbFail_ReturnsFailSubdir)
{
    const QString p = thumbnailPath("/tmp/foo.png", ThumbFail);
    EXPECT_TRUE(p.contains("/fail/"));
}

// ---------------- thumbnailExist ----------------
TEST_F(ut_imageutils, ThumbnailExist_WhenNoCache_ReturnsFalse)
{
    // 用一个独特路径,确保缓存不存在
    EXPECT_FALSE(thumbnailExist("/tmp/definitely_not_existing_unique_path.png", ThumbLarge));
}

// ---------------- generateThumbnail ----------------
TEST_F(ut_imageutils, GenerateThumbnail_WhenValidPng_ReturnsTrueAndCreatesCache)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "thumb.png", 100, 100);
    EXPECT_TRUE(generateThumbnail(path));
    EXPECT_TRUE(thumbnailExist(path, ThumbLarge));
}

// ---------------- getThumbnail ----------------
TEST_F(ut_imageutils, GetThumbnail_WhenCacheOnlyAndNoCache_ReturnsEmptyPixmap)
{
    const QPixmap pix = getThumbnail("/tmp/no_such_file_unique.png", true);
    EXPECT_TRUE(pix.isNull());
}

TEST_F(ut_imageutils, GetThumbnail_WhenCacheExists_ReturnsNonEmptyPixmap)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "gt.png", 100, 100);
    ASSERT_TRUE(generateThumbnail(path));
    const QPixmap pix = getThumbnail(path, true);
    EXPECT_FALSE(pix.isNull());
}

// ---------------- removeThumbnail ----------------
TEST_F(ut_imageutils, RemoveThumbnail_WhenCalled_RemovesCacheFiles)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "rm.png", 100, 100);
    ASSERT_TRUE(generateThumbnail(path));
    ASSERT_TRUE(thumbnailExist(path, ThumbLarge));
    removeThumbnail(path);
    EXPECT_FALSE(thumbnailExist(path, ThumbLarge));
}

// ---------------- supportedImageFormats ----------------
TEST_F(ut_imageutils, SupportedImageFormats_ReturnsNonEmptyListWithGlobPrefix)
{
    const QStringList list = supportedImageFormats();
    EXPECT_FALSE(list.isEmpty());
    EXPECT_TRUE(list.first().startsWith("*."));
}

// ---------------- imageSupportWallPaper ----------------
TEST_F(ut_imageutils, ImageSupportWallPaper_WhenSupportedPng_ReturnsTrue)
{
    QTemporaryDir dir;
    const QString path = makeTempPng(dir, "wp.png");
    EXPECT_TRUE(imageSupportWallPaper(path));
}

TEST_F(ut_imageutils, ImageSupportWallPaper_WhenUnsupportedExt_ReturnsFalse)
{
    EXPECT_FALSE(imageSupportWallPaper("/tmp/file.zzzunknown"));
}

// ---------------- makeVaultLocalPath ----------------
TEST_F(ut_imageutils, MakeVaultLocalPath_WhenBaseProvided_UsesGivenBase)
{
    const QString p = makeVaultLocalPath("sub/file.txt", "mybase");
    EXPECT_TRUE(p.contains("mybase"));
    EXPECT_TRUE(p.endsWith("sub/file.txt"));
}

TEST_F(ut_imageutils, MakeVaultLocalPath_WhenBaseEmpty_UsesDefaultBase)
{
    const QString p = makeVaultLocalPath("file.txt", "");
    EXPECT_TRUE(p.contains(VAULT_DECRYPT_DIR_NAME));
}

TEST_F(ut_imageutils, MakeVaultLocalPath_WhenPathStartsWithSlash_DoesNotDoubleSlash)
{
    const QString p = makeVaultLocalPath("/file.txt", "base");
    EXPECT_FALSE(p.contains("base//file.txt"));
}

// ---------------- isVaultFile ----------------
TEST_F(ut_imageutils, IsVaultFile_WhenPathUnderVault_ReturnsTrue)
{
    const QString vaultPath = makeVaultLocalPath("file.txt", "");
    EXPECT_TRUE(isVaultFile(vaultPath));
}

TEST_F(ut_imageutils, IsVaultFile_WhenRegularPath_ReturnsFalse)
{
    EXPECT_FALSE(isVaultFile("/tmp/some_regular_file.png"));
}

// ---------------- isCanRemove ----------------
TEST_F(ut_imageutils, IsCanRemove_WhenRegularPath_ReturnsTrue)
{
    EXPECT_TRUE(isCanRemove("/tmp/some_regular_file.png"));
}

TEST_F(ut_imageutils, IsCanRemove_WhenVaultFile_ReturnsFalse)
{
    const QString vaultPath = makeVaultLocalPath("file.txt", "");
    EXPECT_FALSE(isCanRemove(vaultPath));
}

TEST_F(ut_imageutils, IsCanRemove_WhenTrashPath_ReturnsFalse)
{
    const QString trashPath = QDir::homePath() + "/.local/share/Trash/file.png";
    EXPECT_FALSE(isCanRemove(trashPath));
}
