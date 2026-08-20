// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageeditcontroller.h"

#include <algorithm>

#include <QImageReader>
#include <QImageWriter>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QTransform>
#include <QtMath>

#include <opencv2/imgproc.hpp>

ImageEditController::ImageEditController(QObject *parent)
    : QObject(parent)
{
}

bool ImageEditController::canEdit(const QUrl &source) const
{
    const QString path = source.toLocalFile();
    if (path.isEmpty())
        return false;
    QImageReader reader(path);
    if (reader.imageCount() > 1)
        return false;
    static const QStringList formats { "bmp", "jpg", "jpeg", "png", "pgm",
                                       "ppm", "xpm", "ico", "icns" };
    return formats.contains(QFileInfo(path).suffix().toLower());
}

bool ImageEditController::active() const
{
    QReadLocker locker(&m_lock);
    return !m_image.isNull();
}

bool ImageEditController::canRedo() const
{
    QReadLocker locker(&m_lock);
    return m_historyIndex >= 0 && m_historyIndex + 1 < m_history.size();
}

bool ImageEditController::canUndo() const
{
    QReadLocker locker(&m_lock);
    return m_historyIndex > 0;
}

bool ImageEditController::modified() const
{
    QReadLocker locker(&m_lock);
    return m_historyIndex != m_savedHistoryIndex;
}

int ImageEditController::revision() const
{
    QReadLocker locker(&m_lock);
    return m_revision;
}

QImage ImageEditController::image() const
{
    QReadLocker locker(&m_lock);
    return m_image;
}

bool ImageEditController::beginEdit(const QUrl &source, int frameIndex)
{
    if (!canEdit(source))
        return false;
    if (isEditing(source, frameIndex))
        return true;

    QImageReader reader(source.toLocalFile());
    if (frameIndex > 0 && !reader.jumpToImage(frameIndex))
        return false;
    QImage loaded = reader.read();
    if (loaded.isNull())
        return false;

    {
        QWriteLocker locker(&m_lock);
        m_image = loaded.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        m_source = source;
        m_frameIndex = frameIndex;
        m_history = { m_image };
        m_historyIndex = 0;
        m_savedHistoryIndex = 0;
        ++m_revision;
    }
    Q_EMIT activeChanged();
    Q_EMIT historyChanged();
    Q_EMIT revisionChanged();
    return true;
}

QUrl ImageEditController::defaultSaveUrl() const
{
    QReadLocker locker(&m_lock);
    const QFileInfo sourceInfo(m_source.toLocalFile());
    const QString fileName = sourceInfo.completeBaseName() + QStringLiteral("-编辑.") + sourceInfo.suffix();
    return QUrl::fromLocalFile(sourceInfo.dir().filePath(fileName));
}

bool ImageEditController::isOriginalUrl(const QUrl &destination) const
{
    QReadLocker locker(&m_lock);
    const QString destPath = destination.toLocalFile();
    const QString srcPath = m_source.toLocalFile();
    const QFileInfo destInfo(destPath);
    const QFileInfo srcInfo(srcPath);
    const QString destCanonical = destInfo.exists() ? destInfo.canonicalFilePath()
                                                     : destInfo.absoluteFilePath();
    const QString srcCanonical = srcInfo.exists() ? srcInfo.canonicalFilePath()
                                                   : srcInfo.absoluteFilePath();
    return destCanonical == srcCanonical;
}

