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
#ifndef TTBCONTENT_H
#define TTBCONTENT_H

#include <QWidget>
#include <QLabel>
#include "controller/viewerthememanager.h"
//#include <dlistwidget.h>
//#include <DListWidget>
#include <DListWidget>
//#include <DtkWidgets>
//#include "dlistwidget.h"
#include <QListWidget>
#include <DListView>
#include <QAbstractItemModel>
#include <QStandardItem>
#include "controller/dbmanager.h"
#include <DAnchors>
#include <dimagebutton.h>
#include <DThumbnailProvider>
#include <QPropertyAnimation>
DWIDGET_USE_NAMESPACE


class PushButton;
class ReturnButton;
class ElidedLabel;
class QAbstractItemModel;
//class DImageButton;
class ImageButton;
class ImageItem : public QLabel{
    Q_OBJECT
public:
    ImageItem(int index= 0,QWidget *parent = 0){
        _index = index;
    };
    void setIndexNow(int i){
        _indexNow = i;
    };
signals:
    void imageItemclicked(int index,int indexNow);
protected:
    void mousePressEvent(QMouseEvent *ev){
        emit imageItemclicked(_index,_indexNow);
    }
private:
    int _index;
    int _indexNow;
};
class TTBContent : public QLabel
{
    Q_OBJECT
public:
    explicit TTBContent(bool inDB, DBImgInfoList m_infos , QWidget *parent = 0);

signals:
    void clicked();
    void imageClicked(int index,int addIndex);
    void resetTransform(bool fitWindow);
    void rotateClockwise();
    void rotateCounterClockwise();

    void removed();
    void imageEmpty(bool v);
    void contentWidthChanged(int width);
    void showPrevious();
    void showNext();

public slots:
    void setCurrentDir(QString text);
    void setImage(const QString &path);
    void updateCollectButton();

private slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
    void updateFilenameLayout();

protected:
    void resizeEvent(QResizeEvent *event);

private:
#ifndef LITE_DIV
    PushButton *m_folderBtn;
    ReturnButton *m_returnBtn;
#endif
    bool m_inDB;

    PushButton* m_adaptImageBtn;
    PushButton* m_adaptScreenBtn;
    PushButton* m_clBT;
    PushButton* m_rotateLBtn;
    PushButton* m_rotateRBtn;
    PushButton* m_trashBtn;
    DImageButton *m_preButton;
    DImageButton *m_nextButton;
    ElidedLabel* m_fileNameLabel;
    DWidget *m_imgList;
    DWidget *m_imgListView;
    DBImgInfoList m_imgInfos ;
    QString m_imagePath;
    int m_windowWidth;
    int m_contentWidth;
    int m_lastIndex = 0;
    int m_nowIndex = 0;
};

#endif // TTLCONTENT_H
