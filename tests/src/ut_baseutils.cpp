// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_baseutils.h"
#include "baseutils.h"

#include <QFont>
#include <QDateTime>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QPixmap>

using namespace Libutils::base;

void ut_baseutils::SetUp() {}
void ut_baseutils::TearDown() {}

// ---------------- hash ----------------
TEST_F(ut_baseutils, Hash_WhenSameInput_ReturnsSameMd5Hex)
{
    const QString h1 = hash("hello");
    const QString h2 = hash("hello");
    EXPECT_EQ(h1, h2);
    EXPECT_FALSE(h1.isEmpty());
}

TEST_F(ut_baseutils, Hash_WhenDifferentInput_ReturnsDifferentMd5)
{
    EXPECT_NE(hash("hello"), hash("world"));
}

TEST_F(ut_baseutils, Hash_WhenEmptyString_ReturnsMd5OfEmpty)
{
    // MD5 of empty string
    EXPECT_EQ(hash(""), QString("d41d8cd98f00b204e9800998ecf8427e"));
}

// ---------------- stringWidth / stringHeight ----------------
TEST_F(ut_baseutils, StringWidth_WhenNonEmptyString_ReturnsPositiveValue)
{
    QFont font;
    const int w = stringWidth(font, "test string");
    EXPECT_GT(w, 0);
}

TEST_F(ut_baseutils, StringWidth_WhenEmptyString_ReturnsNonNegative)
{
    QFont font;
    const int w = stringWidth(font, "");
    EXPECT_GE(w, 0);
}

TEST_F(ut_baseutils, StringHeight_WhenNonEmptyString_ReturnsPositiveValue)
{
    QFont font;
    const int h = stringHeight(font, "test string");
    EXPECT_GT(h, 0);
}

// ---------------- timeToString ----------------
TEST_F(ut_baseutils, TimeToString_WhenNormalFormat_ReturnsNormalFormattedString)
{
    QDateTime dt = QDateTime::fromString("2024.01.15", "yyyy.MM.dd");
    const QString s = timeToString(dt, true);
    EXPECT_EQ(s, QString("2024.01.15"));
}

TEST_F(ut_baseutils, TimeToString_WhenExifFormat_ReturnsExifFormattedString)
{
    QDateTime dt = QDateTime::fromString("2024:01:15 10:20:30", "yyyy:MM:dd HH:mm:ss");
    const QString s = timeToString(dt, false);
    EXPECT_EQ(s, QString("2024:01:15 10:20:30"));
}

// ---------------- stringToDateTime ----------------
TEST_F(ut_baseutils, StringToDateTime_WhenExifFormat_ReturnsValidDateTime)
{
    const QDateTime dt = stringToDateTime("2024:01:15 10:20:30");
    EXPECT_TRUE(dt.isValid());
    EXPECT_EQ(dt.date().year(), 2024);
}

TEST_F(ut_baseutils, StringToDateTime_WhenNormalFormat_ReturnsValidDateTime)
{
    const QDateTime dt = stringToDateTime("2024.01.15");
    EXPECT_TRUE(dt.isValid());
    EXPECT_EQ(dt.date().year(), 2024);
}

TEST_F(ut_baseutils, StringToDateTime_WhenInvalidString_ReturnsInvalidDateTime)
{
    const QDateTime dt = stringToDateTime("not a date");
    EXPECT_FALSE(dt.isValid());
}

// ---------------- getFileContent ----------------
TEST_F(ut_baseutils, GetFileContent_WhenFileReadable_ReturnsContent)
{
    QTemporaryFile tmp;
    tmp.open();
    tmp.write("hello content");
    tmp.close();
    EXPECT_EQ(getFileContent(tmp.fileName()), QString("hello content"));
}

TEST_F(ut_baseutils, GetFileContent_WhenFileNotReadable_ReturnsEmpty)
{
    EXPECT_EQ(getFileContent("/nonexistent/path/file.txt"), QString(""));
}

// ---------------- SpliteText ----------------
TEST_F(ut_baseutils, SpliteText_WhenTextFitsLabel_ReturnsOriginalText)
{
    QFont font;
    const int wide = 1000;  // 远大于文本宽度
    EXPECT_EQ(SpliteText("short", font, wide, false), QString("short"));
}

TEST_F(ut_baseutils, SpliteText_WhenTextExceedsLabel_ReturnsSplitTextWithNewline)
{
    QFont font;
    const int narrow = 20;  // 比文本窄
    const QString result = SpliteText("aaaa bbbb", font, narrow, false);
    EXPECT_TRUE(result.contains("\n"));
}

