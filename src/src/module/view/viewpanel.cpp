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
#include "viewpanel.h"
#include "application.h"
#include "contents/imageinfowidget.h"
#include "contents/ttbcontent.h"
#include "controller/configsetter.h"
#include "controller/divdbuscontroller.h"
#include "controller/signalmanager.h"
#include "navigationwidget.h"
#include "scen/imageview.h"
#include "utils/snifferimageformat.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/imagebutton.h"
#include "widgets/printhelper.h"
#include "widgets/printoptionspage.h"
#include "frame/renamedialog.h"
#include "accessibility/ac-desktop-define.h"

#include <QApplication>
#include <QDebug>
#include <DFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QPixmapCache>
#include <QProcess>
#include <QProxyStyle>
#include <QResizeEvent>
#include <QtConcurrent>
#include <QThread>
#include <QPainter>
#include <DMessageBox>
#include <DRecentManager>
#include <QImageReader>

#include "utils/imageutils.h"

using namespace Dtk::Core;
using namespace Dtk::Widget;
typedef DFileDialog QFDToDFileDialog;

namespace {
//LMH0603删除按键延迟
const int DELAY_DESTROY_TIME=500;
const int DELAY_HIDE_CURSOR_INTERVAL = 3000;
// const QSize ICON_SIZE = QSize(48, 40);

}  // namespace
const int First_Load_Image = 100;
const int Load_Image_Count = 50;
ViewPanel::ViewPanel(QWidget *parent)
    : ModulePanel(parent)
    , m_hideCursorTid(0)
    , m_isInfoShowed(false)
    , m_viewB(nullptr)
    , m_info(nullptr)
    , m_stack(nullptr)
    , m_dtr(nullptr)
{
#ifndef LITE_DIV
    m_vinfo.inDatabase = false;
#endif
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    initStack();
    setObjectName(VIEW_PANEL_WIDGET);
    m_stack->setObjectName(VIEW_PANEL_STACK);
#ifdef OPENACCESSIBLE
    setAccessibleName(VIEW_PANEL_WIDGET);
    m_stack->setAccessibleName(VIEW_PANEL_STACK);
#endif
    /*lmh0722*/
    if(0==dApp->m_timer){
        initFloatingComponent();

        initConnectOpenImage();
        initConnect();
        initShortcut();
#ifndef LITE_DIV
        initFileSystemWatcher();
#endif

        initPopupMenu();

        setAcceptDrops(true);
        setContextMenuPolicy(Qt::CustomContextMenu);
        installEventFilter(this);

        //heyi test
        qRegisterMetaType<DBImgInfoList>("DBImgInfoList");
        m_nosupportformat << "jp2" << "dds" << "psd" << "pcx" << "exr" << "avi" << "ct" << "pict" << "pic";
    }
    else {
        m_stack->setCurrentIndex(1);
        initConnectOpenImage();
        QTimer::singleShot(dApp->m_timer, [=]{
            initFloatingComponent();

            initConnect();
            initShortcut();
#ifndef LITE_DIV
            initFileSystemWatcher();
#endif

            initPopupMenu();

            setAcceptDrops(true);
            setContextMenuPolicy(Qt::CustomContextMenu);
            installEventFilter(this);

            //heyi test
            qRegisterMetaType<DBImgInfoList>("DBImgInfoList");
            m_nosupportformat << "jp2" << "dds" << "psd" << "pcx" << "exr" << "avi" << "ct" << "pict" << "pic";
        });
    }
}

QString ViewPanel::moduleName()
{
    return "ViewPanel";
}

void ViewPanel::initConnect()
{
    //heyi  test
    connect(dApp->signalM, &SignalManager::sigisThumbnailsContainPath, this, &ViewPanel::slotThumbnailContainPath);
    connect(dApp->signalM, &SignalManager::sigGetLastThumbnailPath, this, &ViewPanel::slotGetLastThumbnailPath);
    connect(dApp->signalM, &SignalManager::sigGetFirstThumbnailpath, this, &ViewPanel::slotGetFirstThumbnailPath);
    connect(dApp->signalM, &SignalManager::sigLoadfrontSlideshow, this, &ViewPanel::SlotLoadFrontThumbnailsAndClearTail);
    connect(dApp->signalM, &SignalManager::sigLoadTailThumbnail, this, &ViewPanel::slotLoadTailThumbnailsAndClearFront);
    connect(this, &ViewPanel::sendLoadOver, this, &ViewPanel::sendSignal, Qt::QueuedConnection);
    connect(dApp, &Application::endThread, this, [ = ]() {
        m_bThreadExit = true;
    });

    connect(dApp, &Application::dynamicLoadFinished, this, [ = ]() {
        //开启延时删除标志定时器
        connect(&m_timer, &QTimer::timeout, this, [ = ]() {
            m_timer.stop();
            if(ttbc)
            {
                ttbc->setIsConnectDel(true);
                m_bAllowDel = true;
                ttbc->disableDelAct(true);
            }
        });

        m_timer.start(2000);
    });

    connect(this, &ViewPanel::sendDynamicLoadPaths, dApp, &Application::loadPixThread,Qt::QueuedConnection);
    connect(dApp->signalM, &SignalManager::updateFileName, this, [ = ](const QString & filename) {
        if (filename != "") {
            m_finish = true;
        } else {
            m_finish = false;
        }
    });

    connect(dApp->signalM, &SignalManager::gotoPanel, this, [ = ](ModulePanel * p) {
        if (p != this) {
            emit dApp->signalM->showTopToolbar();
        } else {
            emit dApp->signalM->showTopToolbar();
            emit dApp->signalM->hideBottomToolbar(true);
            //            emit dApp->signalM->showBottomToolbar();
        }
    });
    connect(dApp->signalM, &SignalManager::showExtensionPanel, this,
            [ = ] { m_isInfoShowed = true; });
    connect(dApp->signalM, &SignalManager::hideExtensionPanel, this,
            [ = ] { m_isInfoShowed = false; });

    qRegisterMetaType<SignalManager::ViewInfo>("SignalManager::ViewInfo");
    connect(dApp->signalM, &SignalManager::viewImageNoNeedReload,
    this, [ = ](QString & filename) {
//        emit imageChanged(filename);
//        openImage(filename);
        int fileindex = imageIndex(filename);
        showImage(fileindex, 0);
    });
    connect(dApp->signalM, &SignalManager::viewImage, this,
    [ = ](const SignalManager::ViewInfo & vinfo) {
        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(),
                                                       (m_infos.size() > 1));
        emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());

        onViewImage(vinfo);
        if (nullptr == vinfo.lastPanel) {
            return;
        } else if (vinfo.lastPanel->moduleName() == "AlbumPanel" ||
                   vinfo.lastPanel->moduleName() == "ViewPanel") {
            m_currentImageLastDir = vinfo.album;
            emit viewImageFrom(vinfo.album);
        } else if (vinfo.lastPanel->moduleName() == "TimelinePanel") {
            m_currentImageLastDir = tr("Timeline");
            emit viewImageFrom(tr("Timeline"));
        }
        // TODO: there will be some others panel
    });

#ifndef LITE_DIV
    connect(dApp->signalM, &SignalManager::removedFromAlbum, this,
    [ = ](const QString & album, const QStringList & paths) {
        if (!isVisible() || album != m_vinfo.album || m_vinfo.album.isEmpty()) {
            return;
        }
        foreach (QString path , paths) {
            if (imageIndex(path) == imageIndex(m_current->filePath)) {
                removeCurrentImage();
            }
        }
    });
#endif
    connect(dApp->signalM, &SignalManager::imagesRemoved, this, [ = ](const DBImgInfoList & infos) {
        if (m_infos.length() > 0) {
            removeCurrentImage();
            ttbc->setIsConnectDel(true);
            m_bAllowDel = true;
            ttbc->disableDelAct(true);
        }

        infos.size();
        //add by heyi 判断当前图片是否被旋转
        m_viewB->rotatePixCurrent();
        updateMenuContent();
    });
    connect(m_viewB, &ImageView::mouseHoverMoved, this, &ViewPanel::mouseMoved);
    connect(m_viewB, &ImageView::sigUpdateImageView, this, &ViewPanel::slotUpdateImageView);
    connect(this, &ViewPanel::sigStopshowThread, m_viewB, &ImageView::SlotStopShowThread);
    connect(m_emptyWidget, &ThumbnailWidget::mouseHoverMoved, this, &ViewPanel::mouseMoved);
    //接受信号管理器信号，打开FileDialog
    connect(dApp->signalM, &SignalManager::sigOpenFileDialog, this, [=] {
        emit m_emptyWidget->openImageInDialog();
    });
#ifdef LITE_DIV

    //LMH
    if (nullptr == m_dtr)
    {
        m_dtr = new QTimer(this);
        m_dtr->setSingleShot(true);
        m_dtr->setInterval(DELAY_DESTROY_TIME);
    }
#endif
}

