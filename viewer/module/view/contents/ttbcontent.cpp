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
#include "ttbcontent.h"
#include "application.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"

#include "controller/configsetter.h"
#include "controller/dbmanager.h"
#include "controller/signalmanager.h"
#include "widgets/elidedlabel.h"
#include "widgets/pushbutton.h"
#include "widgets/returnbutton.h"

#include <DImageButton>
#include <DSpinner>
#include <DThumbnailProvider>
#include <QAbstractItemModel>
#include <QDebug>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QTimer>
#include <QtMath>
#include <QPainterPath>

#define TTB 0

DWIDGET_USE_NAMESPACE
namespace {
const int LEFT_MARGIN = 10;
const QSize ICON_SIZE = QSize(50, 50);
const QString FAVORITES_ALBUM = "My favorite";
const int ICON_SPACING = 10;
const int RETURN_BTN_MAX = 200;
const int FILENAME_MAX_LENGTH = 600;
const int RIGHT_TITLEBAR_WIDTH = 100;
const int LEFT_SPACE = 20;
const QString LOCMAP_SELECTED_DARK = ":/assets/dark/images/58 drak.svg";
const QString LOCMAP_NOT_SELECTED_DARK = ":/assets/dark/images/imagewithbg-dark.svg";
const QString LOCMAP_SELECTED_LIGHT = ":/assets/light/images/58.svg";
const QString LOCMAP_NOT_SELECTED_LIGHT = ":/assets/light/images/imagewithbg.svg";
const QString LOCMAP_SELECTED_DAMAGED_DARK = ":/assets/dark/images/picture_damaged-58_drak.svg";
const QString LOCMAP_NOT_SELECTED_DAMAGED_DARK = ":/assets/dark/images/picture_damaged_dark.svg";
const QString LOCMAP_SELECTED_DAMAGED_LIGHT = ":/assets/light/images/picture_damaged_58.svg";
const QString LOCMAP_NOT_SELECTED_DAMAGED_LIGHT = ":/assets/light/images/picture_damaged.svg";

const int TOOLBAR_MINIMUN_WIDTH = 610 - 3;
const int TOOLBAR_JUSTONE_WIDTH = 310;
const int RT_SPACING = 20 + 5;
const int TOOLBAR_HEIGHT = 60;

const int TOOLBAR_DVALUE = 114 + 8;

const int THUMBNAIL_WIDTH = 32;
const int THUMBNAIL_ADD_WIDTH = 32;
const int THUMBNAIL_LIST_ADJUST = 9 + 5;
const int THUMBNAIL_VIEW_DVALUE = 496 + 10;

const unsigned int IMAGE_TYPE_JEPG = 0xFFD8FF;
const unsigned int IMAGE_TYPE_JPG1 = 0xFFD8FFE0;
const unsigned int IMAGE_TYPE_JPG2 = 0xFFD8FFE1;
const unsigned int IMAGE_TYPE_JPG3 = 0xFFD8FFE8;
const unsigned int IMAGE_TYPE_PNG = 0x89504e47;
const unsigned int IMAGE_TYPE_GIF = 0x47494638;
const unsigned int IMAGE_TYPE_TIFF = 0x49492a00;
const unsigned int IMAGE_TYPE_BMP = 0x424d;
}  // namespace
static bool bMove = false;
char *getImageType(QString filepath)
{
    char *ret = nullptr;
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    // Read and check the header
    quint32 magic;
    in >> magic;
    switch (magic) {
    case IMAGE_TYPE_JEPG:
    case IMAGE_TYPE_JPG1:
    case IMAGE_TYPE_JPG2:
    case IMAGE_TYPE_JPG3:
        //文件类型为 JEPG
        ret = QString("JEPG").toLatin1().data();
        break;
    case IMAGE_TYPE_PNG:
        //文件类型为 png
        ret = QString("PNG").toLatin1().data();
        break;
    case IMAGE_TYPE_GIF:
        //文件类型为 GIF
        ret = QString("GIF").toLatin1().data();
        break;
    case IMAGE_TYPE_TIFF:
        //文件类型为 TIFF
        ret = QString("TIFF").toLatin1().data();
        break;
    case IMAGE_TYPE_BMP:
        //文件类型为 BMP
        ret = QString("BMP").toLatin1().data();
        break;
    default:
        ret = nullptr;
        break;
    }
    return ret;
};

MyImageListWidget::MyImageListWidget(QWidget *parent)
    : DWidget(parent), m_timer(new QTimer(this))
{
    setMouseTracking(true);

    m_timer->setSingleShot(200);
}

bool MyImageListWidget::ifMouseLeftPressed()
{
    return bmouseleftpressed;
}

void MyImageListWidget::setObj(QObject *obj)
{
    m_obj = obj;
//     this->setAttribute(Qt::WA_AcceptTouchEvents);
    static_cast<QWidget *>(m_obj)->setAttribute(Qt::WA_AcceptTouchEvents);
    static_cast<QWidget *>(m_obj)->setMouseTracking(true);
    static_cast<QWidget *>(m_obj)->setAcceptDrops(false);
    static_cast<QWidget *>(m_obj)->grabGesture(Qt::PinchGesture);
    static_cast<QWidget *>(m_obj)->grabGesture(Qt::SwipeGesture);
}

