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
#ifndef IMGVIEWWIDGET_H
#define IMGVIEWWIDGET_H

#include <QWidget>
#include <QLabel>
//#include "controller/viewerthememanager.h"
//#include <dlistwidget.h>
//#include <DListWidget>
#include <DSpinner>
//#include <DtkWidgets>
//#include "dlistwidget.h"
#include <QListWidget>
#include <DListView>
#include <QAbstractItemModel>
#include <QStandardItem>
//#include "dbmanager/dbmanager.h"
#include <DAnchors>
#include <dimagebutton.h>
#include <DThumbnailProvider>
#include <QPropertyAnimation>
#include <QHBoxLayout>
#include <DIconButton>
#include <DBlurEffectWidget>
#include <DGuiApplicationHelper>
#include <DLabel>
#include <QTime>
//#include "imageengine/imageengineobject.h"

#include "image-viewer_global.h"
DWIDGET_USE_NAMESPACE

class ElidedLabel;
class QAbstractItemModel;
//class DImageButton;
class ImageButton;
class MyImageListWidget;
class ImageItem;
class ImgViewListView;

class MyImageListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyImageListWidget(QWidget *parent = nullptr);
    ~MyImageListWidget() override;

    void setAllFile(QList<imageViewerSpace::ItemInfo> itemInfos, QString path); //设置需要展示的所有缩略图
    //通过路径获取图片信息
    imageViewerSpace::ItemInfo getImgInfo(QString path);
    //获取当前图片信息
    imageViewerSpace::ItemInfo getCurrentImgInfo();
    //将选中的项居中
    void setSelectCenter();
    //获取当前所有展示图片数量
    int getImgCount();
    //清空缩略图
    void clearListView();

protected:
signals:
    void openImg(int index, QString path);
private:
    void resetSelectImg();
public slots:
    //列表点击事件
    void onClicked(const QModelIndex &index);

    void onScrollBarValueChanged(int value);

    //查看下一张
    void openNext();
    //查看前一张
    void openPre();
    //移除当前选中
    void removeCurrent();
    //旋转图片
    void rotate(int matrix);
    //设置当前图片
    void setCurrentPath(const QString &path);

    //获取所有路径
    QStringList getAllPath();
private:
    ImgViewListView *m_listview = nullptr;
    QPoint m_pressPoint;//鼠标按下位置
    QPoint m_pressListviewPoint;//鼠标按下时列表位置
    QPoint m_releasePoint;//鼠标释放位置
    bool m_mousePress = false;//鼠标是否按下
    QTime m_mousePressTime;//记录鼠标按下的时间点
    int m_time;//鼠标按下移动持续时间，毫秒
    int m_moveSpeed = 0;
//    QPropertyAnimation *m_correctAnimation = nullptr;//纠偏动画
};

#endif // IMGVIEWWIDGET_H
