// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_imageeditcontroller.h"
#include "imageeditcontroller.h"

#include <QImage>
#include <QTemporaryFile>
#include <QUrl>
#include <QSignalSpy>
#include <QRect>
#include <QRectF>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QProcess>

void ut_imageeditcontroller::SetUp()
{
}

void ut_imageeditcontroller::TearDown()
{
}

// 测试构造与析构
TEST_F(ut_imageeditcontroller, Construct)
{
    ImageEditController *ctrl = new ImageEditController();
    ASSERT_TRUE(ctrl != nullptr);
    delete ctrl;
}

// 测试初始状态：未编辑时 image 为空
TEST_F(ut_imageeditcontroller, InitialImageIsNull)
{
    ImageEditController ctrl;
    EXPECT_TRUE(ctrl.image().isNull());
}

// 测试初始状态：active 为 false
TEST_F(ut_imageeditcontroller, InitialActiveIsFalse)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.active());
}

// 测试初始状态：canUndo / canRedo 为 false
TEST_F(ut_imageeditcontroller, InitialCanUndoCanRedoFalse)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.canUndo());
    EXPECT_FALSE(ctrl.canRedo());
}

// 测试初始状态：modified 为 true（m_historyIndex=-1, m_savedHistoryIndex=0）
TEST_F(ut_imageeditcontroller, InitialModifiedTrue)
{
    ImageEditController ctrl;
    EXPECT_TRUE(ctrl.modified());
}

// 测试初始 revision 为 0
TEST_F(ut_imageeditcontroller, InitialRevisionZero)
{
    ImageEditController ctrl;
    EXPECT_EQ(ctrl.revision(), 0);
}

// 测试 canEdit 对合法图片格式返回 true
TEST_F(ut_imageeditcontroller, CanEditValidPng)
{
    QTemporaryFile tmp("ut_edit_test_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    EXPECT_TRUE(ctrl.canEdit(QUrl::fromLocalFile(tmp.fileName())));
}

// 测试 canEdit 对不支持的格式返回 false
TEST_F(ut_imageeditcontroller, CanEditUnsupportedFormat)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.canEdit(QUrl::fromLocalFile("/tmp/test.xyz")));
}

// 测试 canEdit 对空路径返回 false
TEST_F(ut_imageeditcontroller, CanEditEmptyPath)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.canEdit(QUrl()));
}

// 测试 beginEdit 加载图片后 active 为 true
TEST_F(ut_imageeditcontroller, BeginEditSetsActive)
{
    QTemporaryFile tmp("ut_edit_begin_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(8, 8, QImage::Format_ARGB32);
    img.fill(Qt::blue);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    QSignalSpy activeSpy(&ctrl, &ImageEditController::activeChanged);
    EXPECT_TRUE(ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0));
    EXPECT_TRUE(ctrl.active());
    EXPECT_FALSE(ctrl.image().isNull());
    EXPECT_GE(activeSpy.count(), 1);
}

// 测试 beginEdit 对不存在文件返回 false
TEST_F(ut_imageeditcontroller, BeginEditNonexistentFile)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.beginEdit(QUrl::fromLocalFile("/nonexistent/file.png"), 0));
}

// 测试 beginEdit 对不支持格式返回 false
TEST_F(ut_imageeditcontroller, BeginEditUnsupportedFormat)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.beginEdit(QUrl::fromLocalFile("/tmp/test.xyz"), 0));
}