bool MyImageListWidget::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)

    if (e->type() == QEvent::MouseButtonPress) {
        bmouseleftpressed = true;
        QMouseEvent *mouseEvent = (QMouseEvent *)e;
        m_prepoint = mouseEvent->globalPos();
        qDebug() << "m_prepoint:" << m_prepoint;
    }
    //lmh0819当手指拖动，有另外一只手指介入，则不会触发release事件，所以造成这样，当qt组修复这个问题，可以屏蔽掉代码，解决bug43181
    if (e->type() == QEvent::TouchBegin || e->type() == QEvent::TouchUpdate ||
            e->type() == QEvent::TouchEnd) {
        if(e->type() == QEvent::TouchBegin){
            m_maxTouchPoints=0;
        }
        else if(e->type() == QEvent::TouchUpdate){
            QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(e);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
            if(touchPoints.size()>m_maxTouchPoints){
                m_maxTouchPoints=touchPoints.size();
            }
        }
        if(e->type() == QEvent::TouchEnd){
            if(m_maxTouchPoints>=2){
                qDebug()<<e->type();
                m_iRet=false;
                bMove=false;
                bmouseleftpressed = false;
                if(m_currentImageItem){
                    m_currentImageItem->emitClickEndSig();
                }
                m_vecPoint.clear();
                m_lastPoint=QPoint(0,0);
                emit mouseLeftReleased();
                return false;
            }
        }
    }

    if (e->type() == QEvent::MouseButtonRelease) {
        bmouseleftpressed = false;
qDebug()<<e->type();

        int firsttolast=0;
        if(m_vecPoint.size()>0){
            firsttolast=m_vecPoint.first().x()-m_vecPoint.last().x();
        }

        m_iRet=true;
        QPropertyAnimation *animation = new QPropertyAnimation((DWidget *)m_obj, "pos");
        animation->setEasingCurve(QEasingCurve::NCurveTypes);
        animation->setStartValue(((DWidget *)m_obj)->pos());
        if(m_vecPoint.size()<20){
            animation->deleteLater();
            m_iRet=false;
            bMove=false;
            bmouseleftpressed = false;
            m_vecPoint.clear();
            m_lastPoint=QPoint(0,0);
            emit mouseLeftReleased();
            return false;
        }
        else if(firsttolast<20 && firsttolast>-20 ){
            animation->deleteLater();
            m_iRet=false;
            bMove=false;
            bmouseleftpressed = false;
            if(m_currentImageItem){
                m_currentImageItem->emitClickEndSig();
            }
            m_vecPoint.clear();
            m_lastPoint=QPoint(0,0);
            emit mouseLeftReleased();
            return false;
        }
        else if(firsttolast>=20 && firsttolast<50){
            animation->setDuration(1000);
            animation->setEndValue(QPoint(((DWidget *)m_obj)->x()-40, ((DWidget *)m_obj)->y()));
        }
        else if(firsttolast<=-20 && firsttolast>-50){
            animation->setDuration(1000);
            animation->setEndValue(QPoint(((DWidget *)m_obj)->x()+40, ((DWidget *)m_obj)->y()));
        }
        else if(firsttolast>=50 && firsttolast<100){
            animation->setDuration(800);
            animation->setKeyValueAt(0.5, QPoint(((DWidget *)m_obj)->x()-70, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(1, QPoint(((DWidget *)m_obj)->x()-100, ((DWidget *)m_obj)->y()));
            //            animation->setEndValue(QPoint(((DWidget *)m_obj)->x()-100, ((DWidget *)m_obj)->y()));
        }
        else if(firsttolast<=-50 && firsttolast>-100){
            animation->setDuration(800);
            animation->setKeyValueAt(0.5, QPoint(((DWidget *)m_obj)->x()+70, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(1, QPoint(((DWidget *)m_obj)->x()+100, ((DWidget *)m_obj)->y()));
            //            animation->setEndValue(QPoint(((DWidget *)m_obj)->x()+100, ((DWidget *)m_obj)->y()));
        }
        else if(firsttolast>=100 && firsttolast<200){
            animation->setDuration(800);
            animation->setKeyValueAt(0.5, QPoint(((DWidget *)m_obj)->x()-130, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(0.7, QPoint(((DWidget *)m_obj)->x()-170, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(1, QPoint(((DWidget *)m_obj)->x()-200, ((DWidget *)m_obj)->y()));
            //            animation->setEndValue(QPoint(((DWidget *)m_obj)->x()-200, ((DWidget *)m_obj)->y()));
        }
        else if(firsttolast<=-100&& firsttolast>-200){
            animation->setDuration(800);
            animation->setKeyValueAt(0.5, QPoint(((DWidget *)m_obj)->x()+130, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(0.7, QPoint(((DWidget *)m_obj)->x()+170, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(1, QPoint(((DWidget *)m_obj)->x()+200, ((DWidget *)m_obj)->y()));
            //            animation->setEndValue(QPoint(((DWidget *)m_obj)->x()+200, ((DWidget *)m_obj)->y()));
        }
        else if(firsttolast>=200){
            animation->setDuration(800);
            animation->setKeyValueAt(0.3, QPoint(((DWidget *)m_obj)->x()-150, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(0.5, QPoint(((DWidget *)m_obj)->x()-220, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(0.7, QPoint(((DWidget *)m_obj)->x()-260, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(1, QPoint(((DWidget *)m_obj)->x()-300, ((DWidget *)m_obj)->y()));
            //            animation->setEndValue(QPoint(((DWidget *)m_obj)->x()-300, ((DWidget *)m_obj)->y()));
        }
        else if(firsttolast<=-200){
            animation->setDuration(800);
            animation->setKeyValueAt(0.3, QPoint(((DWidget *)m_obj)->x()+150, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(0.5, QPoint(((DWidget *)m_obj)->x()+220, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(0.7, QPoint(((DWidget *)m_obj)->x()+260, ((DWidget *)m_obj)->y()));
            animation->setKeyValueAt(1, QPoint(((DWidget *)m_obj)->x()+300, ((DWidget *)m_obj)->y()));
            //            animation->setEndValue(QPoint(((DWidget *)m_obj)->x()+300, ((DWidget *)m_obj)->y()));
        }
        connect(animation, &QPropertyAnimation::finished, [=]{
            m_iRet=false;
            bMove=false;
            bmouseleftpressed = false;
            m_vecPoint.clear();
            if(m_currentImageItem){
                m_currentImageItem->emitClickEndSig();
            }
            m_lastPoint=QPoint(0,0);
            dynamic_cast<DWidget *>(m_obj)->resize(dynamic_cast<DWidget *>(m_obj)->width()+1,dynamic_cast<DWidget *>(m_obj)->height());

        });
        /*lmh0727*/
        QThread *th = QThread::create([=]() {
            if(m_bthreadMutex){
                return;
            }
            m_bthreadMutex=true;
            QMutexLocker locker(&m_threadMutex);
            while(m_iRet &&bMove)
            {
                QThread::msleep(50);
                /*lmh0727*/

                QObjectList list = dynamic_cast<DWidget *>(m_obj)->children();
                int middle = (this->geometry().left() + this->geometry().right()) / 2;
                int listLeft = dynamic_cast<DWidget *>(m_obj)->geometry().left();
                if(list.size()>2){
                    for (int i = 2; i < list.size(); i++) {
                        ImageItem *img=dynamic_cast<ImageItem *>(list.at(i));
                        if (nullptr == img) {
                            continue;
                        }
                        int left = this->geometry().left() + img->geometry().left() + listLeft;
                        int right = this->geometry().left() + img->geometry().right() + listLeft;
                        if (left <= middle && middle < right) {
                            if(m_currentImageItem!=img){
                                img->emitClickSig(img->getPath());
                                m_currentImageItem=img;
                            }
                            break;
                        }
                    }
                }
            }
            m_iRet=false;
            bMove=false;
            bmouseleftpressed = false;
            m_vecPoint.clear();
            if(m_currentImageItem){
                m_currentImageItem->emitClickEndSig();
            }
            m_bthreadMutex=false;
        });
        connect(th, &QThread::finished, th, &QObject::deleteLater);
        th->start();
        animation->start(QAbstractAnimation::DeleteWhenStopped);

//        emit mouseLeftReleased();
    }
    if (e->type() == QEvent::Leave && obj == m_obj) {
        bmouseleftpressed = false;
//        emit mouseLeftReleased();
    }
    if (e->type() == QEvent::MouseMove && bmouseleftpressed) {
        if(m_iRet){
            return false;
        }
        bMove=true;
        QMouseEvent *mouseEvent = (QMouseEvent *)e;
        QPoint p = mouseEvent->globalPos();
        if(m_vecPoint.size()<20){
            m_vecPoint.push_back(p);
            return false;
        }
        else{
            m_vecPoint.pop_front();
            m_vecPoint.push_back(p);
        }
        ((DWidget *)m_obj)
        ->move(((DWidget *)m_obj)->x() + p.x() - m_prepoint.x(), ((DWidget *)m_obj)->y());
        m_prepoint = p;

        QPoint CurrentcoursePoint = mouseEvent->globalPos();//获取当前光标的位置
        if(m_lastPoint==QPoint(0,0)){
            m_lastPoint=CurrentcoursePoint;
        }
        if((CurrentcoursePoint.x()-m_lastPoint.x())<32 &&(CurrentcoursePoint.x()-m_lastPoint.x())>-32  /*||(CurrentcoursePoint.x()-m_lastPoint.x())>64 ||(CurrentcoursePoint.x()-m_lastPoint.x())<-64*/)
        {
            return false;
        }
        m_lastPoint=CurrentcoursePoint;
        /*lmh0727*/

        QObjectList list = dynamic_cast<DWidget *>(m_obj)->children();
        int middle = (this->geometry().left() + this->geometry().right()) / 2;
        int listLeft = dynamic_cast<DWidget *>(m_obj)->geometry().left();
        if(list.size()>2){
            for (int i = 2; i < list.size(); i++) {
                ImageItem *img=dynamic_cast<ImageItem *>(list.at(i));
                if (nullptr == img) {
                    continue;
                }
                int left = this->geometry().left() + img->geometry().left() + listLeft;
                int right = this->geometry().left() + img->geometry().right() + listLeft;
                if (left <= middle && middle < right) {
                    if(m_currentImageItem!=img){
                        img->emitClickSig(img->getPath());
                        m_currentImageItem=img;
                    }
                    break;
                }
            }
        }
    }
    return false;
}

ImageItem::ImageItem(int index, QString path, char *imageType, QWidget *parent)
    :DLabel(parent)
{
    Q_UNUSED(imageType);
//    Q_UNUSED(parent);
    _index = index;
    _path = path;
    if (dApp->m_imagemap.contains(path)) {
        _pixmap = dApp->m_imagemap.value(path);
    }
    _image = new DLabel(this);
    connect(dApp, &Application::sigFinishLoad, this, [ = ](QString mapPath) {
        if (mapPath == _path || mapPath == "") {
            if (dApp->m_imagemap.contains(_path)) {
                if(!dApp->m_imagemap.value(_path).isNull())
                    _pixmap = dApp->m_imagemap.value(_path);
                update();
                bFirstUpdate = false;
            }
        }
    });
}

void ImageItem::emitClickSig(QString path)
{
    emit imageMoveclicked(path);
}

void ImageItem::emitClickEndSig()
{
    emit imageItemclicked(_index,_indexNow,false);
};

void ImageItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

    QPainter painter(this);

    painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform |
                           QPainter::Antialiasing);
    QRect backgroundRect = rect();
    QRect pixmapRect;
    QFileInfo fileinfo(_path);
    QString str = fileinfo.suffix();
    if (_index == _indexNow ||58==this->size().width()) {
        QPainterPath backgroundBp;
        QRect reduceRect = QRect(backgroundRect.x() + 1, backgroundRect.y() + 1,
                                 backgroundRect.width() - 2, backgroundRect.height() - 2);
        backgroundBp.addRoundedRect(reduceRect, 8, 8);
        painter.setClipPath(backgroundBp);
        painter.fillRect(
            reduceRect,
            QBrush(DGuiApplicationHelper::instance()->applicationPalette().highlight().color()));
//LMH解决选中竖图，大小改变0509,屏蔽掉修改图片形状
//        if (_pixmap.width() > _pixmap.height()) {
//            _pixmap = _pixmap.copy((_pixmap.width() - _pixmap.height()) / 2, 0, _pixmap.height(),
//                                   _pixmap.height());
//        } else if (_pixmap.width() < _pixmap.height()) {
//            _pixmap = _pixmap.copy(0, (_pixmap.height() - _pixmap.width()) / 2, _pixmap.width(),
//                                   _pixmap.width());
//        }

        pixmapRect.setX(backgroundRect.x() + 5);
        pixmapRect.setY(backgroundRect.y() + 5);
        pixmapRect.setWidth(backgroundRect.width() - 10);
        pixmapRect.setHeight(backgroundRect.height() - 10);
        //修复透明图片被选中后透明地方变成绿色
        QPainterPath bg0;
        bg0.addRoundedRect(pixmapRect, 4, 4);
        painter.setClipPath(bg0);
        if (themeType == DGuiApplicationHelper::LightType)
            painter.fillRect(pixmapRect, QBrush(Qt::white));
        else if (themeType == DGuiApplicationHelper::DarkType)
            painter.fillRect(pixmapRect, QBrush(Qt::black));
        if (!_pixmap.isNull()) {
            //            painter.fillRect(pixmapRect,
            //            QBrush(DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color()));
        }

        if (themeType == DGuiApplicationHelper::DarkType) {
            if (bFirstUpdate)
                m_pixmapstring = LOCMAP_SELECTED_DARK;
            else
                m_pixmapstring = LOCMAP_SELECTED_DAMAGED_DARK;
        } else {
            if (bFirstUpdate)
                m_pixmapstring = LOCMAP_SELECTED_LIGHT;
            else
                m_pixmapstring = LOCMAP_SELECTED_DAMAGED_LIGHT;
        }

        QPixmap pixmap = utils::base::renderSVG(m_pixmapstring, QSize(60, 60));
        QPainterPath bg;
        bg.addRoundedRect(pixmapRect, 4, 4);
        if (_pixmap.isNull()) {
            painter.setClipPath(bg);
            //            painter.drawPixmap(pixmapRect, m_pixmapstring);
            QIcon icon(m_pixmapstring);
            icon.paint(&painter, pixmapRect);
        }
    } else {
        pixmapRect.setX(backgroundRect.x() + 1);
        pixmapRect.setY(backgroundRect.y() + 0);
        pixmapRect.setWidth(backgroundRect.width() - 2);
        pixmapRect.setHeight(backgroundRect.height() - 0);

        QPainterPath bg0;
        bg0.addRoundedRect(pixmapRect, 4, 4);
        painter.setClipPath(bg0);

        if (!_pixmap.isNull()) {
            //            painter.fillRect(pixmapRect,
            //            QBrush(DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color()));
        }

        if (themeType == DGuiApplicationHelper::DarkType) {
            if (bFirstUpdate)
                m_pixmapstring = LOCMAP_NOT_SELECTED_DARK;
            else
                m_pixmapstring = LOCMAP_NOT_SELECTED_DAMAGED_DARK;
        } else {
            if (bFirstUpdate)
                m_pixmapstring = LOCMAP_NOT_SELECTED_LIGHT;
            else
                m_pixmapstring = LOCMAP_NOT_SELECTED_DAMAGED_LIGHT;
        }

        QPixmap pixmap = utils::base::renderSVG(m_pixmapstring, QSize(30, 40));
        QPainterPath bg;
        bg.addRoundedRect(pixmapRect, 4, 4);
        if (_pixmap.isNull()) {
            painter.setClipPath(bg);
            //            painter.drawPixmap(pixmapRect, m_pixmapstring);

            QIcon icon(m_pixmapstring);
            icon.paint(&painter, pixmapRect);
        }
    }
    //    QPixmap blankPix = _pixmap;
    //    blankPix.fill(Qt::white);

    //    QRect whiteRect;
    //    whiteRect.setX(pixmapRect.x() + 1);
    //    whiteRect.setY(pixmapRect.y() + 1);
    //    whiteRect.setWidth(pixmapRect.width() - 2);
    //    whiteRect.setHeight(pixmapRect.height() - 2);
    QPainterPath bg1;
    bg1.addRoundedRect(pixmapRect, 4, 4);
    painter.setClipPath(bg1);

    //    painter.drawPixmap(pixmapRect, blankPix);
    painter.drawPixmap(pixmapRect, _pixmap);

    painter.save();
    painter.setPen(
        QPen(DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color(), 1));
    painter.drawRoundedRect(pixmapRect, 4, 4);
    painter.restore();
}

TTBContent::TTBContent(bool inDB, DBImgInfoList m_infos,bool flag, QWidget *parent)
    : QLbtoDLabel(parent)
{
    m_NotImageViewFlag = flag;
    setWindoeSize(inDB, m_infos, parent);
    initBtn();
    toolbarSigConnection();
}

void TTBContent::setWindoeSize(bool inDB, DBImgInfoList m_infos, QWidget *parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    m_windowWidth = std::max(this->window()->width(),
                             ConfigSetter::instance()->value("MAINWINDOW", "WindowWidth").toInt());
    m_imgInfos = m_infos;
    m_imgInfos_size = m_imgInfos.size();
    if (m_imgInfos.size() <= 1) {
        m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
    } else if (m_imgInfos.size() <= 3) {
        m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
    } else {
        m_contentWidth =
            qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)),
                 qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) +
            THUMBNAIL_LIST_ADJUST;
    }

    setFixedWidth(m_contentWidth);
    setFixedHeight(70);
    m_inDB = inDB;
    connect(parent, SIGNAL(sigResize()), this, SLOT(onResize()));
}

void TTBContent::initBtn()
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(LEFT_MARGIN, 0, LEFT_MARGIN, 1);
    hb->setSpacing(0);
    m_preButton = new DIconButton(this);
    m_preButton->setObjectName("PreviousButton");
    m_preButton->setFixedSize(ICON_SIZE);
    m_preButton->setIcon(QIcon::fromTheme("dcc_previous"));
    m_preButton->setIconSize(QSize(36, 36));
    m_preButton->setToolTip(tr("Previous"));

    // nextButton
    m_nextButton = new DIconButton(this);
    m_nextButton->setObjectName("NextButton");
    m_nextButton->setFixedSize(ICON_SIZE);
    m_nextButton->setIcon(QIcon::fromTheme("dcc_next"));
    m_nextButton->setIconSize(QSize(36, 36));
    m_nextButton->setToolTip(tr("Next"));

    m_preButton_spc = new DWidget;
    m_nextButton_spc = new DWidget;
    m_preButton_spc->hide();
    m_nextButton_spc->hide();
    m_preButton_spc->setFixedSize(QSize(10, 50));
    m_nextButton_spc->setFixedSize(QSize(40, 50));
    //    hb->addWidget(btPre);
    hb->addWidget(m_preButton);
    hb->addWidget(m_preButton_spc);
    //    hb->addWidget(btNext);
    hb->addWidget(m_nextButton);
    hb->addWidget(m_nextButton_spc);

    // adaptImageBtn
    m_adaptImageBtn = new DIconButton(this);
    m_adaptImageBtn->setObjectName("AdaptBtn");
    m_adaptImageBtn->setFixedSize(ICON_SIZE);
    m_adaptImageBtn->setIcon(QIcon::fromTheme("dcc_11"));
    m_adaptImageBtn->setIconSize(QSize(36, 36));
    m_adaptImageBtn->setToolTip(tr("1:1 Size"));
    m_adaptImageBtn->setCheckable(true);
    hb->addWidget(m_adaptImageBtn);
    hb->addSpacing(ICON_SPACING);

    // adaptScreenBtn
    m_adaptScreenBtn = new DIconButton(this);
    m_adaptScreenBtn->setFixedSize(ICON_SIZE);
    m_adaptScreenBtn->setObjectName("AdaptScreenBtn");
    m_adaptScreenBtn->setIcon(QIcon::fromTheme("dcc_fit"));
    m_adaptScreenBtn->setIconSize(QSize(36, 36));
    m_adaptScreenBtn->setToolTip(tr("Fit to window"));
    //    m_adaptScreenBtn->setCheckable(true);
    hb->addWidget(m_adaptScreenBtn);
    hb->addSpacing(ICON_SPACING);

    // rotateLBtn
    m_rotateLBtn = new DIconButton(this);
    m_rotateLBtn->setFixedSize(ICON_SIZE);
    m_rotateLBtn->setIcon(QIcon::fromTheme("dcc_left"));
    m_rotateLBtn->setIconSize(QSize(36, 36));
    m_rotateLBtn->setToolTip(tr("Rotate counterclockwise"));
    hb->addWidget(m_rotateLBtn);
    hb->addSpacing(ICON_SPACING);

    // rotateRBtn
    m_rotateRBtn = new DIconButton(this);
    m_rotateRBtn->setFixedSize(ICON_SIZE);
    m_rotateRBtn->setIcon(QIcon::fromTheme("dcc_right"));
    m_rotateRBtn->setIconSize(QSize(36, 36));
    m_rotateRBtn->setToolTip(tr("Rotate clockwise"));
    hb->addWidget(m_rotateRBtn);
    hb->addSpacing(ICON_SPACING);

    m_imgListView_prespc = new DWidget;
    m_imgListView_prespc->setFixedSize(QSize(10, 50));
    hb->addWidget(m_imgListView_prespc);
    m_imgListView_prespc->hide();
    m_imgListView = new MyImageListWidget();
    m_imgList = new DWidget(m_imgListView);
    m_imgListView->setObj(m_imgList);
    m_imgList->installEventFilter(m_imgListView);
    if (m_imgInfos.size() <= 3) {
        m_imgList->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
    } else {
        m_imgList->setFixedSize(
            QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)),
                       qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) -
                  THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST,
                  TOOLBAR_HEIGHT));
    }

    m_imgList->setDisabled(false);
    m_imglayout = new QHBoxLayout();
    m_imglayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    //    m_imglayout->setMargin(0);
    m_imglayout->setContentsMargins(0, 1, 0, 0);
    m_imglayout->setSpacing(0);
//LMH0605
    DLabel *pLabel=new DLabel (this);
    pLabel->setMaximumSize(QSize(0,58));
    pLabel->setFixedSize(QSize(0,58));
    pLabel->resize(QSize(0,58));
    m_imglayout->addWidget(pLabel);

    m_imgList->setLayout(m_imglayout);

    if (m_imgInfos.size() <= 3) {
        m_imgListView->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
    } else {
        m_imgListView->setFixedSize(
            QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)),
                       qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) -
                  THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST,
                  TOOLBAR_HEIGHT));
    }

    QPalette palette;
    palette.setColor(QPalette::Background, QColor(0, 0, 0, 0));  // 最后一项为透明度
    m_imgList->setPalette(palette);
    m_imgListView->setPalette(palette);

    hb->addWidget(m_imgListView);

    m_imgListView_spc = new DWidget;
    m_imgListView_spc->setFixedSize(QSize(24 - 9 + 10, 50));  // temp
    hb->addWidget(m_imgListView_spc);
    m_imgListView_spc->hide();

    m_fileNameLabel = new ElidedLabel();
    m_trashBtn = new DIconButton(this);
    m_trashBtn->setFixedSize(ICON_SIZE);
    m_trashBtn->setObjectName("TrashBtn");
    m_trashBtn->setIcon(QIcon::fromTheme("dcc_delete"));
    m_trashBtn->setIconSize(QSize(36, 36));
    m_trashBtn->setToolTip(tr("Delete"));

    hb->addWidget(m_trashBtn);
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    bool theme = false;
    if (themeType == DGuiApplicationHelper::DarkType) {
        theme = true;
    } else {
        theme = false;
    }
    slotTheme(theme);
}

void TTBContent::toolbarSigConnection()
{
    //拖动后移动多少
    connect(m_imgListView, &MyImageListWidget::mouseLeftReleased, this, [ = ] {
        int movex = m_imgList->x();
        if (movex >= 0)
        {
            if (m_imgList->width() < m_imgListView->width()) {
                if (m_imgList->width() + movex > m_imgListView->width()) {
                    movex = (m_imgListView->width() - m_imgList->width()) / 2;
                } else {
                    return;
                }
            } else {
                movex = 0;
                emit dApp->signalM->sendLoadSignal(true);
            }
        } else
        {
            if (m_imgList->width() < m_imgListView->width()) {
                movex = (m_imgListView->width() - m_imgList->width()) / 2;
            } else {
                if (movex < m_imgListView->width() - m_imgList->width()) {
                    movex = m_imgListView->width() - m_imgList->width();
                    emit dApp->signalM->sendLoadSignal(false);
                }
            }
        }

        m_imgList->move(movex, m_imgList->y());
        m_nLastMove = movex;
    });

    connect(dApp->signalM, &SignalManager::picDelete, this, [ = ] {
        if (window()->isFullScreen())
        {
            emit dApp->signalM->sigShowFullScreen();
        }
        m_imgInfos_size = m_imgInfos_size - 1;
        int windowWidth = this->window()->geometry().width();
        if (m_imgInfos_size <= 1)
        {
            m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
        } else if (m_imgInfos_size <= 3)
        {
            m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
            m_imgListView->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
        } else
        {
            m_contentWidth =
            qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos_size - 3)),
                 qMax(windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) +
            THUMBNAIL_LIST_ADJUST;
            m_imgListView->setFixedSize(
                QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos_size - 3)),
                           qMax(windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) -
                      THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST,
                      TOOLBAR_HEIGHT));
        }
        setFixedWidth(m_contentWidth);
    });

    connect(m_preButton, &DIconButton::clicked, this, [ = ] { emit showPrevious(); });
    connect(m_nextButton, &DIconButton::clicked, this, [ = ] { emit showNext(); });
    connect(m_adaptImageBtn, &DIconButton::clicked, this, [ = ] {
        m_adaptImageBtn->setChecked(true);
        if (!badaptImageBtnChecked)
        {
            badaptImageBtnChecked = true;
            emit resetTransform(false);
        }
    });

    connect(m_adaptScreenBtn, &DIconButton::clicked, this, [ = ] {
        emit resetTransform(true);
        setAdaptButtonChecked(false);
    });
    connect(m_rotateLBtn, &DIconButton::clicked, this, &TTBContent::rotateCounterClockwise);
    connect(m_rotateRBtn, &DIconButton::clicked, this, &TTBContent::rotateClockwise);
    connect(m_trashBtn, &DIconButton::clicked, this, &TTBContent::removed);

    connect(dApp->signalM, &SignalManager::isAdapt, this, [ = ](bool immediately) {
        if (immediately) {
            setAdaptButtonChecked(true);
        } else {
            setAdaptButtonChecked(false);
        }
    });
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
    this, [ = ]() {
        DGuiApplicationHelper::ColorType themeType =
            DGuiApplicationHelper::instance()->themeType();
        bool theme = false;
        if (themeType == DGuiApplicationHelper::DarkType) {
            theme = true;
        } else {
            theme = false;
        }
        slotTheme(theme);
    });
    connect(dApp->signalM, &SignalManager::sigLoadHeadThunbnail, this, &TTBContent::ReInitFirstthumbnails);
}



void TTBContent::disCheckAdaptImageBtn()
{
    m_adaptImageBtn->setChecked(false);
    badaptImageBtnChecked = false;
}

void TTBContent::checkAdaptImageBtn()
{
    m_adaptImageBtn->setChecked(true);
    badaptImageBtnChecked = true;
}

void TTBContent::setAdaptButtonChecked(bool flag)
{
    Q_UNUSED(flag);
    //    if (btAdapt->isChecked() != flag) {
    //        btAdapt->setChecked(flag);
    //    }
    //    if (m_adaptImageBtn->isChecked() != flag) {
    //        m_adaptImageBtn->setChecked(flag);
    //    }
}
void TTBContent::slotTheme(bool theme)
{
    DPalette pa;
    QString rStr;
    if (theme) {
        rStr = "dark";
        //        pa = btPre->palette();
        //        pa.setColor(DPalette::Light, QColor("#444444"));
        //        pa.setColor(DPalette::Dark, QColor("#444444"));
        //        btPre->setPalette(pa);

        //        pa = btNext->palette();
        //        pa.setColor(DPalette::Light, QColor("#444444"));
        //        pa.setColor(DPalette::Dark, QColor("#444444"));
        //        btNext->setPalette(pa);

        //        pa = btAdapt->palette();
        //        pa.setColor(DPalette::Light, QColor("#444444"));
        //        pa.setColor(DPalette::Dark, QColor("#444444"));
        //        btAdapt->setPalette(pa);

        //        pa = btFit->palette();
        //        pa.setColor(DPalette::Light, QColor("#444444"));
        //        pa.setColor(DPalette::Dark, QColor("#444444"));
        //        btFit->setPalette(pa);

        //        pa = btLeft->palette();
        //        pa.setColor(DPalette::Light, QColor("#444444"));
        //        pa.setColor(DPalette::Dark, QColor("#444444"));
        //        btLeft->setPalette(pa);

        //        pa = btRight->palette();
        //        pa.setColor(DPalette::Light, QColor("#444444"));
        //        pa.setColor(DPalette::Dark, QColor("#444444"));
        //        btRight->setPalette(pa);

        //        pa = btTrash->palette();
        //        pa.setColor(DPalette::Light, QColor("#444444"));
        //        pa.setColor(DPalette::Dark, QColor("#444444"));
        //        btTrash->setPalette(pa);
    } else {
        rStr = "light";
        //        pa = btPre->palette();
        //        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        //        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        //        btPre->setPalette(pa);

        //        pa = btNext->palette();
        //        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        //        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        //        btNext->setPalette(pa);

        //        pa = btAdapt->palette();
        //        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        //        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        //        btAdapt->setPalette(pa);

        //        pa = btFit->palette();
        //        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        //        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        //        btFit->setPalette(pa);

        //        pa = btLeft->palette();
        //        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        //        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        //        btLeft->setPalette(pa);

        //        pa = btRight->palette();
        //        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        //        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        //        btRight->setPalette(pa);

        //        pa = btTrash->palette();
        //        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        //        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        //        btTrash->setPalette(pa);
    }

    //    btPre->setPropertyPic(QString(":/assets/%1/icons/previous_normal.svg").arg(rStr),
    //                          QString(":/assets/%1/icons/previous_hover.svg").arg(rStr),
    //                          QString(":/assets/%1/icons/previous_press.svg").arg(rStr));
    //    btNext->setPropertyPic(QString(":/assets/%1/icons/next_normal.svg").arg(rStr),
    //                           QString(":/assets/%1/icons/next_hover.svg").arg(rStr),
    //                           QString(":/assets/%1/icons/next_press.svg").arg(rStr));
    //    btAdapt->setPropertyPic(QString(":/assets/%1/icons/1_1_normal.svg").arg(rStr),
    //                            QString(":/assets/%1/icons/1_1_hover.svg").arg(rStr),
    //                            QString(":/assets/%1/icons/1_1_press.svg").arg(rStr),
    //                            QString(":/assets/%1/icons/1_1_checked.svg").arg(rStr));
    //    btFit->setPropertyPic(QString(":/assets/%1/icons/fit_normal.svg").arg(rStr),
    //                          QString(":/assets/%1/icons/fit_hover.svg").arg(rStr),
    //                          QString(":/assets/%1/icons/fit_press.svg").arg(rStr));
    //    btLeft->setPropertyPic(QString(":/assets/%1/icons/left_normal.svg").arg(rStr),
    //                           QString(":/assets/%1/icons/left_hover.svg").arg(rStr),
    //                           QString(":/assets/%1/icons/left_press.svg").arg(rStr));
    //    btRight->setPropertyPic(QString(":/assets/%1/icons/right_normal.svg").arg(rStr),
    //                            QString(":/assets/%1/icons/right_hover.svg").arg(rStr),
    //                            QString(":/assets/%1/icons/right_press.svg").arg(rStr));
    //    btTrash->setPropertyPic(QString(":/assets/%1/icons/delete.svg").arg(rStr),
    //                            QString(":/assets/%1/icons/delete.svg").arg(rStr),
    //                            QString(":/assets/%1/icons/delete.svg").arg(rStr));
}

void TTBContent::OnSetimglist(int currindex, QString filename, QString filepath)
{
    m_imgInfos[currindex].fileName = filename;
    m_imgInfos[currindex].filePath = filepath;
}

void TTBContent::OnChangeItemPath(int currindex, QString path)
{
    QList<ImageItem *> labelList = m_imgList->findChildren<ImageItem *>();
    if(labelList.count() == 0) return;
    ImageItem *item = labelList.at(currindex);
    item->SetPath(path);
    item->setObjectName(path);
}

void TTBContent::updateFilenameLayout()
{
    using namespace utils::base;
    DFontSizeManager::instance()->bind(m_fileNameLabel, DFontSizeManager::T8);
    QFontMetrics fm(DFontSizeManager::instance()->get(DFontSizeManager::T8));
    QString filename = QFileInfo(m_imagePath).fileName();
    QString name;

    int strWidth = fm.boundingRect(filename).width();
    int leftMargin = 0;
    int m_leftContentWidth = 0;
#ifndef LITE_DIV
    if (m_inDB)
        m_leftContentWidth =
            m_returnBtn->buttonWidth() + 6 + (ICON_SIZE.width() + 2) * 6 + LEFT_SPACE;
    else {
        m_leftContentWidth = m_folderBtn->width() + 8 + (ICON_SIZE.width() + 2) * 5 + LEFT_SPACE;
    }
#else
    // 39 为logo以及它的左右margin
    m_leftContentWidth = 5 + (ICON_SIZE.width() + 2) * 5 + 39;
#endif

    int ww = dApp->setter->value("MAINWINDOW", "WindowWidth").toInt();
    m_windowWidth = std::max(std::max(this->window()->geometry().width(), this->width()), ww);
    m_contentWidth = std::max(m_windowWidth - RIGHT_TITLEBAR_WIDTH + 2, 1);
    setFixedWidth(m_contentWidth);
    m_contentWidth = this->width() - m_leftContentWidth;

    if (strWidth > m_contentWidth || strWidth > FILENAME_MAX_LENGTH) {
        name = fm.elidedText(filename, Qt::ElideMiddle,
                             std::min(m_contentWidth - 32, FILENAME_MAX_LENGTH));
        strWidth = fm.boundingRect(name).width();
        leftMargin =
            std::max(0, (m_windowWidth - strWidth) / 2 - m_leftContentWidth - LEFT_MARGIN - 2);
    } else {
        leftMargin = std::max(0, (m_windowWidth - strWidth) / 2 - m_leftContentWidth - 6);
        name = filename;
    }

    m_fileNameLabel->setText(name, leftMargin);
}

bool TTBContent::delPictureFromPath(QString strPath, DBImgInfoList infos, int nCurrent)
{
    bool bRet = true;
    ImageItem *test = m_imgList->findChild<ImageItem *>(strPath);
    if (!test) {
        bRet = false;
        return bRet;
    }

    //qDebug() << "被删除的图片路径：" << test->objectName();
    m_imglayout->removeWidget(test);
    test->setParent(nullptr);
    test->deleteLater();

    m_imgInfos = infos;
    m_nowIndex = nCurrent;
    //删除当前图片之后将当前图片和之后的所有图片index减一
    int i = 0;
    foreach (DBImgInfo var, m_imgInfos) {
        ImageItem *test = m_imgList->findChild<ImageItem *>(var.filePath);
        if (test) {
            test->setIndex(i);
        }

        i++;
    }

//    QList<ImageItem *> labelList = m_imgList->findChildren<ImageItem *>();
//    if (nCurrent != 0) {
//        for (int i = nCurrent; i < labelList.size(); i++) {
//            labelList.at(i)->setIndex(i);
//        }
//    } else {
//        for (int i = 0; i < labelList.size(); i++) {
//            labelList.at(i)->setIndex(i);
//        }
//    }
//    //将所有的NowIndex重置
//    for (int i = 0; i < labelList.size(); i++) {
//        labelList.at(i)->setIndexNow(nCurrent);
//    }

    m_imgList->adjustSize();
    onResize();

    return bRet;
}

void TTBContent::setIsConnectDel(bool bFlags)
{
    if (bFlags) {
        connect(m_trashBtn, &DIconButton::clicked, this, &TTBContent::removed, Qt::UniqueConnection);
    } else {
        m_trashBtn->disconnect();
    }
}

void TTBContent::disableDelAct(bool bFlags)
{
    //对于只读权限的图片，切换图片后等待5秒左右，删除按钮会激活并删除只读图片
    QFileInfo fileinfo(m_strCurImagePath);
    if(fileinfo.isReadable() && fileinfo.isWritable() && bFlags)
        m_trashBtn->setEnabled(bFlags);
}

void TTBContent::recvLoadAddInfos(DBImgInfoList newInfos, bool bFlags)
{
    if (bFlags) {
        loadFront(newInfos);
    } else {
        loadBack(newInfos);
    }
}

bool TTBContent::judgeReloadItem(const DBImgInfoList &inputInfos, DBImgInfoList &localInfos)
{
    bool bRet = false;
    if (inputInfos.size() != localInfos.size()) {
        localInfos.clear();
        localInfos = inputInfos;

        QLayoutItem *child;
        while ((child = m_imglayout->takeAt(0)) != nullptr) {
            m_imglayout->removeWidget(child->widget());
            child->widget()->setParent(nullptr);
            delete child;
        }

        bRet = true;
    }

    return bRet;
}

void TTBContent::reloadItems(DBImgInfoList &inputInfos, QString strCurPath)
{
    int i = 0;
    int num = 32;
    int t = 0;
    QList<ImageItem *> labelList = m_imgList->findChildren<ImageItem *>();

    for (DBImgInfo info : inputInfos) {
        if (labelList.size() != inputInfos.size()) {
            //char *imageType = getImageType(info.filePath);
            ImageItem *imageItem = new ImageItem(i, info.filePath, nullptr);
            imageItem->setFixedSize(QSize(32, 40));
            imageItem->resize(QSize(32, 40));
            imageItem->installEventFilter(m_imgListView);
            imageItem->setObjectName(info.filePath);
            /*LMH0727*/
           // imageItem->setObjectName(QString("%1").arg(m_totalImageItem++));
            m_imglayout->addWidget(imageItem);
            connect(imageItem, &ImageItem::imageMoveclicked, this,
            [ = ](QString path) {
                m_bMoving=false;
                if(bMove){
                    m_lastIndex=m_nowIndex;
                    QList <ImageItem *>labelList=m_imgList->findChildren<ImageItem *>(path);
                    if(labelList.size()>0){
                        ImageItem *img =labelList.at(0);
                        if(nullptr !=img){
                            img->setIndexNow(m_nowIndex);
                            labelList.at(0)->setFixedSize(QSize(58,58));
                            labelList.at(0)->resize(QSize(58,58));
                            QPixmap pix=img->getPixmap();
                            m_nowIndex=img->getIndex();
                            emit showvaguepixmap(pix,labelList.at(0)->getPath());
                        }
                    }
                    if(m_lastIndex >-1){
                        QList <ImageItem *>lastlabelList=m_imgList->findChildren<ImageItem *>(/*QString("%1").arg(m_lastIndex)*/);
                        if(lastlabelList.isEmpty()){
                        }
                        else {
                            for(int i=0;i<lastlabelList.size();i++){
                                if(lastlabelList.at(i)->getIndex()!=m_nowIndex){
                                    lastlabelList.at(i)->setFixedSize(QSize(32,40));
                                    lastlabelList.at(i)->resize(QSize(32,40));
                                    lastlabelList.at(i)->setIndexNow(m_nowIndex);
                                }
                            }
                        }

                    }

                }
            });
            connect(imageItem, &ImageItem::imageItemclicked, this,
            [ = ](int index, int indexNow,bool iRet) {
                m_bMoving=true;

                m_nowIndex = index;
//                emit imageClicked(index, (index - indexNow));
                emit imageMoveEnded(index, (index - indexNow),iRet);
                m_lastIndex=m_nowIndex;
                onResize();
                m_imgList->adjustSize();
            });

        }
        if (strCurPath == info.filePath) {
            t = i;
        }
        i++;
    }

    labelList = m_imgList->findChildren<ImageItem *>();
    m_nowIndex = t;

    int a = (qCeil(m_imgListView->width() - 26) / 32) / 2;
    int b = m_imgInfos.size() - (qFloor(m_imgListView->width() - 26) / 32) / 2;
    if (m_nowIndex > a && m_nowIndex < b) {
        m_startAnimation = 1;
    } else if (m_nowIndex < m_imgInfos.size() - 2 * a && m_nowIndex > -1) {
        m_startAnimation = 2;
    } else if (m_nowIndex > 2 * a - 1 && m_nowIndex < m_imgInfos.size()) {
        m_startAnimation = 3;
    } else {
        m_startAnimation = 0;
    }

    qDebug() << "m_startAnimation=" << m_startAnimation;
//    ImageItem *curitem = m_imgList->findChild<ImageItem *>(strCurPath);

    for (int j = 0; j < labelList.size(); j++) {
        labelList.at(j)->setFixedSize(QSize(num, 40));
        labelList.at(j)->resize(QSize(num, 40));
        labelList.at(j)->setIndexNow(t);
    }
    //还原7b23fc部分代码
    if(labelList.size()>0)
    {
        for (int k = 0;k<labelList.size();k++) {
            if(strCurPath == labelList.at(k)->getPath()){
                labelList.at(k)->setFixedSize(QSize(58, 58));
                labelList.at(k)->resize(QSize(58, 58));
                QPixmap imgpix = dApp->m_imagemap.value(strCurPath);
                if(!imgpix.isNull())
                    labelList.at(k)->updatePic(imgpix);
            }
        }
    }
//    if (curitem) {
//        curitem->setFixedSize(QSize(58, 58));
//        curitem->resize(QSize(58, 58));
//        //LMH0603，造成死锁的情况了，暂时屏蔽掉该锁，导致点击一张svg图片会卡死
//        //dApp->getRwLock().lockForRead();
//        curitem->updatePic(dApp->m_imagemap.value(strCurPath));
//        //dApp->getRwLock().unlock();
//    }

    m_imgListView->show();
}

void TTBContent::showAnimation()
{
    int num = 32;
    int a = (qCeil(m_imgListView->width() - 26) / 32) / 2;
    int b = m_imgInfos.size() - (qFloor(m_imgListView->width() - 26) / 32) / 2;
    if (m_nowIndex > a && m_nowIndex < b) {
        m_startAnimation = 1;
    } else if (m_nowIndex < m_imgInfos.size() - 2 * a && m_nowIndex > -1) {
        m_startAnimation = 2;
    } else if (m_nowIndex > 2 * a - 1 && m_nowIndex < m_imgInfos.size()) {
        m_startAnimation = 3;
    } else {
        m_startAnimation = 0;
    }

    if (1 == m_startAnimation) {
        if (bresized) {
            bresized = false;
            m_imgList->move(
                QPoint((qMin((TOOLBAR_MINIMUN_WIDTH +
                              THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)),
                             (qMax(width() - RT_SPACING, TOOLBAR_MINIMUN_WIDTH))) -
                        496 - 52 + 18) /
                       2 -
                       ((num)*m_nowIndex),
                       0));
        } else {
            QPropertyAnimation *animation = new QPropertyAnimation(m_imgList, "pos");
            animation->setDuration(500);
            animation->setEasingCurve(QEasingCurve::NCurveTypes);
            animation->setStartValue(m_imgList->pos());
            animation->setKeyValueAt(
                1, QPoint((qMin((TOOLBAR_MINIMUN_WIDTH +
                                 THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)),
                                (qMax(width() - RT_SPACING, TOOLBAR_MINIMUN_WIDTH))) -
                           496 - 52 + 18) /
                          2 -
                          ((num)*m_nowIndex),
                          0));
            animation->setEndValue(
                QPoint((qMin((TOOLBAR_MINIMUN_WIDTH +
                              THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)),
                             (qMax(width() - RT_SPACING, TOOLBAR_MINIMUN_WIDTH))) -
                        496 - 52 + 18) /
                       2 -
                       ((num)*m_nowIndex),
                       0));
            animation->start(QAbstractAnimation::DeleteWhenStopped);
            connect(animation, &QPropertyAnimation::finished, animation,
                    &QPropertyAnimation::deleteLater);

            connect(animation, &QPropertyAnimation::finished, this,
                    [ = ] { m_imgList->show(); });
        }
    } else if (2 == m_startAnimation) {
        if (bresized) {
            bresized = false;
            m_imgList->move(QPoint(0, 0));
        } else {
            QPropertyAnimation *animation = new QPropertyAnimation(m_imgList, "pos");
            animation->setDuration(500);
            animation->setEasingCurve(QEasingCurve::NCurveTypes);
            animation->setStartValue(m_imgList->pos());
            animation->setKeyValueAt(1, QPoint(0, 0));
            animation->setEndValue(QPoint(0, 0));
            animation->start(QAbstractAnimation::DeleteWhenStopped);
            connect(animation, &QPropertyAnimation::finished, animation,
                    &QPropertyAnimation::deleteLater);

            connect(animation, &QPropertyAnimation::finished, this,
                    [ = ] { m_imgList->show(); });
        }
    } else if (3 == m_startAnimation) {
        if (bresized) {
            bresized = false;
            m_imgList->move(QPoint(m_imgListView->width() - m_imgList->width() + 5, 0));
        } else {
            QPropertyAnimation *animation = new QPropertyAnimation(m_imgList, "pos");
            animation->setDuration(500);
            animation->setEasingCurve(QEasingCurve::NCurveTypes);
            animation->setStartValue(m_imgList->pos());
            animation->setKeyValueAt(1, QPoint(0, 0));
            animation->setEndValue(
                QPoint(m_imgListView->width() - m_imgList->width() + 5, 0));
            animation->start(QAbstractAnimation::DeleteWhenStopped);
            connect(animation, &QPropertyAnimation::finished, animation,
                    &QPropertyAnimation::deleteLater);

            connect(animation, &QPropertyAnimation::finished, this,
                    [ = ] { m_imgList->show(); });
        }
    } else if (0 == m_startAnimation) {
        m_imgList->show();
    }

    m_imgListView->update();
    m_imgList->update();
}

