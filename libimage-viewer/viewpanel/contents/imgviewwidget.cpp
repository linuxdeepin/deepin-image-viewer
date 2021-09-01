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
#include "imgviewlistview.h"
#include "accessibility/ac-desktop-define.h"

#include <QTimer>
#include <QScroller>
#include <QScrollBar>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QDebug>
#include <QPainterPath>
#include <QAbstractItemModel>
#include <QtMath>

#include <DLabel>
#include <DImageButton>
#include <DThumbnailProvider>
#include <DApplicationHelper>
#include <DSpinner>

#include "service/commonservice.h"


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
    m_listview->setObjectName("ImgViewListView");
//    hb->addWidget(m_listview);
    m_listview->viewport()->installEventFilter(this);
    connect(m_listview, &ImgViewListView::clicked, this, &MyImageListWidget::onClicked);
//    connect(m_listview->selectionModel(), &QItemSelectionModel::selectionChanged,
//            this, &MyImageListWidget::ONselectionChanged);

    connect(m_listview, &ImgViewListView::openImg, this, &MyImageListWidget::openImg);
    connect(m_listview->horizontalScrollBar(), &QScrollBar::valueChanged, this, &MyImageListWidget::onScrollBarValueChanged);
    initAnimation();

    //解决释放到程序外,不会动画的问题
    connect(CommonService::instance(), &CommonService::sigMouseRelease, this, [ = ] {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - 100 > m_lastReleaseTime)
        {
            m_lastReleaseTime = currentTime;
            animationStart(true, 0, 400);
        }
    });

}
#include "service/imagedataservice.h"
bool MyImageListWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::Leave) {
        qDebug() << "QEvent::Leave" << obj;
    }
    if (e->type() == QEvent::MouseButtonPress) {

        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
        m_pressPoint = mouseEvent->globalPos();
        m_movePoint = mouseEvent->globalPos();
        qDebug() << m_movePoint;
        m_moveViewPoint = mouseEvent->globalPos();

        m_timer->start();
        m_movePoints.clear();

        m_preListGeometryLeft = m_listview->geometry().left();

        m_listview->update();
        qDebug() << "------------getCount = " << ImageDataService::instance()->getCount();
    }
    if (e->type() == QEvent::MouseButtonRelease) {
        if (m_movePoints.size() > 0) {
            int endPos = m_movePoints.last().x() - m_movePoints.first().x();
            //过滤掉触屏点击时的move误操作
            if (abs(m_movePoints.last().x() - m_movePoints.first().x()) > 15) {
                animationStart(false, endPos, 500);
            } else {
                animationStart(true, 0, 400);
            }
        }
        //松手，动画重置按钮应该所在的位置
//        animationStart(true, 0, 400);
    }
    if (e->type() == QEvent::MouseMove || e->type() == QEvent::TouchUpdate) {
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
        QPoint p = mouseEvent->globalPos();
        if (m_movePoints.size() < 20) {
            m_movePoints.push_back(p);
        } else {
            m_movePoints.pop_front();
            m_movePoints.push_back(p);
        }
        m_listview->move(m_listview->x() + p.x() - m_moveViewPoint.x(), m_listview->y());
        m_moveViewPoint = p;

        if (m_movePoint.x() - p.x() >= 32) {
            m_listview->openNext();
            m_movePoint = QPoint(m_movePoint.x() - 32, m_movePoint.y());
        } else if (m_movePoint.x() - p.x() <= -32) {
            m_listview->openPre();
            m_movePoint = QPoint(m_movePoint.x() + 32, m_movePoint.y());
        }

        return true;
    }
    return QWidget::eventFilter(obj, e);
}

void MyImageListWidget::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "111111111";
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

