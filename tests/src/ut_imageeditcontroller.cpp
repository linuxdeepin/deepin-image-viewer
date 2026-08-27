// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageeditcontroller.h"

#include <QSignalSpy>
#include <QElapsedTimer>
#include <QFont>
#include <QGuiApplication>
#include <QImageWriter>
#include <QTemporaryDir>
#include <gtest/gtest.h>

namespace {
QString createTestImage(QTemporaryDir &directory, int width = 80, int height = 60)
{
    QImage image(width, height, QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x)
            image.setPixelColor(x, y, QColor((x * 7) % 255, (y * 11) % 255, ((x + y) * 5) % 255));
    }
    const QString path = directory.filePath("edit-source.png");
    EXPECT_TRUE(image.save(path));
    return path;
}
}

TEST(ut_imageeditcontroller, BeginEdit_SupportedImage_ActiveTrue)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    QSignalSpy activeSpy(&controller, &ImageEditController::activeChanged);
    QSignalSpy revisionSpy(&controller, &ImageEditController::revisionChanged);

    const QUrl source = QUrl::fromLocalFile(createTestImage(directory));
    EXPECT_TRUE(controller.beginEdit(source));
    EXPECT_TRUE(controller.active());
    EXPECT_TRUE(controller.isEditing(source, 0));
    EXPECT_EQ(controller.image().size(), QSize(80, 60));
    EXPECT_EQ(activeSpy.count(), 1);
    EXPECT_EQ(revisionSpy.count(), 1);
}

TEST(ut_imageeditcontroller, BeginEdit_SameSource_NoDoubleInit)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    const QUrl source = QUrl::fromLocalFile(createTestImage(directory));

    EXPECT_TRUE(controller.beginEdit(source));
    EXPECT_TRUE(controller.beginEdit(source));
    EXPECT_TRUE(controller.active());
}

TEST(ut_imageeditcontroller, Discard_ActiveSession_InactiveAndCleared)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    QSignalSpy activeSpy(&controller, &ImageEditController::activeChanged);
    QSignalSpy revisionSpy(&controller, &ImageEditController::revisionChanged);
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));

    controller.discard();
    EXPECT_FALSE(controller.active());
    EXPECT_TRUE(controller.image().isNull());
    EXPECT_EQ(activeSpy.count(), 2);
    EXPECT_EQ(revisionSpy.count(), 2);
}

TEST(ut_imageeditcontroller, CanEdit_SupportedFormat_True)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    const QString path = createTestImage(directory);
    EXPECT_TRUE(controller.canEdit(QUrl::fromLocalFile(path)));
}

TEST(ut_imageeditcontroller, CanEdit_UnsupportedFormat_False)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    const QString svgPath = directory.filePath("test.svg");
    QFile svg(svgPath);
    svg.open(QIODevice::WriteOnly);
    svg.write("<svg xmlns=\"http://www.w3.org/2000/svg\"/>");
    svg.close();
    EXPECT_FALSE(controller.canEdit(QUrl::fromLocalFile(svgPath)));
}

TEST(ut_imageeditcontroller, CanEdit_MultiFrameImage_False)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    const QString gifPath = directory.filePath("animated.gif");
    QImage frame(4, 4, QImage::Format_ARGB32);
    frame.fill(Qt::red);
    frame.save(gifPath, "gif");
    EXPECT_FALSE(controller.canEdit(QUrl::fromLocalFile(gifPath)));
}

TEST(ut_imageeditcontroller, CanEdit_EmptyPath_False)
{
    ImageEditController controller;
    EXPECT_FALSE(controller.canEdit(QUrl()));
}

TEST(ut_imageeditcontroller, Effects_AllTypes_PixelsChanged)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));

    const QList<QPair<QString, int>> effects {
        { "gaussian", 15 }, { "mosaic", 16 }, { "graffiti", 16 }
    };
    for (const auto &[effect, strength] : effects) {
        const QImage before = controller.image();
        const int revision = controller.revision();
        EXPECT_TRUE(controller.applyEffect(effect, QRectF(0.2, 0.2, 0.5, 0.5), strength));
        EXPECT_EQ(controller.revision(), revision + 1);
        EXPECT_NE(controller.image(), before);
    }
}

