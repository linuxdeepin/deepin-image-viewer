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
#include <QTimer>
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
#include <DGuiApplicationHelper>
#include <DLabel>
#include "iconbutton.h"
#include <DLabel>

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;
//#define HEYI

class PushButton;
class ReturnButton;
class ElidedLabel;
class QAbstractItemModel;
//class DImageButton;
class ImageButton;
class MyImageListWidget;

enum LOAD_DIRECTION {
    LOAD_LEFT = 0,  //向左加载
    LOAD_RIGHT,     //向右加载
    NOT_LOAD        //不加载
};
class ImageItem : public DLabel
{
    Q_OBJECT
public:
    ImageItem(int index = 0, QString path = NULL, char *imageType = NULL, QWidget *parent = 0);

    /**
     * @brief setIndexNow   设置当前显示图片索引位置
     * @param i             索引标识号
     */
    void setIndexNow(int i)
    {
        _indexNow = i;

    }

    void setPic(QImage image)
    {
        _image->setPixmap(QPixmap::fromImage(image.scaled(60, 50)));
    }

    /**
     * @brief updatePic     更新图元缩略图
     * @param pixmap        新传入的缩略图
     */
    void updatePic(QPixmap pixmap)
    {
        _pixmap = pixmap;
        update();
    }

    /**
     * @brief setIndex  设置当前图元索引位置
     * @param index     传入的索引值
     */
    void setIndex(int index)
    {
        _index = index;
    }

    /**
     * @brief SetPath   设置当前图元路径
     * @param path      传入的路径
     */
    void SetPath(QString path)
    {
        _path = path;
    }

    /**
     * @brief getPath   获取当前图元路径
     * @return          返回当前图元路径
     */
    inline QString getPath()
    {
        return _path;
    }

    /**
     * @brief getIndex  获取当前图元索引
     * @return          返回当前图元索引
     */
    inline int getIndex()
    {
        return _index;
    }
    /**
     * @brief getPixmap
     * 获取pixmap
     * @return
     */
    QPixmap getPixmap()
    {
        return _pixmap;
    }

    void emitClickSig(int index);
    void emitClickEndSig();
signals:
    void imageItemclicked(int index, int indexNow);
    void imageMoveclicked(int index);
protected:
    void mouseReleaseEvent(QMouseEvent *ev) override
    {
        ev->x();
        bmouserelease = true;
    }

    void mousePressEvent(QMouseEvent *ev) override
    {
        ev->x();
        bmouserelease = false;
        QEventLoop loop;
        QTimer::singleShot(200, &loop, SLOT(quit()));
        loop.exec();
        if (bmouserelease)
            emit imageItemclicked(_index, _indexNow);
    }

    void paintEvent(QPaintEvent *event) override;

private:
    int _index;
    int _indexNow = -1;
    DLabel *_image = nullptr;
    QString _path;
    QPixmap _pixmap;
    DSpinner *m_spinner;
    QString m_pixmapstring;
    bool bFirstUpdate = true;
    bool bmouserelease = false;

};
class TTBContent : public QLbtoDLabel
{
    Q_OBJECT
public:
    explicit TTBContent(bool inDB, DBImgInfoList m_infos, QWidget *parent = nullptr);

    /**
     * @brief setWindoeSize 缩略图窗口大小设置
     * @param inDB          是否连接数据库
     * @param m_infos       需要显示的缩略图信息集合
     * @param parent        缩略图控件的父对象
     */
    void setWindoeSize(bool inDB, DBImgInfoList m_infos, QWidget *parent);

    /**
     * @brief initBtn   生成工具栏按钮
     */
    void initBtn();

    /**
     * @brief toolbarSigConnection 工具栏信号连接
     */
    void toolbarSigConnection();


signals:
    void clicked();

