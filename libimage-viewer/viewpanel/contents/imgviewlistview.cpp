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
#include "imgviewlistview.h"

#include <math.h>
#include <QDebug>
#include <QDrag>
#include <QFileInfo>
#include <QImageReader>
#include <QMimeData>
#include <QScrollBar>
#include <QMutex>
#include <QScroller>

#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/unionimage.h"
#include "imageengine.h"
#include "service/commonservice.h"

#include "accessibility/ac-desktop-define.h"

ImgViewListView::ImgViewListView(QWidget *parent)
    :  DListView(parent)
{
    m_model = new QStandardItemModel(this);
    m_delegate = new ImgViewDelegate(this);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSpacing(ITEM_SPACING);
    setDragEnabled(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    QListView::setFlow(QListView::LeftToRight);
    QListView::setWrapping(false);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);

//    setUniformItemSizes(true);

    setItemDelegate(m_delegate);
    setModel(m_model);
//    installEventFilter(viewport());

    setMouseTracking(true);
    this->viewport()->setMouseTracking(true);

    connect(ImageEngine::instance(), &ImageEngine::sigOneImgReady, this, &ImgViewListView::slotOneImgReady);
}

ImgViewListView::~ImgViewListView()
{
}

void ImgViewListView::setAllFile(QList<imageViewerSpace::ItemInfo> itemInfos, QString path)
{
    qDebug() << "---" << __FUNCTION__ << "---path = " << path;
    m_model->clear();
    m_currentPath = path;
    int count = itemInfos.size();
    for (int i = 0; i < count; i++) {
        imageViewerSpace::ItemInfo info = itemInfos.at(i);
        if (info.path == path) {
            info.imgWidth = ITEM_CURRENT_WH;
            info.imgHeight = ITEM_CURRENT_WH;
            m_currentRow = i;
        } else {
            info.imgWidth = ITEM_NORMAL_WIDTH;
            info.imgHeight = ITEM_NORMAL_HEIGHT;
        }
        QStandardItem *item = new QStandardItem;
        QVariant infoVariant;
        infoVariant.setValue(info);
        item->setData(infoVariant, Qt::DisplayRole);

        item->setData(QVariant(QSize(info.imgWidth, info.imgHeight)), Qt::SizeHintRole);
        m_model->appendRow(item);
    }

    doItemsLayout();

//    this->setFixedSize((2 * (count + 1) + 40 * count + 10), 60);
}

int ImgViewListView::getSelectIndexByPath(QString path)
{
    int index = -1;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex itemIndex = m_model->index(i, 0);
        imageViewerSpace::ItemInfo info = itemIndex.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
        if (info.path == path) {
            return i;
        }
    }
    return index;
}

void ImgViewListView::setSelectCenter()
{
    QModelIndex itemIndex = m_model->index(m_currentRow, 0);
    QRect rect = this->visualRect(itemIndex);
    this->horizontalScrollBar()->setValue(rect.x());
}

void ImgViewListView::openNext()
{
    if (m_currentRow == (m_model->rowCount() - 1)) {
        return;
    }

    QModelIndex currentIndex = m_model->index(m_currentRow, 0);
    QModelIndex nextIndex = m_model->index((m_currentRow + 1), 0);
    if (!nextIndex.isValid()) {
        return;
    }

    imageViewerSpace::ItemInfo info = nextIndex.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
    if (info.path.isEmpty()) {
        return;
    }

    if (currentIndex.isValid()) {
        //重置前一个选中项的宽高
        m_model->setData(currentIndex,
                         QVariant(QSize(ImgViewListView::ITEM_NORMAL_WIDTH, ImgViewListView::ITEM_NORMAL_HEIGHT)), Qt::SizeHintRole);
    }

    if (nextIndex.isValid()) {
        //重置新选中项的宽高
        m_model->setData(nextIndex,
                         QVariant(QSize(ImgViewListView::ITEM_CURRENT_WH, ImgViewListView::ITEM_CURRENT_WH)), Qt::SizeHintRole);

    }
    doItemsLayout();

    m_currentRow = m_currentRow + 1;
    m_currentPath = info.path;

    loadFiftyRight();

    startMoveToLeftAnimation();

    emit openImg(m_currentRow, m_currentPath);
}

void ImgViewListView::openPre()
{
    if (m_currentRow <= 0) {
        return;
    }

    QModelIndex currentIndex = m_model->index(m_currentRow, 0);
    QModelIndex preIndex = m_model->index((m_currentRow - 1), 0);
    if (!preIndex.isValid()) {
        return;
    }

    imageViewerSpace::ItemInfo info = preIndex.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
    if (info.path.isEmpty()) {
        return;
    }

    if (currentIndex.isValid()) {
        //重置前一个选中项的宽高
        m_model->setData(currentIndex,
                         QVariant(QSize(ImgViewListView::ITEM_NORMAL_WIDTH, ImgViewListView::ITEM_NORMAL_HEIGHT)), Qt::SizeHintRole);
    }

    if (preIndex.isValid()) {
        //重置新选中项的宽高
        m_model->setData(preIndex,
                         QVariant(QSize(ImgViewListView::ITEM_CURRENT_WH, ImgViewListView::ITEM_CURRENT_WH)), Qt::SizeHintRole);

    }
    doItemsLayout();

    m_currentRow = m_currentRow - 1;
    m_currentPath = info.path;

    emit openImg(m_currentRow, m_currentPath);
}

