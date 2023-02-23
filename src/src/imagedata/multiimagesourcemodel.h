// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTIIMAGESOURCEMODEL_H
#define MULTIIMAGESOURCEMODEL_H

#include "types.h"
#include "imageinfo.h"

#include <QAbstractListModel>

class MultiImageSourceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)
    Q_PROPERTY(int focusImageIndex READ focusImageIndex WRITE setFocusImageIndex NOTIFY focusImageIndexChanged)

public:
    explicit MultiImageSourceModel(QObject *parent = nullptr);
    ~MultiImageSourceModel() override;

    void setSourceModel(QAbstractItemModel *sourceModel);
    QAbstractItemModel *sourceModel() const;
    Q_SIGNAL void sourceModelChanged();

    void setFocusImageIndex(int index);
    int focusImageIndex() const;
    Q_SIGNAL void focusImageIndexChanged();

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    Q_SLOT void onImageInfoChanged();
    Q_SLOT void onSourceModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

private:
    QAbstractItemModel *imageModel = nullptr;
    int focusIndex = 0;
    int frameCount = 0;
    QUrl imagePath;

    ImageInfo info;
};

#endif // MULTIIMAGESOURCEMODEL_H
