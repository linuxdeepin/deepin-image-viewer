/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangYong <zhangyong@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef IMGVIEWDELEGATE_H
#define IMGVIEWDELEGATE_H

#include <QObject>
#include <QDateTime>
#include <QStyledItemDelegate>
#include <QDebug>

#include "image-viewer_global.h"

class TDThumbnailThread;
class ImgViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    enum DelegateType {
        NullType = 0,
        AllPicViewType,
        AlbumViewType,
        AlbumViewPhoneType,
        TimeLineViewType,
        SearchViewType
    };

    explicit ImgViewDelegate(QObject *parent = nullptr);

    void setItemSize(QSize size);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) Q_DECL_OVERRIDE;

signals:

private:
    ItemInfo itemData(const QModelIndex &index) const;

public:

private:

};

#endif // ALBUMDELEGATE_H
