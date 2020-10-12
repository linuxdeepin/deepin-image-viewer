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
#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>
#include "dbmanager.h"

class ModulePanel;
class SignalManager : public QObject
{
    Q_OBJECT
public:
    static SignalManager *instance();
    // For view images
    struct ViewInfo {
        ModulePanel *lastPanel {nullptr};  // For back to the last panel
        int viewMainWindowID = 0;
        bool fullScreen = false;
        bool slideShow = false;
    //#ifndef LITE_DIV
    //        bool inDatabase = true;
    //#else
        static constexpr bool inDatabase = false;
    //#endif
        QString album = QString();
        QString path;                       // Specific current open one
        QStringList paths = QStringList();  // Limit the view range
    };


signals:
    void enableMainMenu(bool enable);
    void updateTopToolbarLeftContent(QWidget *content);
    void updateTopToolbarMiddleContent(QWidget *content);
    void updateBottomToolbarContent(QWidget *content, bool wideMode = false);
    void updateTopToolbar();
    void updateBottomToolbar(bool wideMode = false);
    void updateExtensionPanelContent(QWidget *content);
    void showTopToolbar();
    void hideTopToolbar(bool immediately = false);
    void showBottomToolbar();
    void hideBottomToolbar(bool immediately = false);
    void showExtensionPanel();
    void hideExtensionPanel(bool immediately = false);
    void extensionPanelHeight(int height);
    void sendPathlist(QStringList pathlist, QString path);
    void enterView(bool immediately = false);
    void enterScaledMode(bool immediately = false);
    void isAdapt(bool immediately = false);
    void usbOutIn(bool immediately = false);
    void hideNavigation();
    void picInUSB(bool immediately = false);
    void picNotExists(bool immediately = false);
    void fileDeleted(QString);

    /**
     * @brief fileCreate    被监测的文件夹有新文件加入
     * @param strFilePath   新加入的文件路径
     */
    void fileCreate(QString strFilePath);

    void picOneClear();
    void loadingDisplay(bool immediately = false);
    void picDelete();
    void allPicDelete();
    void changetitletext(QString);

    void gotoTimelinePanel();
    void gotoSearchPanel(const QString &keyWord = "");
    void gotoPanel(ModulePanel *panel);
    void backToMainPanel();
    void activeWindow();
    void showSlidePanel(int index);

    void imagesInserted(const DBImgInfoList infos);
    void imagesRemoved(const DBImgInfoList &infos);

    void editImage(const QString &path);
    void showImageInfo(const QString &path);
    void showInFileManager(const QString &path);
    void startSlideShow(const ViewInfo &vinfo, bool inDB = true);
    void setFirstImg(const QImage &img);
    void LoadSlideShow(bool bFlag);
    void updateButton();
    void updatePauseButton();
    void sigStartTimer();
    void initButton();
    void imagesRemovedPar(const DBImgInfoList &infos);
    void sigESCKeyStopSlide();
    void sigESCKeyActivated();
    void hideSlidePanel();
    void viewImageNoNeedReload(QString &filename/*int &fileindex*/);

    void viewImage(const ViewInfo &vinfo);
    void updateFileName(const QString &fileName);
    void resizeFileName();

    // Handle by album
    void gotoAlbumPanel(const QString &album = "");
    void createAlbum(QStringList imgPath = QStringList());
    void importDir(const QString &dir);
    void insertedIntoAlbum(const QString &album, const QStringList &paths);
    void removedFromAlbum(const QString &album, const QStringList &paths);
    void sigMouseMove();
    void sigShowFullScreen();

    void sigImageOutTitleBar(bool b);

    // Handle By shuwenzhi
    /**
     * @brief sigLoadHeadThunbnail
     * Load first thumbnail by files
     * @param infos
     * thumbnails files
     */
    void sigLoadHeadThunbnail(const DBImgInfoList infos);
    /**
     * @brief sigLoadSlideshow
     * Load first slideshow by files
     */
    void sigLoadfrontSlideshow();
    /**
     * @brief sigNoneedLoadfrontslideshow
     * No need to load slides
     */
    void sigNoneedLoadfrontslideshow();
    /**
     * @brief sendLoadSignal    发送向前加载或者向后加载信号
     * @param bFlags            true为头部加载，false为尾部加载
     */
    void sendLoadSignal(bool bFlags);

    /**
     * @brief sigDrawingBoard
     * open Deepin-Image-Draw
     * @param paths
     * Images Paths
     */
    void sigDrawingBoard(QStringList paths);

    /**
     * @brief sigGetLastThumbnailPath
     * get last thumbnail path
     * @param path
     * last thumbnail path
     */
    void sigGetLastThumbnailPath(QString& path);
    /**
     * @brief sigLoadTailThumbnail
     * Load last thumbnails
     * @param infos
     * thumbnails files
     */
    void sigLoadTailThumbnail();
    /**
     * @brief sigGetFirstThumbnailpath
     * get first thumbnail
     * @param path
     * thumbnail path
     */
    void sigGetFirstThumbnailpath(QString& path);

    /**
     * @brief sigOpenFileDialog
     * 弹出FileDialog信号
     */
    void sigOpenFileDialog();
    /**
     * @brief sigGifImage
     * 发送Qimage更新信号
     * @author lmh
     */
    void sigGifImageRe();

    /**
     * @brief sigUpdateThunbnail
     * 改变缩略图信号
     * @param path
     * 缩略图路径
     */
    void sigUpdateThunbnail(QString path);

    void UpdateNavImg();

private:
    explicit SignalManager(QObject *parent = nullptr);

private:
    static SignalManager *m_signalManager;
};

#endif  // SIGNALMANAGER_H