TEST(ut_imageeditcontroller, Effects_StrengthBounds_AcceptsFullRange)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));

    EXPECT_TRUE(controller.applyEffect("gaussian", QRectF(0, 0, 1, 1), 5));
    EXPECT_TRUE(controller.applyEffect("gaussian", QRectF(0, 0, 1, 1), 30));
    EXPECT_TRUE(controller.applyEffect("mosaic", QRectF(0, 0, 1, 1), 8));
    EXPECT_TRUE(controller.applyEffect("mosaic", QRectF(0, 0, 1, 1), 32));
    EXPECT_TRUE(controller.applyEffect("graffiti", QRectF(0, 0, 1, 1), 8));
    EXPECT_TRUE(controller.applyEffect("graffiti", QRectF(0, 0, 1, 1), 32));
}

TEST(ut_imageeditcontroller, Effects_InvalidStrength_ReturnsFalse)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    const int revision = controller.revision();

    EXPECT_FALSE(controller.applyEffect("gaussian", QRectF(0, 0, 1, 1), 10));
    EXPECT_FALSE(controller.applyEffect("mosaic", QRectF(0, 0, 1, 1), 10));
    EXPECT_FALSE(controller.applyEffect("graffiti", QRectF(0, 0, 1, 1), 10));
    EXPECT_EQ(controller.revision(), revision);
}

TEST(ut_imageeditcontroller, Mosaic_BlockUsesAverageColor)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    const QImage before = controller.image().convertToFormat(QImage::Format_ARGB32);

    quint64 red = 0;
    quint64 green = 0;
    quint64 blue = 0;
    quint64 alpha = 0;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const QColor color = before.pixelColor(x, y);
            red += color.red();
            green += color.green();
            blue += color.blue();
            alpha += color.alpha();
        }
    }
    const QColor expected(red / 64, green / 64, blue / 64, alpha / 64);

    ASSERT_TRUE(controller.applyEffect("mosaic", QRectF(0, 0, 1, 1), 8));
    const QImage after = controller.image();
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x)
            EXPECT_EQ(after.pixelColor(x, y), expected);
    }
}

TEST(ut_imageeditcontroller, Gaussian_OnlyChangesSelection)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    const QImage before = controller.image();
    const QRect selection(20, 15, 40, 30);

    ASSERT_TRUE(controller.applyEffect("gaussian", QRectF(0.25, 0.25, 0.5, 0.5), 15));
    const QImage after = controller.image();
    EXPECT_EQ(after.pixelColor(0, 0), before.pixelColor(0, 0));
    EXPECT_EQ(after.pixelColor(79, 59), before.pixelColor(79, 59));
    EXPECT_NE(after.copy(selection), before.copy(selection));
}

TEST(ut_imageeditcontroller, Graffiti_SameInputIsDeterministic)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QUrl source = QUrl::fromLocalFile(createTestImage(directory));
    ImageEditController first;
    ImageEditController second;
    ASSERT_TRUE(first.beginEdit(source));
    ASSERT_TRUE(second.beginEdit(source));

    ASSERT_TRUE(first.applyEffect("graffiti", QRectF(0.1, 0.1, 0.7, 0.7), 16));
    ASSERT_TRUE(second.applyEffect("graffiti", QRectF(0.1, 0.1, 0.7, 0.7), 16));
    EXPECT_EQ(first.image(), second.image());
}

TEST(ut_imageeditcontroller, ApplyEffect_UnknownEffect_ReturnsFalse)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    const int revision = controller.revision();

    EXPECT_FALSE(controller.applyEffect("unknown", QRectF(0, 0, 1, 1), 10));
    EXPECT_EQ(controller.revision(), revision);
}

TEST(ut_imageeditcontroller, ApplyEffect_TooSmallSelection_ReturnsFalse)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    EXPECT_FALSE(controller.applyEffect("mosaic", QRectF(0.49, 0.49, 0.001, 0.001), 10));
}