void ImgViewListView::removeCurrent()
{
    //当前显示数量大于3
    if (m_model->rowCount() > 1) {
        //删除最后一个,继续显示第一张
        qDebug() << "---" << __FUNCTION__ << "---m_currentRow = " << m_currentRow;
        qDebug() << "---" << __FUNCTION__ << "---m_model->rowCount() = " << m_model->rowCount();
        if (m_currentRow == (m_model->rowCount() - 1)) {
            QModelIndex index = m_model->index(0, 0);
            onClicked(index);
            m_model->removeRow(m_model->rowCount() - 1);
            if (m_model->rowCount() > 0) {
                this->horizontalScrollBar()->setValue(0);
            }
        } else {
            //显示下一张
            QModelIndex index = m_model->index((m_currentRow + 1), 0);
            onClicked(index);
            m_currentRow = m_currentRow - 1;
            //m_currentRow在onClicked中已经被修改了
            m_model->removeRow((m_currentRow));
        }
    } else if (m_model->rowCount() == 1) {
        //数量只有一张
        m_model->clear();
        m_currentRow = -1;
        m_currentPath = "";
    }
}

void ImgViewListView::slotOneImgReady(QString path, imageViewerSpace::ItemInfo pix)
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex index = m_model->index(i, 0);
        imageViewerSpace::ItemInfo data = index.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
        if (data.path == path) {
            pix.imgWidth = data.imgWidth;
            pix.imgHeight = data.imgHeight;
            cutPixmap(pix);
            QVariant meta;
            meta.setValue(pix);
            m_model->setData(index, meta, Qt::DisplayRole);
            this->update(index);
            this->viewport()->update();
            break;
        }
    }
}

void ImgViewListView::onClicked(const QModelIndex &index)
{
    if (index.row() == m_currentRow) {
        return;
    }
    imageViewerSpace::ItemInfo info = index.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
    if (info.path.isEmpty()) {
        return;
    }

    QModelIndex currentIndex = m_model->index(m_currentRow, 0);
    if (currentIndex.isValid()) {
        //重置前一个选中项的宽高
        m_model->setData(currentIndex,
                         QVariant(QSize(ImgViewListView::ITEM_NORMAL_WIDTH, ImgViewListView::ITEM_NORMAL_HEIGHT)), Qt::SizeHintRole);
    }
    //重置新选中项的宽高
    m_model->setData(index,
                     QVariant(QSize(ImgViewListView::ITEM_CURRENT_WH, ImgViewListView::ITEM_CURRENT_WH)), Qt::SizeHintRole);

    m_currentRow = index.row();
    m_currentPath = info.path;
    qDebug() << "---" << __FUNCTION__ << "---m_currentRow = " << m_currentRow;
    qDebug() << "---" << __FUNCTION__ << "---info.path = " << info.path;
    //刷新界面
    doItemsLayout();
    //如果点击的是最后一个则向前移动
    startMoveToLeftAnimation();
    //提前加载后面图片缩略图
    loadFiftyRight();

    emit openImg(m_currentRow, m_currentPath);
}

void ImgViewListView::cutPixmap(imageViewerSpace::ItemInfo &iteminfo)
{
    int width = iteminfo.image.width();
    if (width == 0)
        width = 180;
    int height = iteminfo.image.height();
    if (abs((width - height) * 10 / width) >= 1) {
        QRect rect = iteminfo.image.rect();
        int x = rect.x() + width / 2;
        int y = rect.y() + height / 2;
        if (width > height) {
            x = x - height / 2;
            y = 0;
            iteminfo.image = iteminfo.image.copy(x, y, height, height);
        } else {
            y = y - width / 2;
            x = 0;
            iteminfo.image = iteminfo.image.copy(x, y, width, width);
        }
    }
}
//加载后50张
void ImgViewListView::loadFiftyRight()
{
    int count = 0;
    for (int i = m_currentRow; i < m_model->rowCount(); i++) {
        count++;
        QModelIndex indexImg = m_model->index(i, 0);
        imageViewerSpace::ItemInfo infoImg = indexImg.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
        if (infoImg.image.isNull()) {
//            if (!ImageEngineApi::instance()->m_imgLoaded.contains(infoImg.path)) {
//                emit ImageEngineApi::instance()->sigLoadThumbnailIMG(infoImg.path);
//                ImageEngineApi::instance()->m_imgLoaded.append(infoImg.path);
//            }
        }
        if (count == 50) {
            break;
        }
    }
}

void ImgViewListView::startMoveToLeftAnimation()
{
    if (m_moveAnimation == nullptr) {
        m_moveAnimation = new QPropertyAnimation(this->horizontalScrollBar(), "value", this);
    }

    m_moveAnimation->setDuration(100);
    m_moveAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_moveAnimation->setStartValue(this->horizontalScrollBar()->value());
    m_moveAnimation->setEndValue((this->horizontalScrollBar()->value() + 32));
    //如果点击的是最后一个则向左滑动
    QRect rect = this->visualRect(m_model->index(m_currentRow, 0));
    if ((rect.x() + 52) >= (this->width() - 32)) {
        if (m_moveAnimation->state() == QPropertyAnimation::State::Running) {
            m_moveAnimation->stop();
        }
        m_moveAnimation->start();
    }
}

const QString ImgViewListView::getPathByRow(int row)
{
    QString result;
    if (row <= (m_model->rowCount() - 1)) {
        QModelIndex index = m_model->index(row, 0);
        if (index.isValid()) {
            imageViewerSpace::ItemInfo info = index.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
            result = info.path;
        }
    }
    return result;
}
