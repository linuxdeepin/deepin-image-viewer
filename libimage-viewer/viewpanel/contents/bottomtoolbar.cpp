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
#include "bottomtoolbar.h"

#include <QTimer>
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
#include "imgviewwidget.h"
#include "accessibility/ac-desktop-define.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/unionimage.h"
#include "service/commonservice.h"
#include "imageengine.h"
DWIDGET_USE_NAMESPACE
namespace {
/////////

/////////////
const int LEFT_MARGIN = 10;
const QSize ICON_SIZE = QSize(50, 50);
const int ICON_SPACING = 10;
const int FILENAME_MAX_LENGTH = 600;
const int RIGHT_TITLEBAR_WIDTH = 100;
const QString LOCMAP_SELECTED_DARK = ":/resources/dark/images/58 drak.svg";
const QString LOCMAP_NOT_SELECTED_DARK = ":/resources/dark/images/imagewithbg-dark.svg";
const QString LOCMAP_SELECTED_LIGHT = ":/resources/light/images/58.svg";
const QString LOCMAP_NOT_SELECTED_LIGHT = ":/resources/light/images/imagewithbg.svg";

const int TOOLBAR_MINIMUN_WIDTH = 782;
const int TOOLBAR_JUSTONE_WIDTH_ALBUM = 532;
const int TOOLBAR_JUSTONE_WIDTH_LOCAL = 350;
const int RT_SPACING = 20;
const int TOOLBAR_HEIGHT = 60;

const int TOOLBAR_DVALUE = 114 + 8;

const int THUMBNAIL_WIDTH = 32;
const int THUMBNAIL_ADD_WIDTH = 32;
const int THUMBNAIL_LIST_ADJUST = 9;
const int THUMBNAIL_VIEW_DVALUE = 668;

const int LOAD_LEFT_RIGHT = 25;     //前后加载图片数（动态）

}  // namespace

BottomToolbar::BottomToolbar(QWidget *parent) : DFloatingWidget(parent)
{
//    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
//    m_windowWidth = std::max(this->window()->width(),
//                             ConfigSetter::instance()->value("MAINWINDOW", "WindowWidth").toInt());
    initUI();
    initConnection();
}

BottomToolbar::~BottomToolbar()
{

}

int BottomToolbar::getAllFileCount()
{
    if (m_imgListWidget) {
        return m_imgListWidget->getImgCount();
    } else {
        return -1;
    }
}

int BottomToolbar::getToolbarWidth()
{
    //默认值，下面会重新计算
    int width = 300;
    if (CommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeLocal) {
        width = 0;
        m_backButton->setVisible(false);
        m_clBT->setVisible(false);
        //看图，本地图片
        width += LEFT_RIGHT_MARGIN * 2;//左右边距
        if (m_preButton->isVisible()) {
            width += m_preButton->width() + ICON_SPACING;//上一张宽度加边距
            width += m_nextButton->width() + ICON_SPACING;//上一张宽度加边距
            width += m_spaceWidget->width();//特殊控件宽度
        }
        width += m_adaptImageBtn->width() + ICON_SPACING;//适应图片
        width += m_adaptScreenBtn->width() + ICON_SPACING;//适应屏幕
        width += m_rotateLBtn->width() + ICON_SPACING;//左旋
        width += m_ocrBtn->width() + ICON_SPACING;//OCR
        width += m_rotateRBtn->width() + ICON_SPACING;//右旋
        width += m_trashBtn->width();//右旋
        if (m_imgListWidget->getImgCount() <= 1) {
            width += 0;
        } else {
            width += ImgViewListView::ITEM_CURRENT_WH;
            width += (m_imgListWidget->getImgCount() - 1) * (ImgViewListView::ITEM_CURRENT_WH + ImgViewListView::ITEM_SPACING);
        }
    } else if (CommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum) {
        //相册
        width = 0;
        m_backButton->setVisible(true);
        m_clBT->setVisible(true);
    }
    return width;
}

imageViewerSpace::ItemInfo BottomToolbar::getCurrentItemInfo()
{
    return m_imgListWidget->getCurrentImgInfo();
}

void BottomToolbar::setCurrentPath(const QString &path)
{
    m_imgListWidget->setCurrentPath(path);
}

QStringList BottomToolbar::getAllPath()
{
    return m_imgListWidget->getAllPath();
}