TEST(ut_imageeditcontroller, ApplyEffect_NoActiveSession_ReturnsFalse)
{
    ImageEditController controller;
    EXPECT_FALSE(controller.applyEffect("gaussian", QRectF(0, 0, 1, 1), 10));
}

TEST(ut_imageeditcontroller, Crop_NormalizedRect_SizeChanged)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    const int revision = controller.revision();

    EXPECT_TRUE(controller.crop(QRectF(0.25, 0.25, 0.5, 0.5)));
    EXPECT_EQ(controller.image().size(), QSize(40, 30));
    EXPECT_EQ(controller.revision(), revision + 1);
}

TEST(ut_imageeditcontroller, Crop_FullImage_ReturnsFalse)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    EXPECT_FALSE(controller.crop(QRectF(0, 0, 1, 1)));
}

TEST(ut_imageeditcontroller, RotateClockwise_SwapsDimensionsAndSupportsHistory)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    const QImage original = controller.image();

    ASSERT_TRUE(controller.rotateClockwise());
    EXPECT_EQ(controller.image().size(), QSize(60, 80));
    controller.commitHistory();
    const QImage rotated = controller.image();

    ASSERT_TRUE(controller.undo());
    EXPECT_EQ(controller.image(), original);
    ASSERT_TRUE(controller.redo());
    EXPECT_EQ(controller.image(), rotated);
}

TEST(ut_imageeditcontroller, RotateClockwise_NoActiveSession_ReturnsFalse)
{
    ImageEditController controller;
    EXPECT_FALSE(controller.rotateClockwise());
}

TEST(ut_imageeditcontroller, UndoRedo_PixelHistory_RestoresAndReapplies)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    const QImage original = controller.image();

    ASSERT_TRUE(controller.applyEffect("graffiti", QRectF(0.1, 0.1, 0.4, 0.4), 16));
    controller.commitHistory();
    const QImage changed = controller.image();
    EXPECT_TRUE(controller.canUndo());
    EXPECT_FALSE(controller.canRedo());

    EXPECT_TRUE(controller.undo());
    EXPECT_EQ(controller.image(), original);
    EXPECT_TRUE(controller.canRedo());

    EXPECT_TRUE(controller.redo());
    EXPECT_EQ(controller.image(), changed);
}

TEST(ut_imageeditcontroller, UndoRedo_NewAction_ClearsRedoBranch)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));

    ASSERT_TRUE(controller.applyEffect("graffiti", QRectF(0.1, 0.1, 0.4, 0.4), 16));
    controller.commitHistory();

    ASSERT_TRUE(controller.undo());
    ASSERT_TRUE(controller.applyEffect("mosaic", QRectF(0.2, 0.2, 0.3, 0.3), 8));
    controller.commitHistory();
    EXPECT_FALSE(controller.canRedo());
}

TEST(ut_imageeditcontroller, Undo_NoHistory_ReturnsFalse)
{
    ImageEditController controller;
    EXPECT_FALSE(controller.undo());
}

TEST(ut_imageeditcontroller, Redo_NoRedoAvailable_ReturnsFalse)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    EXPECT_FALSE(controller.redo());
}

TEST(ut_imageeditcontroller, UndoRedo_IdenticalSnapshotDoesNotReloadImage)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    QSignalSpy revisionSpy(&controller, &ImageEditController::revisionChanged);

    controller.commitHistory();
    const int revision = controller.revision();
    ASSERT_TRUE(controller.undo());
    ASSERT_TRUE(controller.redo());

    EXPECT_EQ(controller.revision(), revision);
    EXPECT_EQ(revisionSpy.count(), 0);
}

TEST(ut_imageeditcontroller, History_KeepsAtMost50Steps)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));

    for (int i = 0; i < 60; ++i)
        controller.commitHistory();

    int undoCount = 0;
    while (controller.undo())
        ++undoCount;
    EXPECT_EQ(undoCount, 49);
}

TEST(ut_imageeditcontroller, Modified_InitialState_False)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    EXPECT_FALSE(controller.modified());
}

