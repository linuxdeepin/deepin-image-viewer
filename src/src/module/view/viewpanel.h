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
#ifndef VIEWPANEL_H
#define VIEWPANEL_H

#include "controller/dbmanager.h"
#include "controller/viewerthememanager.h"
#include "danchors.h"
#include "lockwidget.h"
#include "module/modulepanel.h"
#include "thumbnailwidget.h"
#include "contents/ttbcontent.h"
#include "contents/ttlcontent.h"

#include <DDesktopServices>
#include <DFileWatcher>
#include <DMenu>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonObject>
#include <QReadWriteLock>
#include <QTimer>
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
//初始加载张数
#define LOAD_NUMBER 100

class ImageButton;
class ImageInfoWidget;
class ImageView;
class ImageWidget;
class NavigationWidget;
class QFileSystemWatcher;
class SlideEffectPlayer;
class QTimer;

#include <DLabel>
#include <DStackedWidget>

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;
typedef DStackedWidget QSWToDStackedWidget;

class ViewPanel : public ModulePanel
{

    Q_OBJECT
public:
    enum MenuItemId {
        IdFullScreen,
        IdExitFullScreen,
        IdStartSlideShow,
        IdRename,
        IdPrint,
        IdAddToAlbum,
        IdCopy,
        IdMoveToTrash,
        IdRemoveFromTimeline,
        IdRemoveFromAlbum,
        IdAddToFavorites,
        IdRemoveFromFavorites,
        IdShowNavigationWindow,
        IdHideNavigationWindow,
        IdRotateClockwise,
        IdRotateCounterclockwise,
        IdSetAsWallpaper,
        IdDisplayInFileManager,
        IdImageInfo,
        IdSubMenu,
        IdDraw
    };

    explicit ViewPanel(QWidget *parent = nullptr);

    QString moduleName() Q_DECL_OVERRIDE;
    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *bottomTopLeftContent();
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;
    int getPicCount()
    {
        return m_infos.count();
    }
    bool getPicExict()
    {
        return !QFileInfo(m_infos.first().filePath).exists();
    }
    QMenu * getMenu()
    {
        return m_menu;
    }
    /**
    * @brief clearMenu  清空右键菜单
    */
    void clearMenu();
//    void AddDataToList(LOAD_DIRECTION Dirction, int pages = 30);

//    /**
//     * @brief getPathsFromCurrent   获取当前位置和上一张和下一张图片路径
//     * @param nCurrent              当前图片的位置
//     * @return
//     */
//    QStringList getPathsFromCurrent(int nCurrent);

//    /**
//    * @brief refreshPixmap  点击缩略图刷新图片
//    * @param strPath        缩略图路径
//    */
//    void refreshPixmap(QString strPath);

signals:
    void updateCollectButton();
    void imageChanged(const QString &path, DBImgInfoList infos);
    void viewImageFrom(QString dir);
    void mouseMoved();
    void updateTopLeftWidthChanged(int width);
    void updateTopLeftContentImage(const QString &path);
    void updatePath();

    /**
     * @brief sendLoadOver  加载完成信号
     * @param infos         加载完成的图片信息集合
     * @param nCurrent      当前需要显示的图片索引
     */
    void sendLoadOver(DBImgInfoList infos, int nCurrent);

    /**
     * @brief changeHideFlag    改变隐藏属性信号
     * @param bFlags            true为隐藏，false为不隐藏
     */
    void changeHideFlag(bool bFlags);

    /**
     * @brief hidePreNextBtn    置灰上一张下一张按钮
     * @param bShowAll          bShowAll表示是否显示全部左右按钮
     * @param bFlag             false表示第一张，true最后一张
     */
    void hidePreNextBtn(bool bShowAll, bool bFlag);

    /**
     * @brief sendAllImageInfos 后台加载完成之后将所有图片信息发送给操作控件
     * @param allInfos          所有信息集合
     */
    void sendAllImageInfos(DBImgInfoList allInfos);

    void changeitempath(int, QString);

    void SetImglistPath(int, QString, QString);

    /**
     * @brief sigResize     通知重置大小信号
     */
    void sigResize();

    /**
     * @brief disableDel    未完全加载完图片信息时禁止删除图片
     * @param bFlags        true为禁用，false为不禁用
     */
    void disableDel(bool bFlags);

    /**
     * @brief sendLoadAddInfos  发送需要加载的信息，向前或者向后
     * @param allInfos          动态加载的图片信息
     * @param bFlags            true为头部加载，false尾部加载
     */
    void sendLoadAddInfos(DBImgInfoList allInfos, bool bFlags);

    /**
     * @brief sendDynamicLoadPaths  发送动态加载路径,包括加载大图
     * @param paths                 需要动态加载的图片路径
     */
    void sendDynamicLoadPaths(QStringList paths);

    /**
     * @brief sigsendslideshowlist  发送新的list到slideshow
     * @param bflag
     * @param infosldeshow
     */
    void sigsendslideshowlist(bool bflag, DBImgInfoList infosldeshow);

