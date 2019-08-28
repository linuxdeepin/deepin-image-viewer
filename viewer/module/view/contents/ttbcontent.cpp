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
#include <dlabel.h>
#include <QAbstractItemModel>
#include <dimagebutton.h>
#include <DThumbnailProvider>

DWIDGET_USE_NAMESPACE
namespace {
const int LEFT_MARGIN = 10;
const QSize ICON_SIZE = QSize(50, 50);
const QString FAVORITES_ALBUM = "My favorite";
const int ICON_SPACING = 3;
const int RETURN_BTN_MAX = 200;
const int FILENAME_MAX_LENGTH = 600;
const int RIGHT_TITLEBAR_WIDTH = 100;
const int LEFT_SPACE = 20;
}  // namespace

TTBContent::TTBContent(bool inDB,
                       DBImgInfoList m_infos ,
                       QWidget *parent) : QLabel(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    m_windowWidth = std::max(this->window()->width(),
        ConfigSetter::instance()->value("MAINWINDOW", "WindowWidth").toInt());
//    m_contentWidth = std::max(m_windowWidth - RIGHT_TITLEBAR_WIDTH, 1);
    m_imgInfos = m_infos;
    if ( m_imgInfos.size() <= 1 ) {
        m_contentWidth = 310;
    } else {
        m_contentWidth = 1280;
    }

    setFixedWidth(m_contentWidth);
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(LEFT_MARGIN, 0, 0, 0);
    hb->setSpacing(0);
    m_inDB = inDB;
#ifndef LITE_DIV
    m_returnBtn = new ReturnButton();
    m_returnBtn->setMaxWidth(RETURN_BTN_MAX);
    m_returnBtn->setMaximumWidth(RETURN_BTN_MAX);
    m_returnBtn->setObjectName("ReturnBtn");
    m_returnBtn->setToolTip(tr("Back"));

    m_folderBtn = new PushButton();
    m_folderBtn->setFixedSize(QSize(24, 24));
    m_folderBtn->setObjectName("FolderBtn");
    m_folderBtn->setToolTip(tr("Image management"));
    if(m_inDB) {
        hb->addWidget(m_returnBtn);
    } else {
       hb->addWidget(m_folderBtn);
    }
    hb->addSpacing(20);

    connect(m_returnBtn, &ReturnButton::clicked, this, [=] {
        emit clicked();
    });
    connect(m_folderBtn, &PushButton::clicked, this, [=] {
        emit clicked();
    });
    connect(m_returnBtn, &ReturnButton::returnBtnWidthChanged, this, [=]{
        updateFilenameLayout();
    });
#endif
    // Adapt buttons////////////////////////////////////////////////////////////

     m_preButton = new DImageButton();
     m_nextButton = new DImageButton();
     m_preButton->setObjectName("PreviousButton");
     m_nextButton->setObjectName("NextButton");
     if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
         m_preButton->setNormalPic(":/resources/dark/images/previous_normal.svg");
         m_preButton->setHoverPic(":/resources/dark/images/previous_hover.svg");
         m_preButton->setPressPic(":/resources/dark/images/previous_press.svg");
         m_preButton->setCheckedPic(":/resources/dark/images/previous_press.svg");

         m_nextButton->setNormalPic(":/resources/dark/images/next_normal.svg");
         m_nextButton->setHoverPic(":/resources/dark/images/next_hover.svg");
         m_nextButton->setPressPic(":/resources/dark/images/next_press.svg");
         m_nextButton->setCheckedPic(":/resources/dark/images/next_press.svg");

     } else {
         m_preButton->setNormalPic(":/resources/light/images/previous_normal.svg");
         m_preButton->setHoverPic(":/resources/light/images/previous_hover.svg");
         m_preButton->setPressPic(":/resources/light/images/previous_press.svg");
         m_preButton->setCheckedPic(":/resources/light/images/previous_press.svg");

         m_nextButton->setNormalPic(":/resources/light/images/next_normal.svg");
         m_nextButton->setHoverPic(":/resources/light/images/next_hover.svg");
         m_nextButton->setPressPic(":/resources/light/images/next_press.svg");
         m_nextButton->setCheckedPic(":/resources/light/images/next_press.svg");
     }
     m_preButton->setFixedSize(ICON_SIZE);
     m_nextButton->setFixedSize(ICON_SIZE);
     m_preButton->setToolTip(tr("Previous"));
     m_nextButton->setToolTip(tr("Next"));
     m_preButton->hide();
     m_nextButton->hide();
     hb->addWidget(m_preButton);
     hb->addSpacing(ICON_SPACING);
     hb->addWidget(m_nextButton);
     hb->addSpacing(ICON_SPACING);
     connect(m_preButton, &DImageButton::clicked, this, [=] {
         emit showPrevious();
     });
     connect(m_nextButton, &DImageButton::clicked, this, [=] {
         emit showNext();
     });

    m_adaptImageBtn = new PushButton();
    m_adaptImageBtn->setObjectName("AdaptBtn");
    m_adaptImageBtn->setFixedSize(ICON_SIZE);

    m_adaptImageBtn->setToolTip(tr("1:1 Size"));
    hb->addWidget(m_adaptImageBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_adaptImageBtn, &PushButton::clicked, this, [=] {
        emit resetTransform(false);
    });

    m_adaptScreenBtn = new PushButton();
    m_adaptScreenBtn->setFixedSize(ICON_SIZE);
    m_adaptScreenBtn->setObjectName("AdaptScreenBtn");
    m_adaptScreenBtn->setToolTip(tr("Fit to window"));
    hb->addWidget(m_adaptScreenBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_adaptScreenBtn, &PushButton::clicked, this, [=] {
        emit resetTransform(true);
    });

    // Collection button////////////////////////////////////////////////////////
    m_clBT = new PushButton();
    m_clBT->setFixedSize(ICON_SIZE);
    m_clBT->setObjectName("CollectBtn");
