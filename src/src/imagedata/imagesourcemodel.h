// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGESOURCEMODEL_H
#define IMAGESOURCEMODEL_H

#include "types.h"

#include <QAbstractListModel>

class ImageSourceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ImageSourceModel(QObject *parent = nullptr);
    ~ImageSourceModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE int indexForImagePath(const QString &file);
    Q_SLOT void setImageFiles(const QStringList &files);
    Q_SLOT void removeImage(const QString &fileName);

private:
    QList<QString> imageUrlList;       ///< 图像信息列表，部分信息保存至全局缓存中
};

#endif  // IMAGESOURCEMODEL_H