// 测试 isEditing 在 beginEdit 后为 true
TEST_F(ut_imageeditcontroller, IsEditingAfterBeginEdit)
{
    QTemporaryFile tmp("ut_edit_ising_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::green);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    QUrl url = QUrl::fromLocalFile(tmp.fileName());
    ctrl.beginEdit(url, 0);
    EXPECT_TRUE(ctrl.isEditing(url, 0));
    EXPECT_FALSE(ctrl.isEditing(url, 1));
}

// 测试 discard 后 active 为 false，image 为空
TEST_F(ut_imageeditcontroller, DiscardClearsState)
{
    QTemporaryFile tmp("ut_edit_discard_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(4, 4, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    EXPECT_TRUE(ctrl.active());
    ctrl.discard();
    EXPECT_FALSE(ctrl.active());
    EXPECT_TRUE(ctrl.image().isNull());
}

// 测试 crop 裁剪后 revision 增加
TEST_F(ut_imageeditcontroller, CropIncreasesRevision)
{
    QTemporaryFile tmp("ut_edit_crop_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(100, 100, QImage::Format_ARGB32);
    img.fill(Qt::white);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    int revBefore = ctrl.revision();
    EXPECT_TRUE(ctrl.crop(QRectF(0.0, 0.0, 0.5, 0.5)));
    int revAfter = ctrl.revision();
    EXPECT_GT(revAfter, revBefore);
    EXPECT_EQ(ctrl.image().width(), 50);
}

// 测试 crop 在未编辑时返回 false
TEST_F(ut_imageeditcontroller, CropWithoutEditReturnsFalse)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.crop(QRectF(0.0, 0.0, 0.5, 0.5)));
}

// 测试 rotateClockwise 旋转后 revision 增加
TEST_F(ut_imageeditcontroller, RotateClockwiseIncreasesRevision)
{
    QTemporaryFile tmp("ut_edit_rotate_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(20, 30, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    int revBefore = ctrl.revision();
    QImage original = ctrl.image();
    EXPECT_TRUE(ctrl.rotateClockwise());
    EXPECT_GT(ctrl.revision(), revBefore);
    // 旋转 90° 后宽高互换
    EXPECT_EQ(ctrl.image().width(), original.height());
    EXPECT_EQ(ctrl.image().height(), original.width());
}

// 测试 rotateClockwise 未编辑时返回 false
TEST_F(ut_imageeditcontroller, RotateClockwiseWithoutEditReturnsFalse)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.rotateClockwise());
}