bool ImageEditController::saveComposite(const QUrl &destination, const QVariantList &annotations)
{
    QImage result;
    QString sourceLocalFile;
    {
        QReadLocker locker(&m_lock);
        if (m_image.isNull())
            return false;
        result = m_image;
        sourceLocalFile = m_source.toLocalFile();
    }
    const QByteArray sourceFormat = QImageReader(sourceLocalFile).format();

    const QString targetPath = destination.toLocalFile();
    if (targetPath.isEmpty()) {
        Q_EMIT saveFailed(tr("Invalid file path."));
        return false;
    }

    QByteArray targetFormat = QImageReader::imageFormat(targetPath);
    if (targetFormat.isEmpty())
        targetFormat = QFileInfo(targetPath).suffix().toLower().toLatin1();
    // Normalize synonymous extensions so .jpg and .jpeg are interchangeable.
    auto normalizeFormat = [](const QByteArray &format) {
        if (format == "jpg" || format == "jpeg")
            return QByteArray("jpeg");
        return format;
    };
    const QByteArray normalizedSource = normalizeFormat(sourceFormat);
    if (normalizeFormat(targetFormat) != normalizedSource) {
        Q_EMIT saveFailed(tr("The file format must match the original image."));
        return false;
    }

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    const QFont defaultFont = painter.font();
    for (const QVariant &annotationValue : annotations) {
        const QVariantMap annotation = annotationValue.toMap();
        const QString type = annotation.value(QStringLiteral("type")).toString();
        const QVariantList rawPoints = annotation.value(QStringLiteral("points")).toList();
        if (rawPoints.size() < 2)
            continue;
        QVector<QPointF> points;
        for (const QVariant &rawPoint : rawPoints) {
            const QPointF normalized = rawPoint.toPointF();
            points.push_back(QPointF(normalized.x() * result.width(), normalized.y() * result.height()));
        }
        const QColor color(annotation.value(QStringLiteral("color")).toString());
        const qreal width = qMax(1.0, annotation.value(QStringLiteral("width")).toDouble()
                                       * result.width());
        QPen pen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        const QRectF bounds(points.first(), points.last());
        if (type == QLatin1String("rect")) {
            painter.drawRect(bounds.normalized());
        } else if (type == QLatin1String("ellipse")) {
            painter.drawEllipse(bounds.normalized());
        } else if (type == QLatin1String("text") || type == QLatin1String("number")) {
            const QRectF textBounds = bounds.normalized();
            QFont font = defaultFont;
            const QString fontFamily = annotation.value(QStringLiteral("fontFamily")).toString();
            if (!fontFamily.isEmpty())
                font.setFamily(fontFamily);
            font.setPixelSize(qMax(8, qRound(textBounds.height() * 0.8)));
            painter.setFont(font);
            if (type == QLatin1String("number")) {
                painter.drawEllipse(textBounds);
                painter.drawText(textBounds, Qt::AlignCenter,
                                 QString::number(annotation.value(QStringLiteral("number")).toInt()));
            } else {
                painter.drawText(textBounds, Qt::AlignLeft | Qt::AlignVCenter,
                                 annotation.value(QStringLiteral("text")).toString());
            }
        } else {
            QPainterPath path(points.first());
            for (int i = 1; i < points.size(); ++i)
                path.lineTo(points[i]);
            painter.drawPath(path);
            if (type == QLatin1String("arrow")) {
                const QPointF start = points.at(points.size() - 2);
                const QPointF end = points.last();
                const qreal angle = qAtan2(end.y() - start.y(), end.x() - start.x());
                const qreal length = qMax(10.0, width * 4);
                const qreal spread = M_PI / 7;
                painter.drawLine(end, end - QPointF(length * qCos(angle - spread),
                                                     length * qSin(angle - spread)));
                painter.drawLine(end, end - QPointF(length * qCos(angle + spread),
                                                     length * qSin(angle + spread)));
            }
        }
    }
    painter.end();

    QSaveFile file(targetPath);
    if (!file.open(QIODevice::WriteOnly)) {
        Q_EMIT saveFailed(file.errorString());
        return false;
    }
    QImageWriter writer(&file, normalizedSource);
    if (!writer.write(result)) {
        Q_EMIT saveFailed(writer.errorString());
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        Q_EMIT saveFailed(file.errorString());
        return false;
    }
    return true;
}

void ImageEditController::discard()
{
    {
        QWriteLocker locker(&m_lock);
        if (m_image.isNull())
            return;
        m_image = QImage();
        m_source = QUrl();
        m_frameIndex = 0;
        m_history.clear();
        m_historyIndex = -1;
        m_savedHistoryIndex = 0;
        ++m_revision;
    }
    Q_EMIT activeChanged();
    Q_EMIT historyChanged();
    Q_EMIT revisionChanged();
}

