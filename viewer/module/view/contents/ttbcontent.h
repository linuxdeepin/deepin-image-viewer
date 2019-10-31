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
#include <DSpinner>
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
#include <QHBoxLayout>
#include <DIconButton>
#include <DBlurEffectWidget>
DWIDGET_USE_NAMESPACE


class PushButton;
class ReturnButton;
class ElidedLabel;
class QAbstractItemModel;
//class DImageButton;
class ImageButton;
class ImageItem : public DLabel{
    Q_OBJECT
public:
    ImageItem(int index= 0,QString path = NULL,char *imageType = NULL, QWidget *parent = 0);
    void setIndexNow(int i){
        _indexNow = i;

        if (_index == _indexNow)
        {
            m_spinner->move(21, 21);
        }
    };
    void setPic(QImage image){
      _image->setPixmap(QPixmap::fromImage(image.scaled(60,50)));
    };
signals:
    void imageItemclicked(int index,int indexNow);
protected:
    void mousePressEvent(QMouseEvent *ev){
        emit imageItemclicked(_index,_indexNow);
    }
    void paintEvent(QPaintEvent *event){
        QPainter painter(this);
//        painter.drawPixmap(rect(),QPixmap(_path).scaled(60,50));

        painter.setRenderHints(QPainter::HighQualityAntialiasing |
                                QPainter::SmoothPixmapTransform |
                                QPainter::Antialiasing);

        QRect backgroundRect = rect();
        QRect pixmapRect;
        if (_index == _indexNow)
        {
            QPainterPath backgroundBp;
            backgroundBp.addRoundedRect(backgroundRect, 8, 8);
            painter.setClipPath(backgroundBp);

            painter.fillRect(backgroundRect, QBrush(QColor("#2CA7F8")));

//            QPixmap selectedPixmap;
//            selectedPixmap = utils::base::renderSVG(":/resources/images/other/photo_checked.svg", QSize(data.width, data.height));

//            painter->drawPixmap(backgroundRect, selectedPixmap);
            pixmapRect.setX(backgroundRect.x()+4);
            pixmapRect.setY(backgroundRect.y()+4);
            pixmapRect.setWidth(backgroundRect.width()-8);
            pixmapRect.setHeight(backgroundRect.height()-8);
        }else {
            pixmapRect.setX(backgroundRect.x()+1);
            pixmapRect.setY(backgroundRect.y()+0);
            pixmapRect.setWidth(backgroundRect.width()-2);
            pixmapRect.setHeight(backgroundRect.height()-0);
        }

        QPainterPath bg_white;
        bg_white.addRoundedRect(pixmapRect, 4, 4);
        painter.setClipPath(bg_white);
        painter.fillRect(pixmapRect, QBrush(QColor("#FFFFFF")));

        QPainterPath bp1;
        bp1.addRoundedRect(pixmapRect, 4, 4);
        painter.setClipPath(bp1);

//        QPixmap pixmapItem;
//        pixmapItem.load(_path);

        painter.drawPixmap(pixmapRect, _pixmap);

    };
private:
    int _index;
    int _indexNow = -1;
    DLabel *_image=nullptr;
    QString _path = NULL;
    QPixmap _pixmap;
    DSpinner* m_spinner;

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
    void setImage(const QString &path,DBImgInfoList infos);
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

    DIconButton* m_adaptImageBtn;
    DIconButton* m_adaptScreenBtn;
//    DIconButton* m_clBT;
    DIconButton* m_rotateLBtn;
    DIconButton* m_rotateRBtn;
    DIconButton* m_trashBtn;
    DIconButton *m_preButton;
    DIconButton *m_nextButton;
    ElidedLabel* m_fileNameLabel;
    DWidget *m_imgList;
    QHBoxLayout *m_imglayout;
    DWidget *m_imgListView;
    DWidget *m_imgListView_prespc;
    DWidget *m_imgListView_spc;
    DWidget *m_preButton_spc;
    DWidget *m_nextButton_spc;
    DBImgInfoList m_imgInfos ;
    QString m_imagePath;
    int m_windowWidth;
    int m_contentWidth;
    int m_lastIndex = 0;
    int m_nowIndex = 0;
};

#endif // TTLCONTENT_H
