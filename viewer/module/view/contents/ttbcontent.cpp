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

#include "widgets/pushbutton.h"
#include "widgets/returnbutton.h"
#include "controller/dbmanager.h"
#include "controller/configsetter.h"
#include "widgets/elidedlabel.h"
#include "controller/signalmanager.h"

#include <QTimer>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QDebug>
#include <QAbstractItemModel>
#include <DImageButton>
#include <DThumbnailProvider>
#include <DSpinner>
#include <QtMath>

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
const QString LOCMAP_SELECTED_DARK = ":/resources/dark/images/58 drak.svg";
const QString LOCMAP_NOT_SELECTED_DARK = ":/resources/dark/images/imagewithbg-dark.svg";
const QString LOCMAP_SELECTED_LIGHT = ":/resources/light/images/58.svg";
const QString LOCMAP_NOT_SELECTED_LIGHT = ":/resources/light/images/imagewithbg.svg";

const int TOOLBAR_MINIMUN_WIDTH = 610;
const int TOOLBAR_JUSTONE_WIDTH = 310;
const int RT_SPACING = 20;
const int TOOLBAR_HEIGHT = 60;

const int TOOLBAR_DVALUE = 114 + 8;

const int THUMBNAIL_WIDTH = 32;
const int THUMBNAIL_ADD_WIDTH = 32;
const int THUMBNAIL_LIST_ADJUST = 9;
const int THUMBNAIL_VIEW_DVALUE = 496;

const unsigned int IMAGE_TYPE_JEPG = 0xFFD8FF;
const unsigned int IMAGE_TYPE_JPG1 = 0xFFD8FFE0;
const unsigned int IMAGE_TYPE_JPG2 = 0xFFD8FFE1;
const unsigned int IMAGE_TYPE_JPG3 = 0xFFD8FFE8;
const unsigned int IMAGE_TYPE_PNG = 0x89504e47;
const unsigned int IMAGE_TYPE_GIF = 0x47494638;
const unsigned int IMAGE_TYPE_TIFF = 0x49492a00;
const unsigned int IMAGE_TYPE_BMP = 0x424d;
}  // namespace

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
        ret = "JEPG";
        break;
    case IMAGE_TYPE_PNG:
        //文件类型为 png
        ret = "PNG";
        break;
    case IMAGE_TYPE_GIF:
        //文件类型为 GIF
        ret = "GIF";
        break;
    case IMAGE_TYPE_TIFF:
        //文件类型为 TIFF
        ret = "TIFF";
        break;
    case IMAGE_TYPE_BMP:
        //文件类型为 BMP
        ret = "BMP";
        break;
    default:
        ret = nullptr;
        break;
    }
    return ret;
};

MyImageListWidget::MyImageListWidget(QWidget *parent)
    : DWidget(parent)
{
    setMouseTracking(true);
}

bool MyImageListWidget::ifMouseLeftPressed()
{
    return bmouseleftpressed;
}

void MyImageListWidget::setObj(QObject *obj)
{
    m_obj = obj;
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

    if (e->type() == QEvent::MouseButtonRelease) {
        bmouseleftpressed = false;
        emit mouseLeftReleased();
    }
    if (e->type() == QEvent::Leave && obj == m_obj) {
        bmouseleftpressed = false;
        emit mouseLeftReleased();
    }
    if (e->type() == QEvent::MouseMove && bmouseleftpressed) {
        QMouseEvent *mouseEvent = (QMouseEvent *)e;
        QPoint p = mouseEvent->globalPos();
        ((DWidget *)m_obj)->move(((DWidget *)m_obj)->x() + p.x() - m_prepoint.x(), ((DWidget *)m_obj)->y());
        m_prepoint = p;
    }
    return false;
}