bool ImageEditController::isEditing(const QUrl &source, int frameIndex) const
{
    QReadLocker locker(&m_lock);
    return !m_image.isNull() && m_source == source && m_frameIndex == frameIndex;
}

QRect ImageEditController::pixelRect(const QRectF &normalizedRect) const
{
    const QRectF normalized = normalizedRect.normalized().intersected(QRectF(0, 0, 1, 1));
    const int left = qFloor(normalized.left() * m_image.width());
    const int top = qFloor(normalized.top() * m_image.height());
    const int right = qCeil(normalized.right() * m_image.width());
    const int bottom = qCeil(normalized.bottom() * m_image.height());
    return QRect(left, top, right - left, bottom - top).intersected(m_image.rect());
}

bool ImageEditController::applyEffect(const QString &effect, const QRectF &normalizedRect, int strength)
{
    QWriteLocker locker(&m_lock);
    if (m_image.isNull())
        return false;
    const QRect rect = pixelRect(normalizedRect);
    if (rect.width() < 2 || rect.height() < 2)
        return false;

    if (effect == QLatin1String("gaussian")) {
        if (strength != 5 && strength != 15 && strength != 30)
            return false;
        applyGaussianBlur(rect, strength);
    } else if (effect == QLatin1String("mosaic")) {
        if (strength != 8 && strength != 16 && strength != 32)
            return false;
        applyMosaic(rect, strength);
    } else if (effect == QLatin1String("graffiti")) {
        if (strength != 8 && strength != 16 && strength != 32)
            return false;
        applyGraffiti(rect, strength);
    } else {
        return false;
    }

    ++m_revision;
    locker.unlock();
    Q_EMIT revisionChanged();
    return true;
}

bool ImageEditController::crop(const QRectF &normalizedRect)
{
    QWriteLocker locker(&m_lock);
    if (m_image.isNull())
        return false;
    const QRect rect = pixelRect(normalizedRect);
    if (rect.width() < 2 || rect.height() < 2 || rect == m_image.rect())
        return false;

    m_image = m_image.copy(rect);
    ++m_revision;
    locker.unlock();
    Q_EMIT revisionChanged();
    return true;
}

void ImageEditController::commitHistory()
{
    QWriteLocker locker(&m_lock);
    if (m_image.isNull())
        return;
    if (m_historyIndex + 1 < m_history.size()) {
        if (m_savedHistoryIndex > m_historyIndex)
            m_savedHistoryIndex = -1;
        m_history.resize(m_historyIndex + 1);
    }
    m_history.push_back(m_image);
    const int maxHistorySteps = 50;
    while (m_history.size() > maxHistorySteps) {
        m_history.removeFirst();
        --m_historyIndex;
        if (m_savedHistoryIndex >= 0)
            --m_savedHistoryIndex;
    }
    m_historyIndex = m_history.size() - 1;
    locker.unlock();
    Q_EMIT historyChanged();
}

void ImageEditController::markSaved()
{
    QWriteLocker locker(&m_lock);
    m_savedHistoryIndex = m_historyIndex;
    locker.unlock();
    Q_EMIT historyChanged();
}

bool ImageEditController::undo()
{
    QWriteLocker locker(&m_lock);
    if (m_historyIndex <= 0)
        return false;
    --m_historyIndex;
    m_image = m_history[m_historyIndex];
    ++m_revision;
    locker.unlock();
    Q_EMIT revisionChanged();
    Q_EMIT historyChanged();
    return true;
}

bool ImageEditController::redo()
{
    QWriteLocker locker(&m_lock);
    if (m_historyIndex < 0 || m_historyIndex + 1 >= m_history.size())
        return false;
    ++m_historyIndex;
    m_image = m_history[m_historyIndex];
    ++m_revision;
    locker.unlock();
    Q_EMIT revisionChanged();
    Q_EMIT historyChanged();
    return true;
}