void TTBContent::setBtnAttribute(const QString strPath)
{
    //lmh0826 bug44815
    if(!m_bMoving){
        m_rotateLBtn->setEnabled(false);
        m_rotateRBtn->setEnabled(false);
        m_trashBtn->setEnabled(false);
        m_adaptImageBtn->setEnabled(false);
        m_adaptScreenBtn->setEnabled(false);
        return;
    }
    if(!utils::image::imageSupportRead(strPath)){
        m_rotateLBtn->setEnabled(false);
        m_rotateRBtn->setEnabled(false);
        m_trashBtn->setEnabled(false);
        m_adaptImageBtn->setEnabled(false);
        m_adaptScreenBtn->setEnabled(false);
        return;
    }
    //判断是否加载完成，未完成将旋转按钮禁用
    if (m_bIsHide) {
        m_rotateLBtn->setEnabled(false);
        m_rotateRBtn->setEnabled(false);
        m_trashBtn->setEnabled(false);
    } else {
        m_rotateLBtn->setEnabled(true);
        m_rotateRBtn->setEnabled(true);
        m_trashBtn->setEnabled(true);
    }

    if ((QFileInfo(strPath).isReadable() && !QFileInfo(strPath).isWritable()) || (QFileInfo(strPath).suffix() == "gif")) {
        //gif图片可以删除
        if ((QFileInfo(strPath).suffix() == "gif")) {
            m_trashBtn->setDisabled(false);
        } else {
            m_trashBtn->setDisabled(true);

        }

        m_rotateLBtn->setDisabled(true);
        m_rotateRBtn->setDisabled(true);
    } else {
        m_trashBtn->setDisabled(false);
        if (utils::image::imageSupportSave(strPath)) {
            m_rotateLBtn->setDisabled(false);
            m_rotateRBtn->setDisabled(false);
        } else {
            if ((QFileInfo(strPath).suffix() == "svg")) {
                m_rotateLBtn->setDisabled(false);
                m_rotateRBtn->setDisabled(false);
            } else {
                m_rotateLBtn->setDisabled(true);
                m_rotateRBtn->setDisabled(true);
            }
        }
    }

}

