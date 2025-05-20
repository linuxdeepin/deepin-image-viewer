// SPDX-FileCopyrightText: 2023-2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "livetextanalyzer.h"

#include <QVariant>
#include <QApplication>
#include <QScreen>

#include <deepin-ocr-plugin-manager/deepinocrplugindef.h>
#include <deepin-ocr-plugin-manager/deepinocrplugin.h>

#include <QtConcurrent/QtConcurrent>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

LiveTextAnalyzer::LiveTextAnalyzer(QObject *parent)
    : QQuickImageProvider(Image),
      ocrDriver(new DeepinOCRPlugin::DeepinOCRDriver)
{
    qCDebug(logImageViewer) << "Initializing LiveTextAnalyzer";
    ocrDriver->loadDefaultPlugin();

    // FIXME: Loong64 with llvm are temporarily unstable, disable GPU for now.
#if !defined(_loongarch) && !defined(__loongarch__) && !defined(__loongarch64)
    qCDebug(logImageViewer) << "Enabling GPU acceleration for OCR";
    ocrDriver->setUseHardware({ { DeepinOCRPlugin::HardwareID::GPU_Vulkan, 0 } });
#else
    qCDebug(logImageViewer) << "GPU acceleration disabled for LoongArch";
    ocrDriver->setUseHardware({});
#endif

    // 退出时终止文本识别
    connect(qApp, &QCoreApplication::aboutToQuit, this, &LiveTextAnalyzer::breakAnalyze);

    if (QScreen *screen = qApp->primaryScreen()) {
        pixelRatio = screen->devicePixelRatio();
        qCDebug(logImageViewer) << "Screen pixel ratio:" << pixelRatio;
    }
}

void LiveTextAnalyzer::setImage(const QImage &image)
{
    qCDebug(logImageViewer) << "Setting new image for OCR analysis, size:" << image.size();
    imageCache = image;

    QImage image_copy = image.convertToFormat(QImage::Format_RGB888);
    // If the device pixel ratio is > 1, we need to reset the width and height to get the actual position.
    if (pixelRatio > 1) {
        image_copy = image_copy.scaled(
                image.width() / pixelRatio, image.height() / pixelRatio, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        qCDebug(logImageViewer) << "Scaled image for high DPI display, new size:" << image_copy.size();
    }

    ocrDriver->setMatrix(image_copy.height(),
                         image_copy.width(),
                         image_copy.bits(),
                         static_cast<size_t>(image_copy.bytesPerLine()),
                         DeepinOCRPlugin::PixelType::Pixel_RGB);
}

void LiveTextAnalyzer::analyze(const QString &token)
{
    qCDebug(logImageViewer) << "Starting OCR analysis with token:" << token;
    // 此处使用token来标记本次识别的目标，后续的识别结果也随token发出
    // 外部调用的时候也凭借收到的token判断是否采用此次的识别结果
    // 以此来解决QML的信号延迟问题，但仅降低此问题的复现概率，没有完全解决
    QtConcurrent::run([this, token]() {
        while (ocrDriver->isRunning()) { };   // 等待之前的分析结束
        bool result = ocrDriver->analyze();
        qCDebug(logImageViewer) << "OCR analysis completed with token:" << token << "result:" << result;
        emit analyzeFinished(result, token);
    });
}

void LiveTextAnalyzer::breakAnalyze()
{
    if (ocrDriver->isRunning()) {
        qCDebug(logImageViewer) << "Breaking current OCR analysis";
        ocrDriver->breakAnalyze();
    }
}

QVariant LiveTextAnalyzer::liveBlock() const
{
    auto boxes = ocrDriver->getTextBoxes();
    qCDebug(logImageViewer) << "Getting text blocks, count:" << boxes.size();

    QList<QVariant> result;
    for (auto &box : boxes) {
        QList<QVariant> temp;
        for (size_t i = 0; i != box.points.size(); ++i) {
            temp.push_back(box.points[i].first);
            temp.push_back(box.points[i].second);
        }
        temp.push_back(box.angle);
        result.push_back(temp);
    }

    return result;
}

QVariant LiveTextAnalyzer::charBox(int blockIndex) const
{
    if (static_cast<size_t>(blockIndex) >= ocrDriver->getTextBoxes().size()) {
        qCWarning(logImageViewer) << "Invalid block index:" << blockIndex;
        return QVariant();
    }

    auto boxes = ocrDriver->getCharBoxes(static_cast<size_t>(blockIndex));
    qCDebug(logImageViewer) << "Getting character boxes for block:" << blockIndex << "count:" << boxes.size();

    QList<QVariant> result;

    float base = boxes[0].points[0].first;
    result.push_back(0);
    for (auto &box : boxes) {
        result.push_back(box.points[1].first - base);
    }

    return result;
}

QString LiveTextAnalyzer::textResult(int blockIndex, int startIndex, int len) const
{
    if (static_cast<size_t>(blockIndex) >= ocrDriver->getTextBoxes().size() || startIndex < 0 || len <= 0) {
        qCWarning(logImageViewer) << "Invalid parameters for text result - block:" << blockIndex
                                  << "start:" << startIndex << "len:" << len;
        return "";
    }

    QString fullStr(ocrDriver->getResultFromBox(static_cast<size_t>(blockIndex)).c_str());
    QString result = fullStr.mid(startIndex, len);
    qCDebug(logImageViewer) << "Getting text result for block:" << blockIndex
                            << "start:" << startIndex << "len:" << len
                            << "result length:" << result.length();
    return result;
}

// 格式：random_index
QImage LiveTextAnalyzer::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    auto startIndex = id.indexOf("_") + 1;
    size_t index = id.mid(startIndex).toUInt();

    if (index >= ocrDriver->getTextBoxes().size()) {
        qCWarning(logImageViewer) << "Invalid text box index:" << index;
        return QImage();
    }

    // Combined with Image fileMode, show the original image
    auto box = ocrDriver->getTextBoxes()[index];
    QRect rect(QPoint(static_cast<int>(box.points[0].first * pixelRatio), static_cast<int>(box.points[0].second * pixelRatio)),
               QPoint(static_cast<int>(box.points[2].first * pixelRatio), static_cast<int>(box.points[2].second * pixelRatio)));

    qCDebug(logImageViewer) << "Requesting image for text box:" << index << "rect:" << rect;

    QImage image = imageCache.copy(rect);
    if (size != nullptr) {
        *size = image.size();
    }
    if (requestedSize.width() > 0 && requestedSize.height() > 0) {
        image = image.scaled(requestedSize);
        qCDebug(logImageViewer) << "Scaled image to requested size:" << requestedSize;
    }
    return image;
}