void BottomToolbar::disCheckAdaptImageBtn()
{
    qDebug() << "---" << __FUNCTION__ << "---";
    m_adaptImageBtn->setChecked(false);
    badaptImageBtnChecked = false;
}
void BottomToolbar::disCheckAdaptScreenBtn()
{
    m_adaptScreenBtn->setChecked(false);
    badaptScreenBtnChecked = false;
}

void BottomToolbar::checkAdaptImageBtn()
{
    qDebug() << "---" << __FUNCTION__ << "---";
    m_adaptImageBtn->setChecked(true);
    badaptImageBtnChecked = true;
}
void BottomToolbar::checkAdaptScreenBtn()
{
    m_adaptScreenBtn->setChecked(true);
    badaptScreenBtnChecked = true;
}

void BottomToolbar::deleteImage()
{
    if (m_imgListWidget->getImgCount() == 0)
        return;
    //移除正在展示照片
    if (m_imgListWidget) {
        m_imgListWidget->removeCurrent();
    }
    emit removed();     //删除数据库图片
}

void BottomToolbar::onBackButtonClicked()
{
    //2020/6/9 DJH 优化退出全屏，不再闪出退出全屏的间隙 31331
    this->setVisible(false);
    this->setVisible(true);
}

void BottomToolbar::onAdaptImageBtnClicked()
{
    emit resetTransform(false);
    m_adaptImageBtn->setChecked(true);
    if (!badaptImageBtnChecked) {
        badaptImageBtnChecked = true;
    }
}

void BottomToolbar::onAdaptScreenBtnClicked()
{
    emit resetTransform(true);
    m_adaptScreenBtn->setChecked(true);
    if (!badaptScreenBtnChecked) {
        badaptScreenBtnChecked = true;
    }
}

void BottomToolbar::onclBTClicked()
{
    if (true == m_bClBTChecked) {
//        DBManager::instance()->removeFromAlbum(COMMON_STR_FAVORITES, QStringList(m_currentpath), AlbumDBType::Favourite);
    } else {
//        DBManager::instance()->insertIntoAlbum(COMMON_STR_FAVORITES, QStringList(m_currentpath), AlbumDBType::Favourite);
//        emit dApp->signalM->insertedIntoAlbum(COMMON_STR_FAVORITES, QStringList(m_currentpath));
    }
}

void BottomToolbar::onRotateLBtnClicked()
{
    onRotate(-90);
    emit rotateClockwise();
}

void BottomToolbar::onRotateRBtnClicked()
{
    onRotate(90);
    emit rotateCounterClockwise();
}

void BottomToolbar::onTrashBtnClicked()
{
    deleteImage();
//    emit dApp->signalM->deleteByMenu();
}

void BottomToolbar::onNextButton()
{
    if (m_imgListWidget) {
        m_imgListWidget->openNext();
    }
}

void BottomToolbar::onPreButton()
{
    if (m_imgListWidget) {
        m_imgListWidget->openPre();
    }
}

void BottomToolbar::onRotate(int matrix)
{
    m_imgListWidget->rotate(matrix);
}

void BottomToolbar::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
//    m_windowWidth =  this->window()->geometry().width();
//    if (m_imgListWidget->getImgCount() <= 1) {
//        m_contentWidth = TOOLBAR_JUSTONE_WIDTH_LOCAL;
//    } else if (m_imgListWidget->getImgCount() <= 3) {
//        m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
//        //todo设置大小
////        m_imgListView->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
//        m_imgListWidget->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
//    } else {
//        m_contentWidth = qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgListWidget->getImgCount() - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) + THUMBNAIL_LIST_ADJUST;
//        m_imgListWidget->setFixedSize(QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgListWidget->getImgCount() - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) - THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST, TOOLBAR_HEIGHT));
//    }
//    setFixedWidth(m_contentWidth);
}

void BottomToolbar::setAllFile(QString path, QStringList paths)
{
    //每次打开清空一下缩略图
    m_imgListWidget->clearListView();
    if (paths.size() <= 1) {
        m_preButton->setVisible(false);
        m_nextButton->setVisible(false);
        m_spaceWidget->setVisible(false);
    } else {
        m_preButton->setVisible(true);
        m_nextButton->setVisible(true);
    }

    QList<imageViewerSpace::ItemInfo> itemInfos;
    for (int i = 0; i < paths.size(); i++) {
        imageViewerSpace::ItemInfo info;
        info.path = paths.at(i);
        itemInfos << info;
    }

    m_imgListWidget->setAllFile(itemInfos, path);
}

