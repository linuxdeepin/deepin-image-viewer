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
#include "contents/ttlcontent.h"
#include "contents/ttmcontent.h"
#include "controller/configsetter.h"
#include "controller/divdbuscontroller.h"
#include "controller/signalmanager.h"
#include "navigationwidget.h"
#include "scen/imageview.h"
#include "snifferimageformat.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/imagebutton.h"
#include "widgets/printhelper.h"
#include "widgets/printoptionspage.h"
#include "frame/renamedialog.h"

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QPixmapCache>
#include <QProcess>
#include <QProxyStyle>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QtConcurrent>
#include <QThread>
#include <QPainter>

#include <DRecentManager>

#include "imageutils.h"

using namespace Dtk::Core;
using namespace Dtk::Widget;

namespace {

const int DELAY_HIDE_CURSOR_INTERVAL = 3000;
// const QSize ICON_SIZE = QSize(48, 40);

}  // namespace
const int First_Load_Image = 100;
ViewPanel::ViewPanel(QWidget *parent)
    : ModulePanel(parent)
    , m_hideCursorTid(0)
    , m_isInfoShowed(false)
    , m_viewB(nullptr)
    , m_info(nullptr)
    , m_stack(nullptr)
{
#ifndef LITE_DIV
    m_vinfo.inDatabase = false;
#endif
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    initStack();
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
}

QString ViewPanel::moduleName()
{
    return "ViewPanel";
}

void ViewPanel::initConnect()
{
    //heyi  test
    connect(this, &ViewPanel::sendLoadOver, this, &ViewPanel::sendSignal, Qt::QueuedConnection);
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
    connect(dApp->signalM, &SignalManager::viewImage, this,
    [ = ](const SignalManager::ViewInfo & vinfo) {
        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(),
                                                       (m_infos.size() > 1));
        emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());

        onViewImage(vinfo);
        if (NULL == vinfo.lastPanel) {
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
        for (QString path : paths) {
            if (imageIndex(path) == imageIndex(m_current->filePath)) {
                removeCurrentImage();
            }
        }
    });
#endif
    connect(dApp->signalM, &SignalManager::imagesRemoved, this, [ = ](const DBImgInfoList & infos) {
        if (m_infos.length() > 0) {
            removeCurrentImage();
        }

        updateMenuContent();
    });
    connect(m_viewB, &ImageView::mouseHoverMoved, this, &ViewPanel::mouseMoved);
    connect(m_emptyWidget, &ThumbnailWidget::mouseHoverMoved, this, &ViewPanel::mouseMoved);

#ifdef LITE_DIV
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
        QFileDialog::getOpenFileNames(this, tr("Open Image"), pictureFolder, filter, nullptr,
                                      QFileDialog::HideNameFilterDetails);

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
#endif
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


void ViewPanel::AddDataToList(LOAD_DIRECTION Dirction, int pages)
{
    DBImgInfo info;
    if (Dirction == LOAD_LEFT) {
        for (; m_firstindex < m_infosAll.size(); m_firstindex++) {
            if (m_infos.at(m_firstindex).fileName == m_infosAll.at(m_firstindex).fileName) {
                break;
            }
        }
        if (m_firstindex - pages < 0)
            m_firstindex = 0;
        else
            m_firstindex -= pages;
    }
}


bool ViewPanel::PopRenameDialog(QString &filepath, QString &filename)
{
    RenameDialog *renamedlg =  new RenameDialog(filepath);
    if (renamedlg->exec()) {
        //重命名从窗口确定后修改文件名词并修改窗口标题
        QFile file(filepath);
        filepath = renamedlg->GetFilePath();
        filename = renamedlg->GetFileName();
        bool bOk = file.rename(filepath);
        emit dApp->signalM->changetitletext(renamedlg->GetFileName());
        return bOk;
    }
    return false;
}


void ViewPanel::startFileWatcher()
{
    if (m_currentFilePath == "") {
        qDebug() << "startFileWatcher return";
        return;
    }

    m_fileManager = new DFileWatcher(m_currentFilePath, this);
    m_fileManager->startWatcher();
    qDebug() << "!!!!!!!!!!!!!!!!!startFileWatcher!!!!!!!!!!!!!!!!!!!!!!!!!!"
             << m_fileManager->startWatcher() << "=" << m_currentFilePath;

    connect(m_fileManager, &DFileWatcher::fileDeleted, this, [ = ]() {
        qDebug() << "!!!!!!!!!!!!!!!!!FileDeleted!!!!!!!!!!!!!!!!!!!!!!!!!!";
        //        updateLocalImages();
        emit dApp->signalM->fileDeleted();
    });

    connect(dApp->signalM, &SignalManager::fileDeleted, this, [ = ]() {
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
            emit dApp->signalM->sigImageOutTitleBar(false);
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
            emit dApp->signalM->sigImageOutTitleBar(false);
        }
    });
}

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