void TTBContent::clickLoad(const int nCurrent)
{
    if (nCurrent == m_imgInfos.size() - 1) {
        dApp->signalM->sendLoadSignal(false);
    }
}

void TTBContent::OnUpdateThumbnail(QString path)
{
    ImageItem* item = m_imgList->findChild<ImageItem*>(path);
    if(!item) return;
    QPixmap imgpix = dApp->m_imagemap.value(path);
    if(!imgpix.isNull())
        item->updatePic(imgpix);
}

void TTBContent::DisEnablettbButton()
{
    m_rotateLBtn->setEnabled(false);
    m_rotateRBtn->setEnabled(false);
    m_adaptImageBtn->setEnabled(false);
    m_adaptScreenBtn->setEnabled(false);
    m_NotImageViewFlag = true;
}

void TTBContent::onChangeHideFlags(bool bFlags)
{
    m_bIsHide = bFlags;
    //heyi test 判断是否是第一次打开然后隐藏上一张和下一张按钮
    if (bFlags || m_imgInfos.size() <= 1) {
        m_preButton->hide();
        m_preButton_spc->hide();
        m_nextButton->hide();
        m_nextButton_spc->hide();
        m_preButton->setDisabled(true);
        m_preButton_spc->setDisabled(true);
        m_nextButton->setDisabled(true);
        m_nextButton_spc->setDisabled(true);
    } else {
        m_preButton->show();
        m_nextButton->show();
        m_preButton_spc->show();
        m_nextButton_spc->show();
        m_preButton->setDisabled(false);
        m_preButton_spc->setDisabled(false);
        m_nextButton->setDisabled(false);
        m_nextButton_spc->setDisabled(false);
    }

    //判断是否加载完成，未完成将旋转和删除按钮禁用

//    if (bFlags) {
//        m_rotateLBtn->setEnabled(false);
//        m_rotateRBtn->setEnabled(false);
//        m_trashBtn->setEnabled(false);
//    } else {
//        m_rotateLBtn->setEnabled(true);
//        m_rotateRBtn->setEnabled(true);
//        m_trashBtn->setEnabled(true);
//    }

    if ((QFileInfo(m_imagePath).isReadable() && !QFileInfo(m_imagePath).isWritable()) || (QFileInfo(m_imagePath).suffix() == "gif")) {
        //gif图片可以删除
        if ((QFileInfo(m_imagePath).suffix() == "gif")) {
            m_trashBtn->setDisabled(false);
        } else {
            m_trashBtn->setDisabled(true);

        }

        m_rotateLBtn->setDisabled(true);
        m_rotateRBtn->setDisabled(true);
    } else {
        m_trashBtn->setDisabled(false);
        if (utils::image::imageSupportSave(m_imagePath)) {
            m_rotateLBtn->setDisabled(false);
            m_rotateRBtn->setDisabled(false);
        } else {
            if (QFileInfo(m_imagePath).suffix() == "svg") {
                m_rotateLBtn->setDisabled(false);
                m_rotateRBtn->setDisabled(false);
            } else {
                m_rotateLBtn->setDisabled(true);
                m_rotateRBtn->setDisabled(true);
            }
        }
    }
}