#ifndef LITE_DIV
    if (m_inDB) {
        hb->addWidget(m_clBT);
        connect(m_clBT, &PushButton::clicked, this, [=] {
            if (DBManager::instance()->isImgExistInAlbum(FAVORITES_ALBUM, m_imagePath)) {
                DBManager::instance()->removeFromAlbum(FAVORITES_ALBUM, QStringList(m_imagePath));
            }
            else {
                DBManager::instance()->insertIntoAlbum(FAVORITES_ALBUM, QStringList(m_imagePath));
            }
            updateCollectButton();
        });
        updateCollectButton();
    }
#endif

    m_rotateLBtn = new PushButton();
    m_rotateLBtn->setFixedSize(ICON_SIZE);
    m_rotateLBtn->setObjectName("RotateBtn");
    m_rotateLBtn->setToolTip(tr("Rotate counterclockwise"));
    hb->addWidget(m_rotateLBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_rotateLBtn, &PushButton::clicked,
            this, &TTBContent::rotateCounterClockwise);

    m_rotateRBtn = new PushButton();
    m_rotateRBtn->setFixedSize(ICON_SIZE);
    m_rotateRBtn->setObjectName("RotateCounterBtn");
    m_rotateRBtn->setToolTip(tr("Rotate clockwise"));
    hb->addWidget(m_rotateRBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_rotateRBtn, &PushButton::clicked,
            this, &TTBContent::rotateClockwise);


//    m_imgList = new DListView();
    m_imgList = new DListWidget;
    //显示图标
    m_imgList->setViewMode(QListView::IconMode);
    //设置图标可不可以移动
    m_imgList->setMovement(QListView::Static);
    //设置图标的大小
    m_imgList->setIconSize(QSize(30,40));
    //设置网格的大小
    m_imgList->setGridSize(QSize(30,40));
//    m_imgList->setGeometry(0,0,480,272);
    m_imgList->setResizeMode(QListView::Adjust);
    //定义QListWidget对象
//    m_imgList->resize(786,40);
    //设置QListWidget的显示模式
//    m_imgList->setViewMode(QListView::IconMode);
    //设置QListWidget中单元项的图片大小
//    m_imgList->setIconSize(QSize(100,100));
    //设置QListWidget中单元项的间距
//    m_imgList->setSpacing(10);
    //设置自动适应布局调整（Adjust适应，Fixed不适应），默认不适应
//    m_imgList->setResizeMode(QListWidget::Adjust);
    //设置不能移动
//    imageList->setMovement(QListWidget::Static);

    //显示QListWidget
//    m_imgList->show();
    m_imgList->setDisabled(false);
    m_imgList->setHidden(true);
    hb->addWidget(m_imgList);

    m_trashBtn = new PushButton();
    m_trashBtn->setFixedSize(ICON_SIZE);
    m_trashBtn->setObjectName("TrashBtn");
    m_trashBtn->setToolTip(tr("Delete"));
    hb->addWidget(m_trashBtn);

    m_fileNameLabel = new ElidedLabel();
//    hb->addWidget(m_fileNameLabel);
    connect(m_trashBtn, &PushButton::clicked, this, &TTBContent::removed);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &TTBContent::onThemeChanged);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &TTBContent::onThemeChanged);
//     connect(dApp->signalM, &SignalManager::updateTopToolbar, this, [=]{
//         updateFilenameLayout();
//     });
}