ImageItem::ImageItem(int index, QString path, char *imageType, QWidget *parent)
{
    _index = index;
    _path = path;

    if (dApp->m_imagemap.contains(path)) {
        _pixmap = dApp->m_imagemap.value(path);
    }
    _image = new DLabel(this);
    connect(dApp, &Application::sigFinishLoad, this, [ = ] {
        if (dApp->m_imagemap.contains(_path))
        {
            _pixmap = dApp->m_imagemap.value(_path);
            update();
        }
    });
};
void ImageItem::paintEvent(QPaintEvent *event)
{

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

    QPainter painter(this);

    painter.setRenderHints(QPainter::HighQualityAntialiasing |
                           QPainter::SmoothPixmapTransform |
                           QPainter::Antialiasing);

    QRect backgroundRect = rect();
    QRect pixmapRect;

    if (_index == _indexNow) {
        QPainterPath backgroundBp;
        backgroundBp.addRoundedRect(backgroundRect, 8, 8);
        painter.setClipPath(backgroundBp);
        painter.fillRect(backgroundRect, QBrush(QColor("#2CA7F8")));

        if (_pixmap.width() > _pixmap.height()) {
            _pixmap = _pixmap.copy((_pixmap.width() - _pixmap.height()) / 2, 0, _pixmap.height(), _pixmap.height());
        } else if (_pixmap.width() < _pixmap.height()) {
            _pixmap = _pixmap.copy(0, (_pixmap.height() - _pixmap.width()) / 2, _pixmap.width(), _pixmap.width());
        }

        pixmapRect.setX(backgroundRect.x() + 4);
        pixmapRect.setY(backgroundRect.y() + 4);
        pixmapRect.setWidth(backgroundRect.width() - 8);
        pixmapRect.setHeight(backgroundRect.height() - 8);

        QColor cl;
        if (themeType == DGuiApplicationHelper::DarkType) {
            cl = QColor(Qt::white);
        } else {
            cl = QColor(Qt::black);
        }
        cl.setAlphaF(0.1);
        QPainterPath bg0;
        bg0.addRoundedRect(pixmapRect, 4, 4);
        painter.setClipPath(bg0);
        painter.fillRect(pixmapRect, QBrush(cl));


        if (themeType == DGuiApplicationHelper::DarkType) {
            m_pixmapstring = LOCMAP_SELECTED_DARK;
        } else {
            m_pixmapstring = LOCMAP_SELECTED_LIGHT;
        }

        QPixmap pixmap = utils::base::renderSVG(m_pixmapstring, QSize(60, 60));
        QPainterPath bg;
        bg.addRoundedRect(pixmapRect, 4, 4);
        if (_pixmap.isNull()) {
            painter.setClipPath(bg);
            painter.drawPixmap(pixmapRect, m_pixmapstring);
        }

    } else {
        pixmapRect.setX(backgroundRect.x() + 1);
        pixmapRect.setY(backgroundRect.y() + 0);
        pixmapRect.setWidth(backgroundRect.width() - 2);
        pixmapRect.setHeight(backgroundRect.height() - 0);

        QColor cl;
        if (themeType == DGuiApplicationHelper::DarkType) {
            cl = QColor(Qt::white);
        } else {
            cl = QColor(Qt::black);
        }
        cl.setAlphaF(0.1);
        QPainterPath bg0;
        bg0.addRoundedRect(pixmapRect, 4, 4);
        painter.setClipPath(bg0);
        painter.fillRect(pixmapRect, QBrush(cl));

        if (themeType == DGuiApplicationHelper::DarkType) {
            m_pixmapstring = LOCMAP_NOT_SELECTED_DARK;
        } else {
            m_pixmapstring = LOCMAP_NOT_SELECTED_LIGHT;
        }

        QPixmap pixmap = utils::base::renderSVG(m_pixmapstring, QSize(30, 40));
        QPainterPath bg;
        bg.addRoundedRect(pixmapRect, 4, 4);
        if (_pixmap.isNull()) {
            painter.setClipPath(bg);
            painter.drawPixmap(pixmapRect, m_pixmapstring);
        }
    }

    QPixmap blankPix = _pixmap;
    blankPix.fill(Qt::white);

    QRect whiteRect;
    whiteRect.setX(pixmapRect.x() + 1);
    whiteRect.setY(pixmapRect.y() + 1);
    whiteRect.setWidth(pixmapRect.width() - 2);
    whiteRect.setHeight(pixmapRect.height() - 2);

    QPainterPath bg1;
    bg1.addRoundedRect(whiteRect, 4, 4);
    painter.setClipPath(bg1);

    painter.drawPixmap(whiteRect, blankPix);
    painter.drawPixmap(whiteRect, _pixmap);

};
TTBContent::TTBContent(bool inDB,
                       DBImgInfoList m_infos,
                       QWidget *parent) : QLabel(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    m_windowWidth = std::max(this->window()->width(),
                             ConfigSetter::instance()->value("MAINWINDOW", "WindowWidth").toInt());
//    m_contentWidth = std::max(m_windowWidth - RIGHT_TITLEBAR_WIDTH, 1);
    m_imgInfos = m_infos;
    m_imgInfos_size = m_imgInfos.size();
    if ( m_imgInfos.size() <= 1 ) {
        m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
    } else if (m_imgInfos.size() <= 3 ) {
        m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
    } else {
        m_contentWidth = qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) + THUMBNAIL_LIST_ADJUST;
    }