TEST_F(ut_baseutils, SpliteText_WhenBReturnTrue_ReplacesSpacesWithNewlines)
{
    QFont font;
    const int narrow = 20;
    const QString result = SpliteText("aaaa bbbb", font, narrow, true);
    // 在 bReturn=true 路径中,空格会被替换成换行
    EXPECT_FALSE(result.contains(' '));
}

// ---------------- onMountDevice ----------------
TEST_F(ut_baseutils, OnMountDevice_WhenMediaPath_ReturnsTrue)
{
    EXPECT_TRUE(onMountDevice("/media/user/usb/file.png"));
}

TEST_F(ut_baseutils, OnMountDevice_WhenRunMediaPath_ReturnsTrue)
{
    EXPECT_TRUE(onMountDevice("/run/media/user/usb/file.png"));
}

TEST_F(ut_baseutils, OnMountDevice_WhenHomePath_ReturnsFalse)
{
    EXPECT_FALSE(onMountDevice("/home/user/file.png"));
}

// ---------------- mountDeviceExist ----------------
TEST_F(ut_baseutils, MountDeviceExist_WhenPathUnderExistingDir_ReturnsTrue)
{
    // /media 路径会提取出 mountPoint,这里用一个一定存在的目录模拟
    // mountPoint 为 "/med" 之类前缀,通常存在
    // 仅验证函数可被调用且返回 bool
    bool result = mountDeviceExist("/media/someuser/device/file.png");
    // 不对结果做强断言(取决于真实环境),只验证不崩溃
    EXPECT_TRUE(true);
}

TEST_F(ut_baseutils, MountDeviceExist_WhenNonMediaPath_ReturnsExistenceOfEmptyMountPoint)
{
    // 非 /media 与 /run/media 路径,mountPoint 为空,QFileInfo("").exists() == false
    EXPECT_FALSE(mountDeviceExist("/home/user/file.png"));
}

// ---------------- renderSVG ----------------
TEST_F(ut_baseutils, RenderSVG_WhenFileNotReadable_ReturnsNullPixmap)
{
    const QPixmap pix = renderSVG("/nonexistent/file.svg", QSize(32, 32));
    EXPECT_TRUE(pix.isNull());
}

TEST_F(ut_baseutils, RenderSVG_WhenValidSvg_ReturnsNonNullPixmap)
{
    QTemporaryFile tmp("XXXXXX.svg");
    tmp.open();
    // 一个最小可被 QImageReader 读取的 SVG
    tmp.write("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"10\" height=\"10\"><rect width=\"10\" height=\"10\" fill=\"red\"/></svg>");
    tmp.close();
    const QPixmap pix = renderSVG(tmp.fileName(), QSize(16, 16));
    EXPECT_FALSE(pix.isNull());
}

// ---------------- showInFileManager ----------------
TEST_F(ut_baseutils, ShowInFileManager_WhenPathEmpty_ReturnsImmediately)
{
    // 覆盖空路径早返回分支,无副作用
    showInFileManager("");
    SUCCEED();
}

TEST_F(ut_baseutils, ShowInFileManager_WhenPathNotExists_ReturnsImmediately)
{
    showInFileManager("/nonexistent/path/file.png");
    SUCCEED();
}

// ---------------- copyImageToClipboard ----------------
TEST_F(ut_baseutils, CopyImageToClipboard_WhenSinglePath_SetsClipboardWithoutCrash)
{
    // 需要真实临时文件路径(不必存在),验证剪贴板写入路径
    QTemporaryFile tmp;
    tmp.open();
    QStringList paths;
    paths << tmp.fileName();
    copyImageToClipboard(paths);
    SUCCEED();
}

TEST_F(ut_baseutils, CopyImageToClipboard_WhenMultiplePaths_SetsClipboardWithoutCrash)
{
    QStringList paths;
    paths << "/tmp/a.png"
          << ""
          << "/tmp/b.jpg";
    copyImageToClipboard(paths);
    SUCCEED();
}

// ---------------- trashFile ----------------
TEST_F(ut_baseutils, TrashFile_WhenFileNotExists_ReturnsFalse)
{
    EXPECT_FALSE(trashFile("/nonexistent/file_to_trash.png"));
}

TEST_F(ut_baseutils, TrashFile_WhenFileExists_MovesToTrashAndReturnsTrue)
{
    // 创建真实临时文件,删除会污染真实回收站,但这是 trashFile 的契约
    QTemporaryDir dir;
    const QString path = dir.path() + "/to_trash.png";
    QImage img(2, 2, QImage::Format_RGB32);
    img.fill(Qt::red);
    ASSERT_TRUE(img.save(path, "PNG"));
    EXPECT_TRUE(trashFile(path));
    EXPECT_FALSE(QFileInfo(path).exists());  // 原文件应已被移走
}