void ViewPanel::initConnectOpenImage()
{
    connect(m_emptyWidget, &ThumbnailWidget::previousRequested, this, &ViewPanel::showPrevious);
    connect(m_emptyWidget, &ThumbnailWidget::nextRequested, this, &ViewPanel::showNext);
    connect(m_lockWidget, &LockWidget::previousRequested, this, &ViewPanel::showPrevious);
    connect(m_lockWidget, &LockWidget::nextRequested, this, &ViewPanel::showNext);
    connect(m_emptyWidget, &ThumbnailWidget::openImageInDialog, this, [this] {
        QString filter = tr("All images");

        filter.append('(');
        filter.append(utils::image::supportedImageFormats().join(" "));
        filter.append(')');

        static QString cfgGroupName = QStringLiteral("General"),
        cfgLastOpenPath = QStringLiteral("LastOpenPath");
        QString pictureFolder = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        QDir existChecker(pictureFolder);
        if (!existChecker.exists())
        {
            pictureFolder = QDir::currentPath();
        }
        pictureFolder =
        dApp->setter->value(cfgGroupName, cfgLastOpenPath, pictureFolder).toString();

        const QStringList &image_list =
        QFDToDFileDialog::getOpenFileNames(this, tr("Open Image"), pictureFolder, filter, nullptr,
                                           QFDToDFileDialog::HideNameFilterDetails);

        if (image_list.isEmpty())
            return;

        SignalManager::ViewInfo vinfo;

        vinfo.path = image_list.first();
        vinfo.paths = image_list;

        QFileInfo firstFileInfo(vinfo.path);
        dApp->setter->setValue(cfgGroupName, cfgLastOpenPath, firstFileInfo.path());

        emit dApp->signalM->enterView(true);
        qDebug() << "emit dApp->signalM->enterView(true)..................m_emptyWidget";
        qDebug() << "加载到onViewImage前，viewpanel.cpp 205行";

        onViewImage(vinfo);
    });
}

#ifndef LITE_DIV
void ViewPanel::initFileSystemWatcher()
{
    // Watch the local file changed if it open from file manager
    QFileSystemWatcher *sw = new QFileSystemWatcher(this);
    connect(dApp->signalM, &SignalManager::viewImage, this,
    [ = ](const SignalManager::ViewInfo & info) {
        if (!sw->directories().isEmpty()) {
            sw->removePaths(sw->directories());
        }

        if (!info.inDatabase) {
            sw->addPath(QFileInfo(info.path).dir().absolutePath());
            sw->addPath(QFileInfo(info.path).absolutePath());
        }
    });
    connect(sw, &QFileSystemWatcher::directoryChanged, this, [ = ] {
        if (m_current == m_infos.cend() || m_infos.isEmpty())
            return;
        updateLocalImages();
    });
    connect(sw, &QFileSystemWatcher::fileChanged, this, [ = ] {
        if (m_current == m_infos.cend() || m_infos.isEmpty())
            return;
        updateLocalImages();
    });
}
#endif


//void ViewPanel::AddDataToList(LOAD_DIRECTION Dirction, int pages)
//{
//    DBImgInfo info;
//    if (Dirction == LOAD_LEFT) {
//        for (; m_firstindex < m_infosAll.size(); m_firstindex++) {
//            if (m_infos.at(m_firstindex).fileName == m_infosAll.at(m_firstindex).fileName) {
//                break;
//            }
//        }
//        if (m_firstindex - pages < 0)
//            m_firstindex = 0;
//        else
//            m_firstindex -= pages;
//    }
//}

//QStringList ViewPanel::getPathsFromCurrent(int nCurrent)
//{
//    QStringList pathsList;
//    if (nCurrent - 1 >= 0) {
//        pathsList.append(m_infos.at(m_current - 1).filePath);
//    }

//    if (nCurrent + 1 <= m_infos.size() - 1) {
//        pathsList.append(m_infos.at(m_current + 1).filePath);
//    }
//    if (m_infos.size() > m_current)
//        pathsList.append(m_infos.at(m_current).filePath);

//    return pathsList;
//}

//void ViewPanel::refreshPixmap(QString strPath)
//{
//    QMutexLocker locker(&dApp->getRwLock());
//    if (strPath.isEmpty()) {
//        return;
//    }
//    //dApp->getRwLock().lockForWrite();
//    QPixmap pixmap(strPath);
//    dApp->m_imagemap.insert(strPath, pixmap.scaledToHeight(100,  Qt::FastTransformation));
//   // dApp->getRwLock().unlock();

//    emit dApp->finishLoadSlot(strPath);

//}

bool ViewPanel::PopRenameDialog(QString &filepath, QString &filename)
{
    RenameDialog *renamedlg =  new RenameDialog(filepath,this);
    killTimer(m_hideCursorTid);
    m_hideCursorTid = 0;
    m_viewB->viewport()->setCursor(Qt::ArrowCursor);
#ifndef USE_TEST
    if (renamedlg->exec()) {
#else
    if (true) {
#endif
        //点击DTK的关闭按钮返回的是QDialog::Aceepted
        if (!m_menu||!m_menu->isVisible()) {
            m_viewB->viewport()->setCursor(Qt::BlankCursor);
        }
        m_hideCursorTid = startTimer(DELAY_HIDE_CURSOR_INTERVAL);
        //重命名从窗口确定后修改文件名词并修改窗口标题
        QFile file(filepath);
        filepath = renamedlg->GetFilePath();
        filename = renamedlg->GetFileName();
        bool bOk = file.rename(filepath);
        if (bOk)
            emit dApp->signalM->updateFileName(renamedlg->GetFileName());
        return bOk;
    }else {
        if (!m_menu||!m_menu->isVisible()) {
            m_viewB->viewport()->setCursor(Qt::BlankCursor);
        }
        m_hideCursorTid = startTimer(DELAY_HIDE_CURSOR_INTERVAL);
    }
    return false;
}

//#include <QFileSystemWatcher>
void ViewPanel::startFileWatcher()
{
    if (m_currentFilePath == "") {
        qDebug() << "startFileWatcher return";
        return;
    }

    m_fileManager = new DFileWatcher(m_currentFilePath, this);
//    QFileSystemWatcher *haha = new QFileSystemWatcher();
//    haha->addPath(m_currentFilePath);
//    connect(haha, &QFileSystemWatcher::fileChanged, this, [ = ](const QString & path) {
//        QString xixi = path;
//    });

//    connect(haha, &QFileSystemWatcher::directoryChanged, this, [ = ](const QString & path) {
//        QString xixi = path;
//    });

    m_fileManager->startWatcher();
    qDebug() << "!!!!!!!!!!!!!!!!!startFileWatcher!!!!!!!!!!!!!!!!!!!!!!!!!!"
             << m_fileManager->startWatcher() << "=" << m_currentFilePath;

    connect(m_fileManager, &DFileWatcher::fileDeleted, this, [ = ](const QUrl & url) {
        qDebug() << "!!!!!!!!!!!!!!!!!FileDeleted!!!!!!!!!!!!!!!!!!!!!!!!!!";
        emit dApp->signalM->fileDeleted(url.path());
    });

    connect(m_fileManager, &DFileWatcher::subfileCreated, this, [ = ](const QUrl & url) {
        qDebug() << "!!!!!!!!!!!!!!!!!subfileCreated!!!!!!!!!!!!!!!!!!!!!!!!!!";
        emit dApp->signalM->fileCreate(url.path());
    });

    connect(dApp->signalM, &SignalManager::fileDeleted, this, [ = ](QString deletedpath) {
        if (!QFileInfo(m_currentImagePath).exists() && m_infos.count() < 2) {
            qDebug() << "fileDeleted";
            emit dApp->signalM->hideNavigation();
            emit dApp->signalM->hideExtensionPanel();
            emit dApp->signalM->picNotExists(false);
            emit dApp->signalM->hideBottomToolbar(true);
            emit dApp->signalM->enterView(true);
            qDebug() << "emit dApp->signalM->enterView(true)..................picClear";
            emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(),
                                                           (m_infos.size() > 1));
            m_stack->setCurrentIndex(1);
            if(m_emptyWidget)
                m_emptyWidget->setThumbnailImage(QPixmap());
            emit dApp->signalM->sigImageOutTitleBar(false);
            emit dApp->signalM->changetitletext("");
        } else {
            removeImagePath(deletedpath);
        }
    });

    connect(dApp->signalM, &SignalManager::picOneClear, this, [ = ]() {
        if (!QFileInfo(m_currentImagePath).exists() && m_infos.count() == 1) {
            qDebug() << "fileDeleted";
            emit dApp->signalM->hideNavigation();
            emit dApp->signalM->hideExtensionPanel();
            emit dApp->signalM->picNotExists(false);
            emit dApp->signalM->hideBottomToolbar(true);
            emit dApp->signalM->enterView(true);
            qDebug() << "emit dApp->signalM->enterView(true)..................picClear";
            emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(),
                                                           (m_infos.size() > 1));
            m_stack->setCurrentIndex(1);
            if(m_emptyWidget)
                m_emptyWidget->setThumbnailImage(QPixmap());
            emit dApp->signalM->sigImageOutTitleBar(false);
        }
    });

}