    /**
     * @brief imageClicked  点击图元触发
     * @param index         被点击图元的索引号
     * @param addIndex      点击的图元与之前显示图元索引值之差
     */
    void imageClicked(int index, int addIndex);
    /*lmh0731*/
    void imageMoveEnded(int index, int addIndex,bool iRet);
    void resetTransform(bool fitWindow);

    /**
     * @brief rotateClockwise   顺时针旋转
     */
    void rotateClockwise();

    /**
     * @brief rotateCounterClockwise    逆时针旋转
     */
    void rotateCounterClockwise();

    void removed();
    void imageEmpty(bool v);
    void contentWidthChanged(int width);

    /**
     * @brief showPrevious  显示上一张图片
     */
    void showPrevious();

    /**
     * @brief showNext      显示下一张图片
     */
    void showNext();

    void ttbcontentClicked();

    void showvaguepixmap(QPixmap,QString path=nullptr);

public slots:
    void setCurrentDir(QString text);

    /**
     * @brief setImage  显示加载缩略图
     * @param path      当前选中显示图片路径
     * @param infos     当前需要显示的所有缩略图信息集合
     */
    void setImage(const QString path, DBImgInfoList infos);

    void updateCollectButton();

    /**
     * @brief slotTheme     系统主题更改
     * @param theme         更改的主题样式
     */
    void slotTheme(bool theme);
    void setAdaptButtonChecked(bool flag);
    void disCheckAdaptImageBtn();
    void checkAdaptImageBtn();

    /**
     * @brief onChangeHideFlags     接收到信号之后更改隐藏标志符号
     * @param bFlags                true为隐藏，false不隐藏
     */
    void onChangeHideFlags(bool bFlags);

    /**
     * @brief loadBack      动态向后加载
     * @param infos         加载的图片信息集合
     */
    void loadBack(DBImgInfoList infos);

    /**
     * @brief loadFront     动态向前加载
     * @param infos         加载的图片信息集合
     */
    void loadFront(DBImgInfoList infos);

    /**
     * @brief ReInit
     * clear thumbnails and Load front thumbnails
     * @param infos
     * Loaded files list
     */
    void ReInitFirstthumbnails(DBImgInfoList infos);

    /**
     * @brief receveAllIamgeInfos   接收加载完毕之后的所有图片信息
     * @param AllImgInfos           所有图片信息集合
     */
    void receveAllIamgeInfos(DBImgInfoList AllImgInfos);

    /**
     * @brief onHidePreNextBtn  置灰上一张下一张按钮
     * @param bShowAll          表示是否显示全部左右按钮，true为全部显示，false为不全部显示
     * @param bFlag             false表示第一张，true最后一张
     */
    void onHidePreNextBtn(bool bShowAll, bool bFlag);

    /**
     * @brief OnChangeItemPath  重命名改变itemImage路径
     * @param currindex         当前图片的索引
     * @param path              图片的新路径
     */
    void OnChangeItemPath(int currindex, QString path);

    /**
     * @brief OnSetimglist  重命名改变m_imgInfos路径
     * @param currindex     当前图片索引号
     * @param filename      更改后的图片名称
     * @param filepath      更好后的图片路径
     */
    void OnSetimglist(int currindex, QString filename, QString filepath);

    /**
     * @brief onResize  重置缩略图控件大小
     */
    void onResize();

    /**
     * @brief delPictureFromPath    根据路径从布局中删除指定的图片
     * @param strPath               被删除的图片路径
     * @param infos                 删除后的图片信息集合
     * @param nCurrent              当前选中位置
     * @return                      true为删除成功，false为删除失败
     */
    bool delPictureFromPath(QString strPath, DBImgInfoList infos, int nCurrent);

    /**
     * @brief setIsConnectDel   设置删除按钮信号是否连接
     * @param bFlasg            true连接，false断开连接
     */
    void setIsConnectDel(bool bFlasg);

    /**
     * @brief disableDelAct     第一次加载100张时禁止使用删除按钮,按钮置灰色
     * @param bFlags            true删除按钮可用，false为删除按钮置灰
     */
    void disableDelAct(bool bFlags);