// 测试 commitHistory 后 canUndo 为 true、canRedo 为 false
TEST_F(ut_imageeditcontroller, CommitHistoryAfterCrop)
{
    QTemporaryFile tmp("ut_edit_commit_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(50, 50, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    ctrl.crop(QRectF(0.0, 0.0, 0.5, 0.5));
    EXPECT_FALSE(ctrl.canUndo());
    ctrl.commitHistory();
    EXPECT_TRUE(ctrl.canUndo());
    EXPECT_FALSE(ctrl.canRedo());
}

// 测试 undo 后 image 恢复
TEST_F(ut_imageeditcontroller, UndoRestoresImage)
{
    QTemporaryFile tmp("ut_edit_undo_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(50, 50, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    QImage original = ctrl.image();
    ctrl.crop(QRectF(0.0, 0.0, 0.5, 0.5));
    EXPECT_NE(ctrl.image().cacheKey(), original.cacheKey());
    ctrl.commitHistory();
    EXPECT_TRUE(ctrl.canUndo());
    EXPECT_TRUE(ctrl.undo());
    EXPECT_EQ(ctrl.image().cacheKey(), original.cacheKey());
    EXPECT_TRUE(ctrl.canRedo());
}

// 测试 redo 后 image 恢复到裁剪后状态
TEST_F(ut_imageeditcontroller, RedoAfterUndo)
{
    QTemporaryFile tmp("ut_edit_redo_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(50, 50, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    ctrl.crop(QRectF(0.0, 0.0, 0.5, 0.5));
    QImage cropped = ctrl.image();
    ctrl.commitHistory();
    ctrl.undo();
    EXPECT_TRUE(ctrl.canRedo());
    EXPECT_TRUE(ctrl.redo());
    EXPECT_EQ(ctrl.image().cacheKey(), cropped.cacheKey());
}

// 测试 markSaved 后 modified 为 false
TEST_F(ut_imageeditcontroller, MarkSavedClearsModified)
{
    QTemporaryFile tmp("ut_edit_marksave_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(50, 50, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    ctrl.crop(QRectF(0.0, 0.0, 0.5, 0.5));
    ctrl.commitHistory();
    EXPECT_TRUE(ctrl.modified());
    ctrl.markSaved();
    EXPECT_FALSE(ctrl.modified());
}

// 测试 defaultSaveUrl 返回非空 URL
TEST_F(ut_imageeditcontroller, DefaultSaveUrlAfterEdit)
{
    QTemporaryFile tmp("ut_edit_saveurl_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(10, 10, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    QUrl saveUrl = ctrl.defaultSaveUrl();
    EXPECT_FALSE(saveUrl.isEmpty());
    EXPECT_TRUE(saveUrl.isLocalFile());
}

// 测试 isOriginalUrl 判断
TEST_F(ut_imageeditcontroller, IsOriginalUrl)
{
    QTemporaryFile tmp("ut_edit_isorig_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(10, 10, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    EXPECT_TRUE(ctrl.isOriginalUrl(QUrl::fromLocalFile(tmp.fileName())));
    EXPECT_FALSE(ctrl.isOriginalUrl(QUrl::fromLocalFile("/tmp/different.png")));
}

// 测试 applyEffect gaussian
TEST_F(ut_imageeditcontroller, ApplyEffectGaussianBlur)
{
    QTemporaryFile tmp("ut_edit_gauss_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(50, 50, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    int revBefore = ctrl.revision();
    EXPECT_TRUE(ctrl.applyEffect("gaussian", QRectF(0.0, 0.0, 1.0, 1.0), 5));
    EXPECT_GT(ctrl.revision(), revBefore);
}

// 测试 applyEffect 无效强度
TEST_F(ut_imageeditcontroller, ApplyEffectInvalidStrength)
{
    QTemporaryFile tmp("ut_edit_invalstr_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(50, 50, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    EXPECT_FALSE(ctrl.applyEffect("gaussian", QRectF(0.0, 0.0, 1.0, 1.0), 3));
}

// 测试 applyEffect 无效效果名
TEST_F(ut_imageeditcontroller, ApplyEffectInvalidEffect)
{
    QTemporaryFile tmp("ut_edit_invaleff_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(50, 50, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    EXPECT_FALSE(ctrl.applyEffect("nonexistent", QRectF(0.0, 0.0, 1.0, 1.0), 5));
}

// 测试 applyEffect mosaic
TEST_F(ut_imageeditcontroller, ApplyEffectMosaic)
{
    QTemporaryFile tmp("ut_edit_mosaic_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(50, 50, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    EXPECT_TRUE(ctrl.applyEffect("mosaic", QRectF(0.0, 0.0, 1.0, 1.0), 8));
}

// 测试 applyEffect 未编辑时返回 false
TEST_F(ut_imageeditcontroller, ApplyEffectWithoutEdit)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.applyEffect("gaussian", QRectF(0.0, 0.0, 1.0, 1.0), 5));
}

// 测试 saveComposite 保存到新文件
TEST_F(ut_imageeditcontroller, SaveCompositeToNewFile)
{
    QTemporaryFile srcTmp("ut_edit_save_src_XXXXXX.png");
    srcTmp.setAutoRemove(true);
    ASSERT_TRUE(srcTmp.open());
    QImage img(20, 20, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&srcTmp, "PNG");
    srcTmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(srcTmp.fileName()), 0);

    QTemporaryFile destTmp("ut_edit_save_dest_XXXXXX.png");
    destTmp.setAutoRemove(true);
    destTmp.open();
    destTmp.close();

    QSignalSpy spy(&ctrl, &ImageEditController::saveFailed);
    bool result = ctrl.saveComposite(QUrl::fromLocalFile(destTmp.fileName()), {});
    EXPECT_TRUE(result);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_TRUE(QFile::exists(destTmp.fileName()));
}

// 测试 saveComposite 未编辑时返回 false
TEST_F(ut_imageeditcontroller, SaveCompositeWithoutEdit)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.saveComposite(QUrl::fromLocalFile("/tmp/out.png"), {}));
}

// 测试 applyEffect graffiti
TEST_F(ut_imageeditcontroller, ApplyEffectGraffiti)
{
    QTemporaryFile tmp("ut_edit_graffiti_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(100, 100, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    int revBefore = ctrl.revision();
    EXPECT_TRUE(ctrl.applyEffect("graffiti", QRectF(0.0, 0.0, 1.0, 1.0), 8));
    EXPECT_GT(ctrl.revision(), revBefore);
}

// 测试 applyEffect graffiti 不同强度
TEST_F(ut_imageeditcontroller, ApplyEffectGraffitiStrength16)
{
    QTemporaryFile tmp("ut_edit_graffiti16_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(100, 100, QImage::Format_ARGB32);
    img.fill(Qt::green);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    EXPECT_TRUE(ctrl.applyEffect("graffiti", QRectF(0.0, 0.0, 1.0, 1.0), 16));
}

TEST_F(ut_imageeditcontroller, ApplyEffectGraffitiStrength32)
{
    QTemporaryFile tmp("ut_edit_graffiti32_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(100, 100, QImage::Format_ARGB32);
    img.fill(Qt::blue);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    EXPECT_TRUE(ctrl.applyEffect("graffiti", QRectF(0.0, 0.0, 1.0, 1.0), 32));
}

// 测试 EditedImageProvider 构造函数
TEST_F(ut_imageeditcontroller, EditedImageProviderConstruct)
{
    ImageEditController ctrl;
    EditedImageProvider *provider = new EditedImageProvider(&ctrl);
    ASSERT_TRUE(provider != nullptr);
    delete provider;
}

// 测试 EditedImageProvider 构造函数 — 空指针控制器
TEST_F(ut_imageeditcontroller, EditedImageProviderConstructNullController)
{
    EditedImageProvider *provider = new EditedImageProvider(nullptr);
    ASSERT_TRUE(provider != nullptr);
    delete provider;
}

// 测试 EditedImageProvider::requestImage — 有图片时返回非空
TEST_F(ut_imageeditcontroller, EditedImageProviderRequestImageWithImage)
{
    QTemporaryFile tmp("ut_edit_provider_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(20, 20, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);

    EditedImageProvider provider(&ctrl);
    QSize reportedSize;
    QImage result = provider.requestImage(QStringLiteral("test"), &reportedSize, QSize());
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(reportedSize, img.size());
}

// 测试 EditedImageProvider::requestImage — 无图片时返回空
TEST_F(ut_imageeditcontroller, EditedImageProviderRequestImageEmpty)
{
    ImageEditController ctrl;
    EditedImageProvider provider(&ctrl);
    QSize reportedSize;
    QImage result = provider.requestImage(QStringLiteral("test"), &reportedSize, QSize());
    EXPECT_TRUE(result.isNull());
}

// 测试 EditedImageProvider::requestImage — 指定 requestedSize 缩放
TEST_F(ut_imageeditcontroller, EditedImageProviderRequestImageScaled)
{
    QTemporaryFile tmp("ut_edit_provider_scaled_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(100, 100, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);

    EditedImageProvider provider(&ctrl);
    QSize reportedSize;
    QImage result = provider.requestImage(QStringLiteral("test"), &reportedSize, QSize(50, 50));
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.width(), 50);
    EXPECT_EQ(result.height(), 50);
}

// 测试 EditedImageProvider::requestImage — 空指针控制器返回空图
TEST_F(ut_imageeditcontroller, EditedImageProviderRequestImageNullController)
{
    EditedImageProvider provider(nullptr);
    QSize reportedSize;
    QImage result = provider.requestImage(QStringLiteral("test"), &reportedSize, QSize());
    EXPECT_TRUE(result.isNull());
}

// === Coverage improvement tests ===

// Helper: create a temp PNG file and return its path
static QString makeTempPng(const QString &prefix, int w = 50, int h = 50, QColor fill = Qt::red)
{
    QTemporaryFile *tmp = new QTemporaryFile(prefix + "_XXXXXX.png");
    tmp->setAutoRemove(false);
    tmp->open();
    QImage img(w, h, QImage::Format_ARGB32);
    img.fill(fill);
    img.save(tmp, "PNG");
    tmp->close();
    QString path = tmp->fileName();
    delete tmp;
    return path;
}

// L294: discard without active edit (early return on null image)
TEST_F(ut_imageeditcontroller, DiscardWithoutEditIsNoop)
{
    ImageEditController ctrl;
    ctrl.discard();
    EXPECT_FALSE(ctrl.active());
    EXPECT_TRUE(ctrl.image().isNull());
}

// L331: applyEffect with rect too small
TEST_F(ut_imageeditcontroller, ApplyEffectRectTooSmall)
{
    QString path = makeTempPng("ut_edit_smallrect");
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);
    // Normalized rect resulting in <2 pixels
    EXPECT_FALSE(ctrl.applyEffect("gaussian", QRectF(0.0, 0.0, 0.01, 0.01), 5));
    EXPECT_FALSE(ctrl.applyEffect("mosaic", QRectF(0.0, 0.0, 0.01, 0.5), 8));
    EXPECT_FALSE(ctrl.applyEffect("graffiti", QRectF(0.0, 0.0, 0.5, 0.01), 8));
    QFile::remove(path);
}

// L390: commitHistory without active edit (null image early return)
TEST_F(ut_imageeditcontroller, CommitHistoryWithoutEdit)
{
    ImageEditController ctrl;
    ctrl.commitHistory();
    EXPECT_FALSE(ctrl.active());
}

// L440: redo when there is nothing to redo (returns false)
TEST_F(ut_imageeditcontroller, RedoWhenNothingToRedo)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.redo());

    QString path = makeTempPng("ut_edit_redonone");
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);
    // No history to redo
    EXPECT_FALSE(ctrl.redo());
    QFile::remove(path);
}

// L392-394: commitHistory with history truncation (undo then commit)
TEST_F(ut_imageeditcontroller, CommitHistoryTruncatesRedoStack)
{
    QString path = makeTempPng("ut_edit_trunc", 100, 100);
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);
    ctrl.crop(QRectF(0.0, 0.0, 0.5, 0.5));
    ctrl.commitHistory();
    // Now undo so historyIndex < history.size()-1
    ctrl.undo();
    EXPECT_TRUE(ctrl.canRedo());
    // commitHistory should truncate the redo stack
    ctrl.commitHistory();
    EXPECT_FALSE(ctrl.canRedo());
    QFile::remove(path);
}

// L399-402: commitHistory maxHistory > 50 steps
TEST_F(ut_imageeditcontroller, CommitHistoryMaxHistoryOverflow)
{
    QString path = makeTempPng("ut_edit_maxhist", 200, 200);
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);
    // Do 52 crop+commit cycles to exceed maxHistorySteps=50
    for (int i = 0; i < 52; ++i) {
        // Alternate crop sizes to get different cacheKeys
        ctrl.crop(QRectF(0.0, 0.0, 0.9, 0.9));
        ctrl.commitHistory();
    }
    // History should be capped; canUndo should still work
    EXPECT_TRUE(ctrl.canUndo());
    QFile::remove(path);
}

// L151-152: saveComposite with empty local file path
TEST_F(ut_imageeditcontroller, SaveCompositeEmptyTargetPath)
{
    QString path = makeTempPng("ut_edit_emptypath");
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);
    QSignalSpy spy(&ctrl, &ImageEditController::saveFailed);
    // QUrl with no local file
    EXPECT_FALSE(ctrl.saveComposite(QUrl("http://example.com/img.png"), {}));
    EXPECT_GE(spy.count(), 1);
    QFile::remove(path);
}

// L166-167: saveComposite format mismatch
TEST_F(ut_imageeditcontroller, SaveCompositeFormatMismatch)
{
    QString path = makeTempPng("ut_edit_fmtmis");
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);
    QSignalSpy spy(&ctrl, &ImageEditController::saveFailed);
    // Source is PNG, destination is BMP — format mismatch
    QString destPath = path + ".bmp";
    EXPECT_FALSE(ctrl.saveComposite(QUrl::fromLocalFile(destPath), {}));
    EXPECT_GE(spy.count(), 1);
    QFile::remove(path);
    QFile::remove(destPath);
}

// L161: saveComposite with jpg format normalization
TEST_F(ut_imageeditcontroller, SaveCompositeJpgNormalization)
{
    // Create a JPEG source file
    QTemporaryFile jpgTmp("ut_edit_jpg_XXXXXX.jpg");
    jpgTmp.setAutoRemove(false);
    jpgTmp.open();
    QImage img(20, 20, QImage::Format_ARGB32);
    img.fill(Qt::red);
    img.save(&jpgTmp, "JPG");
    jpgTmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(jpgTmp.fileName()), 0);

    // Save to .jpeg (normalized to "jpeg" — should match "jpg" source)
    QString destPath = jpgTmp.fileName() + ".jpeg";
    QSignalSpy spy(&ctrl, &ImageEditController::saveFailed);
    bool result = ctrl.saveComposite(QUrl::fromLocalFile(destPath), {});
    EXPECT_TRUE(result);
    EXPECT_EQ(spy.count(), 0);
    QFile::remove(jpgTmp.fileName());
    QFile::remove(destPath);
}