void ViewPanel::sendSignal(DBImgInfoList infos, int nCurrent)
{
    if (infos.size() >= 1) {
        m_bFinishFirstLoad = true;
        m_bIsFirstLoad = false;
    }
    //第一次加载为所有图片不进行刷新
    if(infos.size() == m_infos.size())
        return;
    m_infos = infos;
    QStringList pathlist;
    qDebug() << "xixixixi" << QThread::currentThreadId();

    for (int loop = 0; loop < m_infos.size(); loop++) {
        pathlist.append(m_infos.at(loop).filePath);
    }

    if (pathlist.count() > 0) {
        emit dApp->signalM->sendPathlist(pathlist, m_currentImagePath);
        emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(), (m_infos.size() > 1));
        emit changeHideFlag(false);
        if (m_current == 0) {
            emit hidePreNextBtn(false, false);
        } else if (m_current == (m_infos.size() - 1)) {
            emit hidePreNextBtn(false, true);
        }
    }
}

#ifdef LITE_DIV
bool compareByString(const DBImgInfo &str1, const DBImgInfo &str2)
{
    static QCollator sortCollator;

    sortCollator.setNumericMode(true);

    return sortCollator.compare(str1.fileName, str2.fileName) < 0;
}

// 将迭代器中的数据初始化给m_infos
void ViewPanel::eatImageDirIterator()
{
    if (!m_imageDirIterator)
        return;

    const QString currentImageFile = m_infos.at(m_current).filePath;
    DBImgInfo infoNow = m_infos.at(m_current);
    //m_infos.clear();

    //设置初始化加载图片数量
    int nCurrentNum = 0;
    while (m_imageDirIterator->hasNext()) {
        //判断是否达到初始化加载张数
        if (nCurrentNum >= LOAD_NUMBER) {
            //break;
        }

        DBImgInfo info;

        info.filePath = m_imageDirIterator->next();
        info.fileName = m_imageDirIterator->fileInfo().fileName();

        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForFile(info.filePath, QMimeDatabase::MatchContent);
        QMimeType mt1 = db.mimeTypeForFile(info.filePath, QMimeDatabase::MatchExtension);
        //qDebug() << info.filePath << "&&&&&&&&&&&&&&" << m_imageDirIterator->fileInfo().fileName()
        //<< m_imageDirIterator->fileInfo().filePath() << mt.name() << "mt1" << mt1.name();
        QString str = m_imageDirIterator->fileInfo().suffix();
        //        if (str.isEmpty()) {
        if (mt.name().startsWith("image/") || mt.name().startsWith("video/x-mng") ||
                mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
            if (utils::image::supportedImageFormats().contains("*." + str, Qt::CaseInsensitive)) {
                if (!m_infos.contains(info)) {
                    m_infos.append(info);
                }
            } else if (str.isEmpty()) {
                if (!m_infos.contains(info)) {
                    m_infos.append(info);
                }
            }

        }
        //        } else {
        //            if (mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
        //                if (utils::image::supportedImageFormats().contains("*." + str,
        //                Qt::CaseInsensitive)) {
        //                    m_infos.append(info);
        //                }
        //            }
        //        }
        nCurrentNum++;
    }

    m_imageDirIterator.reset(nullptr);
    //std::sort(m_infos.begin(), m_infos.end(), compareByString);

    auto cbegin = 0;
    m_current = cbegin;

    while (cbegin < m_infos.size()) {
        if (m_infos.at(cbegin).filePath == currentImageFile) {
            m_current = cbegin;
            break;
        }

        ++cbegin;
    }
}

