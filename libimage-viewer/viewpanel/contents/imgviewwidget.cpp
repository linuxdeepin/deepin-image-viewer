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
#include "imgviewwidget.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/unionimage.h"

#include "accessibility/ac-desktop-define.h"

#include <QTimer>
#include <QScroller>
#include <QScrollBar>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QDebug>
#include <QPainterPath>
#include <DLabel>
#include <QAbstractItemModel>
#include <DImageButton>
#include <DThumbnailProvider>
#include <DApplicationHelper>
#include <DSpinner>
#include <QtMath>

#include "imgviewlistview.h"

DWIDGET_USE_NAMESPACE

const int Distance_factor = 4;//距离因子

MyImageListWidget::MyImageListWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(0);
    this->setLayout(hb);
    m_listview = new ImgViewListView(this);
    m_listview->setObjectName("m_imgListWidget");
    hb->addWidget(m_listview);

    connect(m_listview, &ImgViewListView::clicked, this, &MyImageListWidget::onClicked);
    connect(m_listview, &ImgViewListView::openImg, this, &MyImageListWidget::openImg);
    connect(m_listview->horizontalScrollBar(), &QScrollBar::valueChanged, this, &MyImageListWidget::onScrollBarValueChanged);
}

MyImageListWidget::~MyImageListWidget()
{
}

void MyImageListWidget::setAllFile(QList<imageViewerSpace::ItemInfo> itemInfos, QString path)
{
    m_listview->setAllFile(itemInfos, path);
    this->setVisible(itemInfos.size() > 1);
    setSelectCenter();
    emit openImg(m_listview->getSelectIndexByPath(path), path);
}

imageViewerSpace::ItemInfo MyImageListWidget::getImgInfo(QString path)
{
    imageViewerSpace::ItemInfo info;
    for (int i = 0; i < m_listview->m_model->rowCount(); i++) {
        QModelIndex indexImg = m_listview->m_model->index(i, 0);
        imageViewerSpace::ItemInfo infoImg = indexImg.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
        if (infoImg.path == path) {
            info = infoImg;
            break;
        }
    }
    return info;
}

imageViewerSpace::ItemInfo MyImageListWidget::getCurrentImgInfo()
{
    imageViewerSpace::ItemInfo infoImg;
    if (m_listview->m_currentRow < m_listview->m_model->rowCount()) {
        QModelIndex indexImg = m_listview->m_model->index(m_listview->m_currentRow, 0);
        infoImg = indexImg.data(Qt::DisplayRole).value<imageViewerSpace::ItemInfo>();
    }
    return infoImg;
}
//将选中项移到最前面，后期可能有修改，此时获取的列表宽度不正确
void MyImageListWidget::setSelectCenter()
{
    m_listview->setSelectCenter();
}

int MyImageListWidget::getImgCount()
{
    qDebug() << "---" << __FUNCTION__ << "---m_listview->m_model->rowCount() = " << m_listview->m_model->rowCount();
    return m_listview->m_model->rowCount();
}

void MyImageListWidget::clearListView()
{
    m_listview->m_model->clear();
}

void MyImageListWidget::removeCurrent()
{
    m_listview->removeCurrent();
    this->setVisible(getImgCount() > 1);
}

void MyImageListWidget::rotate(int matrix)
{
    m_listview->rotate(matrix);
}

void MyImageListWidget::setCurrentPath(const QString &path)
{
    m_listview->setCurrentPath(path);
}

QStringList MyImageListWidget::getAllPath()
{
    return m_listview->getAllPath();
}

void MyImageListWidget::onScrollBarValueChanged(int value)
{
    QModelIndex index = m_listview->indexAt(QPoint((m_listview->width() - 15), 10));
    if (!index.isValid()) {
        index = m_listview->indexAt(QPoint((m_listview->width() - 20), 10));
    }
}

void MyImageListWidget::openNext()
{
    m_listview->openNext();
}

void MyImageListWidget::openPre()
{
    m_listview->openPre();
}

void MyImageListWidget::onClicked(const QModelIndex &index)
{
    m_listview->onClicked(index);
}
