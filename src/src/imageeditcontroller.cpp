// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageeditcontroller.h"

#include <QImageReader>
#include <QImageWriter>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QSaveFile>
#include <QtMath>

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
    QRectF normalized = normalizedRect.normalized().intersected(QRectF(0, 0, 1, 1));
    return QRect(qFloor(normalized.left() * m_image.width()),
                 qFloor(normalized.top() * m_image.height()),
                 qCeil(normalized.width() * m_image.width()),
                 qCeil(normalized.height() * m_image.height())).intersected(m_image.rect());
}

bool ImageEditController::applyEffect(const QString &effect, const QRectF &normalizedRect, int strength)
{
    QWriteLocker locker(&m_lock);
    if (m_image.isNull())
        return false;
    const QRect rect = pixelRect(normalizedRect);
    if (rect.width() < 2 || rect.height() < 2)
        return false;

    strength = qBound(1, strength, 50);
    if (effect == QLatin1String("gaussian"))
        applyBoxBlur(rect, qMax(1, strength / 2));
    else if (effect == QLatin1String("mosaic"))
        applyMosaic(rect, qMax(2, strength));
    else if (effect == QLatin1String("graffiti"))
        applyGraffiti(rect, qMax(4, strength));
    else
        return false;

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

void ImageEditController::applyBoxBlur(const QRect &rect, int radius)
{
    const QRect sampleRect = rect.adjusted(-radius * 3, -radius * 3,
                                           radius * 3, radius * 3).intersected(m_image.rect());
    QImage blurred = m_image.copy(sampleRect);
    const int scaleFactor = qBound(2, radius + 1, 16);
    const QSize reduced(qMax(1, blurred.width() / scaleFactor),
                        qMax(1, blurred.height() / scaleFactor));
    blurred = blurred.scaled(reduced, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                     .scaled(blurred.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QPainter painter(&m_image);
    painter.drawImage(rect.topLeft(), blurred, rect.translated(-sampleRect.topLeft()));
}

void ImageEditController::applyMosaic(const QRect &rect, int blockSize)
{
    QImage region = m_image.copy(rect);
    const QSize reduced(qMax(1, region.width() / blockSize), qMax(1, region.height() / blockSize));
    region = region.scaled(reduced, Qt::IgnoreAspectRatio, Qt::FastTransformation)
                   .scaled(region.size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    QPainter(&m_image).drawImage(rect.topLeft(), region);
}

void ImageEditController::applyGraffiti(const QRect &rect, int spacing)
{
    QPainter painter(&m_image);
    painter.setClipRect(rect);
    painter.fillRect(rect, QColor(0, 110, 190, 210));
    QPen pen(QColor(255, 255, 255, 90), qMax(2, spacing / 4), Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pen);
    for (int offset = -rect.height(); offset < rect.width(); offset += spacing) {
        painter.drawLine(rect.left() + offset, rect.top(), rect.left() + offset + rect.height(), rect.bottom());
        painter.drawLine(rect.left() + offset, rect.bottom(), rect.left() + offset + rect.height(), rect.top());
    }
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