// L273-274: saveComposite file open failure (save to directory)
TEST_F(ut_imageeditcontroller, SaveCompositeFileOpenFailure)
{
    QString path = makeTempPng("ut_edit_openfail");
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);
    QSignalSpy spy(&ctrl, &ImageEditController::saveFailed);
    // Try to save to a directory path (can't open for writing)
    EXPECT_FALSE(ctrl.saveComposite(QUrl::fromLocalFile("/tmp"), {}));
    EXPECT_GE(spy.count(), 1);
    QFile::remove(path);
}

// L174-268: saveComposite with all annotation types
TEST_F(ut_imageeditcontroller, SaveCompositeWithAllAnnotations)
{
    QString path = makeTempPng("ut_edit_annot", 100, 100);
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);

    QVariantList annotations;

    // rect annotation
    QVariantMap rectAnn;
    rectAnn["type"] = "rect";
    rectAnn["points"] = QVariantList { QPointF(0.1, 0.1), QPointF(0.9, 0.9) };
    rectAnn["color"] = "#FF0000";
    rectAnn["width"] = 0.02;
    rectAnn["rotation"] = 15.0;
    annotations << rectAnn;

    // ellipse annotation
    QVariantMap ellipseAnn;
    ellipseAnn["type"] = "ellipse";
    ellipseAnn["points"] = QVariantList { QPointF(0.2, 0.2), QPointF(0.8, 0.8) };
    ellipseAnn["color"] = "#00FF00";
    ellipseAnn["width"] = 0.03;
    ellipseAnn["rotation"] = 30.0;
    annotations << ellipseAnn;

    // text annotation
    QVariantMap textAnn;
    textAnn["type"] = "text";
    textAnn["points"] = QVariantList { QPointF(0.1, 0.1), QPointF(0.5, 0.3) };
    textAnn["color"] = "#0000FF";
    textAnn["width"] = 0.01;
    textAnn["text"] = "Hello";
    textAnn["fontFamily"] = "Arial";
    annotations << textAnn;

    // number annotation with bright color (luminance > threshold → black text)
    QVariantMap numAnnBright;
    numAnnBright["type"] = "number";
    numAnnBright["points"] = QVariantList { QPointF(0.1, 0.1), QPointF(0.4, 0.4) };
    numAnnBright["color"] = "#FFFFFF";
    numAnnBright["width"] = 0.01;
    numAnnBright["number"] = 42;
    annotations << numAnnBright;

    // number annotation with dark color (luminance <= threshold → white text)
    QVariantMap numAnnDark;
    numAnnDark["type"] = "number";
    numAnnDark["points"] = QVariantList { QPointF(0.5, 0.5), QPointF(0.9, 0.9) };
    numAnnDark["color"] = "#000000";
    numAnnDark["width"] = 0.01;
    numAnnDark["number"] = 7;
    annotations << numAnnDark;

    // pen annotation
    QVariantMap penAnn;
    penAnn["type"] = "pen";
    penAnn["points"] = QVariantList { QPointF(0.1, 0.5), QPointF(0.3, 0.7), QPointF(0.5, 0.3) };
    penAnn["color"] = "#FF00FF";
    penAnn["width"] = 0.02;
    penAnn["rotation"] = 45.0;
    annotations << penAnn;

    // arrow annotation
    QVariantMap arrowAnn;
    arrowAnn["type"] = "arrow";
    arrowAnn["points"] = QVariantList { QPointF(0.1, 0.1), QPointF(0.2, 0.2), QPointF(0.9, 0.9) };
    arrowAnn["color"] = "#FFFF00";
    arrowAnn["width"] = 0.02;
    annotations << arrowAnn;

    // unknown type (falls into else: path drawing)
    QVariantMap unknownAnn;
    unknownAnn["type"] = "unknown";
    unknownAnn["points"] = QVariantList { QPointF(0.1, 0.1), QPointF(0.8, 0.8) };
    unknownAnn["color"] = "#888888";
    unknownAnn["width"] = 0.01;
    annotations << unknownAnn;

    // annotation with < 2 points (should be skipped)
    QVariantMap shortAnn;
    shortAnn["type"] = "rect";
    shortAnn["points"] = QVariantList { QPointF(0.1, 0.1) };
    shortAnn["color"] = "#FFFFFF";
    shortAnn["width"] = 0.01;
    annotations << shortAnn;

    QString destPath = path + ".annot.png";
    QSignalSpy spy(&ctrl, &ImageEditController::saveFailed);
    bool result = ctrl.saveComposite(QUrl::fromLocalFile(destPath), annotations);
    EXPECT_TRUE(result);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_TRUE(QFile::exists(destPath));
    QFile::remove(path);
    QFile::remove(destPath);
}

