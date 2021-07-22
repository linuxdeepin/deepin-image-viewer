/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
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
#ifndef VIEWPANEL_H
#define VIEWPANEL_H

#include <QFrame>

#include <DStackedWidget>

#include "scen/imageview.h"

DWIDGET_USE_NAMESPACE

class BottomToolbar;
class ViewPanel : public QFrame
{

    Q_OBJECT
public:
//    enum MenuItemId {
//        IdFullScreen,
//        IdExitFullScreen,
//        IdStartSlideShow,
//        IdRename,
//        IdPrint,
//        IdAddToAlbum,
//        IdCopy,
//        IdMoveToTrash,
//        IdRemoveFromTimeline,
//        IdRemoveFromAlbum,
//        IdAddToFavorites,
//        IdRemoveFromFavorites,
//        IdShowNavigationWindow,
//        IdHideNavigationWindow,
//        IdRotateClockwise,
//        IdRotateCounterclockwise,
//        IdSetAsWallpaper,
//        IdDisplayInFileManager,
//        IdImageInfo,
//        IdSubMenu,
//        IdDraw,
//        IdOcr
//    };

    explicit ViewPanel(QWidget *parent = nullptr);
    ~ViewPanel();

    void loadImage(const QString &path);
private :
    //刷新底部工具栏大小与位置
    void resetBottomToolbarGeometry(bool visible);
protected:
    void resizeEvent(QResizeEvent *e) override;
private :
    DStackedWidget *m_stack = nullptr;
    ImageView *m_view = nullptr;
    BottomToolbar *m_bottomToolbar = nullptr;
};
#endif  // VIEWPANEL_H