//    qDebug()<<"init:m_contentWidth=============="<<m_contentWidth;
    setFixedWidth(m_contentWidth);
    setFixedHeight(70);
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(LEFT_MARGIN, 0, LEFT_MARGIN, 0);
    hb->setSpacing(0);
    m_inDB = inDB;
    // Adapt buttons////////////////////////////////////////////////////////////

    m_preButton = new DIconButton(this);
    m_nextButton = new DIconButton(this);


    m_preButton->setFixedSize(ICON_SIZE);
    m_nextButton->setFixedSize(ICON_SIZE);

    m_preButton->setIcon(QIcon::fromTheme("dcc_previous"));
    m_preButton->setIconSize(QSize(36, 36));
    m_preButton->setToolTip(tr("Previous"));

    m_nextButton->setIcon(QIcon::fromTheme("dcc_next"));
    m_nextButton->setIconSize(QSize(36, 36));
    m_nextButton->setToolTip(tr("Next"));

    m_preButton->hide();
    m_nextButton->hide();
    m_preButton_spc = new DWidget;
    m_nextButton_spc = new DWidget;
    m_preButton_spc->hide();
    m_nextButton_spc->hide();
    m_preButton_spc->setFixedSize(QSize(10, 50));
    m_nextButton_spc->setFixedSize(QSize(40, 50));
//     m_nextButton->setContentsMargins(0,0,30,0);
//     hb->addSpacing(ICON_SPACING);
    hb->addWidget(m_preButton);
    hb->addWidget(m_preButton_spc);
//     hb->addSpacing(ICON_SPACING);
    hb->addWidget(m_nextButton);
    hb->addWidget(m_nextButton_spc);
//     hb->addSpacing(ICON_SPACING*4);
    connect(m_preButton, &DIconButton::clicked, this, [ = ] {
        emit showPrevious();
    });
    connect(m_nextButton, &DIconButton::clicked, this, [ = ] {
        emit showNext();
    });

    m_adaptImageBtn = new DIconButton(this);
//    m_adaptImageBtn->setObjectName("AdaptBtn");
    m_adaptImageBtn->setFixedSize(ICON_SIZE);
    m_adaptImageBtn->setIcon(QIcon::fromTheme("dcc_11"));
    m_adaptImageBtn->setIconSize(QSize(36, 36));
    m_adaptImageBtn->setToolTip(tr("1:1 Size"));
    hb->addWidget(m_adaptImageBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_adaptImageBtn, &DIconButton::clicked, this, [ = ] {
        emit resetTransform(false);
    });

    m_adaptScreenBtn = new DIconButton(this);
    m_adaptScreenBtn->setFixedSize(ICON_SIZE);
    m_adaptScreenBtn->setIcon(QIcon::fromTheme("dcc_fit"));
    m_adaptScreenBtn->setIconSize(QSize(36, 36));
    m_adaptScreenBtn->setToolTip(tr("Fit to window"));
    hb->addWidget(m_adaptScreenBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_adaptScreenBtn, &DIconButton::clicked, this, [ = ] {
        emit resetTransform(true);
    });

    // Collection button////////////////////////////////////////////////////////
