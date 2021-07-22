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
DWIDGET_USE_NAMESPACE
namespace {
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
const int TOOLBAR_JUSTONE_WIDTH = 532;
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

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(LEFT_MARGIN, 0, LEFT_MARGIN, 3);
    hb->setSpacing(0);
    // Adapt buttons////////////////////////////////////////////////////////////
    m_backButton = new DIconButton(this);
    m_backButton->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_backButton, BottomToolbar_Back_Button);
//    AC_SET_ACCESSIBLE_NAME(m_backButton, BottomToolbar_Back_Button);
    m_backButton->setIcon(QIcon::fromTheme("dcc_back"));
    m_backButton->setIconSize(QSize(36, 36));
    m_backButton->setToolTip(tr("Back"));

    hb->addWidget(m_backButton);
    hb->addStretch();
    //hb->addSpacing(ICON_SPACING * 5);
    connect(m_backButton, &DIconButton::clicked, this, &BottomToolbar::onBackButtonClicked);

    // preButton
    m_preButton = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_preButton, BottomToolbar_Pre_Button);
//    AC_SET_ACCESSIBLE_NAME(m_preButton, BottomToolbar_Pre_Button);
    m_preButton->setFixedSize(ICON_SIZE);
    m_preButton->setIcon(QIcon::fromTheme("dcc_previous"));
    m_preButton->setIconSize(QSize(36, 36));
    m_preButton->setToolTip(tr("Previous"));

    m_preButton->hide();

    connect(m_preButton, &DIconButton::clicked, this, &BottomToolbar::onPreButton);
    // nextButton
    m_nextButton = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_nextButton, BottomToolbar_Next_Button);
//    AC_SET_ACCESSIBLE_NAME(m_nextButton, BottomToolbar_Next_Button);
    m_nextButton->setFixedSize(ICON_SIZE);
    m_nextButton->setIcon(QIcon::fromTheme("dcc_next"));
    m_nextButton->setIconSize(QSize(36, 36));
    m_nextButton->setToolTip(tr("Next"));

    m_nextButton->hide();

//    connect(parent, SIGNAL(sigResize()), this, SLOT(onResize()));
    connect(m_nextButton, &DIconButton::clicked, this, &BottomToolbar::onNextButton);

    hb->addWidget(m_preButton);
    hb->addSpacing(ICON_SPACING);
    hb->addWidget(m_nextButton);
    hb->addSpacing(ICON_SPACING);

    // adaptImageBtn
    m_adaptImageBtn = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_adaptImageBtn, BottomToolbar_AdaptImg_Button);
//    AC_SET_ACCESSIBLE_NAME(m_adaptImageBtn, BottomToolbar_AdaptImg_Button);
    m_adaptImageBtn->setFixedSize(ICON_SIZE);
    m_adaptImageBtn->setIcon(QIcon::fromTheme("dcc_11"));
    m_adaptImageBtn->setIconSize(QSize(36, 36));
    m_adaptImageBtn->setToolTip(tr("1:1 Size"));
    m_adaptImageBtn->setCheckable(true);


    hb->addWidget(m_adaptImageBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_adaptImageBtn, &DIconButton::clicked, this, &BottomToolbar::onAdaptImageBtnClicked);


    // adaptScreenBtn
    m_adaptScreenBtn = new DIconButton(this);
    m_adaptScreenBtn->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_adaptScreenBtn, BottomToolbar_AdaptScreen_Button);
//    AC_SET_ACCESSIBLE_NAME(m_adaptScreenBtn, BottomToolbar_AdaptScreen_Button);
    m_adaptScreenBtn->setIcon(QIcon::fromTheme("dcc_fit"));
    m_adaptScreenBtn->setIconSize(QSize(36, 36));
    m_adaptScreenBtn->setToolTip(tr("Fit to window"));
//    m_adaptScreenBtn->setCheckable(true);


    hb->addWidget(m_adaptScreenBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_adaptScreenBtn, &DIconButton::clicked, this, &BottomToolbar::onAdaptScreenBtnClicked);

    // Collection button
    m_clBT = new DIconButton(this);
    m_clBT->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_clBT, BottomToolbar_Collect_Button);
//    AC_SET_ACCESSIBLE_NAME(m_clBT, BottomToolbar_Collect_Button);

    connect(m_clBT, &DIconButton::clicked, this, &BottomToolbar::onclBTClicked);

    hb->addWidget(m_clBT);
    hb->addSpacing(ICON_SPACING);

    // rotateLBtn
    m_rotateLBtn = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_rotateLBtn, BottomToolbar_Rotate_Left_Button);
