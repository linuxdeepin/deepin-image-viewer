// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multiimagesourcemodel.h"
#include "imagesourcemodel.h"

MultiImageSourceModel::MultiImageSourceModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QObject::connect(&info, &ImageInfo::infoChanged, this, &MultiImageSourceModel::onImageInfoChanged);
}

MultiImageSourceModel::~MultiImageSourceModel() {}

void MultiImageSourceModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (imageModel == sourceModel) {
        return;
    }

    if (imageModel) {
        QObject::disconnect(imageModel, &QAbstractItemModel::dataChanged, this, &MultiImageSourceModel::onSourceModelDataChanged);
    }

    imageModel = sourceModel;

    if (imageModel) {
        QObject::connect(imageModel, &QAbstractItemModel::dataChanged, this, &MultiImageSourceModel::onSourceModelDataChanged);
    }

    emit sourceModelChanged();
}

QAbstractItemModel *MultiImageSourceModel::sourceModel() const
{
    return imageModel;
}

void MultiImageSourceModel::setFocusImageIndex(int index)
{
    if (focusIndex != index) {
        focusIndex = index;

        if (imageModel) {
            QModelIndex mIndex = imageModel->index(index, 0);
            QUrl tmpPath = imageModel->data(mIndex, Types::ImageUrlRole).toUrl();

            if (tmpPath != imagePath) {
                // Wait image info ready.
                info.setSource(tmpPath);
            } else {
                // For same path, if need update, ImageInfo will
                // auto refresh data and send infoChanged() signal.
            }
        }

        emit focusImageIndexChanged();
    }
}

int MultiImageSourceModel::focusImageIndex() const
{
    return focusIndex;
}

QHash<int, QByteArray> MultiImageSourceModel::roleNames() const
{
    return {{Types::ImageUrlRole, "imageUrl"}, {Types::FrameIndexRole, "frameIndex"}};
}

QVariant MultiImageSourceModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return {};
    }

    switch (role) {
        case Types::ImageUrlRole:
            return imagePath;
        case Types::FrameIndexRole:
            return index.row();
        default:
            break;
    }

    return {};
}

int MultiImageSourceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return frameCount;
}

void MultiImageSourceModel::onImageInfoChanged()
{
    beginResetModel();
    imagePath = info.source();
    frameCount = info.frameCount();
    endResetModel();
}

void MultiImageSourceModel::onSourceModelDataChanged(const QModelIndex &topLeft,
                                                     const QModelIndex &bottomRight,
                                                     const QVector<int> &roles)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(roles);

    // 数据可能变更
    if (bottomRight.row() <= focusIndex) {
        ImageSourceModel *imageModel = qobject_cast<ImageSourceModel *>(sourceModel());
        if (imageModel) {
            int index = imageModel->indexForImagePath(info.source().toString());
            if (focusIndex != index) {
                setFocusImageIndex(index);
            }
        }
    }
}
