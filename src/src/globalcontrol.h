// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALCONTROL_H
#define GLOBALCONTROL_H

#include "imagedata/imageinfo.h"
#include "types.h"

#include <QObject>
#include <QUrl>
#include <QBasicTimer>

class ImageSourceModel;
class GlobalControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ImageSourceModel *globalModel READ globalModel NOTIFY globalModelChanged)
    Q_PROPERTY(QUrl currentSource READ currentSource WRITE setCurrentSource NOTIFY currentSourceChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int currentFrameIndex READ currentFrameIndex WRITE setCurrentFrameIndex NOTIFY currentFrameIndexChanged)
    Q_PROPERTY(int imageCount READ imageCount NOTIFY imageCountChanged)
    Q_PROPERTY(int currentRotation READ currentRotation WRITE setCurrentRotation NOTIFY currentRotationChanged)
    Q_PROPERTY(bool hasPreviousImage READ hasPreviousImage NOTIFY hasPreviousImageChanged)
    Q_PROPERTY(bool hasNextImage READ hasNextImage NOTIFY hasNextImageChanged)

public:
    explicit GlobalControl(QObject *parent = nullptr);
    ~GlobalControl() override;

    void setGlobalModel(ImageSourceModel *model);
    ImageSourceModel *globalModel() const;
    Q_SIGNAL void globalModelChanged();

    void setCurrentSource(const QUrl &source);
    QUrl currentSource() const;
    Q_SIGNAL void currentSourceChanged();
    void setCurrentIndex(int index);
    int currentIndex() const;
    Q_SIGNAL void currentIndexChanged();
    void setCurrentFrameIndex(int frameIndex);
    int currentFrameIndex() const;
    Q_SIGNAL void currentFrameIndexChanged();
    int imageCount() const;
    Q_SIGNAL void imageCountChanged();

    void setCurrentRotation(int angle);
    int currentRotation();
    Q_SIGNAL void currentRotationChanged();
    Q_SIGNAL void requestRotateCacheImage();

    bool hasPreviousImage() const;
    Q_SIGNAL void hasPreviousImageChanged();
    bool hasNextImage() const;
    Q_SIGNAL void hasNextImageChanged();
    Q_INVOKABLE bool previousImage();
    Q_INVOKABLE bool nextImage();
    Q_INVOKABLE bool firstImage();
    Q_INVOKABLE bool lastImage();

    Q_SLOT void setImageFiles(const QStringList &imageFiles, const QString &openFile);
    Q_SLOT void removeImage(const QUrl &removeImage);
    Q_SLOT void renameImage(const QUrl &oldName, const QUrl &newName);

    Q_SLOT void submitImageChangeImmediately();
    Q_SIGNAL void requestRotateImage(const QString &localPath, int rotation);

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    void checkSwitchEnable();

private:
    int index = 0;
    int frameIndex = 0;
    ImageInfo currentImage;

    ImageSourceModel *sourceModel = nullptr;
    bool hasPrevious = false;
    bool hasNext = false;

    int imageRotation = 0;    // 当前图片旋转角度
    QBasicTimer submitTimer;  // 图片变更提交定时器
};

#endif  // GLOBALCONTROL_H
