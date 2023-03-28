// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globalcontrol.h"
#include "types.h"
#include "imagedata/imagesourcemodel.h"

#include <QEvent>
#include <QDebug>

enum Interval {
    SubmitInterval = 200,  // 图片变更提交定时间隔 200ms
};

/**
   @class GlobalControl
   @brief QML单例类，全局数据控制，用于提供图片展示过程中的数据、切换控制等。
 */

GlobalControl::GlobalControl(QObject *parent)
    : QObject(parent)
    , sourceModel(new ImageSourceModel(this))
{
}

GlobalControl::~GlobalControl()
{
    submitImageChangeImmediately();
}

/**
   @brief 设置全局使用的数据模型为 \a model
 */
void GlobalControl::setGlobalModel(ImageSourceModel *model)
{
    if (sourceModel) {
        sourceModel->deleteLater();
    }

    sourceModel = model;
    Q_EMIT globalModelChanged();
}

/**
   @return 返回全局的数据模型
 */
ImageSourceModel *GlobalControl::globalModel() const
{
    return sourceModel;
}

/**
   @brief 设置当前显示的图片源为 \a source , 若图片源列表中无此图片，则不进行处理
 */
void GlobalControl::setCurrentSource(const QUrl &source)
{
    if (currentImage.source() == source) {
        return;
    }

    int index = sourceModel->indexForImagePath(source);
    if (-1 != index) {
        setCurrentIndex(index);
    }
}

/**
   @return 返回当前设置的图片url地址
 */
QUrl GlobalControl::currentSource() const
{
    return currentImage.source();
}

/**
   @brief 设置当前展示的图片索引为 \a index
 */
void GlobalControl::setCurrentIndex(int index)
{
    int validIndex = qBound(0, index, imageCount() - 1);
    if (this->index != validIndex) {
        submitImageChangeImmediately();

        // 更新图像信息，无论变更均更新
        QUrl image = sourceModel->data(sourceModel->index(validIndex), Types::ImageUrlRole).toUrl();
        currentImage.setSource(image);
        Q_EMIT currentSourceChanged();

        this->index = index;
        Q_EMIT currentIndexChanged();

        checkSwitchEnable();
    }
}

/**
   @return 返回当前展示的图片索引
 */
int GlobalControl::currentIndex() const
{
    return index;
}

/**
   @brief 设置当前展示的多页图图片索引为 \a frameIndex
 */
void GlobalControl::setCurrentFrameIndex(int frameIndex)
{
    int validFrameIndex = qBound(0, frameIndex, currentImage.frameCount() - 1);
    if (this->frameIndex != validFrameIndex) {
        submitImageChangeImmediately();

        this->frameIndex = validFrameIndex;
        Q_EMIT currentFrameIndexChanged();

        checkSwitchEnable();
    }
}

/**
   @return 返回当前展示的多页图索引
 */
int GlobalControl::currentFrameIndex() const
{
    return frameIndex;
}

/**
   @return 返回当前图片总数
 */
int GlobalControl::imageCount() const
{
    return sourceModel->rowCount();
}

/**
   @brief 设置当前图片旋转角度为 \a angle , 此变更不会立即更新，
        等待提交定时器结束后更新。
 */
void GlobalControl::setCurrentRotation(int angle)
{
    if (imageRotation != angle) {
        if (0 != (angle % 90)) {
            qWarning() << QString("Image rotate angle must be a multiple of 90 degrees, current is %1 .").arg(angle);
        }

        // 计算相较上一次是否需要交换宽高，angle 为 0 时特殊处理，不调整
        if (angle && !!((angle - imageRotation) % 180)) {
            currentImage.swapWidthAndHeight();
        }

        imageRotation = angle;
        // 保证更新界面旋转前刷新缓存，为0时同样通知，用以复位状态
        Q_EMIT requestRotateCacheImage();
        Q_EMIT currentRotationChanged();

        // 启动提交定时器
        submitTimer.start(SubmitInterval, this);
    }
}

/**
   @return 返回当前图片的旋转角度
 */
int GlobalControl::currentRotation()
{
    return imageRotation;
}

/**
   @return 返回是否可切换到前一张图片
 */
bool GlobalControl::hasPreviousImage() const
{
    return hasPrevious;
}

/**
   @return 返回是否可切换到后一张图片
 */
bool GlobalControl::hasNextImage() const
{
    return hasNext;
}

/**
   @return 切换到前一张图片并返回是否切换成功
 */
bool GlobalControl::previousImage()
{
    submitImageChangeImmediately();

    if (hasPreviousImage()) {
        Q_ASSERT(sourceModel);
        if (Types::MultiImage == currentImage.type()) {
            if (frameIndex > 0) {
                setCurrentFrameIndex(frameIndex - 1);
                return true;
            }
        }

        if (index > 0) {
            setCurrentIndex(index - 1);

            if (Types::MultiImage == currentImage.type()) {
                setCurrentFrameIndex(currentImage.frameCount() - 1);
            } else {
                setCurrentFrameIndex(0);
            }
            return true;
        }
    }

    return false;
}

/**
   @return 切换到后一张图片并返回是否切换成功
 */
