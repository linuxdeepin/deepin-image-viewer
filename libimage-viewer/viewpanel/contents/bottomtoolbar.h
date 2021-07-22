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
#ifndef BOTTOMTOOBAR_H
#define BOTTOMTOOBAR_H

#include <QWidget>
#include <QLabel>
#include <DListWidget>
#include <DSpinner>
#include <QListWidget>
#include <DListView>
#include <QAbstractItemModel>
#include <QStandardItem>
#include <DAnchors>
#include <DFloatingWidget>
#include <dimagebutton.h>
#include <DThumbnailProvider>
#include <QPropertyAnimation>
#include <QHBoxLayout>
#include <DIconButton>
#include <DBlurEffectWidget>
#include <DGuiApplicationHelper>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class ElidedLabel;
class QAbstractItemModel;
//class DImageButton;
class ImageButton;
class ImgViewListView;
class ImageItem;
class MyImageListWidget;

class BottomToolbar : public DFloatingWidget
{
    Q_OBJECT
public:
    struct TTBContentData {
        int index;
//        ImageDataSt data;
    };
//    explicit TTBContent(bool inDB, DBImgInfoList m_infos, QWidget *parent = 0);
    explicit BottomToolbar(QWidget *parent = nullptr);
    ~BottomToolbar() override
    {
    }

//    void reLoad();
    bool setCurrentItem();
    void updateScreen();
//    void updateScreenNoAnimation();
    int itemLoadedSize();
    //设置需要浏览的图片信息
//    void setAllFileInfo(const SignalManager::ViewInfo &info);
    //获取所有图片数量
    int getAllFileCount();
signals:
    void resetTransform(bool fitWindow);
    void rotateClockwise();
    void rotateCounterClockwise();

    void removed();
    void imageEmpty(bool v);
    void contentWidthChanged(int width);
    void showPrevious();
    void feedBackCurrentIndex(int index, const QString &path);
    //平板需求，退出时重置ttb显隐
    void resetShoworHide();

public slots:
    void setCurrentDir(const QString &text);
    void setImage(const QString &path);
    void updateCollectButton();

    void onResize();
    void disCheckAdaptImageBtn();
    void disCheckAdaptScreenBtn();
    void checkAdaptImageBtn();
    void checkAdaptScreenBtn();
    void deleteImage();
    void onNextButton();
    void onPreButton();
    void onBackButtonClicked();
    void onAdaptImageBtnClicked();
    void onAdaptScreenBtnClicked();
    void onclBTClicked();
    void onRotateLBtnClicked();
    void onRotateRBtnClicked();
    void onTrashBtnClicked();

private slots:

protected:
    void resizeEvent(QResizeEvent *event) override;
public:
    QString m_imageType;

private:
#ifndef LITE_DIV
    PushButton *m_folderBtn;
    ReturnButton *m_returnBtn;
#endif
    bool m_bClBTChecked;

    DIconButton *m_adaptImageBtn = nullptr;
    DIconButton *m_adaptScreenBtn = nullptr;
    DIconButton *m_clBT = nullptr;
    DIconButton *m_rotateLBtn = nullptr;
    DIconButton *m_rotateRBtn = nullptr;
    DIconButton *m_trashBtn = nullptr;
    DIconButton *m_preButton = nullptr;
    DIconButton *m_nextButton = nullptr;
    DIconButton *m_backButton = nullptr;
    MyImageListWidget *m_imgListWidget = nullptr;
    int m_windowWidth;
    int m_contentWidth;
    bool badaptImageBtnChecked = false;
    bool badaptScreenBtnChecked = false;
    QString m_currentpath = "";
};

#endif // BOTTOMTOOBAR_H