//    m_clBT = new DIconButton(this);
//    m_clBT->setFixedSize(ICON_SIZE);

//    m_clBT->setIcon(QIcon::fromTheme("dcc_left"));
//    m_clBT->setIconSize(QSize(36,36));


    m_rotateLBtn = new DIconButton(this);
    m_rotateLBtn->setFixedSize(ICON_SIZE);
    m_rotateLBtn->setIcon(QIcon::fromTheme("dcc_left"));
    m_rotateLBtn->setIconSize(QSize(36, 36));
    m_rotateLBtn->setToolTip(tr("Rotate counterclockwise"));
    hb->addWidget(m_rotateLBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_rotateLBtn, &DIconButton::clicked,
            this, &TTBContent::rotateCounterClockwise);

    m_rotateRBtn = new DIconButton(this);
    m_rotateRBtn->setFixedSize(ICON_SIZE);
    m_rotateRBtn->setIcon(QIcon::fromTheme("dcc_right"));
    m_rotateRBtn->setIconSize(QSize(36, 36));
    m_rotateRBtn->setToolTip(tr("Rotate clockwise"));
    hb->addWidget(m_rotateRBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_rotateRBtn, &DIconButton::clicked,
            this, &TTBContent::rotateClockwise);

    m_imgListView_prespc = new DWidget;
    m_imgListView_prespc->setFixedSize(QSize(10, 50));
    hb->addWidget(m_imgListView_prespc);
    m_imgListView_prespc->hide();

//    m_imgListView = new DWidget();
    m_imgListView = new MyImageListWidget();
    m_imgList = new DWidget(m_imgListView);
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
            }
        } else
        {
            if (m_imgList->width() < m_imgListView->width()) {
                movex = (m_imgListView->width() - m_imgList->width()) / 2;
            } else {
                if (movex <  m_imgListView->width() - m_imgList->width()) {
                    movex =  m_imgListView->width() - m_imgList->width();
                }
            }
        }
        m_imgList->move(movex, m_imgList->y());
    });
    m_imgListView->setObj(m_imgList);
    m_imgList->installEventFilter(m_imgListView);
    if (m_imgInfos.size() <= 3 ) {
        m_imgList->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
    } else {
        m_imgList->setFixedSize(QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) - THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST, TOOLBAR_HEIGHT));
    }
//    qDebug()<<"init:m_imgList.width=============="<<m_imgList->width();

    m_imgList->setDisabled(false);
    m_imgList->setHidden(true);
    m_imglayout = new QHBoxLayout();
    m_imglayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_imglayout->setMargin(0);
    m_imglayout->setSpacing(0);
    m_imgList->setLayout(m_imglayout);

    if (m_imgInfos.size() <= 3 ) {
        m_imgListView->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
    } else {
        m_imgListView->setFixedSize(QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) - THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST, TOOLBAR_HEIGHT));
    }
//    qDebug()<<"init:m_imgListView.width=============="<<m_imgListView->width();
    m_imgListView->hide();
    QPalette palette ;
    palette.setColor(QPalette::Background, QColor(0, 0, 0, 0)); // 最后一项为透明度
    m_imgList->setPalette(palette);
    m_imgListView->setPalette(palette);
    hb->addWidget(m_imgListView);

    m_imgListView_spc = new DWidget;
    m_imgListView_spc->setFixedSize(QSize(24 - 9, 50)); //temp
    hb->addWidget(m_imgListView_spc);
    m_imgListView_spc->hide();
    m_trashBtn = new DIconButton(this);
    m_trashBtn->setFixedSize(ICON_SIZE);
    m_trashBtn->setIcon(QIcon::fromTheme("dcc_delete"));
    m_trashBtn->setIconSize(QSize(36, 36));
    m_trashBtn->setToolTip(tr("Delete"));
    hb->addWidget(m_trashBtn);