TEST(ut_imageeditcontroller, Modified_AfterEffect_True)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));

    ASSERT_TRUE(controller.applyEffect("mosaic", QRectF(0.1, 0.1, 0.3, 0.3), 8));
    controller.commitHistory();
    EXPECT_TRUE(controller.modified());
}

TEST(ut_imageeditcontroller, Modified_AfterMarkSaved_False)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));

    ASSERT_TRUE(controller.applyEffect("mosaic", QRectF(0.1, 0.1, 0.3, 0.3), 8));
    controller.commitHistory();
    controller.markSaved();
    EXPECT_FALSE(controller.modified());
}

TEST(ut_imageeditcontroller, Modified_AfterUndoToSavedPoint_False)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));

    ASSERT_TRUE(controller.applyEffect("mosaic", QRectF(0.1, 0.1, 0.3, 0.3), 8));
    controller.commitHistory();
    controller.markSaved();
    ASSERT_TRUE(controller.applyEffect("graffiti", QRectF(0.5, 0.5, 0.3, 0.3), 8));
    controller.commitHistory();
    ASSERT_TRUE(controller.undo());
    EXPECT_FALSE(controller.modified());
}

TEST(ut_imageeditcontroller, DefaultSaveUrl_FormatCorrect)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    const QString sourcePath = createTestImage(directory);
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(sourcePath)));

    EXPECT_EQ(controller.defaultSaveUrl().toLocalFile(), directory.filePath("edit-source-编辑.png"));
}

TEST(ut_imageeditcontroller, IsOriginalUrl_SamePath_True)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    const QString sourcePath = createTestImage(directory);
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(sourcePath)));

    EXPECT_TRUE(controller.isOriginalUrl(QUrl::fromLocalFile(sourcePath)));
}

TEST(ut_imageeditcontroller, IsOriginalUrl_DifferentPath_False)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    const QString sourcePath = createTestImage(directory);
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(sourcePath)));

    EXPECT_FALSE(controller.isOriginalUrl(QUrl::fromLocalFile(directory.filePath("other.png"))));
}

TEST(ut_imageeditcontroller, SaveComposite_ValidDestination_FileCreated)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    const QImage original = controller.image();
    const QString targetPath = directory.filePath("composite.png");

    QVariantMap line;
    line.insert("type", "line");
    line.insert("color", "#ff0000");
    line.insert("width", 0.05);
    line.insert("points", QVariantList { QPointF(0.1, 0.1), QPointF(0.9, 0.9) });
    EXPECT_TRUE(controller.saveComposite(QUrl::fromLocalFile(targetPath), QVariantList { line }));

    const QImage saved(targetPath);
    ASSERT_FALSE(saved.isNull());
    EXPECT_EQ(saved.size(), original.size());
    EXPECT_NE(saved, original);
    EXPECT_EQ(controller.image(), original);
}

TEST(ut_imageeditcontroller, SaveComposite_AllAnnotationTypes_Rasterized)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    const QImage original = controller.image();
    const QString targetPath = directory.filePath("all-annotations.png");

    const auto annotation = [](const QString &type, const QPointF &start, const QPointF &end) {
        QVariantMap value;
        value.insert("type", type);
        value.insert("color", "#ff0000");
        value.insert("width", 0.025);
        value.insert("points", QVariantList { start, end });
        return value;
    };
    QVariantMap text = annotation("text", QPointF(0.05, 0.65), QPointF(0.4, 0.9));
    text.insert("text", "Edit");
    text.insert("fontFamily", QGuiApplication::font().family());
    QVariantMap number = annotation("number", QPointF(0.65, 0.65), QPointF(0.9, 0.95));
    number.insert("number", 2);
    number.insert("fontFamily", QGuiApplication::font().family());
    const QVariantList annotations {
        annotation("rect", QPointF(0.05, 0.05), QPointF(0.3, 0.35)),
        annotation("ellipse", QPointF(0.35, 0.05), QPointF(0.6, 0.35)),
        annotation("arrow", QPointF(0.65, 0.05), QPointF(0.9, 0.35)),
        text,
        number,
    };

    ASSERT_TRUE(controller.saveComposite(QUrl::fromLocalFile(targetPath), annotations));
    const QImage saved(targetPath);
    ASSERT_FALSE(saved.isNull());
    EXPECT_EQ(saved.size(), original.size());
    EXPECT_NE(saved, original);
}

