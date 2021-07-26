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
#include <DAnchors>
#include "scen/imagegraphicsview.h"


DWIDGET_USE_NAMESPACE
class ImageInfoWidget;
class ExtensionPanel;
class NavigationWidget;
class BottomToolbar;
class TopToolbar;
class ViewPanel : public QFrame
{

    Q_OBJECT
public:
    enum MenuItemId {
        IdFullScreen,
        IdExitFullScreen,
        IdStartSlideShow,
        IdRename,
        IdPrint,
        IdAddToAlbum,
        IdCopy,
        IdMoveToTrash,
        IdRemoveFromTimeline,
        IdRemoveFromAlbum,
        IdAddToFavorites,
        IdRemoveFromFavorites,
        IdShowNavigationWindow,
        IdHideNavigationWindow,
        IdRotateClockwise,
        IdRotateCounterclockwise,
        IdSetAsWallpaper,
        IdDisplayInFileManager,
        IdImageInfo,
        IdSubMenu,
        IdDraw,
        IdOcr
    };

    explicit ViewPanel(QWidget *parent = nullptr);
    ~ViewPanel();

    void loadImage(const QString &path, QStringList paths);

    void initConnect();
    //初始化标题栏
    void initTopBar();
    //初始化缩放比和导航窗口
    void initFloatingComponent();
    //初始化缩放比例的窗口
    void initScaleLabel();
    //初始化导航窗口
    void initNavigation();
    //初始化右键菜单
    void initRightMenu();
    //初始化详细信息
    void initExtensionPanel();
    //更新右键菜单
    void updateMenuContent();
    //控制全屏和返回全屏
    void toggleFullScreen();
    //全屏
    void showFullScreen();
    //退出全屏
    void showNormal();
    /**
     * @brief appendAction  添加右键菜单按钮
     * @param id            按钮枚举ID
     * @param text          按钮名称
     * @param shortcut      按钮快捷键
     */
    void appendAction(int id, const QString &text, const QString &shortcut = "");

private slots:
    void onMenuItemClicked(QAction *action);
public slots:
    //刷新底部工具栏大小与位置
    void resetBottomToolbarGeometry(bool visible);
    //打开图片
    void openImg(int index, QString path);
protected:
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void imageChanged(const QString &path);
private :
    DStackedWidget *m_stack = nullptr;
    ImageGraphicsView *m_view = nullptr;
    BottomToolbar *m_bottomToolbar = nullptr;
    ImageInfoWidget *m_info = nullptr;
    ExtensionPanel  *m_extensionPanel {nullptr};
    DAnchors<NavigationWidget> m_nav ;

    TopToolbar *m_topToolbar = nullptr;

    DMenu *m_menu = nullptr;

    bool m_isMaximized = false;

};
#endif  // VIEWPANEL_H