void TTBContent::loadBack(DBImgInfoList infos)
{
    if (infos.size() > 0) {
        onHidePreNextBtn(true, false);
    }
    //先将先前的图元index改变为最新的
    int nAddSize = m_imgInfos.size();

    //遍历infos动态生成图元，并将图元加入布局
    foreach (DBImgInfo info, infos) {
        char *imageType = getImageType(info.filePath);
        ImageItem *imageItem = new ImageItem(nAddSize, info.filePath, imageType);
        imageItem->setFixedSize(QSize(32, 40));
        imageItem->resize(QSize(32, 40));
        imageItem->installEventFilter(m_imgListView);
        imageItem->setObjectName(info.filePath);
        imageItem->setIndexNow(m_nowIndex);
        /*LMH0727*/
        //imageItem->setObjectName(QString("%1").arg(m_totalImageItem++));
        m_imglayout->addWidget(imageItem);
        connect(imageItem, &ImageItem::imageMoveclicked, this,
        [ = ](QString path) {
            m_bMoving=false;
            if(bMove){
                m_lastIndex=m_nowIndex;
                QList <ImageItem *>labelList=m_imgList->findChildren<ImageItem *>(path);
                if(labelList.size()>0){
                    ImageItem *img =labelList.at(0);
                    if(nullptr !=img){
                        img->setIndexNow(m_nowIndex);
                        labelList.at(0)->setFixedSize(QSize(58,58));
                        labelList.at(0)->resize(QSize(58,58));
                        QPixmap pix=img->getPixmap();
                        m_nowIndex=img->getIndex();
                        emit showvaguepixmap(pix,labelList.at(0)->getPath());
                    }
                }
                if(m_lastIndex >-1){
                    QList <ImageItem *>lastlabelList=m_imgList->findChildren<ImageItem *>(/*QString("%1").arg(m_lastIndex)*/);
                    if(lastlabelList.isEmpty()){
                    }
                    else {
                        for(int i=0;i<lastlabelList.size();i++){
                            if(lastlabelList.at(i)->getIndex()!=m_nowIndex){
                                lastlabelList.at(i)->setFixedSize(QSize(32,40));
                                lastlabelList.at(i)->resize(QSize(32,40));
                                lastlabelList.at(i)->setIndexNow(m_nowIndex);
                            }
                        }
                    }

                }

            }
        });
        connect(imageItem, &ImageItem::imageItemclicked, this,
        [ = ](int index, int indexNow,bool iRet) {
            m_bMoving=true;
            m_nowIndex = index;
//                emit imageClicked(index, (index - indexNow));
            emit imageMoveEnded(index, (index - indexNow),iRet);
            m_lastIndex=m_nowIndex;
            onResize();
            m_imgList->adjustSize();
        });

        m_imgInfos.push_back(info);
        m_imgList->adjustSize();
        nAddSize++;
    }

    //将所有图元当前的nowindex重置

    onResize();
    if (m_imgInfos.size() > 3) {
        m_imgList->setFixedSize((m_imgInfos.size() + 1) * THUMBNAIL_WIDTH, TOOLBAR_HEIGHT);
        m_imgList->resize((m_imgInfos.size() + 1) * THUMBNAIL_WIDTH + THUMBNAIL_LIST_ADJUST,
                          TOOLBAR_HEIGHT);

        m_imgList->setContentsMargins(0, 0, 0, 0);

        m_imgList->show();
        m_imgListView->show();

        m_imgList->update();
        m_imgListView->update();
        m_imgList->move(m_nLastMove, m_imgList->y());
    }
}