    /**
     * @brief recvLoadAddInfos  发送需要加载的信息，向前或者向后
     * @param allInfos          新加载的图片信息集合
     * @param bFlags            true为头部加载，false尾部加载
     */
    void recvLoadAddInfos(DBImgInfoList allInfos, bool bFlags);

    /**
     * @brief judgeReloadItem   根据传入的图片信息数量判断是否需要重新生成新的图元
     * @param inputInfos        传入新的图片信息集合
     * @param localInfos        当前所有图片信息集合
     * @return                  返回true表示需要重新生成，false则不需要重新生成
     */
    bool judgeReloadItem(const DBImgInfoList &inputInfos, DBImgInfoList &localInfos);

    /**
     * @brief reloadItems   根据传入的图片信息生成新的缩略图元
     * @param inputInfos    传入的图片信息集合
     * @param strCurPath    当前需要显示的图片路径
     */
    void reloadItems(DBImgInfoList &inputInfos, QString strCurPath);

    /**
     * @brief showAnimation 显示缩略图动画效果
     */
    void showAnimation();

    /**
     * @brief setBtnAttribute 根据当前图片属性设置按钮属性
     * @param strPath   当前图片路径
     */
    void setBtnAttribute(const QString strPath);

    /**
     * @brief clickLoad 添加点击加载功能
     * @param nCurrent  当前点击的位置
     */
    void clickLoad(const int nCurrent);

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

    DIconButton *m_adaptImageBtn {nullptr};
    DIconButton *m_adaptScreenBtn {nullptr};
//    DIconButton* m_clBT;
    DIconButton *m_rotateLBtn {nullptr};
    DIconButton *m_rotateRBtn {nullptr};
    DIconButton *m_trashBtn {nullptr};
    DIconButton *m_preButton {nullptr};
    DIconButton *m_nextButton {nullptr};
    ElidedLabel *m_fileNameLabel;
    DWidget *m_imgList;
    QHBoxLayout *m_imglayout;
//    DWidget *m_imgListView;
    MyImageListWidget *m_imgListView;
    DWidget *m_imgListView_prespc;
    DWidget *m_imgListView_spc;
    DWidget *m_preButton_spc;
    DWidget *m_nextButton_spc;
    //当前显示的图片信息
    DBImgInfoList m_imgInfos ;
    //所有图片信息
    DBImgInfoList m_AllImgInfos ;
    //第一次100张图片的第一张图片路径
    QString m_strFirstPath;
    //第一次100张图片的最后一张路径
    QString m_strEndPsth;
    QString m_imagePath;
    QString m_strCurImagePath;
    int m_windowWidth;
    int m_contentWidth;
    int m_lastIndex = 0;
    int m_nowIndex = 0;
    int m_imgInfos_size = 0;
    int m_startAnimation = 0;

    /*lmh0728计数*/
    int m_totalImageItem=0;
    //上一次拖动后移动的X坐标
    int m_nLastMove = 0;
    bool bresized = true;
    bool badaptImageBtnChecked = false;
    bool badaptScreenBtnChecked = false;
    //heyi test
    bool m_bIsHide = false;
};


class MyImageListWidget : public DWidget
{
    Q_OBJECT
public:
    MyImageListWidget(QWidget *parent = nullptr);
    bool ifMouseLeftPressed();
    void setObj(QObject *obj);
protected:
    bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
signals:
    void mouseLeftReleased();
private:
    QTimer *m_timer = nullptr;
    bool bmouseleftpressed = false;
    QObject *m_obj = nullptr;
    QPoint m_prepoint;
    QPoint m_lastPoint;
    ImageItem *m_currentImageItem=nullptr;
    bool m_iRet=false;
    QVector <QPoint> m_vecPoint;
    QMutex m_threadMutex;
    bool m_bthreadMutex=false;
};



#endif // TTLCONTENT_H