void MyImageListWidget::initAnimation()
{
    m_timer = new QTimer(this);
    m_timer->setInterval(200);
    m_timer->setSingleShot(true);
    if (m_listview) {
        m_resetAnimation = new QPropertyAnimation(m_listview, "pos", m_listview); //和上层m_obj的销毁绑在一起
    }
    connect(m_resetAnimation, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(m_resetAnimation, SIGNAL(valueChanged(const QVariant)), this, SLOT(animationValueChanged(const QVariant)));
}

QString MyImageListWidget::getCurrentPath()
{
    return m_listview->m_currentPath;
}

int MyImageListWidget::getCurrentCount()
{
    return m_listview->m_currentRow;
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

void MyImageListWidget::animationFinished()
{
    //设置type标志用来判断是惯性动画还是复位动画
    if (m_resetAnimation->property("type") == "500") {
        m_resetFinish = false;
        animationStart(true, 0, 400);
    }
    if (m_resetAnimation->property("type") == "400") {
        m_resetFinish = true;
    }
}

void MyImageListWidget::animationValueChanged(const QVariant value)
{
    Q_UNUSED(value)
    if (m_resetAnimation->property("type") != "500") {
        return;
    }
    //惯性滑动
    thumbnailIsMoving();
}

void MyImageListWidget::animationStart(bool isReset, int endPos, int duration)
{
    if (m_resetAnimation->state() == QPropertyAnimation::State::Running) {
        m_resetAnimation->stop();
    }

    int moveX = 0;
    int middle = (this->geometry().right() - this->geometry().left()) / 2 ;
    int itemX = m_listview->x() + m_listview->getCurrentItemX();
    int rowWidth = m_listview->getRowWidth();
    if (rowWidth - m_listview->getCurrentItemX() < (this->geometry().width() / 2)) {
        moveX = this->geometry().width() - rowWidth - m_listview->x() ;
    } else if (m_listview->getCurrentItemX() < (this->geometry().width() / 2)) {
        moveX = 0 - m_listview->pos().x();
    } else if (m_listview->geometry().width() <= this->width()) {
        moveX = 0;
    } else {
        moveX = middle - itemX;
    }

    if (!isReset) {
        moveX = endPos;
    }
    m_resetAnimation->setDuration(duration);
    if (duration == 500) {
        m_resetAnimation->setProperty("type", "500");
    } else {
        m_resetAnimation->setProperty("type", "400");
    }
    m_resetAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_resetAnimation->setStartValue(m_listview->pos());
    m_resetAnimation->setEndValue(QPoint(m_listview->pos().x() + moveX, m_listview->pos().y()));
    m_resetAnimation->start();
}

void MyImageListWidget::stopAnimation()
{
    m_resetAnimation->stop();
}

void MyImageListWidget::thumbnailIsMoving()
{
    if (m_resetAnimation->state() == QPropertyAnimation::State::Running && m_resetAnimation->duration() == 400) {
        return;
    }
//    int endPos = m_movePoints.last().x() - m_movePoints.first().x();
    int offsetLimit = m_listview->geometry().left() - m_preListGeometryLeft;
    if (abs(offsetLimit) <= 32) {
        return;
    }
    qDebug() << offsetLimit;
    if (offsetLimit > 0) {
        m_listview->openPre();
    } else {
        m_listview->openNext();
    }
    m_preListGeometryLeft = m_listview->geometry().left();

}

void MyImageListWidget::moveCenterWidget()
{
    int moveX = 0;
    int middle = (this->geometry().right() - this->geometry().left()) / 2 ;
    int itemX = m_listview->x() + m_listview->getCurrentItemX();
    int rowWidth = m_listview->getRowWidth();
    if (rowWidth - m_listview->getCurrentItemX() < (this->geometry().width() / 2)) {
        moveX = this->geometry().width() - rowWidth - m_listview->x() ;
    } else if (m_listview->getCurrentItemX() < (this->geometry().width() / 2)) {
        moveX = 0 - m_listview->pos().x();
    } else if (m_listview->geometry().width() <= this->width()) {
        moveX = 0;
    } else {
        moveX = middle - itemX;
    }
    m_listview->move(m_listview->x() + moveX, m_listview->y());
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
    moveCenterWidget();
}

void MyImageListWidget::openPre()
{
    m_listview->openPre();
    moveCenterWidget();
}

void MyImageListWidget::onClicked(const QModelIndex &index)
{
    qDebug() << "---------";
    if (m_timer->isActive()) {
        m_listview->onClicked(index);
    }

    animationStart(true, 0, 400);
}

void MyImageListWidget::ONselectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << "---ONselectionChanged------";
    if (!selected.indexes().isEmpty()) {
        m_listview->onClicked(selected.indexes().first());
        animationStart(true, 0, 400);
    }

}