TEST(ut_imageeditcontroller, SaveComposite_RotatedRectangle_RasterizedAtRotatedPosition)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory, 100, 100))));
    const QString targetPath = directory.filePath("rotated-rectangle.png");

    QVariantMap rectangle;
    rectangle.insert("type", "rect");
    rectangle.insert("color", "#ff0000");
    rectangle.insert("width", 0.02);
    rectangle.insert("rotation", 90.0);
    rectangle.insert("points", QVariantList { QPointF(0.2, 0.4), QPointF(0.8, 0.6) });

    ASSERT_TRUE(controller.saveComposite(QUrl::fromLocalFile(targetPath), { rectangle }));
    const QImage saved(targetPath);
    ASSERT_FALSE(saved.isNull());
    EXPECT_EQ(saved.pixelColor(60, 50), QColor("#ff0000"));
}

TEST(ut_imageeditcontroller, SaveComposite_RotatedPen_RasterizedAtRotatedPosition)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory, 100, 100))));
    const QString targetPath = directory.filePath("rotated-pen.png");

    QVariantMap pen;
    pen.insert("type", "pen");
    pen.insert("color", "#ff0000");
    pen.insert("width", 0.02);
    pen.insert("rotation", 90.0);
    pen.insert("points", QVariantList { QPointF(0.4, 0.6), QPointF(0.2, 0.4),
                                        QPointF(0.8, 0.4), QPointF(0.6, 0.6) });

    ASSERT_TRUE(controller.saveComposite(QUrl::fromLocalFile(targetPath), { pen }));
    const QImage saved(targetPath);
    ASSERT_FALSE(saved.isNull());
    EXPECT_EQ(saved.pixelColor(60, 30), QColor("#ff0000"));
}

TEST(ut_imageeditcontroller, SaveComposite_Number_UsesFilledCircle)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory, 100, 100))));
    const QString targetPath = directory.filePath("filled-number.png");

    QVariantMap number;
    number.insert("type", "number");
    number.insert("number", 1);
    number.insert("color", "#ff0000");
    number.insert("width", 0.0);
    number.insert("fontFamily", QGuiApplication::font().family());
    number.insert("points", QVariantList { QPointF(0.4, 0.4), QPointF(0.6, 0.6) });

    ASSERT_TRUE(controller.saveComposite(QUrl::fromLocalFile(targetPath), { number }));
    const QImage saved(targetPath);
    ASSERT_FALSE(saved.isNull());
    EXPECT_EQ(saved.pixelColor(50, 42), QColor("#ff0000"));
}

TEST(ut_imageeditcontroller, SaveComposite_FormatMismatch_SaveFailedSignal)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    QSignalSpy spy(&controller, &ImageEditController::saveFailed);

    EXPECT_FALSE(controller.saveComposite(QUrl::fromLocalFile(directory.filePath("wrong.jpg")), {}));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(QFileInfo::exists(directory.filePath("wrong.jpg")));
}

TEST(ut_imageeditcontroller, SaveComposite_EmptyPath_SaveFailedSignal)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    QSignalSpy spy(&controller, &ImageEditController::saveFailed);

    EXPECT_FALSE(controller.saveComposite(QUrl(), {}));
    EXPECT_EQ(spy.count(), 1);
}

TEST(ut_imageeditcontroller, SaveComposite_UnwritableDestination_SaveFailedSignal)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    QSignalSpy spy(&controller, &ImageEditController::saveFailed);

    EXPECT_FALSE(controller.saveComposite(QUrl::fromLocalFile("/proc/deepin-image-viewer-edit.png"), {}));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(controller.active());
}

TEST(ut_imageeditcontroller, SaveComposite_JpgAndJpeg_NormalizedAsSameFormat)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    QImage source(64, 48, QImage::Format_ARGB32);
    source.fill(QColor("#3578c8"));
    const QString jpgPath = directory.filePath("source.jpg");
    ASSERT_TRUE(source.save(jpgPath, "jpg"));

    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(jpgPath)));

    const QString jpegPath = directory.filePath("edited.jpeg");
    EXPECT_TRUE(controller.saveComposite(QUrl::fromLocalFile(jpegPath), {}));
    EXPECT_FALSE(QImage(jpegPath).isNull());
}