void TTBContent::updateFilenameLayout()
{
    using namespace utils::base;
    QFont font;
    font.setPixelSize(12);
    m_fileNameLabel->setFont(font);
    QFontMetrics fm(font);
    QString filename = QFileInfo(m_imagePath).fileName();
    QString name;

    int strWidth = fm.boundingRect(filename).width();
    int leftMargin = 0;
    int m_leftContentWidth = 0;
#ifndef LITE_DIV
    if (m_inDB)
        m_leftContentWidth = m_returnBtn->buttonWidth() + 6
                + (ICON_SIZE.width()+2)*6 + LEFT_SPACE;
    else
    {
        m_leftContentWidth = m_folderBtn->width()  + 8
                + (ICON_SIZE.width()+2)*5 + LEFT_SPACE;
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

    if (strWidth > m_contentWidth || strWidth > FILENAME_MAX_LENGTH)
    {
        name = fm.elidedText(filename, Qt::ElideMiddle, std::min(m_contentWidth - 32,
                                                                 FILENAME_MAX_LENGTH));
        strWidth = fm.boundingRect(name).width();
        leftMargin = std::max(0, (m_windowWidth - strWidth)/2
                              - m_leftContentWidth - LEFT_MARGIN - 2);
    } else {
        leftMargin = std::max(0, (m_windowWidth - strWidth)/2
                              - m_leftContentWidth - 6);
        name = filename;
    }

    m_fileNameLabel->setText(name, leftMargin);
}

void TTBContent::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/dark/qss/ttl.qss"));
    } else {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/light/qss/ttl.qss"));
    }
}

void TTBContent::setCurrentDir(QString text) {
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
//    m_contentWidth = std::max(m_windowWidth - 100, 1);
//    m_contentWidth = 310;
}

void TTBContent::setImage(const QString &path)
{
    m_imagePath = path;
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

//        QAbstractItemModel *slm = new QAbstractItemModel();
//        QStandardItem *s1=new QStandardItem(QIcon(path),"普通员工");
        if ( m_imgInfos.size() > 0 ) {
//            QStandardItemModel *slm=new QStandardItemModel(this);
            for (DBImgInfo info : m_imgInfos) {
//                QStandardItem *s1=new QStandardItem(QIcon(info.filePath),info.fileName);
//                slm->appendRow(s1);
                //定义QListWidgetItem对象
//                QFileInfo aaa(info.fileName) ;
//                bool ret = DThumbnailProvider::instance()->hasThumbnail(aaa);
//                if(!ret)  {
//                    DThumbnailProvider::instance()->appendToProduceQueue(aaa,DThumbnailProvider::Size::Large,[this](const QString &path){
//                        QListWidgetItem *imageItem = new QListWidgetItem;
//                        imageItem->setBackground(QBrush(QPixmap(path).scaled(30,40)));
//                        imageItem->setSizeHint(QSize(30,40));
//                        m_imgList->setIconSize(QSize(30,40));
//                        m_imgList->addItem(imageItem);
//                                                                         } );
//                }
//                QString imgpath= DThumbnailProvider::instance()->thumbnailFilePath(aaa,DThumbnailProvider::Size::Large);
//                imgpath = DThumbnailProvider::instance()->createThumbnail(aaa,DThumbnailProvider::Size::Large);
                QListWidgetItem *imageItem = new QListWidgetItem;
                imageItem->setBackground(QBrush(QPixmap(info.filePath).scaled(30,40)));
                imageItem->setSizeHint(QSize(30,40));

                //将单元项添加到QListWidget中
                m_imgList->addItem(imageItem);
            }

//            m_imgList->setModel(slm);
//            m_imgList->setDisabled(true);
            m_imgList->show();
            m_preButton->show();
            m_nextButton->show();
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

//    updateFilenameLayout();
    updateCollectButton();
}

void TTBContent::updateCollectButton()
{
    if (! m_clBT)
        return;

    if (m_imagePath.isEmpty() || !QFileInfo(m_imagePath).exists()) {
        m_clBT->setDisabled(true);
        m_clBT->setChecked(false);
    }
    else
        m_clBT->setDisabled(false);

    if (! m_clBT->isEnabled()) {
        m_clBT->setDisabled(true);
    }
#ifndef LITE_DIV
    else if (DBManager::instance()->isImgExistInAlbum(FAVORITES_ALBUM,
                                                      m_imagePath)) {
        m_clBT->setToolTip(tr("Unfavorite"));
        m_clBT->setChecked(true);
    }
#endif
    else {
        m_clBT->setToolTip(tr("Favorite"));
        m_clBT->setChecked(false);
        m_clBT->setDisabled(false);
    }
}