bool GlobalControl::nextImage()
{
    submitImageChangeImmediately();

    if (hasNextImage()) {
        Q_ASSERT(sourceModel);
        if (Types::MultiImage == currentImage.type()) {
            if (frameIndex < currentImage.frameCount() - 1) {
                setCurrentFrameIndex(frameIndex + 1);
                return true;
            }
        }

        if (index < sourceModel->rowCount() - 1) {
            setCurrentIndex(index + 1);

            // 无论是否为多页图，均设置为0
            setCurrentFrameIndex(0);
            return true;
        }
    }

    return false;
}

/**
   @return 切换到首张图片并返回是否切换成功
 */
bool GlobalControl::firstImage()
{
    submitImageChangeImmediately();

    Q_ASSERT(sourceModel);
    if (sourceModel->rowCount()) {
        setCurrentFrameIndex(0);
        setCurrentIndex(0);
        return true;
    }
    return false;
}

/**
   @return 切换到最后图片并返回是否切换成功
 */
bool GlobalControl::lastImage()
{
    submitImageChangeImmediately();

    Q_ASSERT(sourceModel);
    int count = sourceModel->rowCount();
    if (count) {
        setCurrentIndex(count - 1);
        if (Types::MultiImage == currentImage.type()) {
            setCurrentFrameIndex(currentImage.frameCount() - 1);
        }

        return true;
    }
    return false;
}

/**
   @brief 设置打开图片列表 \a filePaths ， 其中 \a openFile 是首个展示的图片路径，
    将更新全局数据源并发送状态变更信号
 */
void GlobalControl::setImageFiles(const QStringList &filePaths, const QString &openFile)
{
    Q_ASSERT(sourceModel);
    // 优先更新数据源
    sourceModel->setImageFiles(QUrl::fromStringList(filePaths));
    int index = filePaths.indexOf(openFile);
    if (-1 == index || filePaths.isEmpty()) {
        index = 0;
    }

    if (this->index != index) {
        this->index = index;
        Q_EMIT currentIndexChanged();
    }
    if (0 != this->frameIndex) {
        this->frameIndex = 0;
        Q_EMIT currentFrameIndexChanged();
    }

    // 更新图像信息，无论变更均更新
    if (currentImage.source() != openFile) {
        currentImage.setSource(openFile);
    }
    Q_EMIT currentSourceChanged();

    checkSwitchEnable();
    Q_EMIT imageCountChanged();
}

/**
   @brief 移除当前图片列表中文件路径为 \a removeImage 的图片，更新当前图片索引
 */
void GlobalControl::removeImage(const QUrl &removeImage)
{
    if (0 != currentRotation()) {
        setCurrentRotation(0);
        submitTimer.stop();
    }

    // 移除当前图片，默认将后续图片前移，currentIndex将不会变更，手动提示更新
    bool needNotify = (index != sourceModel->rowCount() - 1);

    // 模型更新后将自动触发QML切换当前显示图片
    sourceModel->removeImage(removeImage);

    if (needNotify) {
        // 需要提示的情况下不会越界
        QUrl image = sourceModel->data(sourceModel->index(index), Types::ImageUrlRole).toUrl();
        currentImage.setSource(image);

        setCurrentFrameIndex(0);
        Q_EMIT currentSourceChanged();
        Q_EMIT currentIndexChanged();
    }

    checkSwitchEnable();
    Q_EMIT imageCountChanged();
}

/**
   @brief 图片重命名后更新数据，路径由 \a oldName 更新为 \a newName 。
 */
void GlobalControl::renameImage(const QUrl &oldName, const QUrl &newName)
{
    int index = sourceModel->indexForImagePath(oldName);
    if (-1 != index) {
        submitImageChangeImmediately();

        sourceModel->setData(sourceModel->index(index), newName, Types::ImageUrlRole);

        if (oldName == currentImage.source()) {
            // 强制刷新，避免出现重命名为已缓存的删除图片
            currentImage.setSource(newName);
            currentImage.reloadData();

            setCurrentFrameIndex(0);
            Q_EMIT currentSourceChanged();
            Q_EMIT currentIndexChanged();
        }
    }
}

/**
   @brief 提交当前图片的变更到图片文件，将触发文件重新写入磁盘
   @warning 在执行切换、删除、重命名等操作前，需手动执行提交当前图片变更信息的操作
 */
void GlobalControl::submitImageChangeImmediately()
{
    submitTimer.stop();
    int rotation = currentRotation();
    if (0 == rotation) {
        return;
    }

    rotation = rotation % 360;
    if (0 != rotation) {
        // 请求更新图片，同步图片旋转状态到文件中，将覆写文件
        Q_EMIT requestRotateImage(currentImage.source().toLocalFile(), rotation);
    }

    // 重置状态
    setCurrentRotation(0);
}

/**
   @brief 响应定时器事件，此处用于延时更新图片旋转
 */
void GlobalControl::timerEvent(QTimerEvent *event)
{
    if (submitTimer.timerId() == event->timerId()) {
        submitTimer.stop();
        submitImageChangeImmediately();
    }
}

/**
   @brief 根据当前展示图片索引判断是否允许切换前后图片
 */
void GlobalControl::checkSwitchEnable()
{
    Q_ASSERT(sourceModel);
    bool previous = (index > 0 || frameIndex > 0);
    bool next = (index < (sourceModel->rowCount() - 1) || frameIndex < (currentImage.frameCount() - 1));

    if (previous != hasPrevious) {
        hasPrevious = previous;
        Q_EMIT hasPreviousImageChanged();
    }
    if (next != hasNext) {
        hasNext = next;
        Q_EMIT hasNextImageChanged();
    }
}