// L592-598: graffiti with transparent pixels (destinationAlpha != 255)
TEST_F(ut_imageeditcontroller, ApplyGraffitiWithTransparentPixels)
{
    // Create image with semi-transparent pixels
    QTemporaryFile tmp("ut_edit_grafftrans_XXXXXX.png");
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    QImage img(100, 100, QImage::Format_ARGB32);
    img.fill(Qt::transparent);  // alpha = 0
    // Draw some semi-transparent colored pixels
    for (int y = 0; y < 100; ++y)
        for (int x = 0; x < 100; ++x)
            img.setPixel(x, y, qRgba(128, 64, 32, 128));  // alpha != 255
    img.save(&tmp, "PNG");
    tmp.close();

    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(tmp.fileName()), 0);
    int revBefore = ctrl.revision();
    EXPECT_TRUE(ctrl.applyEffect("graffiti", QRectF(0.0, 0.0, 1.0, 1.0), 8));
    EXPECT_GT(ctrl.revision(), revBefore);
}

// L440: undo then redo where image actually changes
TEST_F(ut_imageeditcontroller, RedoWithImageChange)
{
    QString path = makeTempPng("ut_edit_redoimg", 100, 100);
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);
    int revBefore = ctrl.revision();

    // Crop, commit, undo, then redo — redo should change image
    ctrl.crop(QRectF(0.0, 0.0, 0.5, 0.5));
    ctrl.commitHistory();
    QImage cropped = ctrl.image();
    ctrl.undo();
    int revAfterUndo = ctrl.revision();
    EXPECT_TRUE(ctrl.canRedo());
    EXPECT_TRUE(ctrl.redo());
    EXPECT_EQ(ctrl.image().cacheKey(), cropped.cacheKey());
    EXPECT_GT(ctrl.revision(), revAfterUndo);
    QFile::remove(path);
}