void TTBContent::loadFront(DBImgInfoList infos)
{
    if (infos.size() > 0) {
        onHidePreNextBtn(true, false);
    }
    //先将先前的图元index改变为最新的
    int nAddSize = infos.size();
    int nCurrentIndex = 0;
    m_nowIndex = m_nowIndex + nAddSize;
    //重置图元当前信息
    QList<ImageItem *> labelList = m_imgList->findChildren<ImageItem *>();
    for (int i = 0; i < labelList.size(); i++) {
        nCurrentIndex = labelList.at(i)->getIndex();
        labelList.at(i)->setIndex(nCurrentIndex + nAddSize);
        labelList.at(i)->setIndexNow(m_nowIndex);
    }

    //将新插入的图元赋值index
    nAddSize--;
    //遍历infos动态生成图元，并将图元加入布局
    foreach (DBImgInfo info, infos) {
        char *imageType = getImageType(info.filePath);
        ImageItem *imageItem = new ImageItem(nAddSize, info.filePath, imageType);
        imageItem->setFixedSize(QSize(32, 40));
        imageItem->resize(QSize(32, 40));
        imageItem->installEventFilter(m_imgListView);
        imageItem->setObjectName(info.filePath);
        imageItem->setIndexNow(m_nowIndex);
        /*LMH0727*/
        //imageItem->setObjectName(QString("%1").arg(m_totalImageItem++));
        m_imglayout->insertWidget(0, imageItem);
        connect(imageItem, &ImageItem::imageMoveclicked, this,
        [ = ](QString path) {
            m_bMoving=false;
            if(bMove){
                m_lastIndex=m_nowIndex;
                QList <ImageItem *>labelList=m_imgList->findChildren<ImageItem *>(path);
                if(labelList.size()>0){
                    ImageItem *img =labelList.at(0);
                    if(nullptr !=img){
                        img->setIndexNow(m_nowIndex);
                        labelList.at(0)->setFixedSize(QSize(58,58));
                        labelList.at(0)->resize(QSize(58,58));
                        QPixmap pix=img->getPixmap();
                        m_nowIndex=img->getIndex();
                        emit showvaguepixmap(pix,labelList.at(0)->getPath());
                    }
                }
                if(m_lastIndex >-1){
                    QList <ImageItem *>lastlabelList=m_imgList->findChildren<ImageItem *>(/*QString("%1").arg(m_lastIndex)*/);
                    if(lastlabelList.isEmpty()){
                    }
                    else {
                        for(int i=0;i<lastlabelList.size();i++){
                            if(lastlabelList.at(i)->getIndex()!=m_nowIndex){
                                lastlabelList.at(i)->setFixedSize(QSize(32,40));
                                lastlabelList.at(i)->resize(QSize(32,40));
                                lastlabelList.at(i)->setIndexNow(m_nowIndex);
                            }
                        }
                    }

                }

            }
        });
        connect(imageItem, &ImageItem::imageItemclicked, this,
        [ = ](int index, int indexNow,bool iRet) {
            m_bMoving=true;
            m_nowIndex = index;
//                emit imageClicked(index, (index - indexNow));
            emit imageMoveEnded(index, (index - indexNow),iRet);
            m_lastIndex=m_nowIndex;
            onResize();
            m_imgList->adjustSize();
        });

        m_imgInfos.push_front(info);
        m_imgList->adjustSize();
        nAddSize--;
    }

    labelList = m_imgList->findChildren<ImageItem *>();

    onResize();
    m_imgList->adjustSize();
    if (m_imgInfos.size() > 3) {
        m_imgList->setFixedSize((m_imgInfos.size() + 1) * THUMBNAIL_WIDTH, TOOLBAR_HEIGHT);
        m_imgList->resize((m_imgInfos.size() + 1) * THUMBNAIL_WIDTH + THUMBNAIL_LIST_ADJUST,
                          TOOLBAR_HEIGHT);

        m_imgList->setContentsMargins(0, 0, 0, 0);

        m_imgList->show();
        m_imgListView->show();

        m_imgList->update();
        m_imgListView->update();
        m_imgList->move(-34 * infos.size() + 100, m_imgList->y());
    }
}