void ViewPanel::newEatImageDirIterator()
{
    if (!m_imageDirIterator)
        return;

    const QString currentImageFile = m_infos.at(m_current).filePath;
    m_infos.clear();

    while (m_imageDirIterator->hasNext()) {
        DBImgInfo info;

        info.filePath = m_imageDirIterator->next();
        info.fileName = m_imageDirIterator->fileInfo().fileName();

        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForFile(info.filePath, QMimeDatabase::MatchContent);
        QMimeType mt1 = db.mimeTypeForFile(info.filePath, QMimeDatabase::MatchExtension);
        //qDebug() << info.filePath << "&&&&&&&&&&&&&&" << m_imageDirIterator->fileInfo().fileName()
        //<< m_imageDirIterator->fileInfo().filePath() << mt.name() << "mt1" << mt1.name();
        QString str = m_imageDirIterator->fileInfo().suffix();
        //        if (str.isEmpty()) {
        if ("icns" != str) {
            if (mt.name().startsWith("image/") || mt.name().startsWith("video/x-mng") ||
                    mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
                if (utils::image::supportedImageFormats().contains("*." + str, Qt::CaseInsensitive)) {
                    m_infos.append(info);
                } else if (str.isEmpty()) {
                    m_infos.append(info);
                }
            }
        }
        //        } else {
        //            if (mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
        //                if (utils::image::supportedImageFormats().contains("*." + str,
        //                Qt::CaseInsensitive)) {
        //                    m_infos.append(info);
        //                }
        //            }
        //        }
    }

    //m_imageDirIterator.reset(nullptr);
    std::sort(m_infos.begin(), m_infos.end(), compareByString);

    auto cbegin = 0;
    m_current = cbegin;

    while (cbegin < m_infos.size()) {
        if (m_infos.at(cbegin).filePath == currentImageFile) {
            m_current = cbegin;
            break;
        }

        ++cbegin;
    }
}

void ViewPanel::eatImageDirIteratorThread()
{
    //if (m_AllPath.count() < 1) return;
    m_infosAll.clear();
    LoadDirPathFirst(true);
    QStringList pathlist;
    m_current = 0;
    for (; m_current < m_infosAll.size(); m_current++) {
        if (m_infosAll.at(m_current).filePath == m_currentImagePath) {
            break;
        }
    }

    emit sendLoadOver(m_infosAll, m_current);
}
#endif

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

void ViewPanel::onThemeChanged(ViewerThemeManager::AppTheme theme) {}

void ViewPanel::showNormal()
{
    if (m_isMaximized) {
        this->window()->showMaximized();
    } else {
        window()->showNormal();
    }

    emit dApp->signalM->showTopToolbar();
}

void ViewPanel::showFullScreen()
{
    m_isMaximized = window()->isMaximized();
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

const QStringList ViewPanel::paths() const
{
    QStringList list;
    for (DBImgInfo info : m_infos) {
        list << info.filePath;
    }

    return list;
}

QFileInfoList ViewPanel::getFileInfos(const QString &path)
{
    return utils::image::getImagesInfo(QFileInfo(path).path(), false);
}

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

    ttbc = new TTBContent(m_vinfo.inDatabase, m_infos);

    if (!ttbc) {
        return nullptr;
    }

    //heyi test 连接更改隐藏上一张按钮信号槽
    connect(this, &ViewPanel::changeHideFlag, ttbc, &TTBContent::onChangeHideFlags, Qt::QueuedConnection);
    connect(this, &ViewPanel::hidePreNextBtn, ttbc, &TTBContent::onHidePreNextBtn, Qt::QueuedConnection);
    connect(this, &ViewPanel::sendAllImageInfos, ttbc, &TTBContent::receveAllIamgeInfos, Qt::QueuedConnection);
    //    ttlc->setCurrentDir(m_currentImageLastDir);
    if (!m_infos.isEmpty() && m_current < m_infos.size()) {
        ttbc->setImage(m_infos.at(m_current).filePath, m_infos);
    } else {
        ttbc->setImage("", m_infos);
    }

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

            removeCurrentImage();
            DDesktopServices::trash(path);
            emit dApp->signalM->picDelete();
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

const SignalManager::ViewInfo ViewPanel::viewInfo() const
{
    return m_vinfo;
}

bool ViewPanel::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    if (e->type() == QEvent::HideToParent) {
        m_viewB->clear();
    }

    if (e->type() == QEvent::Resize && this->isVisible() && m_finish) {
        emit dApp->signalM->updateTopToolbarLeftContent(toolbarTopLeftContent());
        emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(),
                                                       (m_infos.size() > 1));
        emit dApp->signalM->updateTopToolbarMiddleContent(toolbarTopMiddleContent());
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
}

void ViewPanel::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_hideCursorTid && !m_menu->isVisible() && !m_printDialogVisible) {
        m_viewB->viewport()->setCursor(Qt::BlankCursor);
    }

    ModulePanel::timerEvent(e);
}