//void ViewPanel::disconnectTTbc()
//{
//    if (ttbc) {
//        ttbc->disconnect();
//    }
//}

//void ViewPanel::reConnectTTbc()
//{
//    connect(this, &ViewPanel::sigDisenablebutton, ttbc, &TTBContent::DisEnablettbButton, Qt::UniqueConnection);
//    connect(this, &ViewPanel::changeHideFlag, ttbc, &TTBContent::onChangeHideFlags, Qt::UniqueConnection);
//    connect(this, &ViewPanel::hidePreNextBtn, ttbc, &TTBContent::onHidePreNextBtn, Qt::UniqueConnection);
//    connect(this, &ViewPanel::sendAllImageInfos, ttbc, &TTBContent::receveAllIamgeInfos, Qt::UniqueConnection);
//    connect(this, &ViewPanel::disableDel, ttbc, &TTBContent::disableDelAct, Qt::UniqueConnection);

//    connect(ttbc, &TTBContent::clicked, this, &ViewPanel::backToLastPanel, Qt::UniqueConnection);
//    connect(this, &ViewPanel::viewImageFrom, ttbc,
//    [ = ](const QString & dir) {
//        ttbc->setCurrentDir(dir);
//    }, Qt::UniqueConnection);

//    connect(this, &ViewPanel::imageChanged, ttbc, &TTBContent::setImage, Qt::UniqueConnection);
//    connect(ttbc, &TTBContent::rotateClockwise, this, [ = ] { rotateImage(true); }, Qt::UniqueConnection);
//    connect(ttbc, &TTBContent::rotateCounterClockwise, this, [ = ] { rotateImage(false); }, Qt::UniqueConnection);
//    connect(ttbc, &TTBContent::removed, this, [ = ] {
//        if (m_dtr->isActive()) {
//            return ;
//        }
//        m_dtr->start();
//        if (m_vinfo.inDatabase)
//        {
//            popupDelDialog(m_infos.at(m_current).filePath);
//        } else
//        {
//            const QString path = m_infos.at(m_current).filePath;
//            QFile file(path);
//            if (!file.exists()) {
//                return;
//            }

//            if (removeCurrentImage()) {
//                DDesktopServices::trash(path);
//                emit dApp->signalM->picDelete();
//                ttbc->setIsConnectDel(true);
//                m_bAllowDel = true;
//                ttbc->disableDelAct(true);
//            }
//        }
//    }, Qt::UniqueConnection);

//    connect(ttbc, &TTBContent::resetTransform, this, [ = ](bool fitWindow) {
//        if (fitWindow) {
//            m_viewB->fitWindow_btnclicked();
//        } else {
//            m_viewB->fitImage();
//        }
//        m_viewB->titleBarControl();
//    }, Qt::UniqueConnection);

//    connect(m_viewB, &ImageView::disCheckAdaptImageBtn, ttbc, &TTBContent::disCheckAdaptImageBtn, Qt::UniqueConnection);
//    connect(m_viewB, &ImageView::checkAdaptImageBtn, ttbc, &TTBContent::checkAdaptImageBtn, Qt::UniqueConnection);
//    connect(m_viewB, &ImageView::sigRequestShowVaguePix, ttbc, &TTBContent::OnRequestShowVaguePix, Qt::UniqueConnection);
//    connect(dApp->signalM, &SignalManager::insertedIntoAlbum, ttbc,
//            &TTBContent::updateCollectButton, Qt::UniqueConnection);
//    connect(dApp->signalM, &SignalManager::removedFromAlbum, ttbc,
//            &TTBContent::updateCollectButton, Qt::UniqueConnection);
//    connect(ttbc, &TTBContent::showPrevious, this, [ = ]() {
//        this->showPrevious();
//    }, Qt::UniqueConnection);
//    connect(ttbc, &TTBContent::showNext, this, [ = ]() {
//        this->showNext();
//    }, Qt::UniqueConnection);
//    connect(ttbc, &TTBContent::imageClicked, this,
//    [ = ](int index, int addIndex) {
//        this->showImage(index, addIndex);
//    }, Qt::UniqueConnection);
//    /*lmh0731*/
//    connect(ttbc, &TTBContent::imageMoveEnded, this,
//                    [ = ](int index, int addIndex,bool iRet) {
//        this->m_bIsOpenPicture=iRet;
//        this->showImage(index, addIndex);
//    }, Qt::UniqueConnection);
//}

bool ViewPanel::GetPixmapStatus(QString filename)
{
    QPixmap pic = dApp->m_imagemap.value(filename);
    return !pic.isNull();
}

void ViewPanel::slotCurrentStackWidget(QString &path,bool bpix)
{
    //bpix表示图片加载成功，不用切换到撕裂图widget
     QPixmap pixmapthumb= dApp->m_imagemap.value(path);
     if(pixmapthumb.isNull()||m_infos.count()<=1){
         pixmapthumb = utils::image::getThumbnail(path);
         if(!pixmapthumb.isNull()) bpix = true;
     }else
         bpix = true;
     if (!QFileInfo(path).exists()) {
         if(m_infos.isEmpty())
             m_emptyWidget->setThumbnailImage(QPixmap());
         else
             m_emptyWidget->setThumbnailImage(pixmapthumb);
         m_stack->setCurrentIndex(1);
     } else if (!QFileInfo(path).isReadable() || !bpix) {
         emit sigDisenablebutton();
         //lmh2020/11/12 bug54164
         if(m_viewB){
            emit m_viewB->disCheckAdaptImageBtn();
        }
        m_stack->setCurrentIndex(2);
    } else {
        m_stack->setCurrentIndex(0);
        // open success.
        DRecentData data;
        data.appName = "Deepin Image Viewer";
        data.appExec = "deepin-image-viewer";
        DRecentManager::addItem(path, data);
    }
    updateMenuContent();
}
#ifndef LITE_DIV
void ViewPanel::updateLocalImages()
{
    const QString cp = m_infos.at(m_current).filePath;
    m_infos = getImageInfos(getFileInfos(cp));
    m_current = 0;
    for (; m_current < m_infos.size(); m_current++) {
        if (m_infos.at(m_current).filePath == cp) {
            return;
        }
    }
}
#endif
void ViewPanel::sendSignal(DBImgInfoList infos, int nCurrent)
{
    Q_UNUSED(nCurrent);
    if (infos.size() >= 1) {
        m_bFinishFirstLoad = true;
        m_bIsFirstLoad = false;
        m_bAllowDel = true;
    }
}

void ViewPanel::recvLoadSignal(bool bFlags)
{
    //筛选所有图片格式
    if (!m_CollFileFinish)
        return;
    if (m_infos.size() == m_infosAll.size()) return;
    m_infosadd.clear();
    if (bFlags) {
        if (m_infosHead.isEmpty()) return;
        m_infosadd.clear();
        for (int i = 0; i < Load_Image_Count; ++i) {
            if (m_infosHead.isEmpty()) break;
            DBImgInfo info = m_infosHead.takeLast();
            m_infosadd.append(info);
            m_infos.push_front(info);
            // QFileInfo finfo(info.filePath);
            // QString str = finfo.suffix();
            // if (utils::image::supportedImageFormats().contains("*." + str, Qt::CaseInsensitive) && finfo.isReadable())
            //    m_infoslideshow.push_front(info);
        }
        int begin = 0;
        for (; begin < m_infos.size(); begin++) {
            if (m_infos.at(begin).filePath == m_currentImagePath) {
                break;
            }
        }
        m_current = begin;

    } else {
        if (m_infosTail.isEmpty()) return;
        m_infosadd.clear();
        for (int i = 0; i < Load_Image_Count; ++i) {
            if (m_infosTail.isEmpty()) break;
            DBImgInfo info = m_infosTail.takeFirst();
            m_infosadd.append(info);
            m_infos.append(info);
            //QFileInfo finfo(info.filePath);
            // QString str = finfo.suffix();
            // if (utils::image::supportedImageFormats().contains("*." + str, Qt::CaseInsensitive) && finfo.isReadable())
            //    m_infoslideshow.append(info);
        }

        int begin = 0;
        for (; begin < m_infos.size(); begin++) {
            if (m_infos.at(begin).filePath == m_currentImagePath) {
                break;
            }
        }
        m_current = begin;
    }
    // emit sigsendslideshowlist(bFlags, m_infoslideshow);
    emit sendLoadAddInfos(m_infosadd, bFlags);

    QStringList pathlist;

    for (int loop = 0; loop < m_infosadd.size(); loop++) {
        pathlist.append(m_infosadd.at(loop).filePath);
    }

    if (pathlist.size() > 0) {
        ttbc->setIsConnectDel(false);
        m_bAllowDel = false;
        ttbc->disableDelAct(false);
        emit sendDynamicLoadPaths(pathlist);
    }
}

void ViewPanel::slotExitFullScreen()
{
    if (window()->isFullScreen()) {
        toggleFullScreen();
    } else {
        if (m_vinfo.inDatabase) {
            backToLastPanel();
        } else {
            //dApp->quit();
        }
    }
    emit dApp->signalM->hideExtensionPanel(true);
}