//    hb->addSpacing(ICON_SPACING);

    m_fileNameLabel = new ElidedLabel();
//    hb->addWidget(m_fileNameLabel);
    connect(m_trashBtn, &DIconButton::clicked, this, &TTBContent::removed);
    connect(dApp->signalM, &SignalManager::picDelete, this, [ = ] {
        if (window()->isFullScreen())
        {
            emit dApp->signalM->sigShowFullScreen();
        }
        m_imgInfos_size = m_imgInfos_size - 1;
        int windowWidth =  this->window()->geometry().width();
        if ( m_imgInfos_size <= 1 )
        {
            m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
        } else if ( m_imgInfos_size <= 3 )
        {
            m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
            m_imgListView->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
        } else
        {
            m_contentWidth = qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos_size - 3)), qMax(windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) + THUMBNAIL_LIST_ADJUST;
            m_imgListView->setFixedSize(QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos_size - 3)), qMax(windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) - THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST, TOOLBAR_HEIGHT));
        }
        setFixedWidth(m_contentWidth);
//        qDebug()<<"m_trashBtn:m_contentWidth=============="<<m_contentWidth;
//        qDebug()<<"m_trashBtn:m_imgListView.width=============="<<m_imgListView->width();
    });
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &TTBContent::onThemeChanged);
//     connect(dApp->signalM, &SignalManager::updateTopToolbar, this, [=]{
//         updateFilenameLayout();
//     });
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
        m_leftContentWidth = m_returnBtn->buttonWidth() + 6
                             + (ICON_SIZE.width() + 2) * 6 + LEFT_SPACE;
    else {
        m_leftContentWidth = m_folderBtn->width()  + 8
                             + (ICON_SIZE.width() + 2) * 5 + LEFT_SPACE;
    }
#else
    // 39 为logo以及它的左右margin
    m_leftContentWidth = 5 + (ICON_SIZE.width() + 2) * 5 + 39;
#endif

    int ww = dApp->setter->value("MAINWINDOW",  "WindowWidth").toInt();
    m_windowWidth =  std::max(std::max(this->window()->geometry().width(), this->width()), ww);
    m_contentWidth = std::max(m_windowWidth - RIGHT_TITLEBAR_WIDTH + 2, 1);
    setFixedWidth(m_contentWidth);
    m_contentWidth = this->width() - m_leftContentWidth;

    if (strWidth > m_contentWidth || strWidth > FILENAME_MAX_LENGTH) {
        name = fm.elidedText(filename, Qt::ElideMiddle, std::min(m_contentWidth - 32,
                                                                 FILENAME_MAX_LENGTH));
        strWidth = fm.boundingRect(name).width();
        leftMargin = std::max(0, (m_windowWidth - strWidth) / 2
                              - m_leftContentWidth - LEFT_MARGIN - 2);
    } else {
        leftMargin = std::max(0, (m_windowWidth - strWidth) / 2
                              - m_leftContentWidth - 6);
        name = filename;
    }

    m_fileNameLabel->setText(name, leftMargin);
}

void TTBContent::onThemeChanged(ViewerThemeManager::AppTheme theme)
{

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
    m_windowWidth =  this->window()->geometry().width();
    if ( m_imgInfos_size <= 1 ) {
        m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
    } else if ( m_imgInfos_size <= 3 ) {
        m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
        m_imgListView->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
    } else {
        m_contentWidth = qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos_size - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) + THUMBNAIL_LIST_ADJUST;
        m_imgListView->setFixedSize(QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos_size - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) - THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST, TOOLBAR_HEIGHT));
    }