void ImageEditController::applyGaussianBlur(const QRect &rect, int radius)
{
    const QRect sampleRect = rect.adjusted(-radius, -radius, radius, radius).intersected(m_image.rect());
    QImage source = m_image.copy(sampleRect).convertToFormat(QImage::Format_ARGB32);
    QImage blurred(source.size(), QImage::Format_ARGB32);
    const cv::Mat sourceMat(source.height(), source.width(), CV_8UC4,
                            source.bits(), source.bytesPerLine());
    cv::Mat blurredMat(blurred.height(), blurred.width(), CV_8UC4,
                        blurred.bits(), blurred.bytesPerLine());
    const int kernelSize = radius * 2 + 1;
    cv::GaussianBlur(sourceMat, blurredMat, cv::Size(kernelSize, kernelSize),
                     0, 0, cv::BORDER_REPLICATE);
    QPainter painter(&m_image);
    painter.drawImage(rect.topLeft(), blurred, rect.translated(-sampleRect.topLeft()));
}

void ImageEditController::applyMosaic(const QRect &rect, int blockSize)
{
    QImage region = m_image.copy(rect).convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < region.height(); y += blockSize) {
        for (int x = 0; x < region.width(); x += blockSize) {
            const int blockWidth = qMin(blockSize, region.width() - x);
            const int blockHeight = qMin(blockSize, region.height() - y);
            quint64 red = 0;
            quint64 green = 0;
            quint64 blue = 0;
            quint64 alpha = 0;
            for (int row = y; row < y + blockHeight; ++row) {
                const QRgb *pixels = reinterpret_cast<const QRgb *>(region.constScanLine(row));
                for (int column = x; column < x + blockWidth; ++column) {
                    const QRgb pixel = pixels[column];
                    red += qRed(pixel);
                    green += qGreen(pixel);
                    blue += qBlue(pixel);
                    alpha += qAlpha(pixel);
                }
            }
            const quint64 count = static_cast<quint64>(blockWidth) * blockHeight;
            const QRgb average = qRgba(red / count, green / count, blue / count, alpha / count);
            for (int row = y; row < y + blockHeight; ++row) {
                QRgb *pixels = reinterpret_cast<QRgb *>(region.scanLine(row));
                std::fill(pixels + x, pixels + x + blockWidth, average);
            }
        }
    }
    QPainter painter(&m_image);
    painter.drawImage(rect.topLeft(), region);
}