//    AC_SET_ACCESSIBLE_NAME(m_rotateLBtn, BottomToolbar_Rotate_Left_Button);
    m_rotateLBtn->setFixedSize(ICON_SIZE);
    m_rotateLBtn->setIcon(QIcon::fromTheme("dcc_left"));
    m_rotateLBtn->setIconSize(QSize(36, 36));
    m_rotateLBtn->setToolTip(tr("Rotate counterclockwise"));
    hb->addWidget(m_rotateLBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_rotateLBtn, &DIconButton::clicked, this, &BottomToolbar::onRotateLBtnClicked);

    // rotateRBtn
    m_rotateRBtn = new DIconButton(this);
//    AC_SET_OBJECT_NAME(m_rotateRBtn, BottomToolbar_Rotate_Right_Button);
//    AC_SET_ACCESSIBLE_NAME(m_rotateRBtn, BottomToolbar_Rotate_Right_Button);
    m_rotateRBtn->setFixedSize(ICON_SIZE);
    m_rotateRBtn->setIcon(QIcon::fromTheme("dcc_right"));
    m_rotateRBtn->setIconSize(QSize(36, 36));
    m_rotateRBtn->setToolTip(tr("Rotate clockwise"));

    hb->addWidget(m_rotateRBtn);
    hb->addSpacing(ICON_SPACING + 8);
    connect(m_rotateRBtn, &DIconButton::clicked, this, &BottomToolbar::onRotateRBtnClicked);

    // imgListView
    m_imgListWidget = new MyImageListWidget(this);
    connect(m_imgListWidget, &MyImageListWidget::openImg, this, &BottomToolbar::feedBackCurrentIndex, Qt::QueuedConnection);
//    connect(m_imgListView, &MyImageListWidget::mouseLeftReleased, this, &BottomToolbar::updateScreen);

    //图片少的时候设置列表大小
    //todo设置大小
//    if (m_allfileslist.size() <= 3) {
//        m_imgListView->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
//    } else {
//        m_imgListView->setFixedSize(QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_allfileslist.size() - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) - THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST, TOOLBAR_HEIGHT));
//    }

    hb->addWidget(m_imgListWidget);
    hb->addSpacing(ICON_SPACING + 14);

    m_trashBtn = new DIconButton(this);
    m_trashBtn->setFixedSize(ICON_SIZE);
//    AC_SET_OBJECT_NAME(m_trashBtn, BottomToolbar_Trash_Button);
//    AC_SET_ACCESSIBLE_NAME(m_trashBtn, BottomToolbar_Trash_Button);
    m_trashBtn->setIcon(QIcon::fromTheme("dcc_delete"));
    m_trashBtn->setIconSize(QSize(36, 36));
    m_trashBtn->setToolTip(tr("Delete"));

    hb->addWidget(m_trashBtn);
    hb->addSpacing(10);
    connect(m_trashBtn, &DIconButton::clicked, this, &BottomToolbar::onTrashBtnClicked);
}

//void BottomToolbar::setAllFileInfo(const SignalManager::ViewInfo &info)
//{
//    qDebug() << "---" << __FUNCTION__ << "---";
//    if (info.paths.size() <= 1) {
//        m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
//    } else if (info.paths.size() <= 3) {
//        m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
//        m_preButton->setVisible(true);
//        m_nextButton->setVisible(true);
//    } else {
//        m_preButton->setVisible(true);
//        m_nextButton->setVisible(true);
//        m_contentWidth = qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (info.paths.size() - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) + THUMBNAIL_LIST_ADJUST;
//    }

//    setFixedWidth(m_contentWidth);
//    setFixedHeight(72);

//    m_imgListWidget->setAllFile(info.itemInfos, info.path);
//}

int BottomToolbar::getAllFileCount()
{
    if (m_imgListWidget) {
        return m_imgListWidget->getImgCount();
    } else {
        return -1;
    }
}

int BottomToolbar::itemLoadedSize()
{
    return m_imgListWidget->getImgCount();
}

void BottomToolbar::updateScreen()
{
//todo,不知道干啥的
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
    onResize();
}

void BottomToolbar::onBackButtonClicked()
{
    //2020/6/9 DJH 优化退出全屏，不再闪出退出全屏的间隙 31331
    this->setVisible(false);
    this->setVisible(true);
}

void BottomToolbar::onAdaptImageBtnClicked()
{
    m_adaptImageBtn->setChecked(true);
    if (!badaptImageBtnChecked) {
        badaptImageBtnChecked = true;
        emit resetTransform(false);
    }
}

