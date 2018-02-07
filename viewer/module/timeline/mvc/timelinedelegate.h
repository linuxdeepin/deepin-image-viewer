/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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
#ifndef TIMELINE_DELEGATE_H
#define TIMELINE_DELEGATE_H

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QStyledItemDelegate>

#include "timelineitem.h"
#include "controller/viewerthememanager.h"

class QMutex;
class TLThumbnailThread;
class TimelineDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit TimelineDelegate(QObject *parent = NULL);

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const Q_DECL_OVERRIDE;

signals:
    void thumbnailGenerated(const QString &path);

private:
    TimelineItem::ItemData itemData(const QModelIndex &index) const;
    QPixmap thumbnail(const TimelineItem::ItemData &data) const;
    void startThumbnailThread(const TimelineItem::ItemData &data) const;
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

private:
    QColor m_borderColor;
    QColor m_dateColor;
    QColor m_seperatorColor;
    QString m_defaultThumbnail;
    mutable QMap<QString, TLThumbnailThread *> m_threads;
};

#endif // TIMELINE_DELEGATE_H