//void ViewPanel::slotLoadSlideshow(bool bFlags)
//{
//    if (!m_CollFileFinish)
//        return;
//    if (bFlags) {
//        for (int i = 0; i < Load_Image_Count; ++i) {
//            if (m_infosHead.isEmpty()) break;
//            DBImgInfo info = m_infosHead.takeLast();
//            m_infoslideshow.push_front(info);
//        }
//    } else {
//        for (int i = 0; i < Load_Image_Count; ++i) {
//            if (m_infosTail.isEmpty()) break;
//            DBImgInfo info = m_infosTail.takeFirst();
//            m_infoslideshow.append(info);
//        }
//    }
//    emit sigsendslideshowlist(bFlags, m_infoslideshow);
//}

void ViewPanel::eatImageDirIteratorThread()
{
    //if (m_AllPath.count() < 1) return;
    LoadDirPathFirst(true);
    m_CollFileFinish = true;
//    QStringList pathlist;
//    int begin = 0;
//    for (; begin < m_infosAll.size(); begin++) {
//        if (m_infosAll.at(begin).filePath == m_currentImagePath) {
//            break;
//        }
//    }

    emit sendLoadOver(m_infos, m_current);
}



void ViewPanel::SlotLoadFrontThumbnailsAndClearTail()
{
    if (!m_CollFileFinish)
        return;
    if (m_infosAll.size() == m_infos.size()) {
        emit dApp->signalM->sigNoneedLoadfrontslideshow();
        return;
    }
    m_infosHead.clear();
    m_infos.clear();
    m_infosTail = m_infosAll;
    m_infoslideshow.clear();
    for (int i = 0; i < Load_Image_Count; i++) {
        if (m_infosTail.isEmpty()) break;
        DBImgInfo info = m_infosTail.takeFirst();
        m_infos.append(info);
        QFileInfo file(info.filePath);
        QString str = file.suffix();
//        if (utils::image::supportedImageFormats().contains("*." + str, Qt::CaseInsensitive))
//            m_infoslideshow.append(info);
    }
    QStringList pathlist;
    emit dApp->signalM->sigLoadHeadThunbnail(m_infos);
    //emit sigsendslideshowlist(false, m_infoslideshow);
    for (int loop = 0; loop < m_infos.size(); loop++) {
        pathlist.append(m_infos.at(loop).filePath);
    }

    if (pathlist.size() > 0) {
        ttbc->setIsConnectDel(false);
        m_bAllowDel = false;
        ttbc->disableDelAct(false);
        emit sendDynamicLoadPaths(pathlist);
    }
}

void ViewPanel::slotGetLastThumbnailPath(QString &path)
{
    if(m_infos.size()>1)
    path = m_infos[m_infos.size() - 1].filePath;
}

void ViewPanel::slotThumbnailContainPath(QString path, bool &b)
{
   b = imageIndex(path)==-1?false:true;
}

void ViewPanel::slotLoadTailThumbnailsAndClearFront()
{
    if (!m_CollFileFinish)
        return;
    if (m_infosAll.size() == m_infos.size()) {
        emit dApp->signalM->sigNoneedLoadfrontslideshow();
        return;
    }
    m_infosHead = m_infosAll;
    m_infos.clear();
    m_infosTail.clear();
    m_infoslideshow.clear();
    for (int i = 0; i < Load_Image_Count; i++) {
        if (m_infosHead.isEmpty()) break;
        DBImgInfo info = m_infosHead.takeLast();
        m_infos.insert(0, info);
        QFileInfo file(info.filePath);
        QString str = file.suffix();
//        if (utils::image::supportedImageFormats().contains("*." + str, Qt::CaseInsensitive))
//            m_infoslideshow.append(info);
    }
    QStringList pathlist;
    emit dApp->signalM->sigLoadHeadThunbnail(m_infos);
    //emit sigsendslideshowlist(false, m_infoslideshow);
    for (int loop = 0; loop < m_infos.size(); loop++) {
        pathlist.append(m_infos.at(loop).filePath);
    }

    if (pathlist.size() > 0) {
        ttbc->setIsConnectDel(false);
        m_bAllowDel = false;
        ttbc->disableDelAct(false);
        emit sendDynamicLoadPaths(pathlist);
    }
}

void ViewPanel::slotGetFirstThumbnailPath(QString &path)
{
    if(m_infos.size() > 0)
    path = m_infos[0].filePath;
}

void  ViewPanel::slotUpdateImageView(QString &path)
{
    QPixmap pixmapthumb= dApp->m_imagemap.value(path);
    if(pixmapthumb.isNull())
    {
        pixmapthumb = utils::image::getThumbnail(path);
    }
    if (!QFileInfo(path).exists()) {
        m_emptyWidget->setThumbnailImage(pixmapthumb);
        m_stack->setCurrentIndex(1);
    } else if (!QFileInfo(path).isReadable() || pixmapthumb.isNull()) {
        emit sigDisenablebutton();
        m_stack->setCurrentIndex(2);
    } else if (QFileInfo(path).isReadable() && !QFileInfo(path).isWritable()) {
        m_stack->setCurrentIndex(0);
    } else
        m_stack->setCurrentIndex(0);
}

void ViewPanel::mousePressEvent(QMouseEvent *e)
{
    emit dApp->signalM->hideExtensionPanel();
    //    if (e->button() == Qt::BackButton) {
    //        if (window()->isFullScreen()) {
    //            showNormal();
    //        } else {
    //            backToLastPanel();
    //        }
    //    }

    // support for mouse side buttons.
    if (e->button() == Qt::ForwardButton) {
        showPrevious();
    } else if (e->button() == Qt::BackButton) {
        showNext();
    }
    ModulePanel::mousePressEvent(e);
}

void ViewPanel::onThemeChanged(ViewerThemeManager::AppTheme theme)
{
    if (theme == ViewerThemeManager::Dark) {

    }
}
void ViewPanel::showNormal()
{

    //加入动画效果，掩盖左上角展开的视觉效果，以透明度0-1显示。
    QPropertyAnimation *pAn = new QPropertyAnimation(window(), "windowOpacity");
    pAn->setDuration(50);
    pAn->setEasingCurve(QEasingCurve::Linear);
    pAn->setEndValue(1);
    pAn->setStartValue(0);
    pAn->start(QAbstractAnimation::DeleteWhenStopped);
    if (m_isMaximized) {
        window()->showNormal();
        window()->showMaximized();
    } else {
        window()->showNormal();
    }
//    /*lmh0804改，增加设置窗口取消置顶*/
//    window()->setWindowFlags(Qt::Widget);
//    window()->showNormal();
    emit dApp->signalM->showTopToolbar();
}

void ViewPanel::showFullScreen()
{
    /*lmh0804改，增加设置窗口置顶*/
//    window()->setWindowFlags(window()->windowFlags() | Qt::WindowStaysOnTopHint);
//    window()->setWindowFlags(Qt::Widget);
    m_isMaximized = window()->isMaximized();
    // Full screen then hide bars because hide animation depends on height()
    //加入动画效果，掩盖左上角展开的视觉效果，以透明度0-1显示。

        QPropertyAnimation *pAn = new QPropertyAnimation(window(), "windowOpacity");
        pAn->setDuration(50);
        pAn->setEasingCurve(QEasingCurve::Linear);
        pAn->setEndValue(1);
        pAn->setStartValue(0);
        pAn->start(QAbstractAnimation::DeleteWhenStopped);

        window()->showFullScreen();

    m_hideCursorTid = startTimer(DELAY_HIDE_CURSOR_INTERVAL);
    emit dApp->signalM->sigShowFullScreen();
}

int ViewPanel::imageIndex(const QString &path)
{
    for (int i = 0; i < m_infos.length(); i++) {
        if (m_infos.at(i).filePath == path) {
            return i;
        }
    }

    return -1;
}

DBImgInfoList ViewPanel::getImageInfos(const QFileInfoList &infos)
{
    DBImgInfoList imageInfos;
    for (QFileInfo info : infos) {
        DBImgInfo imgInfo;

        // 在 Qt 5.6 上的一个Bug，QFileInfo("").absoluteFilePath()会返回当前目录的绝对路径
        if (info.isFile()) {
            imgInfo.fileName = info.fileName();
            imgInfo.filePath = info.absoluteFilePath();
        }

        imageInfos << imgInfo;
    }

    return imageInfos;
}

//const QStringList ViewPanel::paths() const
//{
//    QStringList list;
//    for (DBImgInfo info : m_infos) {
//        list << info.filePath;
//    }

//    return list;
//}

const QStringList ViewPanel::slideshowpaths() const
{
    QStringList list;
    for (DBImgInfo info : /*m_infoslideshow*/m_infosAll) {

        list << info.filePath;
    }

    return list;
}
#ifndef LITE_DIV
QFileInfoList ViewPanel::getFileInfos(const QString &path)
{
    return utils::image::getImagesInfo(QFileInfo(path).path(), false);
}
#endif
QWidget *ViewPanel::toolbarBottomContent()
{
    return nullptr;
}