void TTBContent::ReInitFirstthumbnails(DBImgInfoList infos)
{
    //clear thumbnails widget
    m_imgInfos.clear();
    m_nowIndex = 0;
    QLayoutItem *child;
    while ((child = m_imglayout->takeAt(0)) != nullptr) {
        //setParent为NULL，防止删除之后界面不消失
        if (child->widget()) {
            child->widget()->setParent(nullptr);
        }
        delete child;
    }
    loadBack(infos);
}

void TTBContent::receveAllIamgeInfos(DBImgInfoList AllImgInfos)
{
    m_AllImgInfos = AllImgInfos;
    //接收到所有信息之后生成所有的图元
    m_strFirstPath = m_imgInfos.at(0).filePath;
    m_strEndPsth = m_imgInfos.last().filePath;
    m_strCurImagePath = m_imgInfos.at(m_nowIndex).filePath;
}

void TTBContent::onHidePreNextBtn(bool bShowAll, bool bFlag)
{
    if (!bShowAll) {
        if (bFlag) {
            m_nextButton->setEnabled(false);
            m_preButton->setEnabled(true);
        } else {
            m_nextButton->setEnabled(true);
            m_preButton->setEnabled(false);
        }
    } else {
        m_nextButton->setEnabled(true);
        m_preButton->setEnabled(true);
    }
}

void TTBContent::onThemeChanged(ViewerThemeManager::AppTheme theme)
{
    Q_UNUSED(theme);
}

void TTBContent::setCurrentDir(QString text)
{
    if (text == FAVORITES_ALBUM) {
        text = tr("My favorite");
    }

#ifndef LITE_DIV
    m_returnBtn->setText(text);
#endif
}

void TTBContent::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_windowWidth = this->window()->geometry().width();
    if (m_imgInfos_size <= 1) {
        m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
    } else if (m_imgInfos_size <= 3) {
        m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
        m_imgListView->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
    } else {
        m_contentWidth = qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos_size - 3)),
                              qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) +
                         THUMBNAIL_LIST_ADJUST;
        m_imgListView->setFixedSize(
            QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos_size - 3)),
                       qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) -
                  THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST,
                  TOOLBAR_HEIGHT));
    }

    setFixedWidth(m_contentWidth);
    //    setImage(m_imagePath, m_imgInfos);

    QList<ImageItem *> labelList = m_imgList->findChildren<ImageItem *>();
    for (int j = 0; j < labelList.size(); j++) {
        labelList.at(j)->setFixedSize(QSize(32, 40));
        labelList.at(j)->resize(QSize(32, 40));
        labelList.at(j)->setContentsMargins(1, 5, 1, 5);
        labelList.at(j)->setIndexNow(m_nowIndex);
    }
    if (labelList.size() > 0) {
        labelList.at(m_nowIndex)->setFixedSize(QSize(58, 58));
        labelList.at(m_nowIndex)->resize(QSize(58, 58));
        labelList.at(m_nowIndex)->setContentsMargins(0, 0, 0, 0);
    }

    //图动窗口的时候重置缩略图布局
    //emit m_imgListView->mouseLeftReleased();
}