    /**
     * @brief sigStopshowThread
     * 停止滑动缩略图加载原图线程
     */
    void sigStopshowThread();

    void sigDisenablebutton();

protected:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) override;

private:
    /**
     * @brief initConnect   初始化信号连接
     */
    void initConnect();
    /**
     * @brief initConnectOpenImage   启动时间优化，单独拿出filedialog的信号槽函数
     * @author lmh
     * @date   0722
     */
    void initConnectOpenImage();

    void initFileSystemWatcher();

    /**
     * @brief initPopupMenu     初始化右键弹出菜单
     */
    void initPopupMenu();

    /**
     * @brief initShortcut      初始化快捷键
     */
    void initShortcut();

    /**
     * @brief initStack         初始化堆叠窗口
     */
    void initStack();

    /**
     * @brief initViewContent   初始化视图区域信号连接
     */
    void initViewContent();

    void popupDelDialog(const QString path);
    void popupPrintDialog(const QString path);

    // Floating component
    void initFloatingComponent();
    void initScaleLabel();
    void initNavigation();

    // Menu control
    /**
     * @brief appendAction  添加右键菜单按钮
     * @param id            按钮枚举ID
     * @param text          按钮名称
     * @param shortcut      按钮快捷键
     */
    void appendAction(int id, const QString &text, const QString &shortcut = "");
#ifndef LITE_DIV
    DMenu *createAlbumMenu();
    QFileInfoList getFileInfos(const QString &path);
    void viewOnNewProcess(const QStringList &paths);
#endif

    /**
     * @brief onMenuItemClicked     点击右键菜单
     * @param action                被点击的按钮
     */
    void onMenuItemClicked(QAction *action);

    /**
     * @brief updateMenuContent 更新右键菜单
     */
    void updateMenuContent();

    // View control
    /**
     * @brief LoadDirPathFirst  加载当前文件夹图片路径，并判断是否是第一次加载
     * @param bLoadAll          false为第一次，true为加载所有
     */
    void LoadDirPathFirst(bool bLoadAll = false);

    /**
     * @brief onViewImage   打开选中的图片
     * @param vinfo         选中的图片信息
     */
    void onViewImage(const SignalManager::ViewInfo &vinfo);

    /**
     * @brief openImage     打开并显示选中图片
     * @param path          图片路径
     * @param inDB          是否链接数据库
     */
    void openImage(const QString path, bool inDB = true);

    /**
     * @brief removeCurrentImage    删除当前的图片
     * @return                      成功返回true，失败返回false
     */
    bool removeCurrentImage();

    bool removeImagePath(QString path);

    /**
     * @brief rotateImage   旋转图片
     * @param clockWise     true为顺时针旋转，false为逆时针旋转
     */
    void rotateImage(bool clockWise);

    /**
     * @brief showNext  显示下一张图片
     * @return
     */
    bool showNext();

    /**
     * @brief showPrevious  显示上一张
     * @return
     */
    bool showPrevious();

    /**
     * @brief showImage  根据索引值显示图片
     * @param index      传入的索引值
     * @param addIndex   传入的值与现在的值之差
     * @return
     */
    bool showImage(int index, int addIndex);

    // Geometry
    /**
     * @brief toggleFullScreen  全屏切换
     */
    void toggleFullScreen();

    /**
     * @brief showNormal    正常显示
     */
    void showNormal();

    /**
     * @brief showFullScreen    全屏显示
     */
    void showFullScreen();


    /**
     * @brief backToLastPanel
     * quit slideshow
     */
    void backToLastPanel();

    /**
     * @brief imageIndex    图元索引值
     * @param path          图片路径
     * @return              该图元的索引值
     */
    int imageIndex(const QString &path);


    /**
     * @brief getImageInfos     根据传入图片信息转换为详细信息
     * @param infos             传入的图片信息
     * @return                  详细图片信息
     */
    DBImgInfoList getImageInfos(const QFileInfoList &infos);

    /**
     * @brief paths 当前显示图片的路径集合
     * @return      返回当前所有图片路径
     */
//    const QStringList paths() const;
    const QStringList slideshowpaths() const;

    /**
     * @brief PopRenameDialog   重命名窗口处理函数
     * @param filepath          图片路径
     * @param filename          图片名称
     * @return                  成功返回true，失败返回false
     */
    bool PopRenameDialog(QString &filepath, QString &filename);
    void startFileWatcher();

//    /**
//     * @brief disconnectTTbc   断开ttbc所有的信号连接
//     */
//    void disconnectTTbc();

//    /**
//     * @brief reConnectTTbc     重新连接TTBC工具栏所有信号
//     */
//    void reConnectTTbc();
    /**
     * @brief GetPixmapStatus
     * whether the picture is loaded successfully
     * @param filename
     * picture name
     * @return
     */
    bool GetPixmapStatus(QString filename);