QWidget *ViewPanel::toolbarTopLeftContent()
{
    TTLContent *ttlc = new TTLContent(m_vinfo.inDatabase);
    ttlc->setCurrentDir(m_currentImageLastDir);
    if (!m_infos.isEmpty() && m_current < m_infos.size()) {
        ttlc->setImage(m_infos.at(m_current).filePath, m_infos);
    } else {
        ttlc->setImage("", m_infos);
    }

    connect(ttlc, &TTLContent::clicked, this, &ViewPanel::backToLastPanel);
    connect(this, &ViewPanel::viewImageFrom, ttlc,
    [ = ](const QString & dir) {
        ttlc->setCurrentDir(dir);
    });
    //    connect(ttlc, &TTLContent::contentWidthChanged,
    //            this, &ViewPanel::updateTopLeftWidthChanged);
    //    connect(this, &ViewPanel::updateCollectButton,
    //            ttlc, &TTLContent::updateCollectButton);
    connect(this, &ViewPanel::imageChanged, ttlc, &TTLContent::setImage);
    connect(ttlc, &TTLContent::rotateClockwise, this, [ = ] { rotateImage(true); });
    connect(ttlc, &TTLContent::rotateCounterClockwise, this, [ = ] { rotateImage(false); });
    connect(ttlc, &TTLContent::removed, this, [ = ] {
        if (m_vinfo.inDatabase)
        {
            popupDelDialog(m_infos.at(m_current).filePath);
        } else
        {
            const QString path = m_infos.at(m_current).filePath;
            removeCurrentImage();
            utils::base::trashFile(path);
            ttbc->setIsConnectDel(true);
            m_bAllowDel = true;
            ttbc->disableDelAct(true);
        }
    });
    connect(ttlc, &TTLContent::resetTransform, this, [ = ](bool fitWindow) {
        if (fitWindow) {
            m_viewB->fitWindow();
        } else {
            m_viewB->fitImage();
        }
        m_viewB->titleBarControl();
    });
    connect(dApp->signalM, &SignalManager::insertedIntoAlbum, ttlc,
            &TTLContent::updateCollectButton);
    connect(dApp->signalM, &SignalManager::removedFromAlbum, ttlc,
            &TTLContent::updateCollectButton);

    return ttlc;
}

QWidget *ViewPanel::bottomTopLeftContent()
{
    if (m_infos.size() < 1) {
        return nullptr;
    }

    if (ttbc) {
        ttbc->deleteLater();
        ttbc = nullptr;
    }
    bool flag;
    if(m_stack->currentIndex() != 0)
        flag = true;
    else
        flag = false;
    ttbc = new TTBContent(m_vinfo.inDatabase, m_infos, flag,this);

    if (!ttbc) {
        return nullptr;
    }

    connect(this, &ViewPanel::sigDisenablebutton, ttbc, &TTBContent::DisEnablettbButton, Qt::UniqueConnection);
    //heyi test 连接更改隐藏上一张按钮信号槽
    connect(dApp->signalM,&SignalManager::sigUpdateThunbnail,ttbc,&TTBContent::OnUpdateThumbnail);
    connect(this, &ViewPanel::changeHideFlag, ttbc, &TTBContent::onChangeHideFlags);
    connect(this, &ViewPanel::hidePreNextBtn, ttbc, &TTBContent::onHidePreNextBtn);
    connect(this, &ViewPanel::sendAllImageInfos, ttbc, &TTBContent::receveAllIamgeInfos);
    connect(this, &ViewPanel::disableDel, ttbc, &TTBContent::disableDelAct);
    connect(this, &ViewPanel::sendLoadAddInfos, ttbc, &TTBContent::recvLoadAddInfos);

    connect(dApp->signalM, &SignalManager::sendLoadSignal, this, &ViewPanel::recvLoadSignal, Qt::UniqueConnection);
    //    ttlc->setCurrentDir(m_currentImageLastDir);
//    if (!m_infos.isEmpty() && m_current < m_infos.size()) {
//        ttbc->setImage(m_infos.at(m_current).filePath, m_infos);
//    } else {
//        ttbc->setImage("", m_infos);
//    }

    connect(ttbc, &TTBContent::clicked, this, &ViewPanel::backToLastPanel);
    connect(this, &ViewPanel::viewImageFrom, ttbc,
    [ = ](const QString & dir) {
        ttbc->setCurrentDir(dir);
    });
    //    connect(ttlc, &TTLContent::contentWidthChanged,
    //            this, &ViewPanel::updateTopLeftWidthChanged);
    //    connect(this, &ViewPanel::updateCollectButton,
    //            ttlc, &TTLContent::updateCollectButton);
    connect(this, &ViewPanel::imageChanged, ttbc, &TTBContent::setImage);
    connect(ttbc, &TTBContent::rotateClockwise, this, [ = ] { rotateImage(true); });
    connect(ttbc, &TTBContent::rotateCounterClockwise, this, [ = ] { rotateImage(false); });
    connect(ttbc, &TTBContent::removed, this, [ = ] {

        if (m_dtr->isActive()) {
            return ;
        }
        m_dtr->start();
        if (m_vinfo.inDatabase)
        {
            popupDelDialog(m_infos.at(m_current).filePath);
        } else
        {
            const QString path = m_infos.at(m_current).filePath;
            QFile file(path);
            if (!file.exists()) {
                return;
            }
            if (removeCurrentImage()) {
                DDesktopServices::trash(path);
                emit dApp->signalM->picDelete();
                ttbc->setIsConnectDel(true);
                m_bAllowDel = true;
                ttbc->disableDelAct(true);
            }
        }
    });
    connect(ttbc, &TTBContent::resetTransform, this, [ = ](bool fitWindow) {
        if (fitWindow) {
            m_viewB->fitWindow_btnclicked();
        } else {
            m_viewB->fitImage();
        }
        m_viewB->titleBarControl();
    });
    connect(m_viewB, &ImageView::disCheckAdaptImageBtn, ttbc, &TTBContent::disCheckAdaptImageBtn);
    connect(m_viewB, &ImageView::checkAdaptImageBtn, ttbc, &TTBContent::checkAdaptImageBtn);
    connect(m_viewB, &ImageView::sigRequestShowVaguePix, ttbc, &TTBContent::OnRequestShowVaguePix, Qt::UniqueConnection);
    connect(dApp->signalM, &SignalManager::insertedIntoAlbum, ttbc,
            &TTBContent::updateCollectButton);
    connect(dApp->signalM, &SignalManager::removedFromAlbum, ttbc,
            &TTBContent::updateCollectButton);
    connect(ttbc, &TTBContent::showPrevious, this, [ = ]() {
        this->showPrevious();
    });
    connect(ttbc, &TTBContent::showNext, this, [ = ]() {
        this->showNext();
    });
    connect(ttbc, &TTBContent::imageClicked, this,
    [ = ](int index, int addIndex) {
        this->showImage(index, addIndex);
    });
    /*lmh0731*/
    connect(ttbc, &TTBContent::imageMoveEnded, this,
                    [ = ](int index, int addIndex,bool iRet) {
        this->m_bIsOpenPicture=iRet;
        this->showImage(index, addIndex);
    });
    connect(ttbc, &TTBContent::showvaguepixmap, m_viewB, &ImageView::showVagueImage);
    /*lmh0729*/
    /*shuwenzhi*/
    //此函数改变了位置索引与上一张写一张切换冲突，因此重新定一个信号
    connect(ttbc, &TTBContent::sigsetcurrent, this, [=](QString path){
        int begin = 0;
        m_currentImagePath=path;
        for (; begin < m_infos.size(); begin++) {
            if (m_infos.at(begin).filePath == m_currentImagePath) {
                break;
            }
        }
        m_current = begin;
        m_bIsOpenPicture=false;
    });
    return ttbc;
}

QWidget *ViewPanel::toolbarTopMiddleContent()
{
    QWidget *w = new QWidget();
    return w;
}

QWidget *ViewPanel::extensionPanelContent()
{
    //    QWidget *w = new QWidget;
    //    w->setAttribute(Qt::WA_TranslucentBackground);

    //    QVBoxLayout *l = new QVBoxLayout(w);
    //    l->setContentsMargins(0, 0, 0, 0);

    if (!m_info) {
        m_info = new ImageInfoWidget("", "");
    }

    //    l->addSpacing(0);
    //    l->addWidget(m_info);

    return m_info;
}


bool ViewPanel::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    if (e->type() == QEvent::HideToParent) {
        m_viewB->clear();
    }

    if (e->type() == QEvent::Resize && this->isVisible()/* && m_finish*/) {
        // emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        //  emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(),
        //                                                (m_infos.size() > 1));
        emit sigResize();
        //emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
    }

    return false;
}

