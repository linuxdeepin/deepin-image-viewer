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
#include "contents/ttmcontent.h"

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

class ImageButton;
class ImageInfoWidget;
class ImageView;
class ImageWidget;
class NavigationWidget;
class QFileSystemWatcher;
class QLabel;
class QStackedWidget;
class SlideEffectPlayer;
class QTimer;

class ViewPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit ViewPanel(QWidget *parent = 0);

    QString moduleName() Q_DECL_OVERRIDE;
    QWidget *toolbarBottomContent() Q_DECL_OVERRIDE;
    QWidget *toolbarTopLeftContent() Q_DECL_OVERRIDE;
    QWidget *bottomTopLeftContent();
    QWidget *toolbarTopMiddleContent() Q_DECL_OVERRIDE;
    QWidget *extensionPanelContent() Q_DECL_OVERRIDE;
    const SignalManager::ViewInfo viewInfo() const;
    int getPicCount()
    {
        return m_infos.count();
    }
    bool getPicExict()
    {
        return !QFileInfo(m_infos.first().filePath).exists();
    }

signals:
    void updateCollectButton();
    void imageChanged(const QString &path, DBImgInfoList infos);
    void viewImageFrom(QString dir);
    void mouseMoved();
    void updateTopLeftWidthChanged(int width);
    void updateTopLeftContentImage(const QString &path);
    void updatePath();
    //heyi test
    void sendLoadOver(DBImgInfoList infos, int nCurrent);
    void changeHideFlag(bool bFlags);
    //置灰上一张下一张按钮，false表示第一张，true最后一张,bShowAll表示是否显示全部左右按钮
    void hidePreNextBtn(bool bShowAll, bool bFlag);

    void changeitempath(int,QString);
    void SetImglistPath(int,QString,QString);

protected:
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) override;

private:
    void initConnect();
    void initFileSystemWatcher();
    void initPopupMenu();
    void initShortcut();
    void initStack();
    void initViewContent();
    void popupDelDialog(const QString path);
    void popupPrintDialog(const QString path);

    // Floating component
    void initFloatingComponent();
    void initSwitchButtons();
    void initScaleLabel();
    void initNavigation();

    // Menu control
    void appendAction(int id, const QString &text, const QString &shortcut = "");
#ifndef LITE_DIV
    DMenu *createAlbumMenu();
#endif
    void onMenuItemClicked(QAction *action);
    //更新右键菜单
    void updateMenuContent();

    // View control
    void onViewImage(const SignalManager::ViewInfo &vinfo);
    void openImage(const QString &path, bool inDB = true);
    void removeCurrentImage();
    void rotateImage(bool clockWise);
    bool showNext();
    bool showPrevious();
    bool showImage(int index, int addIndex);

    // Geometry
    void toggleFullScreen();
    void showNormal();
    void showFullScreen();

    void viewOnNewProcess(const QStringList &paths);
    void backToLastPanel();

    int imageIndex(const QString &path);
    QFileInfoList getFileInfos(const QString &path);
    DBImgInfoList getImageInfos(const QFileInfoList &infos);
    const QStringList paths() const;
    //重命名窗口处理函数
    bool PopRenameDialog(QString &filepath,QString &filename);
    void startFileWatcher();

private slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

    void updateLocalImages();

    //heyi test  发送显示缩略图的信号
    void sendSignal(DBImgInfoList infos, int nCurrent);

private:
    int m_hideCursorTid;
    bool m_isInfoShowed;
    bool m_isMaximized;

    bool m_printDialogVisible = false;
    int m_topLeftContentWidth = 0;
    ImageView *m_viewB;
    ImageInfoWidget *m_info {nullptr};
    ThumbnailWidget *m_emptyWidget = nullptr;
    DMenu *m_menu;
    QStackedWidget *m_stack {nullptr};
    LockWidget *m_lockWidget;
    TTBContent *ttbc;
    // Floating component
    DAnchors<NavigationWidget> m_nav;

    SignalManager::ViewInfo m_vinfo;
    DBImgInfoList m_infos;
    //heyi test
    DBImgInfoList m_infosFirst;
    //    DBImgInfoList::ConstIterator m_current =NULL;
    int m_current = 0;
#ifdef LITE_DIV
    QScopedPointer<QDirIterator> m_imageDirIterator;

    void eatImageDirIterator();
    //heyi add
    void newEatImageDirIterator();
#endif
    QString m_currentImageLastDir = "";
    QString m_currentImagePath = "";
    DFileWatcher *m_fileManager;
    QString m_currentFilePath = "";
    bool m_finish = false;

    QScrollArea *m_scrollArea {nullptr};

    int          m_fileNum = 0;
    QTimer       m_timer;
    QReadWriteLock m_rwLock;
    volatile bool m_bIsFirstLoad = true;
    //第一次开机是否加载完成
    volatile bool m_bFinishFirstLoad = false;
};
#endif  // VIEWPANEL_H
