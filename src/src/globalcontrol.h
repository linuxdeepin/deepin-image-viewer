// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALCONTROL_H
#define GLOBALCONTROL_H

#include "imagedata/imageinfo.h"

#include <QObject>
#include <QUrl>

class ImageSourceModel;

class GlobalControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ImageSourceModel* globalModel READ globalModel NOTIFY globalModelChanged)
    Q_PROPERTY(QUrl currentSource READ currentSource NOTIFY currentSourceChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int currentFrameIndex READ currentFrameIndex WRITE setCurrentFrameIndex NOTIFY currentFrameIndexChanged)
    Q_PROPERTY(bool hasPreviousImage READ hasPreviousImage NOTIFY hasPreviousImageChanged)
    Q_PROPERTY(bool hasNextImage READ hasNextImage NOTIFY hasNextImageChanged)

    // Const property
    Q_PROPERTY(int thumbnailViewBottom READ thumbnailViewBottom CONSTANT)

public:
    explicit GlobalControl(QObject *parent = nullptr);

    void setGlobalModel(ImageSourceModel *model);
    ImageSourceModel *globalModel() const;
    Q_SIGNAL void globalModelChanged();

    QUrl currentSource() const;
    Q_SIGNAL void currentSourceChanged();

    void setCurrentIndex(int index);
    int currentIndex() const;
    Q_SIGNAL void currentIndexChanged();

    void setCurrentFrameIndex(int frameIndex);
    int currentFrameIndex() const;
    Q_SIGNAL void currentFrameIndexChanged();

    bool hasPreviousImage() const;
    Q_SIGNAL void hasPreviousImageChanged();
    bool hasNextImage() const;
    Q_SIGNAL void hasNextImageChanged();
    Q_INVOKABLE bool previousImage();
    Q_INVOKABLE bool nextImage();
    Q_INVOKABLE bool firstImage();
    Q_INVOKABLE bool lastImage();

    Q_INVOKABLE void setImageFiles(const QStringList &imageFiles, const QString &openFile);

    enum ConstProperty {
        ThumbnailViewHeight = 70,
    };
    int thumbnailViewBottom() const;

private:
    void checkSwitchEnable();

private:
    int index = 0;
    int frameIndex = 0;
    ImageInfo currentImage;

    ImageSourceModel *sourceModel = nullptr;
    bool hasPrevious = false;
    bool hasNext = false;
};

#endif  // GLOBALCONTROL_H