void BottomToolbar::updateCollectButton()
{
    if (m_currentpath.isEmpty()) {
        return;
    }
//    if (DBManager::instance()->isImgExistInAlbum(COMMON_STR_FAVORITES, m_currentpath, AlbumDBType::Favourite)) {
//        m_clBT->setToolTip(tr("Unfavorite"));
//        m_clBT->setIcon(QIcon::fromTheme("dcc_ccollection"));
//        m_clBT->setIconSize(QSize(36, 36));
//        m_bClBTChecked = true;
//    } else {
//        m_clBT->setToolTip(tr("Favorite"));
//        DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
//        Q_UNUSED(themeType);
//        m_clBT->setIcon(QIcon::fromTheme("dcc_collection_normal"));
//        m_clBT->setIconSize(QSize(36, 36));
//        m_bClBTChecked = false;
//    }
}

void BottomToolbar::initUI()
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    this->setLayout(hb);
    hb->setContentsMargins(LEFT_RIGHT_MARGIN, 0, LEFT_RIGHT_MARGIN, 3);
    hb->setSpacing(ICON_SPACING);

    //返回，相册使用
    m_backButton = new DIconButton(this);
    m_backButton->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_backButton, BottomToolbar_Back_Button);
//    AC_SET_ACCESSIBLE_NAME(m_backButton, BottomToolbar_Back_Button);
    m_backButton->setIcon(QIcon::fromTheme("dcc_back"));
    m_backButton->setIconSize(QSize(36, 36));
    m_backButton->setToolTip(tr("Back"));
    m_backButton->setVisible(false);
    hb->addWidget(m_backButton);

    m_spaceWidget = new QWidget(this);
    m_spaceWidget->setFixedSize(ICON_SPACING, ICON_SPACING);
    if (CommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum) {
        hb->addWidget(m_spaceWidget);
        m_backButton->setVisible(true);
    }

    //上一张
    m_preButton = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_preButton, BottomToolbar_Pre_Button);
//    AC_SET_ACCESSIBLE_NAME(m_preButton, BottomToolbar_Pre_Button);
    m_preButton->setFixedSize(ICON_SIZE);
    m_preButton->setIcon(QIcon::fromTheme("dcc_previous"));
    m_preButton->setIconSize(QSize(36, 36));
    m_preButton->setToolTip(tr("Previous"));
    m_preButton->hide();
    hb->addWidget(m_preButton);

    //下一张
    m_nextButton = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_nextButton, BottomToolbar_Next_Button);
//    AC_SET_ACCESSIBLE_NAME(m_nextButton, BottomToolbar_Next_Button);
    m_nextButton->setFixedSize(ICON_SIZE);
    m_nextButton->setIcon(QIcon::fromTheme("dcc_next"));
    m_nextButton->setIconSize(QSize(36, 36));
    m_nextButton->setToolTip(tr("Next"));
    m_nextButton->hide();
    hb->addWidget(m_nextButton);
    if (CommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerType::ImgViewerTypeLocal) {
        hb->addWidget(m_spaceWidget);
    }

    //适应图片
    m_adaptImageBtn = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_adaptImageBtn, BottomToolbar_AdaptImg_Button);
//    AC_SET_ACCESSIBLE_NAME(m_adaptImageBtn, BottomToolbar_AdaptImg_Button);
    m_adaptImageBtn->setFixedSize(ICON_SIZE);
    m_adaptImageBtn->setIcon(QIcon::fromTheme("dcc_11"));
    m_adaptImageBtn->setIconSize(QSize(36, 36));
    m_adaptImageBtn->setToolTip(tr("1:1 Size"));
    m_adaptImageBtn->setCheckable(true);
    hb->addWidget(m_adaptImageBtn);

    //适应屏幕
    m_adaptScreenBtn = new DIconButton(this);
    m_adaptScreenBtn->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_adaptScreenBtn, BottomToolbar_AdaptScreen_Button);
//    AC_SET_ACCESSIBLE_NAME(m_adaptScreenBtn, BottomToolbar_AdaptScreen_Button);
    m_adaptScreenBtn->setIcon(QIcon::fromTheme("dcc_fit"));
    m_adaptScreenBtn->setIconSize(QSize(36, 36));
    m_adaptScreenBtn->setToolTip(tr("Fit to window"));