void ViewPanel::wheelEvent(QWheelEvent *e)
{
    if (m_infos.size() == 0) {
        return;
    }
    if (m_viewB && !m_viewB->path().isEmpty() && QFile(m_viewB->path()).exists())
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
        const QString path = url.toLocalFile();
        if (QFileInfo(path).isDir()) {
            auto finfos = getImagesInfo(path, false);
            for (auto finfo : finfos) {
                if (imageSupportRead(finfo.absoluteFilePath())) {
                    paths << finfo.absoluteFilePath();
                }
            }
        } else if (imageSupportRead(path)) {
            paths << path;
        }
    }

    if (!paths.isEmpty()) {
        emit dApp->signalM->enterView(true);
        qDebug() << "emit dApp->signalM->enterView(true)..................dropEvent";

#ifdef LITE_DIV
        SignalManager::ViewInfo vinfo;

        vinfo.path = paths.first();
        vinfo.paths = paths;

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
    ModulePanel::dragEnterEvent(event);
}
//Load 100 pictures while first
void ViewPanel::LoadDirPathFirst(bool bLoadAll)
{
    int nCount = m_AllPath.count();
    int i = 0;
    int nimgcount = 0;
    int nStartIndex = m_current - First_Load_Image / 2 > 0 ? m_current - First_Load_Image / 2 : 0;
    while (i < nCount) {
        if (!bLoadAll) {
            if (nimgcount > First_Load_Image) break;
        } else {
            nStartIndex = i;
        }
        DBImgInfo info;
        info.filePath = m_AllPath.at(nStartIndex).filePath();
        info.fileName = m_AllPath.at(nStartIndex).fileName();
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForFile(info.filePath, QMimeDatabase::MatchContent);
        QMimeType mt1 = db.mimeTypeForFile(info.filePath, QMimeDatabase::MatchExtension);
        QString str = m_AllPath.at(nStartIndex).suffix();
        nStartIndex++;
        //        if (str.isEmpty()) {
        if (mt.name().startsWith("image/") || mt.name().startsWith("video/x-mng") ||
                mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
            if (utils::image::supportedImageFormats().contains("*." + str, Qt::CaseInsensitive)) {
                // if (!m_infos.contains(info)) {
                nimgcount++;
                if (bLoadAll)
                    m_infosAll.append(info);
                else
                    m_infos.append(info);
                //}
            } else if (str.isEmpty()) {
                //if (!m_infos.contains(info)) {
                nimgcount++;
                if (bLoadAll)
                    m_infosAll.append(info);
                else
                    m_infos.append(info);
                // }
            }

        }
        i++;
    }
}