void ViewPanel::resizeEvent(QResizeEvent *e)
{
    ModulePanel::resizeEvent(e);

    // There will be several times the size change during switch to full process
    // So correct it every times
    if (window()->isFullScreen()) {
        Q_EMIT dApp->signalM->hideExtensionPanel(true);
        Q_EMIT dApp->signalM->hideTopToolbar(true);
    }

    //heyi  test 窗口最大化时进入
    if (window()->isMaximized()) {
        /*QStringList pathlist;

        for (int loop = 0; loop < m_infos.size(); loop++) {
            pathlist.append(m_infos.at(loop).filePath);
        }

        if (pathlist.count() > 0) {
            emit dApp->signalM->sendPathlist(pathlist, m_infos.at(m_current).filePath);
        }*/

        //heyi   如果加载完成发送显示信号否则发送隐藏信号
        if (m_bFinishFirstLoad) {
            emit changeHideFlag(false);
        } else {
            emit changeHideFlag(true);
        }
    }

    //    if (window()->isMaximized()) {
    //        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
    //        emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(),(m_infos.size()
    //        > 1)); emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
    //    }

    if (m_viewB->isFitImage()) {
        m_viewB->fitImage();
    } else if (m_viewB->isFitWindow()) {
        m_viewB->fitWindow();
        emit dApp->signalM->sigImageOutTitleBar(false);
    }

    //lmh2020/11/13退出自适应
    if(m_screentoNormal){
        QRect rect1;
        //解决57306 【专业版1031】【看图】【5.6.3.74】tif中分辨率较高的图片，全屏后被放大显示
        QImageReader* imageReader=m_viewB->getcurrentImgReader();
        if(imageReader && imageReader->imageCount()>1 ){
            rect1 = m_viewB->image().rect();
        }else {
            rect1 = dApp->m_rectmap[m_viewB->path()];
        }
        if ((rect1.width() >= width() || rect1.height() >= height() - 150) && width() > 0 &&
                height() > 0) {
            m_viewB->fitWindow();
        } else {
            m_viewB->fitImage();
        }
        m_screentoNormal=false;
    }
}

void ViewPanel::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_hideCursorTid && (!m_menu ||!m_menu->isVisible()) && !m_printDialogVisible) {
        m_viewB->viewport()->setCursor(Qt::BlankCursor);
    }

    ModulePanel::timerEvent(e);
}

void ViewPanel::wheelEvent(QWheelEvent *e)
{
    if (m_infos.size() == 0) {
        return;
    }
    if (m_viewB && !m_viewB->path().isEmpty() && QFile(m_viewB->path()).exists() && GetPixmapStatus(m_currentImagePath))
        qApp->sendEvent(m_viewB->viewport(), e);
}

void ViewPanel::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    using namespace utils::image;
    QStringList paths;
    for (QUrl url : urls) {
        //修复style问题，取消了path
        //lmh0901判断是否是图片
        if( suffixisImage(url.toLocalFile()) ){
            paths << url.toLocalFile();
        }
    }

    if (!paths.isEmpty()) {
        emit dApp->signalM->enterView(true);
        qDebug() << "emit dApp->signalM->enterView(true)..................dropEvent";

#ifdef LITE_DIV
        SignalManager::ViewInfo vinfo;

        vinfo.path = paths.first();
        vinfo.paths = paths;
//        int ret = QMessageBox::warning(this, tr("My Application"),
//                                       "我套你猴子2",
//                                       QMessageBox::Save | QMessageBox::Discard
//                                       | QMessageBox::Cancel,
//                                       QMessageBox::Save);
        onViewImage(vinfo);
#else
        viewOnNewProcess(paths);
#endif
    }

    event->accept();
    ModulePanel::dropEvent(event);
}

void ViewPanel::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
    event->acceptProposedAction();
    ModulePanel::dragEnterEvent(event);
}

void ViewPanel::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

//Load 100 pictures while first
void ViewPanel::LoadDirPathFirst(bool bLoadAll)
{
    if (m_bThreadExit) {
        return;
    }

    if (bLoadAll) {
        m_infosHead.clear();
        m_infosTail.clear();
        m_infosAll.clear();
    } else
        m_infos.clear();
    int nCount = m_AllPath.count();
    int nimgcount = 0;
    //获取前当前位置前50个文件的位置
    int nStartIndex = m_current - First_Load_Image / 2 > 0 ? m_current - First_Load_Image / 2 : 0;
    if (!bLoadAll)
        m_firstindex = nStartIndex;
    else
        nStartIndex = 0;
    while (nStartIndex < nCount && !m_bThreadExit) {
        if (!bLoadAll) {
            if (nimgcount >= First_Load_Image) {
                break;
            }
        }
        DBImgInfo info;
        info.filePath = m_AllPath.at(nStartIndex).filePath();
        info.fileName = m_AllPath.at(nStartIndex).fileName();
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForFile(info.filePath, QMimeDatabase::MatchContent);
        QMimeType mt1 = db.mimeTypeForFile(info.filePath, QMimeDatabase::MatchExtension);
        QString str = m_AllPath.at(nStartIndex).suffix();

        // if (!m_nosupportformat.contains(str, Qt::CaseSensitive)) {
        if (mt.name().startsWith("image/") || mt.name().startsWith("video/x-mng") ||
                mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
            nimgcount++;
            if (bLoadAll) {
                if (nStartIndex < m_firstindex)
                    m_infosHead.append(info);
                else if (nStartIndex > m_lastindex)
                    m_infosTail.append(info);
                m_infosAll.append(info);
            } else {
                m_infos.append(info);
                m_infosAll.append(info);
            }
        }else {
            //删除不是图片的文件
            m_AllPath.removeOne(info.filePath);
            //当显示区域前面部分有非文件应该删除并将开始的索引-1;
            if(bLoadAll && nStartIndex<m_firstindex) {
                    m_firstindex--;
            }
            nStartIndex--;
            nCount--;
        }
        nStartIndex++;
    }
    if (!bLoadAll) m_lastindex = m_firstindex + nimgcount - 1;
}

bool compareByFileInfo(const QFileInfo &str1, const QFileInfo &str2)
{
    static QCollator sortCollator;

    sortCollator.setNumericMode(true);

    return sortCollator.compare(str1.baseName(), str2.baseName()) < 0;
}

