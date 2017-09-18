/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
#ifndef ALBUMDELEGATE_H
#define ALBUMDELEGATE_H

#include <QObject>
#include <QDateTime>
#include <QStyledItemDelegate>

#include <controller/viewerthememanager.h>

class TDThumbnailThread;
class ThumbnailDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    struct ItemData {
        QString name;
        QString path;
        QPixmap thumbnail;
    };

    explicit ThumbnailDelegate(QObject *parent = nullptr);
    void setIsDataLocked(bool value);

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const Q_DECL_OVERRIDE;

signals:
    void thumbnailGenerated(const QString &path);

private:
    ItemData itemData(const QModelIndex &index) const;
    QPixmap thumbnail(const ThumbnailDelegate::ItemData &data) const;
    void startThumbnailThread(const ThumbnailDelegate::ItemData &data) const;
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

private:
    mutable QMap<QString, TDThumbnailThread *> m_threads;
    QColor m_borderColor;
    QString  m_defaultThumbnail;
};

#endif // ALBUMDELEGATE_H
