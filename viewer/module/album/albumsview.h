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
#ifndef ALBUMSVIEW_H
#define ALBUMSVIEW_H

#include "controller/dbmanager.h"
#include <QListView>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItemModel>

class AlbumDelegate;
class QMenu;
class AlbumsView : public QListView
{
    Q_OBJECT
public:
    explicit AlbumsView(QWidget *parent = 0);
    QModelIndex addAlbum(const DBAlbumInfo &info);
    void createAlbum();
    void updateView();

    int indexOf(const QString &name) const;
    QSize itemSize() const;
    void setItemSizeMultiple(int multiple);
    void popupDelDialog(const QString &albumName);

signals:
    void albumCreated();
    void albumRemoved();
    void changeItemSize(bool increase);
    void openAlbum(const QString &album);
    void startSlideShow(const QStringList &paths);

protected:
    int horizontalOffset() const Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;

private:
    enum MenuItemId {
        IdCreate,
        IdView,
        IdStartSlideShow,
        IdRename,
        IdExport,
        IdCopy,
        IdDelete,
        IdSubMenu,
        IdSeparator
    };

    void initShortcut();

    void appendAction(int id, const QString &text, const QString &shortcut, bool enable = true);
    void appendCreateIcon();
    void destroyEditor(const QModelIndex &index);
    bool isCreateIcon(const QModelIndex &index) const;
    void onClicked(const QModelIndex &index);
    void onDoubleClicked(const QModelIndex &index);
    void onMenuItemClicked(QAction *action);
    QString getAlbumName(const QModelIndex &index);
    const QString getNewAlbumName();
    const QStringList paths(const QString &album) const;
    void removeCreateIcon();
    void updateMenuContent(const QModelIndex &index);

private:
    AlbumDelegate *m_delegate;
    QStandardItemModel *m_model;
    QSize m_itemSize;
    QMenu *m_menu;

};

#endif // ALBUMSVIEW_H
