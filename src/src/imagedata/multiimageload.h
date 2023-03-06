// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTIIMAGELOAD_H
#define MULTIIMAGELOAD_H

#include <QQuickImageProvider>
#include <QImageReader>
#include <QImage>
#include <QMutex>

class MultiImageLoad : public QQuickImageProvider
{
public:
    explicit MultiImageLoad();
    ~MultiImageLoad() override;

    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QImage readNormalImage(const QString &imagePath);
    QImage readMultiImage(const QString &imagePath, int frameIndex);

private:
    QMutex mutex;
    int lastFrameIndex = 0;
    QString lastImagePath;
    QImage lastImage;
    QImageReader imageReader;  // 图像读取类
};

#endif  // MULTIIMAGELOAD_H