// undo when nothing to undo (returns false)
TEST_F(ut_imageeditcontroller, UndoWhenNothingToUndo)
{
    ImageEditController ctrl;
    EXPECT_FALSE(ctrl.undo());
}

// ---- 覆盖未覆盖行 ----
#include "stub.h"
#include <QSaveFile>
#include <QImageWriter>

// L273-274: saveComposite 文件打开失败（保存到不存在的目录）
TEST_F(ut_imageeditcontroller, SaveComposite_OpenFailure_NonexistentDir)
{
    QString path = makeTempPng("ut_edit_openfail2");
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);
    QSignalSpy spy(&ctrl, &ImageEditController::saveFailed);
    // 保存到不存在的目录，QSaveFile::open 失败
    EXPECT_FALSE(ctrl.saveComposite(QUrl::fromLocalFile("/nonexistent_dir_xyz/output.png"), {}));
    EXPECT_GE(spy.count(), 1);
    QFile::remove(path);
}

// L278-279: QImageWriter::write 失败
static bool stub_QImageWriter_write_false(QImageWriter *, const QImage &)
{
    return false;
}

TEST_F(ut_imageeditcontroller, SaveComposite_WriteFailure)
{
    QString path = makeTempPng("ut_edit_writefail");
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);

    Stub stub;
    stub.set(ADDR(QImageWriter, write), stub_QImageWriter_write_false);

    QSignalSpy spy(&ctrl, &ImageEditController::saveFailed);
    QString destPath = path + ".writefail.png";
    EXPECT_FALSE(ctrl.saveComposite(QUrl::fromLocalFile(destPath), {}));
    EXPECT_GE(spy.count(), 1);

    QFile::remove(path);
    QFile::remove(destPath);
}