void TTBContent::setImage(const QString path, DBImgInfoList infos)
{
    if (!infos.isEmpty() && !QFileInfo(path).exists()) {
        if (infos.size() == 1)
            return;
        emit dApp->signalM->picNotExists(true);
        qDebug() << "QFileInfo(path) is not exists.Path:" << path;
    } else {
        emit dApp->signalM->picNotExists(false);
    }

    QList<ImageItem *> labelList = m_imgList->findChildren<ImageItem *>();
    qDebug() << "时间1";
    //判断当前缩略图个数是否等于传入的缩略图数量，不想等全部删除重新生成新的
    judgeReloadItem(infos, m_imgInfos);
    qDebug() << "judgeReloadItem完成";

    if (path.isEmpty() || !QFileInfo(path).exists()) {
        reloadItems(m_imgInfos, path);
        qDebug() << "reloadItems完成";
        if (!bMove) {
            showAnimation();
        }
        qDebug() << "reloadItems完成";

        if (m_nowIndex == 0) {
            m_preButton->setDisabled(true);
        } else {
            m_preButton->setDisabled(false);
        }
        if (m_nowIndex == m_imgInfos.size() - 1) {
            m_nextButton->setDisabled(true);
        } else {
            m_nextButton->setDisabled(false);
        }

        m_adaptImageBtn->setDisabled(true);
        m_adaptScreenBtn->setDisabled(true);
        m_rotateLBtn->setDisabled(true);
        m_rotateRBtn->setDisabled(true);
        m_trashBtn->setDisabled(true);

        m_imgList->setDisabled(false);

    } else {
#if TTB
        m_adaptImageBtn->setDisabled(false);
        m_adaptScreenBtn->setDisabled(false);
#else
        //        btAdapt->setDisabled(false);
        //        btFit->setDisabled(false);
        m_adaptImageBtn->setDisabled(false);
        m_adaptScreenBtn->setDisabled(false);
#endif

        if (m_imgInfos.size() > 3) {
            m_imgList->setFixedSize((m_imgInfos.size() + 1) * THUMBNAIL_WIDTH, TOOLBAR_HEIGHT);
            m_imgList->resize((m_imgInfos.size() + 1) * THUMBNAIL_WIDTH + THUMBNAIL_LIST_ADJUST,
                              TOOLBAR_HEIGHT);

            m_imgList->setContentsMargins(0, 0, 0, 0);
            qDebug() << "reloadItems开始";
            reloadItems(m_imgInfos, path);
            qDebug() << "reloadItems完成";
            if (!bMove) {
                showAnimation();
            }
            qDebug() << "showAnimation完成";

#if TTB
            m_preButton->show();
#else
            //            btPre->show();
            m_preButton->show();
#endif
            m_preButton_spc->show();
#if TTB
            m_nextButton->show();
#else
            //            btNext->show();
            m_nextButton->show();
#endif
            m_nextButton_spc->show();
            m_imgListView_prespc->show();
            m_imgListView_spc->show();

#if TTB
            if (m_nowIndex == 0) {
                m_preButton->setDisabled(true);
            } else {
                m_preButton->setDisabled(false);
            }
            if (m_nowIndex == labelList.size() - 1) {
                m_nextButton->setDisabled(true);
            } else {
                m_nextButton->setDisabled(false);
            }
#else
            if (m_nowIndex == 0) {
                m_preButton->setDisabled(true);
            } else {
                m_preButton->setDisabled(false);
            }
            if (m_nowIndex == m_imgInfos.size() - 1) {
                m_nextButton->setDisabled(true);
            } else {
                m_nextButton->setDisabled(false);
            }
#endif
        } else if (m_imgInfos.size() > 1) {
            m_imgList->setFixedSize((m_imgInfos.size() + 1) * THUMBNAIL_WIDTH, TOOLBAR_HEIGHT);
            m_imgList->resize((m_imgInfos.size() + 1) * THUMBNAIL_WIDTH, TOOLBAR_HEIGHT);

            m_imgList->setContentsMargins(0, 0, 0, 0);
            qDebug() << "reloadItems开始";
            reloadItems(m_imgInfos, path);
            qDebug() << "reloadItems完成";

            m_imgListView->show();
            m_imgList->show();

            m_imgListView->update();
            m_imgList->update();
#if TTB
            m_preButton->show();
#else
            //            btPre->show();
            m_preButton->show();
#endif
            m_preButton_spc->show();
#if TTB
            m_nextButton->show();
#else
            //            btNext->show();
            m_nextButton->show();
#endif
            m_nextButton_spc->show();
            m_imgListView_prespc->show();
            m_imgListView_spc->show();

#if TTB
            if (m_nowIndex == 0) {
                m_preButton->setDisabled(true);
            } else {
                m_preButton->setDisabled(false);
            }
            if (m_nowIndex == labelList.size() - 1) {
                m_nextButton->setDisabled(true);
            } else {
                m_nextButton->setDisabled(false);
            }
#else
            if (m_nowIndex == 0) {
                m_preButton->setDisabled(true);
            } else {
                m_preButton->setDisabled(false);
            }
            if (m_nowIndex == m_imgInfos.size() - 1) {
                m_nextButton->setDisabled(true);
            } else {
                m_nextButton->setDisabled(false);
            }
#endif
        } else {
            m_imgList->hide();
            m_imgListView->hide();
            m_imgListView_prespc->hide();
            m_imgListView_spc->hide();
#if TTB
            m_preButton->hide();
#else
            //            btPre->hide();
            m_preButton->hide();
#endif
            m_preButton_spc->hide();
#if TTB
            m_nextButton->hide();
#else
            //            btNext->hide();
            m_nextButton->hide();
#endif
            m_nextButton_spc->hide();
            m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
            setFixedWidth(m_contentWidth);
        }

#if TTB
        if (QFileInfo(path).isReadable() && !QFileInfo(path).isWritable()) {
            m_trashBtn->setDisabled(true);
            m_rotateLBtn->setDisabled(true);
            m_rotateRBtn->setDisabled(true);
        } else {
            m_trashBtn->setDisabled(false);
            if (utils::image::imageSupportSave(path)) {
                m_rotateLBtn->setDisabled(false);
                m_rotateRBtn->setDisabled(false);
            } else {
                m_rotateLBtn->setDisabled(true);
                m_rotateRBtn->setDisabled(true);
            }
        }
#else
        qDebug() << "setBtnAttribute开始";
        setBtnAttribute(path);
        qDebug() << "setBtnAttribute完成";

#endif
    }
    if(!m_NotImageViewFlag)
    {
        m_adaptImageBtn->setDisabled(true);
        m_adaptScreenBtn->setDisabled(true);
        m_rotateLBtn->setDisabled(true);
        m_rotateRBtn->setDisabled(true);
    }
    m_NotImageViewFlag = true;
    //heyi test 判断是否是第一次打开然后隐藏上一张和下一张按钮
    if (m_bIsHide || m_imgInfos.size() <= 1) {
        m_preButton->hide();
        m_preButton_spc->hide();
        m_nextButton->hide();
        m_nextButton_spc->hide();
    } else {
        m_preButton->show();
        m_nextButton->show();
        m_preButton_spc->show();
        m_nextButton_spc->show();
    }

    m_imagePath = path;
    QString fileName = "";
    if (m_imagePath != "") {
        qDebug() << "1570";
        fileName = QFileInfo(m_imagePath).fileName();
    }
    emit dApp->signalM->updateFileName(fileName);

    m_imgList->adjustSize();
    m_strCurImagePath = path;
    qDebug() << "时间2";
    clickLoad(m_nowIndex);
    /*lmh0728,给m_lastIndex第一次赋值*/
    if(m_nowIndex!=0 &&m_lastIndex==0 ){
        m_lastIndex=m_nowIndex;
    }
}

void TTBContent::updateCollectButton()
{
    //    if (! m_clBT)
    //        return;

    //    if (m_imagePath.isEmpty() || !QFileInfo(m_imagePath).exists()) {
    //        m_clBT->setDisabled(true);
    //        m_clBT->setChecked(false);
    //    }
    //    else
    //        m_clBT->setDisabled(false);

    //    if (! m_clBT->isEnabled()) {
    //        m_clBT->setDisabled(true);
    //    }
    //#ifndef LITE_DIV
    //    else if (DBManager::instance()->isImgExistInAlbum(FAVORITES_ALBUM,
    //                                                      m_imagePath)) {
    //        m_clBT->setToolTip(tr("Unfavorite"));
    //        m_clBT->setChecked(true);
    //    }
    //#endif
    //    else {
    //        m_clBT->setToolTip(tr("Favorite"));
    //        m_clBT->setChecked(false);
    //        m_clBT->setDisabled(false);
    //    }
}

void TTBContent::onResize()
{
    if (m_imgInfos.size() <= 1) {
        m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
    } else if (m_imgInfos.size() <= 3) {
        m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
    } else {
        m_contentWidth =
            qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)),
                 qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) +
            THUMBNAIL_LIST_ADJUST;
    }

    setFixedWidth(m_contentWidth);
    setFixedHeight(70);
    m_windowWidth =  this->window()->geometry().width();
    if (m_imgInfos.size() <= 1) {
        m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
    } else if (m_imgInfos.size() <= 3) {
        m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
        m_imgListView->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
    } else {
        m_contentWidth = qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos_size - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) + THUMBNAIL_LIST_ADJUST;
        m_imgListView->setFixedSize(QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos_size - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) - THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST, TOOLBAR_HEIGHT));
    }

    setFixedWidth(m_contentWidth);
    int a = (qCeil(m_imgListView->width() - 26) / 32) / 2;
    int b = m_imgInfos.size() - (qFloor(m_imgListView->width() - 26) / 32) / 2;
    if (m_nowIndex > a && m_nowIndex < b) {
        m_startAnimation = 1;
    } else if (m_nowIndex < m_imgInfos.size() - 2 * a && m_nowIndex > -1) {
        m_startAnimation = 2;
    } else if (m_nowIndex > 2 * a - 1 && m_nowIndex < m_imgInfos.size()) {
        m_startAnimation = 3;
    } else {
        m_startAnimation = 0;
    }

    if (1 == m_startAnimation) {
        m_imgList->move(QPoint((qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)),
                                     (qMax(width() - RT_SPACING, TOOLBAR_MINIMUN_WIDTH))) - 496 - 228 + 18) / 2 - ((32)*m_nowIndex), 0));
    } else if (2 == m_startAnimation) {
        m_imgList->move(QPoint(0, 0));
    } else if (3 == m_startAnimation) {
        m_imgList->move(QPoint(m_imgListView->width() - m_imgList->width() + 5, 0));
    } else if (0 == m_startAnimation) {
        m_imgList->move(QPoint(0, 0));
    }
}