void ViewPanel::onViewImage(const SignalManager::ViewInfo &vinfo)
{
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
            for (QString path : vinfo.paths) {
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

        openImage(m_infos.at(m_current).filePath);
        //Load 100 pictures while first
        if (!vinfo.path.isEmpty()) {
            QString DirPath = vinfo.path.left(vinfo.path.lastIndexOf("/"));
            QDir _dirinit(DirPath);
            m_AllPath = _dirinit.entryInfoList(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
            m_current = 0;
            for (; m_current < m_AllPath.size(); m_current++) {
                if (m_AllPath.at(m_current).filePath() == vinfo.path) {
                    break;
                }
            }

            LoadDirPathFirst();
            for (; m_current < m_infos.size(); m_current++) {
                if (m_infos.at(m_current).filePath == vinfo.path) {
                    break;
                }
            }
        }

        //eatImageDirIterator();
        QStringList pathlist;

        for (int loop = 0; loop < m_infos.size(); loop++) {
            pathlist.append(m_infos.at(loop).filePath);
        }

        if (pathlist.count() > 0) {
            emit dApp->signalM->sendPathlist(pathlist, vinfo.path);
        }

        emit dApp->signalM->updateBottomToolbarContent(bottomTopLeftContent(), (m_infos.size() > 1));
        m_bFinishFirstLoad = true;
        emit changeHideFlag(false);
        if (pathlist.size() > 0) {
            //mit imageChanged(vinfo.path, m_infos);
        }

        if (m_current == 0) {
            emit hidePreNextBtn(false, false);
        } else if (m_current == (m_infos.size() - 1)) {
            emit hidePreNextBtn(false, true);
        }

        //开启后台加载所有图片信息
        if(m_AllPath.size()>101)
        {
            QThread *loadTh = QThread::create([ = ]() {
                eatImageDirIteratorThread();
            });

            if (loadTh && !vinfo.path.isEmpty()) {
                loadTh->start();
            }
        }
    }
}

void ViewPanel::toggleFullScreen()
{
    if (window()->isFullScreen()) {
        showNormal();
        killTimer(m_hideCursorTid);
        m_hideCursorTid = 0;
        m_viewB->viewport()->setCursor(Qt::ArrowCursor);
    } else {
        showFullScreen();
        if (!m_menu->isVisible()) {
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
    m_current = index;
    openImage(m_infos.at(m_current).filePath, m_vinfo.inDatabase);
    return true;
}

void ViewPanel::removeCurrentImage()
{
    if (m_infos.isEmpty()) {
        return;
    }

#ifdef LITE_DIV
    // 在删除当前图片之前将图片列表初始化完成
    //eatImageDirIterator();
#endif
    m_infos.removeAt(m_current);
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
            m_current = 0;
        }
        openImage(m_infos.at(m_current).filePath, m_vinfo.inDatabase);
        emit dApp->signalM->updateBottomToolbar(m_infos.size() > 1);
    }
    //    if (m_current != m_infos.cend()) {
    //        m_infos.removeAt(imageIndex(m_current->filePath));
    //        if (! showNext()) {
    //            if (! showPrevious()) {
    //                qDebug() << "No images to show!";
    //                m_current = m_infos.cend();
    //                emit imageChanged("",m_infos);
    //                m_emptyWidget->setThumbnailImage(QPixmap());
    //                m_stack->setCurrentIndex(1);
    //            }
    //        }
    //    }else {
    //        m_infos.removeAt(imageIndex(m_current->filePath));
    //        if (m_infos.isEmpty()) {
    //            qDebug() << "No images to show!";
    //            m_current = m_infos.cend();
    //            emit imageChanged("",m_infos);
    //            m_emptyWidget->setThumbnailImage(QPixmap());
    //            m_stack->setCurrentIndex(1);
    //        }else {
    //            if (m_current == m_infos.cend()) {
    //                m_current = m_infos.cbegin();
    //            }
    //            openImage(m_current->filePath, m_vinfo.inDatabase);
    //        }
    //    }
}

void ViewPanel::viewOnNewProcess(const QStringList &paths)
{
    const QString pro = "deepin-image-viewer";
    QProcess *p = new QProcess;
    connect(p, SIGNAL(finished(int)), p, SLOT(deleteLater()));

    QStringList options;
    for (QString path : paths) {
        options << "-o" << path;
    }
    p->start(pro, options);
}

void ViewPanel::initStack()
{
    m_stack = new QStackedWidget;
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

    if (clockWise) {
        m_viewB->rotateClockWise();
    } else {
        m_viewB->rotateCounterclockwise();
    }

    m_viewB->autoFit();
    m_info->updateInfo();

    dApp->m_imageloader->updateImageLoader(QStringList(m_infos.at(m_current).filePath), clockWise);

    emit imageChanged(m_infos.at(m_current).filePath, m_infos);
}

void ViewPanel::initViewContent()
{
    m_viewB = new ImageView;

    connect(m_viewB, &ImageView::doubleClicked, [this]() {
        toggleFullScreen();
    });
    connect(m_viewB, &ImageView::clicked, this, [ = ] { dApp->signalM->hideExtensionPanel(); });
    connect(m_viewB, &ImageView::imageChanged, this, [ = ](QString path) {
        emit imageChanged(path, m_infos);
        // Pixmap is cache in thread, make sure the size would correct after
        // cache is finish
        m_viewB->autoFit();
    });
    connect(m_viewB, &ImageView::previousRequested, this, &ViewPanel::showPrevious);
    connect(m_viewB, &ImageView::nextRequested, this, &ViewPanel::showNext);
    //heyi  test
    connect(dApp, &Application::endApplication, m_viewB, &ImageView::endApp);
}

void ViewPanel::openImage(const QString &path, bool inDB)
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
        }
        if (c && d) {
            emit dApp->signalM->loadingDisplay(true);
        }
    }

    m_viewB->setImage(path);
    updateMenuContent();

    if (m_info) {
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
                    m_emptyWidget->setThumbnailImage(utils::image::getThumbnail(path));
                    m_stack->setCurrentIndex(1);
                }
            }
        }
    });

    if (!QFileInfo(path).exists()) {
        m_emptyWidget->setThumbnailImage(utils::image::getThumbnail(path));
        m_stack->setCurrentIndex(1);
    } else if (!QFileInfo(path).isReadable()) {
        m_stack->setCurrentIndex(2);
    } else if (QFileInfo(path).isReadable() && !QFileInfo(path).isWritable()) {
        m_stack->setCurrentIndex(0);
    } else {
        m_stack->setCurrentIndex(0);

        // open success.
        DRecentData data;
        data.appName = "Deepin Image Viewer";
        data.appExec = "deepin-image-viewer";
        DRecentManager::addItem(path, data);
    }
    if (inDB) {
        emit updateTopLeftContentImage(path);
        //        emit updateCollectButton();
    }

    QTimer::singleShot(0, m_viewB, &ImageView::autoFit);
}