void ImageEditController::applyGraffiti(const QRect &rect, int strength)
{
    const QImage source = m_image.copy(rect).convertToFormat(QImage::Format_ARGB32);
    QImage result = source;
    QImage brush(QStringLiteral(":/res/graffiti_mixer_tip.png"));
    if (brush.isNull())
        return;
    brush = brush.convertToFormat(QImage::Format_ARGB32);

    const int diameter = strength == 8 ? 96 : (strength == 16 ? 174 : 256);
    const qreal stampStep = qMax(1.0, diameter * 0.12);
    const qreal rowStep = qMax(stampStep, diameter * 0.45);
    QRandomGenerator random(static_cast<quint32>(rect.x() * 73856093U)
                            ^ static_cast<quint32>(rect.y() * 19349663U)
                            ^ static_cast<quint32>(rect.width() * 83492791U)
                            ^ static_cast<quint32>(rect.height()));

    QImage baseMask(brush.size(), QImage::Format_Alpha8);
    for (int brushY = 0; brushY < brush.height(); ++brushY) {
        const QRgb *maskPixels = reinterpret_cast<const QRgb *>(brush.constScanLine(brushY));
        uchar *alpha = baseMask.scanLine(brushY);
        for (int brushX = 0; brushX < brush.width(); ++brushX) {
            const QRgb mask = maskPixels[brushX];
            alpha[brushX] = qAlpha(mask) * (255 - qGray(mask)) / 255;
        }
    }

    QVector<QImage> stampMasks;
    const qreal scales[] { 0.85, 1.0, 1.15 };
    const qreal angles[] { -20.0, -10.0, 0.0, 10.0, 20.0 };
    for (const qreal scale : scales) {
        const int targetWidth = qMax(1, qRound(diameter * scale));
        const int targetHeight = qMax(1, qRound(static_cast<qreal>(targetWidth)
                                                * brush.height() / brush.width()));
        const QImage scaled = baseMask.scaled(targetWidth, targetHeight,
                                              Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        for (const qreal angle : angles) {
            QTransform transform;
            transform.rotate(angle);
            stampMasks.push_back(scaled.transformed(transform, Qt::SmoothTransformation));
        }
    }

    QColor reservoir;
    bool hasReservoir = false;

    int rowIndex = 0;
    for (qreal y = 0; y < result.height() + rowStep; y += rowStep, ++rowIndex) {
        const bool reverse = rowIndex % 2;
        for (qreal distance = 0; distance < result.width() + diameter; distance += stampStep) {
            const qreal baseX = reverse ? result.width() - distance : distance;
            const qreal scatter = (random.generateDouble() * 2.0 - 1.0) * diameter * 0.08;
            const QPointF center(baseX, y + scatter);
            const int sampleX = qBound(0, qRound(center.x()), source.width() - 1);
            const int sampleY = qBound(0, qRound(center.y()), source.height() - 1);
            const QColor sampled(source.pixel(sampleX, sampleY));
            if (!hasReservoir) {
                reservoir = sampled;
                hasReservoir = true;
            } else {
                reservoir.setRed(qRound(reservoir.red() * 0.7 + sampled.red() * 0.3));
                reservoir.setGreen(qRound(reservoir.green() * 0.7 + sampled.green() * 0.3));
                reservoir.setBlue(qRound(reservoir.blue() * 0.7 + sampled.blue() * 0.3));
                reservoir.setAlpha(255);
            }

            const QImage &mask = stampMasks.at(random.bounded(static_cast<int>(stampMasks.size())));
            const int stampLeft = qRound(center.x() - mask.width() / 2.0);
            const int stampTop = qRound(center.y() - mask.height() / 2.0);
            const QRect target = QRect(stampLeft, stampTop, mask.width(), mask.height())
                                     .intersected(result.rect());
            for (int targetY = target.top(); targetY <= target.bottom(); ++targetY) {
                const uchar *alpha = mask.constScanLine(targetY - stampTop);
                QRgb *pixels = reinterpret_cast<QRgb *>(result.scanLine(targetY));
                for (int targetX = target.left(); targetX <= target.right(); ++targetX) {
                    const int coverage = alpha[targetX - stampLeft];
                    if (coverage == 0)
                        continue;
                    const QRgb destination = pixels[targetX];
                    const int inverse = 255 - coverage;
                    const int destinationAlpha = qAlpha(destination);
                    if (destinationAlpha == 255) {
                        pixels[targetX] = qRgb((qRed(destination) * inverse + reservoir.red() * coverage) / 255,
                                               (qGreen(destination) * inverse + reservoir.green() * coverage) / 255,
                                               (qBlue(destination) * inverse + reservoir.blue() * coverage) / 255);
                        continue;
                    }

                    const int alphaNumerator = coverage * 255 + destinationAlpha * inverse;
                    pixels[targetX] = qRgba((reservoir.red() * coverage * 255
                                             + qRed(destination) * destinationAlpha * inverse) / alphaNumerator,
                                            (reservoir.green() * coverage * 255
                                             + qGreen(destination) * destinationAlpha * inverse) / alphaNumerator,
                                            (reservoir.blue() * coverage * 255
                                             + qBlue(destination) * destinationAlpha * inverse) / alphaNumerator,
                                            alphaNumerator / 255);
                }
            }
        }
    }
    QPainter imagePainter(&m_image);
    imagePainter.drawImage(rect.topLeft(), result);
}

EditedImageProvider::EditedImageProvider(ImageEditController *controller)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_controller(controller)
{
}

QImage EditedImageProvider::requestImage(const QString &, QSize *size, const QSize &requestedSize)
{
    QImage result = m_controller ? m_controller->image() : QImage();
    if (size)
        *size = result.size();
    if (requestedSize.isValid() && !result.isNull())
        result = result.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return result;
}