//    qDebug()<<"resizeEvent:m_contentWidth=============="<<m_contentWidth;
//    qDebug()<<"resizeEvent:m_imgListView.width=============="<<m_imgListView->width();

    setFixedWidth(m_contentWidth);
    setImage(m_imagePath, m_imgInfos);

    QList<ImageItem *> labelList = m_imgList->findChildren<ImageItem *>();
    for (int j = 0; j < labelList.size(); j++) {
        labelList.at(j)->setFixedSize (QSize(32, 40));
        labelList.at(j)->resize (QSize(32, 40));
        labelList.at(j)->setContentsMargins(1, 5, 1, 5);
        labelList.at(j)->setIndexNow(m_nowIndex);
    }
    if (labelList.size() > 0) {
        labelList.at(m_nowIndex)->setFixedSize (QSize(60, 58));
        labelList.at(m_nowIndex)->resize (QSize(60, 58));
        labelList.at(m_nowIndex)->setContentsMargins(0, 0, 0, 0);
    }
}

void TTBContent::setImage(const QString &path, DBImgInfoList infos)
{
    if (!infos.isEmpty() && !QFileInfo(path).exists()) {
        if (infos.size() == 1)
            return;
        emit dApp->signalM->picNotExists(true);
        qDebug() << "QFileInfo(path) is not exists.Path:" << path;
    }

    if (infos.size() != m_imgInfos.size()) {
        m_imgInfos.clear();
        m_imgInfos = infos;

        QLayoutItem *child;
        while ((child = m_imglayout->takeAt(0)) != 0) {
            m_imglayout->removeWidget(child->widget());
            child->widget()->setParent(0);
            delete child;

        }
    }
    if (path.isEmpty() || !QFileInfo(path).exists()
            || !QFileInfo(path).isReadable()) {
        m_adaptImageBtn->setDisabled(true);
        m_adaptScreenBtn->setDisabled(true);
        m_rotateLBtn->setDisabled(true);
        m_rotateRBtn->setDisabled(true);
        m_trashBtn->setDisabled(true);
        m_imgList->setDisabled(false);
    } else {
        m_adaptImageBtn->setDisabled(false);
        m_adaptScreenBtn->setDisabled(false);


        int t = 0;
        if ( m_imgInfos.size() > 3 ) {
            m_imgList->setFixedSize((m_imgInfos.size() + 1)*THUMBNAIL_WIDTH, TOOLBAR_HEIGHT);
            m_imgList->resize((m_imgInfos.size() + 1)*THUMBNAIL_WIDTH + THUMBNAIL_LIST_ADJUST, TOOLBAR_HEIGHT);

//            qDebug()<<"setImage:m_imgList.width=============="<<m_imgList->width();
//            qDebug()<<"setImage:m_imgListView.width=============="<<m_imgListView->width();

            m_imgList->setContentsMargins(0, 0, 0, 0);

            auto num = 32;

            int i = 0;
            QList<ImageItem *> labelList = m_imgList->findChildren<ImageItem *>();

            for (DBImgInfo info : m_imgInfos) {
                if (labelList.size() != m_imgInfos.size()) {
                    char *imageType = getImageType(info.filePath);
                    ImageItem *imageItem = new ImageItem(i, info.filePath, imageType);
                    imageItem->setFixedSize(QSize(num, 40));
                    imageItem->resize(QSize(num, 40));
                    imageItem->installEventFilter(m_imgListView);

                    m_imglayout->addWidget(imageItem);
                    connect(imageItem, &ImageItem::imageItemclicked, this, [ = ](int index, int indexNow) {
                        emit imageClicked(index, (index - indexNow));
                    });
                }
                if ( path == info.filePath ) {
                    t = i;
                }
                i++;
            }
            labelList = m_imgList->findChildren<ImageItem *>();
            m_nowIndex = t;

            int a = (qCeil(m_imgListView->width() - 26) / 32) / 2;
            int b = m_imgInfos.size() - (qFloor(m_imgListView->width() - 26) / 32) / 2;
//            qDebug()<<"a="<<a;
//            qDebug()<<"b="<<b;
//            qDebug()<<"m_nowIndex="<<m_nowIndex;
//            qDebug()<<"m_imgInfos.size()="<<m_imgInfos.size();
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


            labelList.at(t)->updatePic(dApp->m_imagemap.value(path));

            for (int j = 0; j < labelList.size(); j++) {
                labelList.at(j)->setFixedSize (QSize(num, 40));
                labelList.at(j)->resize (QSize(num, 40));
                labelList.at(j)->setIndexNow(t);
            }
            if (labelList.size() > 0) {
                labelList.at(t)->setFixedSize (QSize(58, 58));
                labelList.at(t)->resize (QSize(58, 58));
            }


            m_imgListView->show();

            if (1 == m_startAnimation) {
                QPropertyAnimation *animation = new QPropertyAnimation(m_imgList, "pos");
                animation->setDuration(500);
                animation->setEasingCurve(QEasingCurve::NCurveTypes);
                animation->setStartValue(m_imgList->pos());
                animation->setKeyValueAt(1,  QPoint((qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)), (qMax(width() - RT_SPACING, TOOLBAR_MINIMUN_WIDTH))) - 496 - 52 + 18) / 2 - ((num)*t), 0));
                animation->setEndValue(QPoint((qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgInfos.size() - 3)), (qMax(width() - RT_SPACING, TOOLBAR_MINIMUN_WIDTH))) - 496 - 52 + 18) / 2 - ((num)*t), 0));
                animation->start(QAbstractAnimation::DeleteWhenStopped);
                connect(animation, &QPropertyAnimation::finished,
                        animation, &QPropertyAnimation::deleteLater);

                connect(animation, &QPropertyAnimation::finished,
                this, [ = ] {
                    m_imgList->show();
                });
            } else if (2 == m_startAnimation) {
                QPropertyAnimation *animation = new QPropertyAnimation(m_imgList, "pos");
                animation->setDuration(500);
                animation->setEasingCurve(QEasingCurve::NCurveTypes);
                animation->setStartValue(m_imgList->pos());
                animation->setKeyValueAt(1,  QPoint(0, 0));
                animation->setEndValue(QPoint(0, 0));
                animation->start(QAbstractAnimation::DeleteWhenStopped);
                connect(animation, &QPropertyAnimation::finished,
                        animation, &QPropertyAnimation::deleteLater);

                connect(animation, &QPropertyAnimation::finished,
                this, [ = ] {
                    m_imgList->show();
                });
            } else if (3 == m_startAnimation) {
                QPropertyAnimation *animation = new QPropertyAnimation(m_imgList, "pos");
                animation->setDuration(500);
                animation->setEasingCurve(QEasingCurve::NCurveTypes);
                animation->setStartValue(m_imgList->pos());
                animation->setKeyValueAt(1,  QPoint(0, 0));
                animation->setEndValue(QPoint(m_imgListView->width() - m_imgList->width() + 5, 0));
                animation->start(QAbstractAnimation::DeleteWhenStopped);
                connect(animation, &QPropertyAnimation::finished,
                        animation, &QPropertyAnimation::deleteLater);

                connect(animation, &QPropertyAnimation::finished,
                this, [ = ] {
                    m_imgList->show();
                });
            } else if (0 == m_startAnimation) {
                m_imgList->show();
            }
            /*
                        QPropertyAnimation *animation = new QPropertyAnimation(m_imgList, "pos");
                        animation->setDuration(500);
                        animation->setEasingCurve(QEasingCurve::NCurveTypes);
                        animation->setStartValue(m_imgList->pos());
            //            animation->setKeyValueAt(1,  QPoint(350-((num)*t),0));
                        animation->setKeyValueAt(1,  QPoint((qMin((TOOLBAR_MINIMUN_WIDTH+THUMBNAIL_ADD_WIDTH*(m_imgInfos.size()-3)),(qMax(width()-RT_SPACING,TOOLBAR_MINIMUN_WIDTH)))-496-52+18)/2-((num)*t),0));
            //            animation->setEndValue(QPoint(350-((num)*t),0));
                        animation->setEndValue(QPoint((qMin((TOOLBAR_MINIMUN_WIDTH+THUMBNAIL_ADD_WIDTH*(m_imgInfos.size()-3)),(qMax(width()-RT_SPACING,TOOLBAR_MINIMUN_WIDTH)))-496-52+18)/2-((num)*t),0));
                        animation->start(QAbstractAnimation::DeleteWhenStopped);
                        connect(animation, &QPropertyAnimation::finished,
                                animation, &QPropertyAnimation::deleteLater);

                        connect(animation, &QPropertyAnimation::finished,
                                this, [=]{
                            m_imgList->show();
                        });
            */

            m_imgListView->update();
            m_imgList->update();
            m_preButton->show();
            m_preButton_spc->show();
            m_nextButton->show();
            m_nextButton_spc->show();
            m_imgListView_prespc->show();
            m_imgListView_spc->show();

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

        } else if (m_imgInfos.size() > 1) {
            m_imgList->setFixedSize((m_imgInfos.size() + 1)*THUMBNAIL_WIDTH, TOOLBAR_HEIGHT);
            m_imgList->resize((m_imgInfos.size() + 1)*THUMBNAIL_WIDTH, TOOLBAR_HEIGHT);

            m_imgList->setContentsMargins(0, 0, 0, 0);

            auto num = 32;

            int i = 0;
            QList<ImageItem *> labelList = m_imgList->findChildren<ImageItem *>();

            for (DBImgInfo info : m_imgInfos) {
                if (labelList.size() != m_imgInfos.size()) {
                    char *imageType = getImageType(info.filePath);
                    ImageItem *imageItem = new ImageItem(i, info.filePath, imageType);
                    imageItem->setFixedSize(QSize(num, 40));
                    imageItem->resize(QSize(num, 40));
                    imageItem->installEventFilter(m_imgListView);

                    m_imglayout->addWidget(imageItem);
                    connect(imageItem, &ImageItem::imageItemclicked, this, [ = ](int index, int indexNow) {
                        emit imageClicked(index, (index - indexNow));
                    });
                }
                if ( path == info.filePath ) {
                    t = i;
                }
                i++;
            }
            labelList = m_imgList->findChildren<ImageItem *>();
            m_nowIndex = t;
            for (int j = 0; j < labelList.size(); j++) {
                labelList.at(j)->setFixedSize (QSize(num, 40));
                labelList.at(j)->resize (QSize(num, 40));
                labelList.at(j)->setIndexNow(t);
            }
            if (labelList.size() > 0) {
                labelList.at(t)->setFixedSize (QSize(58, 58));
                labelList.at(t)->resize (QSize(58, 58));
            }

            m_imgListView->show();
            m_imgList->show();
            /*
                        QPropertyAnimation *animation = new QPropertyAnimation(m_imgList, "pos");
                        animation->setDuration(500);
                        animation->setEasingCurve(QEasingCurve::NCurveTypes);
                        animation->setStartValue(m_imgList->pos());
                        animation->setKeyValueAt(1,  QPoint(31-((num)*t),0));
                        animation->setEndValue(QPoint(31-((num)*t),0));
                        animation->start(QAbstractAnimation::DeleteWhenStopped);
                        connect(animation, &QPropertyAnimation::finished,
                                animation, &QPropertyAnimation::deleteLater);

                        connect(animation, &QPropertyAnimation::finished,
                                this, [=]{
                            m_imgList->show();
                        });
            */
            m_imgListView->update();
            m_imgList->update();
            m_preButton->show();
            m_preButton_spc->show();
            m_nextButton->show();
            m_nextButton_spc->show();
            m_imgListView_prespc->show();
            m_imgListView_spc->show();

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
        } else {
            m_imgList->hide();
            m_imgListView->hide();
            m_imgListView_prespc->hide();
            m_imgListView_spc->hide();
            m_preButton->hide();
            m_preButton_spc->hide();
            m_nextButton->hide();
            m_nextButton_spc->hide();
            m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
            setFixedWidth(m_contentWidth);
        }


        if (QFileInfo(path).isReadable() &&
                !QFileInfo(path).isWritable()) {
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
    }
    m_imagePath = path;
    QString fileName = "";
    if (m_imagePath != "") {
        fileName = QFileInfo(m_imagePath).fileName();
    }
    emit dApp->signalM->updateFileName(fileName);
//    updateFilenameLayout();
//    updateCollectButton();
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