void ViewPanel::onViewImage(const SignalManager::ViewInfo &vinfo)
{
    //检查是否是smb网络传输文件，如果是则不需要缩略图
    //检测到通过mtp外设打开不需要缩略图
    if(vinfo.path.indexOf("smb-share:server=") != -1|| vinfo.path.indexOf("mtp:host=") != -1)
        m_bOnlyOneiImg=true;
    else {
        m_bOnlyOneiImg=false;
    }
    if(dApp->m_LoadThread && dApp->m_LoadThread->isRunning()){
        emit dApp->endThread();
        QThread::msleep(500);
        m_infos.clear();
        m_infosadd.clear();
        m_infosHead.clear();
        m_infosTail.clear();
        m_infosAll.clear();
        m_bThreadExit = false;
    }
    /*swz0806 解决bug 41526 【专业版 sp3】【看图】【5.6.3.23】打开一个目录的图片后，直接将另一个目录拖拽进应用后，会同时存在两个目录的图片*/
    if(!vinfo.path.isEmpty())
    {
        m_infos.clear();
        m_infosadd.clear();
        m_infosHead.clear();
        m_infosTail.clear();
        m_infosAll.clear();
    }
    qDebug() << "onviewimage";
    m_currentFilePath = vinfo.path.left(vinfo.path.lastIndexOf("/"));
    startFileWatcher();
    using namespace utils::base;
    m_vinfo = vinfo;

    if (vinfo.fullScreen) {
        showFullScreen();
    }
    emit dApp->signalM->gotoPanel(this);

    //添加记忆重复打开同一路径帅选

    DBImgInfoList t_infos;
    QDir _dir(vinfo.path);
    bool flag = false;
    //有个风险是删除一个文件，增加一个新文件无法识别
    if (m_fileNum == _dir.entryInfoList(QDir::Files).count()) {
        t_infos = m_infos;
        //剔除无效路径
        for (int i = 0; i < t_infos.count(); i++) {
            if (!QFile::exists(t_infos.at(i).filePath)) {
                t_infos.removeAt(i);
                continue;
            }
            if (t_infos.at(i).filePath == vinfo.path && !flag) {
                flag = true;
            }
        }
    }

    // The control buttons is difference
    if (!vinfo.inDatabase) {
        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(),
                                                       (m_infos.size() > 1));
        emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
    }

    if (flag) {
        m_current = 0;
        m_infos = t_infos;
        if (!vinfo.path.isEmpty()) {
            for (; m_current < m_infos.size(); m_current++) {
                if (m_infos.at(m_current).filePath == vinfo.path) {
                    break;
                }
            }
        }

        if (m_current == m_infos.size()) {
            qWarning() << "The specify path not in view range: " << vinfo.path << vinfo.paths;
            return;
        }

        qDebug() << "";
        openImage(m_infos.at(m_current).filePath);
    } else {
        // Get view range
        if (!vinfo.paths.isEmpty()) {
            QFileInfoList list;
            foreach (QString path , vinfo.paths) {
                list << QFileInfo(path);
            }

            m_infos = getImageInfos(list);
        } else
#ifndef LITE_DIV
        {
            if (vinfo.inDatabase) {
                if (vinfo.album.isEmpty()) {
                    m_infos = DBManager::instance()->getAllInfos();
                } else {
                    m_infos = DBManager::instance()->getInfosByAlbum(vinfo.album);
                }
            } else {
                m_infos = getImageInfos(getFileInfos(vinfo.path));
            }
        }
#else
        {
            QFileInfo info(vinfo.path);

            m_infos = getImageInfos({info});
        }

        //    if (m_infos.size() == 1) {
        m_imageDirIterator.reset(
            new QDirIterator(QFileInfo(m_infos.first().filePath).absolutePath(),
                             /*utils::image::supportedImageFormats(),*/ QDir::Files | QDir::Readable |
                             QDir::NoDotAndDotDot));
        //    } else {
        //        m_imageDirIterator.reset();
        //    }
#endif
        // Get the image which need to open currently
        m_current = 0;
        if (!vinfo.path.isEmpty()) {
            for (; m_current < m_infos.size(); m_current++) {
                if (m_infos.at(m_current).filePath == vinfo.path) {
                    break;
                }
            }
        }

        if (m_current == m_infos.size()) {
            qWarning() << "The specify path not in view range: " << vinfo.path << vinfo.paths;
            return;
        }
        dApp->m_firstLoad = true;
        //Load 100 pictures while first
        if (!vinfo.path.isEmpty()&&!m_bOnlyOneiImg) {
            QString DirPath = vinfo.path.left(vinfo.path.lastIndexOf("/"));
            QDir _dirinit(DirPath);
            m_AllPath = _dirinit.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
            //修复Ｑt带后缀排序错误的问题
            qSort(m_AllPath.begin(),m_AllPath.end(),compareByFileInfo);
            m_current = 0;
            for (; m_current < m_AllPath.size(); m_current++) {
                if (m_AllPath.at(m_current).filePath() == vinfo.path) {
                    break;
                }
            }

            LoadDirPathFirst();
            int begin = 0;
            for (; begin < m_infos.size(); begin++) {
                if (m_infos.at(begin).filePath == vinfo.path) {
                    break;
                }
            }
            m_current = begin;
        }else if(m_bOnlyOneiImg){
            m_current=0;
        }
        openImage(m_infos.at(m_current).filePath);
        //eatImageDirIterator();
//        QStringList pathlist;

//        for (int loop = 0; loop < m_infos.size(); loop++) {
//            pathlist.append(m_infos.at(loop).filePath);
//        }

//        if (pathlist.count() > 0) {
//            //emit dApp->signalM->sendPathlist(pathlist, vinfo.path);
//        }
     //   if(m_infos.size()>0)
      //      emit m_viewB->cacheEnd();
//        QString format;
//        if(!vinfo.path.isEmpty())
//        {
//            QFileInfo fileinfo(vinfo.path);
//            format = fileinfo.suffix();
//        }
//        if(m_infos.size()>0 && !m_infos[0].fileName.isEmpty())
//        {
//            dApp->m_firstLoad = false;
//        }

        emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(), (m_infos.size() > 1));
        //emit changeHideFlag(false);
        m_bAllowDel = false;

        if (m_current == 0) {
            emit hidePreNextBtn(false, false);
        } else if (m_current == (m_infos.size() - 1)) {
            emit hidePreNextBtn(false, true);
        }

        //开启后台加载所有图片信息
        if (m_AllPath.size() > m_infos.size() && !m_bOnlyOneiImg ) {
            if(!vinfo.path.isEmpty()){
                QThread *loadTh = QThread::create([ = ]() {
                    eatImageDirIteratorThread();
                });
                //发送按钮置灰信号
                //emit disableDel(false);
                //m_bAllowDel = false;
                connect(loadTh, &QThread::finished, loadTh, &QObject::deleteLater);
                loadTh->start();
            }


        } else {
            m_bFinishFirstLoad = true;
            m_bAllowDel = true;
            m_CollFileFinish = true;
        }
    }
}

void ViewPanel::toggleFullScreen()
{
    m_viewB->setFitState(false,false);
    m_screentoNormal=true;
    if (window()->isFullScreen()) {
        emit dApp->signalM->sigStopAnimation();
        showNormal();
        killTimer(m_hideCursorTid);
        m_hideCursorTid = 0;
        m_viewB->viewport()->setCursor(Qt::ArrowCursor);
    } else {
        showFullScreen();
        if (!m_menu ||!m_menu->isVisible()) {
            m_viewB->viewport()->setCursor(Qt::BlankCursor);
        }
        m_hideCursorTid = startTimer(DELAY_HIDE_CURSOR_INTERVAL);
    }
}

bool ViewPanel::showPrevious()
{
#ifdef LITE_DIV
    //eatImageDirIterator();
#endif
    m_lastCurrent = m_current;
    if (m_infos.isEmpty() || m_current == 0 || !m_bFinishFirstLoad) {
        return false;
    }

    if (m_current <= 0) {
        //        m_current = m_infos.size()-1;
    } else {
        --m_current;
        if (m_current == 0) {
            emit hidePreNextBtn(false, false);
        } else {
            emit hidePreNextBtn(true, false);
        }
    }

    openImage(m_infos.at(m_current).filePath, m_vinfo.inDatabase);
    return true;
}

bool ViewPanel::showNext()
{
#ifdef LITE_DIV
    //eatImageDirIterator();
#endif
    m_lastCurrent = m_current;
    if (m_infos.size() == m_current) m_current = 0;
    if (m_infos.isEmpty() || m_current == m_infos.size() - 1 || !m_bFinishFirstLoad) {
        return false;
    }

    if (m_current == m_infos.size() - 1) {
        //        m_current = 0;
    } else {
        ++m_current;
        if (m_current == m_infos.size() - 1) {
            emit hidePreNextBtn(false, true);
        } else {
            emit hidePreNextBtn(true, false);
        }
    }

    openImage(m_infos.at(m_current).filePath, m_vinfo.inDatabase);
    return true;
}

bool ViewPanel::showImage(int index, int addindex)
{
#ifdef LITE_DIV
    //eatImageDirIterator();
#endif

    Q_UNUSED(addindex);
    emit sigStopshowThread();
    if (m_infos.isEmpty()) {
        return false;
    }

    //        if(addindex>0){
    //            for (int i=0;i<addindex;i++) {
    //                ++m_current;
    //            }
    //        }else{
    //            for (int i=addindex;i<0;i++) {
    //                --m_current;
    //            }
    //        }
    //判断当前图片是否旋转过，如果被旋转就写入本地文件

    m_lastCurrent = m_current;
    m_current = index;
    /*lmh0729加上
*/
    if(m_infos.at(m_current).filePath == m_currentImagePath&&m_bIsOpenPicture /*&&NULL!=m_currentImagePath*/)
    {
        return false;
    }
//    dApp->getRwLock().unlock();
//    dApp->getRwLock().lockForWrite();
    m_currentImagePath = m_infos.at(m_current).filePath;
    openImage(m_infos.at(m_current).filePath, m_vinfo.inDatabase);
//    dApp->getRwLock().unlock();
m_bIsOpenPicture=true;
    return true;
}

bool ViewPanel::removeCurrentImage()
{
    if (m_infos.isEmpty() || !m_bAllowDel) {
        return false;
    }

    //断开删除信号
    ttbc->setIsConnectDel(false);
    ttbc->disableDelAct(false);
    m_bAllowDel = false;
#ifdef LITE_DIV
    // 在删除当前图片之前将图片列表初始化完成
    //eatImageDirIterator();
#endif

    //lmh2020/11/18解决bug 54962
    int index=this->width()/36;
    if(m_infos[m_current].filePath==m_infosAll[m_infosAll.size()-1].filePath && m_infos.size()<=index){
        dApp->signalM->sendLoadSignal(true);
    }
    else if(m_infos.size()<=index){
        dApp->signalM->sendLoadSignal(false);
    }

    DBImgInfo imginfo = m_infos[m_current];
    m_infos.removeAt(m_current);
    m_infosAll.removeOne(imginfo);
    if (m_infos.isEmpty()) {
        qDebug() << "No images to show!";
        emit dApp->signalM->allPicDelete();
        m_current = 0;
        if (window()->isFullScreen())
            showNormal();
        emit imageChanged("", m_infos);
        emit dApp->signalM->enterView(true);
        qDebug() << "emit dApp->signalM->enterView(false)..................removeCurrentImage";
        emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(),
                                                       (m_infos.size() > 1));
        m_emptyWidget->setThumbnailImage(QPixmap());
        m_stack->setCurrentIndex(1);
    } else {
        if (m_current == m_infos.size()) {
            //LMH0611删除最后一张图片跳转到上一张
            m_current--;
        }
        else {

        }
        ttbc->delPictureFromPath(m_currentImagePath, m_infos, m_current);
        openImage(m_infos.at(m_current).filePath, m_vinfo.inDatabase);
        emit dApp->signalM->updateBottomToolbar(m_infos.size() > 1);


    }