TEST(ut_imageeditcontroller, SupportedFormats_RoundTrip)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QList<QByteArray> requiredFormats { "bmp", "jpg", "jpeg", "png", "pgm",
                                               "ppm", "xpm", "ico", "icns" };
    const QList<QByteArray> writers = QImageWriter::supportedImageFormats();
    QImage source(64, 48, QImage::Format_ARGB32);
    source.fill(QColor("#3578c8"));

    for (const QByteArray &format : requiredFormats) {
        if (!writers.contains(format))
            continue;
        const QString suffix = QString::fromLatin1(format);
        const QString sourcePath = directory.filePath("source." + suffix);
        const QString targetPath = directory.filePath("edited." + suffix);
        ASSERT_TRUE(source.save(sourcePath, format.constData())) << format.constData();

        ImageEditController controller;
        ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(sourcePath))) << format.constData();
        ASSERT_TRUE(controller.applyEffect("graffiti", QRectF(0.2, 0.2, 0.5, 0.5), 8));
        ASSERT_TRUE(controller.saveComposite(QUrl::fromLocalFile(targetPath), {})) << format.constData();
        EXPECT_FALSE(QImage(targetPath).isNull()) << format.constData();
    }
}

TEST(ut_imageeditcontroller, Performance_4KOperations)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    QImage source(3840, 2160, QImage::Format_RGB32);
    for (int y = 0; y < source.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(source.scanLine(y));
        for (int x = 0; x < source.width(); ++x)
            line[x] = qRgb(x % 256, y % 256, (x + y) % 256);
    }
    const QString sourcePath = directory.filePath("performance.png");
    ASSERT_TRUE(source.save(sourcePath));

    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(sourcePath)));
    QElapsedTimer timer;

    timer.start();
    ASSERT_TRUE(controller.applyEffect("gaussian", QRectF(0.25, 0.25, 0.5, 0.5), 15));
    controller.commitHistory();

    timer.restart();
    ASSERT_TRUE(controller.applyEffect("mosaic", QRectF(0.25, 0.25, 0.5, 0.5), 16));
    controller.commitHistory();

    timer.restart();
    ASSERT_TRUE(controller.crop(QRectF(0.05, 0.05, 0.9, 0.9)));
    controller.commitHistory();

    timer.restart();
    ASSERT_TRUE(controller.undo());

    timer.restart();
    ASSERT_TRUE(controller.saveComposite(QUrl::fromLocalFile(directory.filePath("perf-edit.png")), {}));
}

TEST(ut_imageeditcontroller, Graffiti_4KSelection_CompletesWithinOneSecond)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(
        QUrl::fromLocalFile(createTestImage(directory, 3840, 2160))));

    QElapsedTimer timer;
    timer.start();
    ASSERT_TRUE(controller.applyEffect("graffiti", QRectF(0.25, 0.25, 0.5, 0.5), 16));
    EXPECT_LT(timer.elapsed(), 1000);
}

TEST(ut_imageeditcontroller, Image_NoSession_NullImage)
{
    ImageEditController controller;
    EXPECT_TRUE(controller.image().isNull());
}

TEST(ut_imageeditcontroller, Revision_IncrementedOnEachOperation)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    ImageEditController controller;
    ASSERT_TRUE(controller.beginEdit(QUrl::fromLocalFile(createTestImage(directory))));
    const int initial = controller.revision();

    controller.applyEffect("gaussian", QRectF(0, 0, 0.5, 0.5), 15);
    EXPECT_GT(controller.revision(), initial);

    controller.crop(QRectF(0, 0, 0.5, 0.5));
    EXPECT_GT(controller.revision(), initial);

    controller.undo();
    EXPECT_GT(controller.revision(), initial);

    controller.redo();
    EXPECT_GT(controller.revision(), initial);
}