void BottomToolbar::onAdaptScreenBtnClicked()
{
    m_adaptScreenBtn->setChecked(true);
    if (!badaptScreenBtnChecked) {
        badaptScreenBtnChecked = true;
        emit resetTransform(true);
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
    emit rotateCounterClockwise();
}

void BottomToolbar::onRotateRBtnClicked()
{
    emit rotateClockwise();
}

void BottomToolbar::onTrashBtnClicked()
{
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

void BottomToolbar::setCurrentDir(const QString &text)
{
#ifndef LITE_DIV
    if (text == COMMON_STR_FAVORITES) {
        text = tr(COMMON_STR_FAVORITES);
    }
    m_returnBtn->setText(text);
#else
    Q_UNUSED(text)
#endif
}

void BottomToolbar::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_windowWidth =  this->window()->geometry().width();
    if (m_imgListWidget->getImgCount() <= 1) {
        m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
    } else if (m_imgListWidget->getImgCount() <= 3) {
        m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
        //todo设置大小
//        m_imgListView->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
        m_imgListWidget->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
    } else {
        m_contentWidth = qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgListWidget->getImgCount() - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) + THUMBNAIL_LIST_ADJUST;
        m_imgListWidget->setFixedSize(QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgListWidget->getImgCount() - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) - THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST, TOOLBAR_HEIGHT));
    }
    setFixedWidth(m_contentWidth);
}


void BottomToolbar::setImage(const QString &path)
{
    qDebug() << "---" << __FUNCTION__ << "---";
    if ((m_imgListWidget->getImgCount() > 0) && !QFileInfo(path).exists()) {
//        emit dApp->signalM->picNotExists(true);
        m_currentpath = path;
        if (m_imgListWidget->getImgCount() == 1)
            return;
    } else {
//        emit dApp->signalM->picNotExists(false);
    }
    m_currentpath = path;
    ItemInfo info = m_imgListWidget->getImgInfo(path);
    if (info.image.isNull()) {
        m_adaptImageBtn->setDisabled(true);
        m_adaptScreenBtn->setDisabled(true);
        m_rotateLBtn->setDisabled(true);
        m_rotateRBtn->setDisabled(true);
//        m_trashBtn->setDisabled(true);
//        m_imgList->setDisabled(false);
    } else {
        setCurrentItem();
    }
}

bool BottomToolbar::setCurrentItem()
{
    if (m_currentpath.isEmpty() || !QFileInfo(m_currentpath).exists()) {
        m_adaptImageBtn->setDisabled(true);
        m_adaptScreenBtn->setDisabled(true);
        m_rotateLBtn->setDisabled(true);
        m_rotateRBtn->setDisabled(true);
        m_trashBtn->setDisabled(true);
        m_clBT->setDisabled(true);
    } else {
        m_adaptImageBtn->setDisabled(false);
        m_adaptScreenBtn->setDisabled(false);

        updateScreen();
        m_trashBtn->setDisabled(false);

        if (UnionImage_NameSpace::canSave(m_currentpath) && QFile::permissions(m_currentpath).testFlag(QFile::WriteUser)) {
            m_rotateLBtn->setDisabled(false);
            m_rotateRBtn->setDisabled(false);
        } else {
            m_rotateLBtn->setEnabled(false);
            m_rotateRBtn->setEnabled(false);
        }
    }
    QString fileName = "";
    if (m_currentpath != "") {
        fileName = QFileInfo(m_currentpath).fileName();
    }
//    emit dApp0->signalM->updateFileName(fileName);
    updateCollectButton();
    return true;
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

void BottomToolbar::onResize()
{
    m_windowWidth =  this->window()->geometry().width();
    if (m_imgListWidget->getImgCount() <= 1) {
        m_preButton->setVisible(false);
        m_nextButton->setVisible(false);
        m_contentWidth = TOOLBAR_JUSTONE_WIDTH;
    } else if (m_imgListWidget->getImgCount() <= 3) {
        m_contentWidth = TOOLBAR_MINIMUN_WIDTH;
        //todo设置大小
//        m_imgListView->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
        m_imgListWidget->setFixedSize(QSize(TOOLBAR_DVALUE, TOOLBAR_HEIGHT));
    } else {
        m_contentWidth = qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgListWidget->getImgCount() - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) + THUMBNAIL_LIST_ADJUST;
        //todo设置大小
//        m_imgListView->setFixedSize(QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_filelist_size - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) - THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST, TOOLBAR_HEIGHT));
        m_imgListWidget->setFixedSize(QSize(qMin((TOOLBAR_MINIMUN_WIDTH + THUMBNAIL_ADD_WIDTH * (m_imgListWidget->getImgCount() - 3)), qMax(m_windowWidth - RT_SPACING, TOOLBAR_MINIMUN_WIDTH)) - THUMBNAIL_VIEW_DVALUE + THUMBNAIL_LIST_ADJUST, TOOLBAR_HEIGHT));
    }
    setFixedWidth(m_contentWidth);
}