//    ttbc->setIsConnectDel(true);
//    m_bAllowDel = true;
//    ttbc->disableDelAct(true);

    return true;
}

bool ViewPanel::removeImagePath(QString path)
{
    if (m_infos.isEmpty()) {
        return false;
    }
    int currentindex = 0;
    for (; currentindex < m_infos.size(); currentindex++) {
        if (path == m_infos[currentindex].filePath) break;
    }
    if (currentindex == m_current) {
        openImage(m_infos.at(m_current).filePath, m_vinfo.inDatabase);
    }
    return true;
}
#ifndef LITE_DIV
void ViewPanel::viewOnNewProcess(const QStringList &paths)
{
    const QString pro = "deepin-image-viewer";
    QProcess *p = new QProcess;
    connect(p, SIGNAL(finished(int)), p, SLOT(deleteLater()));

    QStringList options;
    foreach (QString path , paths) {
        options << "-o" << path;
    }
    p->start(pro, options);
}
#endif
void ViewPanel::initStack()
{
    m_stack = new QSWToDStackedWidget;
    m_stack->setMouseTracking(true);
    m_stack->setContentsMargins(0, 0, 0, 0);

    // View frame
    initViewContent();
    QVBoxLayout *vl = new QVBoxLayout(this);
    connect(dApp->signalM, &SignalManager::showTopToolbar, this,
            [ = ] { vl->setContentsMargins(0, 0, 0, 0); });
    connect(dApp->signalM, &SignalManager::hideTopToolbar, this,
            [ = ] { vl->setContentsMargins(0, 0, 0, 0); });
    vl->addWidget(m_stack);

    // Empty frame
    m_emptyWidget = new ThumbnailWidget("", "");

    m_stack->addWidget(m_viewB);
    m_stack->addWidget(m_emptyWidget);
    // Lock frame: if the deepin-image-viewer
    // can't access to read the image
    m_lockWidget = new LockWidget("", "");
    m_stack->addWidget(m_lockWidget);
}

void ViewPanel::backToLastPanel()
{
    if (window()->isFullScreen()) {
        showNormal();
    }
    if (m_vinfo.lastPanel) {
        emit dApp->signalM->gotoPanel(m_vinfo.lastPanel);
        emit dApp->signalM->hideExtensionPanel(true);
        emit dApp->signalM->showBottomToolbar();
    } else {
#ifndef LITE_DIV
        // Use dbus interface to make sure it will always back to the
        // main process
        DIVDBusController().backToMainWindow();
#endif
    }
}

void ViewPanel::rotateImage(bool clockWise)
{
    if (m_infos.count() < 1)
        return;

    bool bret = true;
    if (clockWise) {
        bret = m_viewB->rotateClockWise();
    } else {
        bret = m_viewB->rotateCounterclockwise();
    }

    if(!bret) return;
    //实时保存太卡，因此采用2s后延时保存的问题
    if(!m_tSaveImage){
        m_tSaveImage = new QTimer(this);
        connect(m_tSaveImage,&QTimer::timeout,this,[=](){
            m_viewB->rotatePixCurrent();
        });
    }

    m_tSaveImage->setSingleShot(true);
    m_tSaveImage->start(2000);
    m_viewB->autoFit();
    m_info->updateInfo();

    dApp->m_imageloader->updateImageLoader(QStringList(m_infos.at(m_current).filePath), clockWise);

    emit imageChanged(m_infos.at(m_current).filePath, m_infos);
}

void ViewPanel::initViewContent()
{
    if (m_viewB) {
        m_viewB->deleteLater();
    }

    m_viewB = new ImageView;

    connect(m_viewB, &ImageView::doubleClicked, [this]() {
        toggleFullScreen();
    });

    //heyi add
    connect(m_viewB, &ImageView::cacheEnd, this, [ = ]() {
        QStringList pathlist;

        for (int loop = 0; loop < m_infos.size(); loop++) {
            pathlist.append(m_infos.at(loop).filePath);
        }

        if (pathlist.count() > 0) {
            emit dApp->signalM->sendPathlist(pathlist, m_currentImagePath);
        }
    });

    connect(m_viewB, &ImageView::clicked, this, [ = ] { dApp->signalM->hideExtensionPanel(); });
    connect(m_viewB, &ImageView::imageChanged, this, [ = ](QString path) {
        emit imageChanged(path, m_infos);
        // Pixmap is cache in thread, make sure the size would correct after
        // cache is finish
        m_viewB->autoFit();
    });
    connect(m_viewB, &ImageView::sigStackChange, this, &ViewPanel::slotCurrentStackWidget);
    connect(m_viewB, &ImageView::previousRequested, this, &ViewPanel::showPrevious);
    connect(m_viewB, &ImageView::nextRequested, this, &ViewPanel::showNext);
//    connect(m_viewB, SIGNAL(sigShowImage(QImage)), m_viewB, SLOT(showFileImage(QImage)));
    //heyi  test
    connect(dApp, &Application::endApplication, m_viewB, &ImageView::endApp);

}

void ViewPanel::openImage(const QString path, bool inDB)
{


    //    if (! QFileInfo(path).exists()) {
    // removeCurrentImage() will cause timerEvent be trigered again by
    // showNext() or showPrevious(), so delay to remove current image
    // to break the loop
    //        TIMER_SINGLESHOT(100, {removeCurrentImage();}, this);
    //        return;
    //    }

    if (inDB) {
        // TODOisSupportsReading
        // Check whether the thumbnail is been rotated in outside
        //        QtConcurrent::run(utils::image::removeThumbnail, path);
    }
    //解决57405 【专业版1031】【看图】【5.6.3.74】在切换tif多页图片的过程中使用快捷键旋转，会使tif图片旋转且无法查看tif中的其余图片
    if(ttbc){
        ttbc->setAllEnabled(false);
    }
    clearMenu();
    using namespace utils::image;
    using namespace utils::base;

    qDebug() << "获取所有媒体data之前";
    auto metaData = getAllMetaData(path);
    qDebug() << "获取所有媒体data之后";
    QString fileSize = metaData.value("FileSize");
    qDebug() << "FileSize: " << fileSize << fileSize.size();
    QString dimension = metaData.value("Dimension");
    qDebug() << "Dimension: " << dimension << dimension.size();
    if (fileSize.isEmpty() || dimension.isEmpty()) {
        qDebug() << "Warning...";
    } else {
        int a = fileSize.indexOf(" ");
        int b = dimension.indexOf("x");
        bool c = false;
        bool d = false;
        bool f = false; //ftp类型
        //bool g=false;
        if (a > 0) {
            double value = fileSize.leftRef(a).toDouble();
            QString unit = fileSize.split(" ").last();
            if (value > 10.0 && unit == "MB") {
                c = true;
            }
        }
        if (b > 0) {
            int value1 = dimension.leftRef(b).toInt();
            int value2 = dimension.rightRef(b).toInt();
            if (value1 >= 5120 || value2 >= 3200) {
                d = true;
            }
            if (value1 >= 20000 || value2 >= 15000) {
               // g = true;
            }
        }
        if(path.indexOf("ftp:host") != -1) f = true;
        if ((c && d) || f) {
            emit dApp->signalM->loadingDisplay(true);
        }
    }
       m_viewB->setImage(path);
//    //缓存当先现实图片的上一张和下一张
//    if (!path.isEmpty()) {
//        qDebug() << "开始判定缓存时间：";
//        QStringList pathlist = getPathsFromCurrent(m_current);
//        m_viewB->recvPathsToCache(pathlist);
//    }

    if (m_info) {
        qDebug() << path;
        m_info->setImagePath(path);
    }

    m_currentImagePath = path;

    connect(dApp->signalM, &SignalManager::usbOutIn, this, [ = ](bool visible) {
        if (m_currentImagePath == "")
            return;
        if (visible) {
            if (QFileInfo(m_currentImagePath).exists()) {
                m_viewB->setImage(m_currentImagePath);
                m_stack->setCurrentIndex(0);
                QTimer::singleShot(0, m_viewB, &ImageView::autoFit);
            }
        } else {
            if (!QFileInfo(m_currentImagePath).exists()) {
                if (m_infos.count() == 1) {
                    emit dApp->signalM->picOneClear();
                } else {
                    emit dApp->signalM->picInUSB(true);
                    emit dApp->signalM->hideNavigation();
                    emit dApp->signalM->hideExtensionPanel();
                    QPixmap pixmapthumb= dApp->m_imagemap.value(path);
                    if(pixmapthumb.isNull())
                    {
                        pixmapthumb = utils::image::getThumbnail(path);
                    }
                    m_emptyWidget->setThumbnailImage(pixmapthumb);
                    m_stack->setCurrentIndex(1);
                }
            }
        }
    });
    m_stack->setCurrentIndex(0);


    if (inDB) {
        emit updateTopLeftContentImage(path);
        //        emit updateCollectButton();
    }

    //QTimer::singleShot(0, m_viewB, &ImageView::autoFit);
}