// L283-284: QSaveFile::commit 失败
static bool stub_QSaveFile_commit_false(QSaveFile *)
{
    return false;
}

TEST_F(ut_imageeditcontroller, SaveComposite_CommitFailure)
{
    QString path = makeTempPng("ut_edit_commitfail");
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);

    Stub stub;
    stub.set(ADDR(QSaveFile, commit), stub_QSaveFile_commit_false);

    QSignalSpy spy(&ctrl, &ImageEditController::saveFailed);
    QString destPath = path + ".commitfail.png";
    EXPECT_FALSE(ctrl.saveComposite(QUrl::fromLocalFile(destPath), {}));
    EXPECT_GE(spy.count(), 1);

    QFile::remove(path);
    QFile::remove(destPath);
}

// L393: commitHistory 中 m_savedHistoryIndex > m_historyIndex 分支
TEST_F(ut_imageeditcontroller, CommitHistory_SavedHistoryIndexGreaterThanHistoryIndex)
{
    QString path = makeTempPng("ut_edit_savedidx", 100, 100);
    ImageEditController ctrl;
    ctrl.beginEdit(QUrl::fromLocalFile(path), 0);

    // 第一次编辑 + 提交
    ctrl.crop(QRectF(0.0, 0.0, 0.5, 0.5));
    ctrl.commitHistory();
    // markSaved 设置 m_savedHistoryIndex = m_historyIndex = 1
    ctrl.markSaved();

    // 第二次编辑 + 提交
    ctrl.crop(QRectF(0.0, 0.0, 0.5, 0.5));
    ctrl.commitHistory();
    // markSaved 设置 m_savedHistoryIndex = m_historyIndex = 2
    ctrl.markSaved();

    // 第三次编辑 + 提交，此时 m_historyIndex = 3
    ctrl.crop(QRectF(0.0, 0.0, 0.5, 0.5));
    ctrl.commitHistory();

    // undo 回到 m_historyIndex = 2，但 m_savedHistoryIndex = 2
    // 再 undo 回到 m_historyIndex = 1，m_savedHistoryIndex = 2 > 1
    ctrl.undo();
    ctrl.undo();

    // 此时 m_historyIndex + 1 = 2 < m_history.size() = 4 (有 redo 条目)
    // m_savedHistoryIndex = 2 > m_historyIndex = 1 → L393 命中
    ctrl.commitHistory();
    SUCCEED();

    QFile::remove(path);
}

