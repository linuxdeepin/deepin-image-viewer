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
#ifndef IMGVIEWLISTVIEW_H
#define IMGVIEWLISTVIEW_H

#include "imgviewdelegate.h"
#include "dtkwidget_global.h"

#include <QPropertyAnimation>
#include <DPushButton>
#include <DIconButton>
#include <DListView>
#include <QPixmap>
#include <QIcon>
#include <DLabel>
#include <QFileInfo>
#include <QSize>
#include <QStandardItemModel>
#include <QBuffer>
#include <DMenu>
#include <QMouseEvent>
#include <QPointer>
#include <DApplicationHelper>

#include "image-viewer_global.h"

DWIDGET_USE_NAMESPACE

class ImgViewListView : public DListView
{
    Q_OBJECT

public:
    explicit ImgViewListView(QWidget *parent = nullptr);
    ~ImgViewListView() override;

    void setAllFile(QList<imageViewerSpace::ItemInfo> itemInfos, QString path);//设置需要展示的所有缩略图

    int getSelectIndexByPath(QString path);
    //将选中的项居中
    void setSelectCenter();
    //查看下一张
    void openNext();
    //查看前一张
    void openPre();
    //移除当前选中
    void removeCurrent();
    //旋转
    void rotate(int index);
protected:
//    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
public slots:
    //有新的图片加载上来
    void slotOneImgReady(QString path, imageViewerSpace::ItemInfo pix);
    //列表点击事件
    void onClicked(const QModelIndex &index);
private:
    void cutPixmap(imageViewerSpace::ItemInfo &iteminfo);
    //加载后50张图片
    void loadFiftyRight();
    //当点击的是最后一个时，向前移动动画
    void startMoveToLeftAnimation();
    //根据行号获取路径path
    const QString getPathByRow(int row);
signals:
    void openImg(int index, QString path);
public:
    const static int ITEM_NORMAL_WIDTH = 30;//非选中状态宽度
    const static int ITEM_NORMAL_HEIGHT = 40;//非选中状态高度
    const static int ITEM_CURRENT_WH = 50;//当前选中状态宽高
    const static int ITEM_SPACING = 2;//间隔
public:
    ImgViewDelegate *m_delegate = nullptr;
    QStandardItemModel *m_model = nullptr;
    QStringList m_allFileList;//需要展示的全部缩略图路径

    QPropertyAnimation *m_moveAnimation = nullptr;//移动动画

    int m_currentRow = -1;//当前展示项
    int m_pre = -1;
    QString m_currentPath;
};

#endif // THUMBNAILLISTVIEW_H