//    m_adaptScreenBtn->setCheckable(true);
    hb->addWidget(m_adaptScreenBtn);

    //收藏，相册使用
    m_clBT = new DIconButton(this);
    m_clBT->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_clBT, BottomToolbar_Collect_Button);
//    AC_SET_ACCESSIBLE_NAME(m_clBT, BottomToolbar_Collect_Button);
    hb->addWidget(m_clBT);

    //ocr
    m_ocrBtn = new DIconButton(this);
    m_ocrBtn->setFixedSize(ICON_SIZE);
    m_ocrBtn->setIcon(QIcon::fromTheme("dcc_ocr"));
    m_ocrBtn->setIconSize(QSize(36, 36));
    m_ocrBtn->setToolTip(tr("Extract text"));
    hb->addWidget(m_ocrBtn);

    //向左旋转
    m_rotateLBtn = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_rotateLBtn, BottomToolbar_Rotate_Left_Button);
//    AC_SET_ACCESSIBLE_NAME(m_rotateLBtn, BottomToolbar_Rotate_Left_Button);
    m_rotateLBtn->setFixedSize(ICON_SIZE);
    m_rotateLBtn->setIcon(QIcon::fromTheme("dcc_left"));
    m_rotateLBtn->setIconSize(QSize(36, 36));
    m_rotateLBtn->setToolTip(tr("Rotate counterclockwise"));
    hb->addWidget(m_rotateLBtn);

    //向右旋转
    m_rotateRBtn = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_rotateRBtn, BottomToolbar_Rotate_Right_Button);
//    AC_SET_ACCESSIBLE_NAME(m_rotateRBtn, BottomToolbar_Rotate_Right_Button);
    m_rotateRBtn->setFixedSize(ICON_SIZE);
    m_rotateRBtn->setIcon(QIcon::fromTheme("dcc_right"));
    m_rotateRBtn->setIconSize(QSize(36, 36));
    m_rotateRBtn->setToolTip(tr("Rotate clockwise"));
    hb->addWidget(m_rotateRBtn);

    //缩略图列表
    m_imgListWidget = new MyImageListWidget(this);
    hb->addWidget(m_imgListWidget);

    //删除
    m_trashBtn = new DIconButton(this);
    m_trashBtn->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_trashBtn, BottomToolbar_Trash_Button);
//    AC_SET_ACCESSIBLE_NAME(m_trashBtn, BottomToolbar_Trash_Button);
    m_trashBtn->setIcon(QIcon::fromTheme("dcc_delete"));
    m_trashBtn->setIconSize(QSize(36, 36));
    m_trashBtn->setToolTip(tr("Delete"));
    hb->addWidget(m_trashBtn);
}

void BottomToolbar::initConnection()
{
    //返回按钮，相册使用
    connect(m_backButton, &DIconButton::clicked, this, &BottomToolbar::onBackButtonClicked);
    //前一张
    connect(m_preButton, &DIconButton::clicked, this, &BottomToolbar::onPreButton);
    //下一张
    connect(m_nextButton, &DIconButton::clicked, this, &BottomToolbar::onNextButton);
    //适应图片
    connect(m_adaptImageBtn, &DIconButton::clicked, this, &BottomToolbar::onAdaptImageBtnClicked);
    //适应屏幕
    connect(m_adaptScreenBtn, &DIconButton::clicked, this, &BottomToolbar::onAdaptScreenBtnClicked);
    //收藏，相册使用
    connect(m_clBT, &DIconButton::clicked, this, &BottomToolbar::onclBTClicked);
    //向左旋转
    connect(m_rotateLBtn, &DIconButton::clicked, this, &BottomToolbar::onRotateLBtnClicked);
    //向右旋转
    connect(m_rotateRBtn, &DIconButton::clicked, this, &BottomToolbar::onRotateRBtnClicked);
    //缩略图列表，单机打开图片
    connect(m_imgListWidget, &MyImageListWidget::openImg, this, &BottomToolbar::openImg, Qt::QueuedConnection);
    //删除
    connect(m_trashBtn, &DIconButton::clicked, this, &BottomToolbar::onTrashBtnClicked);
    //ocr
    connect(m_ocrBtn, &DIconButton::clicked, this, &BottomToolbar::sigOcr);
}