private slots:
    /**
     *
     * @date 2020/08/28
     * @auther lmh
     * @brief 设置第几个栈窗体
     */
    void slotCurrentStackWidget(QString &path,bool bpix);
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
#ifndef LITE_DIV
    void updateLocalImages();
    /**
     * @brief newEatImageDirIterator    备份初始信息读取代码
     */
    void newEatImageDirIterator();
    /**
     * @brief eatImageDirIterator   迭代获取当前文件夹所有图片信息
     */
    void eatImageDirIterator();
#endif

    /**
     * @brief sendSignal    发送显示缩略图的信号
     * @param infos         当前需要显示缩略图信息集合
     * @param nCurrent      当前显示图片索引号
     */
    void sendSignal(DBImgInfoList infos, int nCurrent);

    /**
     * @brief recvLoadSignal    接受向前加载或者向后加载信号
     * @param bFlags            true为头部加载，false为尾部加载
     */
    void recvLoadSignal(bool bFlags);

    /**
     * @brief slotExitFullScreen    退出全屏
     */
    void slotExitFullScreen();
    //void slotLoadSlideshow(bool bflag);





    /**
     * @brief eatImageDirIteratorThread 线程函数，遍历选中文件夹获取所有图片信息
     */
    void eatImageDirIteratorThread();

    /**
     * @brief LoadFrontThumbnailsAndClearTail
     * Load front thumbnails and clear up tail thumbnails
     */
    void SlotLoadFrontThumbnailsAndClearTail();

    /**
     * @brief slotGetLastThumbnailPath
     * get last thumbnail path
     * @param path
     * last thumbnail path
     */
    void slotGetLastThumbnailPath(QString &path);

    /**
     * @brief slotLoadTailThumbnailsAndClearFront
     * Load tail thumbnails and clear up front thumbnails
     */
    void slotLoadTailThumbnailsAndClearFront();

    /**
     * @brief slotGetFirstThumbnailPath
     * get first thumbnail
     * @param path
     * first thumbnail path
     */
    void slotGetFirstThumbnailPath(QString &path);

    /**
     * @brief slotUpdateImageView
     * 拖动缩略图的时候判断是否需要切换stack窗口
     * @param path
     * 切换的路径
     */
    void slotUpdateImageView(QString &path);

    void slotThumbnailContainPath(QString path, bool &b);
private:
    int m_hideCursorTid;
    bool m_isInfoShowed;
    bool m_isMaximized;

    bool m_printDialogVisible = false;
    int m_topLeftContentWidth = 0;
    ImageView *m_viewB {nullptr};
    ImageInfoWidget *m_info {nullptr};
    ThumbnailWidget *m_emptyWidget = nullptr;
    DMenu *m_menu{nullptr};
    QSWToDStackedWidget *m_stack {nullptr};
    LockWidget *m_lockWidget;
    TTBContent *ttbc = nullptr;
    // Floating component
    DAnchors<NavigationWidget> m_nav;

    SignalManager::ViewInfo m_vinfo;
    DBImgInfoList m_infos;
    //abandonment value
    DBImgInfoList m_infoslideshow;
    DBImgInfoList m_infosadd;
    DBImgInfoList m_infosHead;
    DBImgInfoList m_infosTail;
    //heyi test 优化新增后台加载所有图片信息结构体。
    DBImgInfoList m_infosAll;
    //    DBImgInfoList::ConstIterator m_current =NULL;
    int m_current = 0;
    //存储上一次图片位置
    int m_lastCurrent = 0;
    int m_firstindex = 0;
    int m_lastindex = 0;
    QFileInfoList m_AllPath;
    bool m_CollFileFinish = false;
#ifdef LITE_DIV
    QScopedPointer<QDirIterator> m_imageDirIterator;
#endif
    QString m_currentImageLastDir = "";
    //当前图片路径
    QString m_currentImagePath = "";
    DFileWatcher *m_fileManager;
    //当前选中文件夹路径
    QString m_currentFilePath = "";
    bool m_finish = false;

    QScrollArea *m_scrollArea {nullptr};

    int          m_fileNum = 0;
    QTimer       m_timer;
    QReadWriteLock m_rwLock;
    volatile bool m_bIsFirstLoad = true;
    //第一次开机是否加载完成
    volatile bool m_bFinishFirstLoad = false;
    //是否允许删除
    volatile bool m_bAllowDel = false;
    //排除不支持格式
    QStringList m_nosupportformat;
    //程序关闭时线程退出标志
    volatile bool m_bThreadExit = false;
    //LMH延时Remove
    QTimer *m_dtr = nullptr;

    //lmh0729判断是否判断打开图片与上一张是否相同
    bool m_bIsOpenPicture=true;
    bool m_bOnlyOneiImg=false;

    bool m_screentoNormal=false;
    QTimer* m_tSaveImage = nullptr;//保存旋转图片定时器

};
#endif  // VIEWPANEL_H
