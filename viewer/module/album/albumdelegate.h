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

#include "controller/viewerthememanager.h"

class AlbumDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit AlbumDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;
    void destroyEditor(QWidget* editor,
                       const QModelIndex& index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget* editor,
                       const QModelIndex& index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;
    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const Q_DECL_OVERRIDE;
    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const Q_DECL_OVERRIDE;
    bool isEditFinished();
signals:
    void editingFinished(const QModelIndex &index);

private:
//    void drawBG(const QStyleOptionViewItem &option, QPainter *painter) const;
    void drawTitle(const QStyleOptionViewItem &option,
                   const QModelIndex& index,
                   QPainter *painter) const;
    QPixmap getCompoundPixmap(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;
    const QRect thumbnailRect(const QSize &bgSize) const;
    const QRect yearTitleRect(const QSize &bgSize, const QString &title) const;
    const QString yearTitle(const QDateTime &b, const QDateTime &e) const;
    void onEditFinished();
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

    QPixmap loadScaledPixmap(const QString &source, const int scaledSize) const;


private:
    mutable QModelIndex m_editingIndex;

    QColor m_titleColor;
    QColor m_dateColor;

    QString m_createAlbumNormalPic;
    QString m_createAlbumHoverPic;
    QString m_createAlbumPressPic;

    QString m_addPic;
    QString m_albumBgNormalPic;
    QString m_albumBgPressPic;
};

#endif // ALBUMDELEGATE_H