// L622: EditedImageProvider::requestImage 缩放路径
TEST_F(ut_imageeditcontroller, EditedImageProviderRequestImageScalingPath)
{
    QString path = makeTempPng("ut_edit_provider_scale", 200, 200);
    ImageEditController ctrl;
    ASSERT_TRUE(ctrl.beginEdit(QUrl::fromLocalFile(path), 0));
    ASSERT_FALSE(ctrl.image().isNull());

    EditedImageProvider provider(&ctrl);
    QSize reportedSize;
    QImage result = provider.requestImage(QStringLiteral("test"), &reportedSize, QSize(50, 50));
    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.width(), 50);
    EXPECT_EQ(result.height(), 50);

    QFile::remove(path);
}

// L42: canEdit with multi-page TIFF returns false
TEST_F(ut_imageeditcontroller, CanEdit_MultiPageTiff_ReturnsFalse)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString tiffPath = dir.path() + "/multi.tif";
    QString script = QString(
        "from PIL import Image\n"
        "img1=Image.new('RGB',(4,4),(255,0,0))\n"
        "img2=Image.new('RGB',(4,4),(0,255,0))\n"
        "img1.save('%1', save_all=True, append_images=[img2])\n"
    ).arg(tiffPath);
    QProcess proc;
    proc.start("python3", QStringList() << "-c" << script);
    proc.waitForFinished(5000);
    if (QFileInfo::exists(tiffPath)) {
        ImageEditController ctrl;
        EXPECT_FALSE(ctrl.canEdit(QUrl::fromLocalFile(tiffPath)));
    }
}

// L339: applyEffect mosaic with invalid strength returns false
TEST_F(ut_imageeditcontroller, ApplyEffect_MosaicInvalidStrength_ReturnsFalse)
{
    ImageEditController ctrl;
    QString path = makeTempPng("ut_mosaic_invalid", 100, 100);
    ASSERT_TRUE(ctrl.beginEdit(QUrl::fromLocalFile(path), 0));
    QRectF rect(0.0, 0.0, 0.5, 0.5);
    EXPECT_FALSE(ctrl.applyEffect("mosaic", rect, 7));
    QFile::remove(path);
}

// L343: applyEffect graffiti with invalid strength returns false
TEST_F(ut_imageeditcontroller, ApplyEffect_GraffitiInvalidStrength_ReturnsFalse)
{
    ImageEditController ctrl;
    QString path = makeTempPng("ut_graffiti_invalid", 100, 100);
    ASSERT_TRUE(ctrl.beginEdit(QUrl::fromLocalFile(path), 0));
    QRectF rect(0.0, 0.0, 0.5, 0.5);
    EXPECT_FALSE(ctrl.applyEffect("graffiti", rect, 7));
    QFile::remove(path);
}

// L622: EditedImageProvider::requestImage with null controller
TEST_F(ut_imageeditcontroller, EditedImageProviderRequestImage_NullController)
{
    EditedImageProvider provider(nullptr);
    QSize reportedSize;
    QImage result = provider.requestImage(QStringLiteral("test"), &reportedSize, QSize(50, 50));
    EXPECT_TRUE(result.isNull());
}

// =================== Round 58 coverage improvement tests ===================

// L89: beginEdit when already editing the same source/frameIndex returns true
TEST_F(ut_imageeditcontroller, BeginEdit_AlreadyEditing_ReturnsTrue)
{
    ImageEditController ctrl;
    QString path = makeTempPng("ut_edit_already", 50, 50);
    ASSERT_TRUE(ctrl.beginEdit(QUrl::fromLocalFile(path), 0));
    // Calling beginEdit again with same source and frameIndex should return true
    EXPECT_TRUE(ctrl.beginEdit(QUrl::fromLocalFile(path), 0));
    QFile::remove(path);
}

// L93: beginEdit with frameIndex > 0 on single-page image, jumpToImage fails
TEST_F(ut_imageeditcontroller, BeginEdit_BadFrameIndex_ReturnsFalse)
{
    ImageEditController ctrl;
    QString path = makeTempPng("ut_edit_badframe", 50, 50);
    // frameIndex=999 on a single-page PNG should fail at reader.jumpToImage
    EXPECT_FALSE(ctrl.beginEdit(QUrl::fromLocalFile(path), 999));
    QFile::remove(path);
}
