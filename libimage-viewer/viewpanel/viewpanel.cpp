/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
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
#include "viewpanel.h"

#include <QVBoxLayout>
#include <QShortcut>
#include <QFileInfo>
#include <QDBusInterface>
#include <QGuiApplication>
#include <QScreen>
#include <QApplication>
#include <QStringList>

#include <DDesktopServices>
#include <DMenu>
#include <DFileDialog>

#include "contents/bottomtoolbar.h"
#include "navigationwidget.h"
#include "lockwidget.h"
#include "thumbnailwidget.h"

#include "unionimage/imageutils.h"
#include "unionimage/baseutils.h"
#include "unionimage/unionimage.h"
#include "imageengine.h"
#include "widgets/printhelper.h"
#include "contents/imageinfowidget.h"
#include "widgets/extensionpanel.h"
#include "widgets/toptoolbar.h"
#include "widgets/renamedialog.h"
#include "service/ocrinterface.h"
#include "slideshow/slideshowpanel.h"
#include "service/configsetter.h"

const int BOTTOM_TOOLBAR_HEIGHT = 80;   //底部工具看高
const int BOTTOM_SPACING = 10;          //底部工具栏与底部边缘距离
const int RT_SPACING = 20;
const int TOP_TOOLBAR_HEIGHT = 50;

bool compareByFileInfo(const QFileInfo &str1, const QFileInfo &str2)
{
    static QCollator sortCollator;
    sortCollator.setNumericMode(true);
    return sortCollator.compare(str1.baseName(), str2.baseName()) < 0;
}

QString ss(const QString &text, const QString &defaultValue)
{
    Q_UNUSED(text);
    //采用代码中快捷键不使用配置文件快捷键
    // QString str = dApp->setter->value(SHORTCUTVIEW_GROUP, text, defaultValue).toString();
    QString str = defaultValue;
    str.replace(" ", "");
    return defaultValue;
}

ViewPanel::ViewPanel(QWidget *parent)
    : QFrame(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
    m_stack = new DStackedWidget(this);
    layout->addWidget(m_stack);

    m_view = new ImageGraphicsView(this);
    m_stack->addWidget(m_view);

    //m_bottomToolbar的父为主窗口,就不会出现右键菜单
    m_bottomToolbar = new BottomToolbar(dynamic_cast<QWidget *>(this->parent()));

    setContextMenuPolicy(Qt::CustomContextMenu);

    initRightMenu();
    initFloatingComponent();
    initTopBar();
    initShortcut();
    initLockPanel();
    initThumbnailWidget();
    initConnect();

    setAcceptDrops(true);
//    initExtensionPanel();
}

ViewPanel::~ViewPanel()
{

}

void ViewPanel::loadImage(const QString &path, QStringList paths)
{
    //展示图片
    m_view->setImage(path);
    QFileInfo info(path);
    m_topToolbar->setMiddleContent(info.fileName());
    m_view->resetTransform();
    m_stack->setCurrentWidget(m_view);
    //刷新工具栏
    m_bottomToolbar->setAllFile(path, paths);
    //重置底部工具栏位置与大小
    qDebug() << "---" << __FUNCTION__ << "---111111111111111";
    resetBottomToolbarGeometry(true);

}

void ViewPanel::initConnect()
{
    //缩略图列表，单机打开图片
    connect(m_bottomToolbar, &BottomToolbar::openImg, this, &ViewPanel::openImg);

    connect(m_view, &ImageGraphicsView::imageChanged, this, [ = ](QString path) {
        emit imageChanged(path);
        // Pixmap is cache in thread, make sure the size would correct after
        // cache is finish
        //暂时屏蔽，这里存在疑问，放开会导致每次切换图片，1:1高亮
        // m_view->autoFit();
    });


    //旋转信号
    connect(m_bottomToolbar, &BottomToolbar::rotateClockwise, this, [ = ] {
        this->slotRotateImage(-90);
    });

    connect(m_bottomToolbar, &BottomToolbar::rotateCounterClockwise, this, [ = ] {
        this->slotRotateImage(90);
    });

    //适应窗口和适应图片按钮
    connect(m_bottomToolbar, &BottomToolbar::resetTransform, this, &ViewPanel::slotResetTransform);

    //删除后需要重新布局
    connect(m_bottomToolbar, &BottomToolbar::removed, this, [ = ] {
        //重新布局
        this->resetBottomToolbarGeometry(true);
    }, Qt::DirectConnection);

    //适应窗口的状态更新
    connect(m_view, &ImageGraphicsView::checkAdaptScreenBtn, m_bottomToolbar, &BottomToolbar::checkAdaptImageBtn);
    connect(m_view, &ImageGraphicsView::disCheckAdaptScreenBtn,  m_bottomToolbar, &BottomToolbar::disCheckAdaptScreenBtn);
    connect(m_view, &ImageGraphicsView::checkAdaptImageBtn, m_bottomToolbar, &BottomToolbar::checkAdaptImageBtn);
    connect(m_view, &ImageGraphicsView::disCheckAdaptImageBtn, m_bottomToolbar, &BottomToolbar::disCheckAdaptImageBtn);

    connect(m_bottomToolbar, &BottomToolbar::sigOcr, this, &ViewPanel::slotOcrPicture);

    connect(m_view, &ImageGraphicsView::sigImageOutTitleBar, m_topToolbar, &TopToolbar::setTitleBarTransparent);

    connect(m_view, &ImageGraphicsView::sigMouseMove, this, &ViewPanel::slotBottomMove);

    connect(ImageEngine::instance(), &ImageEngine::sigOneImgReady, this, &ViewPanel::slotOneImgReady);

    connect(m_view, &ImageGraphicsView::UpdateNavImg, this, [ = ]() {
        m_nav->setImage(m_view->image());
        m_nav->setRectInImage(m_view->visibleImageRect());

        //正在滑动缩略图的时候不再显示
        if (m_nav->isVisible()) {
            m_nav->setVisible(false);
        }
    });

    connect(m_view, &ImageGraphicsView::sigFIleDelete, this, [ = ]() {
        this->updateMenuContent();
    });


}

void ViewPanel::initTopBar()
{
    //防止在标题栏右键菜单会触发默认的和主窗口的发生
    m_topToolbar = new TopToolbar(false, dynamic_cast<QWidget *>(this->parent()));
    m_topToolbar->resize(width(), 50);
    m_topToolbar->move(0, 0);
    m_topToolbar->setTitleBarTransparent(false);
}

void ViewPanel::initOcr()
{
    if (!m_ocrInterface) {
        m_ocrInterface = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus(), this);
    }
}

void ViewPanel::initFloatingComponent()
{
    initScaleLabel();
    initNavigation();
}

void ViewPanel::initScaleLabel()
{
    using namespace utils::base;
    DAnchors<DFloatingWidget> scalePerc = new DFloatingWidget(this);
    scalePerc->setBlurBackgroundEnabled(true);

    QHBoxLayout *layout = new QHBoxLayout();
    scalePerc->setLayout(layout);
    DLabel *label = new DLabel();
    layout->addWidget(label);
    scalePerc->setAttribute(Qt::WA_TransparentForMouseEvents);
    scalePerc.setAnchor(Qt::AnchorHorizontalCenter, this, Qt::AnchorHorizontalCenter);
    scalePerc.setAnchor(Qt::AnchorBottom, this, Qt::AnchorBottom);
    scalePerc.setBottomMargin(75 + 14);
    label->setAlignment(Qt::AlignCenter);
//    scalePerc->setFixedSize(82, 48);
    scalePerc->setFixedWidth(90 + 10);
    scalePerc->setFixedHeight(40 + 10);
    scalePerc->adjustSize();
    label->setText("100%");
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T6);
    scalePerc->hide();

    QTimer *hideT = new QTimer(this);
    hideT->setSingleShot(true);
    connect(hideT, &QTimer::timeout, scalePerc, &DLabel::hide);

    connect(m_view, &ImageGraphicsView::scaled, this, [ = ](qreal perc) {
        label->setText(QString("%1%").arg(int(perc)));
        if (perc > 100) {

        } else if (perc == 100.0) {

        } else {

        }
    });
    connect(m_view, &ImageGraphicsView::showScaleLabel, this, [ = ]() {
        scalePerc->show();
        hideT->start(1000);
    });
}

void ViewPanel::initNavigation()
{
    m_nav = new NavigationWidget(this);
    m_nav.setBottomMargin(100);
    m_nav.setLeftMargin(10);
    m_nav.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);
    m_nav.setAnchor(Qt::AnchorBottom, this, Qt::AnchorBottom);

    connect(this, &ViewPanel::imageChanged, this, [ = ](const QString & path) {
        //BUG#93145 去除对path的判断，直接隐藏导航窗口
        Q_UNUSED(path)
        m_nav->setVisible(false);
        m_nav->setImage(m_view->image());
    });

    connect(m_nav, &NavigationWidget::requestMove, [this](int x, int y) {
        m_view->centerOn(x, y);
    });
    connect(m_view, &ImageGraphicsView::transformChanged, [this]() {
        //如果stackindex不为2，全屏会出现导航窗口
        //如果是正在移动的情况，将不会出现导航栏窗口
        if (m_stack->currentWidget() == m_view) {
            m_nav->setVisible((! m_nav->isAlwaysHidden() && ! m_view->isWholeImageVisible()));
            m_nav->setRectInImage(m_view->visibleImageRect());
        }
    });
    m_nav->show();
}

void ViewPanel::initRightMenu()
{
    if (!m_menu) {
        m_menu = new DMenu(this);
        updateMenuContent();
    }
    QShortcut *ctrlm = new QShortcut(QKeySequence("Ctrl+M"), this);
    ctrlm->setContext(Qt::WindowShortcut);
    connect(ctrlm, &QShortcut::activated, this, [ = ] {
        this->customContextMenuRequested(cursor().pos());
    });

    m_menu = new DMenu;
    connect(this, &ViewPanel::customContextMenuRequested, this, [ = ] {
        updateMenuContent();
        m_menu->popup(QCursor::pos());
    });
    connect(m_menu, &DMenu::triggered, this, &ViewPanel::onMenuItemClicked);
}

void ViewPanel::initExtensionPanel()
{
    if (!m_info) {
        m_info = new ImageInfoWidget("", "", this);
        m_info->hide();
    }
    m_info->setImagePath(m_bottomToolbar->getCurrentItemInfo().path);
    if (!m_extensionPanel) {
        m_extensionPanel = new ExtensionPanel(this);
        connect(m_info, &ImageInfoWidget::extensionPanelHeight, m_extensionPanel, &ExtensionPanel::updateRectWithContent);
        connect(m_view, &ImageGraphicsView::clicked, this, [ = ] {
            this->m_extensionPanel->hide();
            this->m_info->show();
        });
    }
}

void ViewPanel::updateMenuContent()
{
    if (!window()->isFullScreen()) {
        resetBottomToolbarGeometry(true);
    }

    if (m_menu) {
        m_menu->clear();
        qDeleteAll(this->actions());

        imageViewerSpace::ItemInfo ItemInfo = m_bottomToolbar->getCurrentItemInfo();

        bool isPic = !ItemInfo.image.isNull();
        if (!isPic) {
            isPic = !m_view->image().isNull();//当前视图是否是图片
        }

        QFileInfo info(ItemInfo.path);
        bool isReadable = info.isReadable();//是否可读
        bool isWritable = info.isWritable();//是否可写
        bool isRotatable = ImageEngine::instance()->isRotatable(ItemInfo.path);//是否可旋转
        imageViewerSpace::PathType pathType = ItemInfo.pathType;//路径类型
        imageViewerSpace::ImageType imageType = ItemInfo.imageType;//图片类型

        if (m_info) {
            m_info->setImagePath(ItemInfo.path);
        }
        if (!isReadable) {
            if (m_thumbnailWidget) {
                m_stack->setCurrentWidget(m_thumbnailWidget);
                //损坏图片不透明
                emit m_view->sigImageOutTitleBar(false);
                m_thumbnailWidget->setThumbnailImage(QPixmap::fromImage(ItemInfo.image));
            }
        } else if (isPic) {
            m_stack->setCurrentWidget(m_view);
            //判断下是否透明
            m_view->titleBarControl();
        } else {
            if (m_lockWidget) {
                m_stack->setCurrentWidget(m_lockWidget);
                //损坏图片不透明
                emit m_view->sigImageOutTitleBar(false);
            }
        }

        //如果是图片，按钮恢复，否则按钮置灰
//        if (isPic) {
//            m_bottomToolbar->setPictureDoBtnClicked(true);
//        } else {
//            m_bottomToolbar->setPictureDoBtnClicked(false);
//        }

        if (imageViewerSpace::ImageTypeDamaged == imageType) {
            return;
        }
        DIconButton *AdaptImageButton = m_bottomToolbar->getBottomtoolbarButton(imageViewerSpace::ButtonTypeAdaptImage);
        DIconButton *AdaptScreenButton = m_bottomToolbar->getBottomtoolbarButton(imageViewerSpace::ButtonTypeAdaptScreen);
        //修复外部删除图片仍然能够使用适应图片和适应窗口的问题
        if (isPic && isReadable) {
            AdaptImageButton->setEnabled(true);
            AdaptScreenButton->setEnabled(true);
        } else {
            AdaptImageButton->setEnabled(false);
            AdaptScreenButton->setEnabled(false);
        }
        if (!isPic) {
            AdaptScreenButton->setChecked(isPic);
            AdaptImageButton->setChecked(isPic);
        }

        if (window()->isFullScreen()) {
            appendAction(IdExitFullScreen, QObject::tr("Exit fullscreen"), ss("Fullscreen", "F11"));
        } else {
            appendAction(IdFullScreen, QObject::tr("Fullscreen"), ss("Fullscreen", "F11"));
        }

        appendAction(IdPrint, QObject::tr("Print"), ss("Print", "Ctrl+P"));

        //ocr按钮,是否是动态图,todo
        DIconButton *OcrButton = m_bottomToolbar->getBottomtoolbarButton(imageViewerSpace::ButtonTypeOcr);
        if (imageViewerSpace::ImageTypeDynamic != imageType && isPic && isReadable) {
            appendAction(IdOcr, QObject::tr("Extract text"), ss("Extract text", "Alt+O"));
            OcrButton->setEnabled(true);
        } else {
            OcrButton->setEnabled(false);
        }

        //如果图片数量大于0才能有幻灯片
        appendAction(IdStartSlideShow, QObject::tr("Slide show"), ss("Slide show", "F5"));

        m_menu->addSeparator();
        if (isReadable) {
            appendAction(IdCopy, QObject::tr("Copy"), ss("Copy", "Ctrl+C"));
        }

        //如果程序有可读可写的权限,才能重命名,todo
        if (isReadable && isWritable) {
            appendAction(IdRename, QObject::tr("Rename"), ss("Rename", "F2"));
        }

        //apple phone的delete没有权限,保险箱无法删除,垃圾箱也无法删除,其他需要判断可读权限,todo

        DIconButton *TrashButton = m_bottomToolbar->getBottomtoolbarButton(imageViewerSpace::ButtonTypeTrash);
        if (imageViewerSpace::PathTypeAPPLE != pathType &&
                imageViewerSpace::PathTypeSAFEBOX != pathType &&
                imageViewerSpace::PathTypeRECYCLEBIN != pathType &&
                isWritable) {
            appendAction(IdMoveToTrash, QObject::tr("Delete"), ss("Throw to trash", "Delete"));
            TrashButton->setEnabled(true);
        } else {
            TrashButton->setEnabled(false);
        }

        m_menu->addSeparator();

        //判断导航栏隐藏,需要添加一个当前是否有图片,todo
        if (isReadable && isPic && !m_view->isWholeImageVisible() && m_nav->isAlwaysHidden()) {
            appendAction(IdShowNavigationWindow, QObject::tr("Show navigation window"),
                         ss("Show navigation window", ""));
        } else if (isReadable && isPic && !m_view->isWholeImageVisible() && !m_nav->isAlwaysHidden()) {
            appendAction(IdHideNavigationWindow, QObject::tr("Hide navigation window"),
                         ss("Hide navigation window", ""));
        }
        //apple手机特殊处理，不具备旋转功能,todo,需要有写的权限
        if (imageViewerSpace::PathTypeAPPLE != pathType
                && imageViewerSpace::PathTypeSAFEBOX != pathType
                && imageViewerSpace::PathTypeRECYCLEBIN != pathType
                && isRotatable && isWritable && isPic) {
            appendAction(IdRotateClockwise, QObject::tr("Rotate clockwise"), ss("Rotate clockwise", "Ctrl+R"));
            appendAction(IdRotateCounterclockwise, QObject::tr("Rotate counterclockwise"),
                         ss("Rotate counterclockwise", "Ctrl+Shift+R"));
            if (m_bottomToolbar) {
                m_bottomToolbar->setRotateBtnClicked(true);
            }

        } else {
            if (m_bottomToolbar) {
                m_bottomToolbar->setRotateBtnClicked(false);
            }

        }

        //需要判断图片是否支持设置壁纸,todo
        if (isPic && utils::image::imageSupportWallPaper(ItemInfo.path)) {
            appendAction(IdSetAsWallpaper, QObject::tr("Set as wallpaper"), ss("Set as wallpaper", "Ctrl+F9"));
        }
        if (isReadable) {
            appendAction(IdDisplayInFileManager, QObject::tr("Display in file manager"),
                         ss("Display in file manager", "Alt+D"));
            appendAction(IdImageInfo, QObject::tr("Image info"), ss("Image info", "Ctrl+I"));
        }
    }
}

void ViewPanel::toggleFullScreen()
{
//    m_view->setFitState(false, false);
    if (window()->isFullScreen()) {
        showNormal();
        m_view->viewport()->setCursor(Qt::ArrowCursor);
    } else {
        showFullScreen();
        if (!m_menu || !m_menu->isVisible()) {
            m_view->viewport()->setCursor(Qt::BlankCursor);
        }
    }
}

void ViewPanel::showFullScreen()
{
    m_isMaximized = window()->isMaximized();
    // Full screen then hide bars because hide animation depends on height()
    //加入动画效果，掩盖左上角展开的视觉效果，以透明度0-1显示。,时间为50ms

    //停止工具栏的动画
    if (m_bottomAnimation) {
        m_bottomAnimation->stop();
    }

    QPropertyAnimation *pAn = new QPropertyAnimation(window(), "windowOpacity");
    pAn->setDuration(50);
    pAn->setEasingCurve(QEasingCurve::Linear);
    pAn->setEndValue(1);
    pAn->setStartValue(0);
    pAn->start(QAbstractAnimation::DeleteWhenStopped);
    //增加切换全屏和默认大小下方工具栏的移动
    connect(pAn, &QPropertyAnimation::destroyed, this, [ = ] {
        slotBottomMove();
    });

    window()->showFullScreen();

}

void ViewPanel::showNormal()
{
    //加入动画效果，掩盖左上角展开的视觉效果，以透明度0-1显示。
    //停止工具栏的动画
    if (m_bottomAnimation) {
        m_bottomAnimation->stop();
    }
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
    //增加切换全屏和默认大小下方工具栏的移动
    connect(pAn, &QPropertyAnimation::destroyed, this, [ = ] {
        m_bottomToolbar->move((width() - m_bottomToolbar->width()) / 2, height() - m_bottomToolbar->height() - 10);
        m_bottomToolbar->update();
    });
}

void ViewPanel::appendAction(int id, const QString &text, const QString &shortcut)
{
    if (m_menu) {
        QAction *ac = new QAction(m_menu);
        addAction(ac);
        ac->setText(text);
        ac->setProperty("MenuID", id);
        ac->setShortcut(QKeySequence(shortcut));
        m_menu->addAction(ac);
    }
}

void ViewPanel::setWallpaper(const QImage &img)
{
    QThread *th1 = QThread::create([ = ]() {
        if (!img.isNull()) {
            QString path = "/tmp/DIVIMG.png";
            img.save("/tmp/DIVIMG.png", "png");
            //202011/12 bug54279
            {
                //设置壁纸代码改变，采用DBus,原方法保留
                if (/*!qEnvironmentVariableIsEmpty("FLATPAK_APPID")*/1) {
                    // gdbus call -e -d com.deepin.daemon.Appearance -o /com/deepin/daemon/Appearance -m com.deepin.daemon.Appearance.Set background /home/test/test.png
                    qDebug() << "SettingWallpaper: " << "flatpak" << path;
                    QDBusInterface interface("com.deepin.daemon.Appearance",
                                                 "/com/deepin/daemon/Appearance",
                                                 "com.deepin.daemon.Appearance");
                    if (interface.isValid()) {
                        QString screenname = QGuiApplication::primaryScreen()->name();
                        QDBusMessage reply = interface.call(QStringLiteral("SetMonitorBackground"), screenname, path);
                        qDebug() << "SettingWallpaper: replay" << reply.errorMessage();
                    }
                }
                // Remove the tmp file
                QTimer::singleShot(5000, [ = ] {
                    QFile(path).remove();
                });


            }
        }
    });
    th1->start();
}

bool ViewPanel::startdragImage(const QStringList &paths)
{
    bool bRet = false;
    QStringList image_list = paths;
    if (image_list.isEmpty())
        return false;

    QString path = image_list.first();
    if ((path.indexOf("smb-share:server=") != -1 || path.indexOf("mtp:host=") != -1 || path.indexOf("gphoto2:host=") != -1)) {
        image_list.clear();
        //判断是否图片格式
        if (ImageEngine::instance()->isImage(path)) {
            image_list << path;
        }
    } else {
        QString DirPath = image_list.first().left(image_list.first().lastIndexOf("/"));
        QDir _dirinit(DirPath);
        QFileInfoList m_AllPath = _dirinit.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
        //修复Ｑt带后缀排序错误的问题
        qSort(m_AllPath.begin(), m_AllPath.end(), compareByFileInfo);

        image_list.clear();
        for (int i = 0; i < m_AllPath.size(); i++) {
            QString path = m_AllPath.at(i).filePath();
            if (path.isEmpty()) {
                continue;
            }
            //判断是否图片格式
            if (ImageEngine::instance()->isImage(path)) {
                image_list << path;
            }
        }
    }
    if (image_list.count() > 0) {
        bRet = true;
    } else {
        bRet = false;
    }
    //解决拖入非图片文件会出现崩溃
    QString loadingPath = "";
    if (image_list.contains(path)) {
        loadingPath = path;
    } else if (image_list.count() > 0) {
        loadingPath = image_list.first();
    }
    //展示当前图片
    loadImage(loadingPath, image_list);
    //启动线程制作缩略图
    if (CommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerTypeLocal) {
        //看图制作全部缩略图
        ImageEngine::instance()->makeImgThumbnail(CommonService::instance()->getImgSavePath(), image_list, image_list.size());
    }
    m_bottomToolbar->thumbnailMoveCenterWidget();
    return bRet;
}

void ViewPanel::setTopBarVisible(bool visible)
{
    if (m_topToolbar) {
        m_topToolbar->setVisible(visible);
    }
}

void ViewPanel::setBottomtoolbarVisible(bool visible)
{
    if (m_bottomToolbar) {
        m_isBottomBarVisble = visible;
        m_bottomToolbar->setVisible(visible);
    }
}

DIconButton *ViewPanel::getBottomtoolbarButton(imageViewerSpace::ButtonType type)
{
    DIconButton *button = nullptr;
    if (m_bottomToolbar) {
        button = m_bottomToolbar->getBottomtoolbarButton(type);
    }
    return button;
}

bool ViewPanel::startChooseFileDialog()
{
    bool bRet = false;
    QString filter = tr("All images");

    filter.append('(');
    filter.append(utils::image::supportedImageFormats().join(" "));
    filter.append(')');

    static QString cfgGroupName = QStringLiteral("General"),
                   cfgLastOpenPath = QStringLiteral("LastOpenPath");
    QString pictureFolder = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QDir existChecker(pictureFolder);
    if (!existChecker.exists()) {
        pictureFolder = QDir::currentPath();
    }

    pictureFolder = ConfigSetter::instance()->value(cfgGroupName, cfgLastOpenPath, pictureFolder).toString();
#ifndef USE_TEST
    QStringList image_list =
        DFileDialog::getOpenFileNames(this, tr("Open Image"), pictureFolder, filter, nullptr,
                                      DFileDialog::HideNameFilterDetails);
#else
    QStringList image_list = QStringList(QApplication::applicationDirPath() + "/test/jpg113.jpg");
#endif
    if (image_list.isEmpty())
        return false;

    QString path = image_list.first();
    QFileInfo firstFileInfo(path);
    ConfigSetter::instance()->setValue(cfgGroupName, cfgLastOpenPath, firstFileInfo.path());

    if ((path.indexOf("smb-share:server=") != -1 || path.indexOf("mtp:host=") != -1 || path.indexOf("gphoto2:host=") != -1)) {
        image_list.clear();
        //判断是否图片格式
        if (ImageEngine::instance()->isImage(path)) {
            image_list << path;
        }
    } else {
        QString DirPath = image_list.first().left(image_list.first().lastIndexOf("/"));
        QDir _dirinit(DirPath);
        QFileInfoList m_AllPath = _dirinit.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
        //修复Ｑt带后缀排序错误的问题
        qSort(m_AllPath.begin(), m_AllPath.end(), compareByFileInfo);

        image_list.clear();
        for (int i = 0; i < m_AllPath.size(); i++) {
            QString path = m_AllPath.at(i).filePath();
            if (path.isEmpty()) {
                continue;
            }
            //判断是否图片格式
            if (ImageEngine::instance()->isImage(path)) {
                image_list << path;
            }
        }
    }
    if (image_list.count() > 0) {
        bRet = true;
    } else {
        bRet = false;
    }
    QString loadingPath;
    if (image_list.contains(path)) {
        loadingPath = path;
    } else {
        loadingPath = image_list.first();
    }
    //展示当前图片
    loadImage(loadingPath, image_list);
    //启动线程制作缩略图
    if (CommonService::instance()->getImgViewerType() == imageViewerSpace::ImgViewerTypeLocal) {
        //看图制作全部缩略图
        ImageEngine::instance()->makeImgThumbnail(CommonService::instance()->getImgSavePath(), image_list, image_list.size());
    }

    return bRet;
}

void ViewPanel::slotBottomMove()
{
    if (m_bottomToolbar) {
        if (window()->isFullScreen()) {
            QPoint pos = mapFromGlobal(QCursor::pos());

            if (height() - 20 < pos.y() && height() > pos.y() && height() == m_bottomToolbar->y()) {
                m_bottomAnimation = new QPropertyAnimation(m_bottomToolbar, "pos");
                m_bottomAnimation->setDuration(200);
                m_bottomAnimation->setEasingCurve(QEasingCurve::NCurveTypes);
                m_bottomAnimation->setStartValue(
                    QPoint((width() - m_bottomToolbar->width()) / 2, m_bottomToolbar->y()));
                m_bottomAnimation->setEndValue(QPoint((width() - m_bottomToolbar->width()) / 2,
                                                      height() - m_bottomToolbar->height() - 10));
                connect(m_bottomAnimation, &QPropertyAnimation::finished, this, [ = ]() {
                    delete m_bottomAnimation;
                    m_bottomAnimation = nullptr;
                });
                m_bottomAnimation->start();
            } else if (height() - m_bottomToolbar->height() - 10 > pos.y() &&
                       height() - m_bottomToolbar->height() - 10 == m_bottomToolbar->y()) {
                m_bottomAnimation = new QPropertyAnimation(m_bottomToolbar, "pos");
                m_bottomAnimation->setDuration(200);
                m_bottomAnimation->setEasingCurve(QEasingCurve::NCurveTypes);
                m_bottomAnimation->setStartValue(
                    QPoint((width() - m_bottomToolbar->width()) / 2, m_bottomToolbar->y()));
                m_bottomAnimation->setEndValue(QPoint((width() - m_bottomToolbar->width()) / 2, height()));
                connect(m_bottomAnimation, &QPropertyAnimation::finished, this, [ = ]() {
                    delete m_bottomAnimation;
                    m_bottomAnimation = nullptr;
                });
                m_bottomAnimation->start();
            }
        } else {
            //如果非全屏，则显示m_bottomToolbar
            if (m_isBottomBarVisble) {
                m_bottomToolbar->setVisible(true);
            }
//            resetBottomToolbarGeometry(true);
//            m_bottomToolbar->move((width() - m_bottomToolbar->width()) / 2, height() - m_bottomToolbar->height() - 10);
//            m_bottomToolbar->move((width() - m_bottomToolbar->width()) / 2, height());
//            resetBottomToolbarGeometry(true);
        }
    }
}

bool ViewPanel::slotOcrPicture()
{
    if (!m_ocrInterface) {
        initOcr();
    }
    QString path = m_bottomToolbar->getCurrentItemInfo().path;
    //图片过大，会导致崩溃，超过4K，智能裁剪
    if (m_ocrInterface != nullptr && m_view != nullptr) {
        QImage image = m_view->image();
        if (image.width() > 5000) {
            image = image.scaledToWidth(5000, Qt::SmoothTransformation);
        }
        if (image.height() > 5000) {
            image = image.scaledToHeight(5000, Qt::SmoothTransformation);
        }
        QFileInfo info(path);
        //采用路径，以防止名字出错
        m_ocrInterface->openImageAndName(image, path);
    }
    return false;
}

void ViewPanel::backImageView(const QString &path)
{
    m_stack->setCurrentWidget(m_view);
    if (path != "") {
//        m_view->setImage(path);
        m_bottomToolbar->setCurrentPath(path);
    }
}

void ViewPanel::initSlidePanel()
{
    if (!m_sliderPanel) {
        m_sliderPanel = new SlideShowPanel(this);
        m_stack->addWidget(m_sliderPanel);
        connect(m_sliderPanel, &SlideShowPanel::hideSlidePanel, this, &ViewPanel::backImageView);
    }
}

void ViewPanel::initLockPanel()
{
    if (!m_lockWidget) {
        m_lockWidget = new LockWidget("", "", this);
        m_stack->addWidget(m_lockWidget);
        connect(m_lockWidget, &LockWidget::sigMouseMove, this, &ViewPanel::slotBottomMove);
    }
}

void ViewPanel::initThumbnailWidget()
{
    if (!m_thumbnailWidget) {
        m_thumbnailWidget = new ThumbnailWidget("", "", this);
        m_stack->addWidget(m_thumbnailWidget);
        connect(m_thumbnailWidget, &ThumbnailWidget::sigMouseMove, this, &ViewPanel::slotBottomMove);
    }
}

void ViewPanel::initShortcut()
{
    QShortcut *sc = nullptr;
    // Delay image toggle

    // Previous
    sc = new QShortcut(QKeySequence(Qt::Key_Left), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [ = ] {
        DIconButton *PreButton = m_bottomToolbar->getBottomtoolbarButton(imageViewerSpace::ButtonTypePre);
        if (PreButton->isEnabled())
        {
            m_bottomToolbar->onPreButton();
        }
    });
    // Next
    sc = new QShortcut(QKeySequence(Qt::Key_Right), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [ = ] {
        DIconButton *NextButton = m_bottomToolbar->getBottomtoolbarButton(imageViewerSpace::ButtonTypeNext);
        if (NextButton->isEnabled())
        {
            m_bottomToolbar->onNextButton();
        }
    });

    // Zoom out (Ctrl++ Not working, This is a confirmed bug in Qt 5.5.0)
    sc = new QShortcut(QKeySequence(Qt::Key_Up), this);
    sc->setContext(Qt::WindowShortcut);
    //fix 36530 当图片读取失败时（格式不支持、文件损坏、没有权限），不能进行缩放操作
    connect(sc, &QShortcut::activated, this, [ = ] {
        qDebug() << "Qt::Key_Up:";
        if (!m_view->image().isNull())
        {
            m_view->setScaleValue(1.1);
        }
    });
    sc = new QShortcut(QKeySequence("Ctrl++"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [ = ] {
        if (QFile(m_view->path()).exists() && !m_view->image().isNull())
            m_view->setScaleValue(1.1);
    });
    sc = new QShortcut(QKeySequence("Ctrl+="), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [ = ] {
        if (QFile(m_view->path()).exists() && !m_view->image().isNull())
            m_view->setScaleValue(1.1);
    });
    // Zoom in
    sc = new QShortcut(QKeySequence(Qt::Key_Down), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [ = ] {
        qDebug() << "Qt::Key_Down:";
        if (QFile(m_view->path()).exists() && !m_view->image().isNull())
            m_view->setScaleValue(0.9);
    });
    sc = new QShortcut(QKeySequence("Ctrl+-"), this);
    sc->setContext(Qt::WindowShortcut);
    connect(sc, &QShortcut::activated, this, [ = ] {
        if (QFile(m_view->path()).exists() && !m_view->image().isNull())
            m_view->setScaleValue(0.9);
    });
    // Esc
    QShortcut *esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    esc->setContext(Qt::WindowShortcut);
    connect(esc, &QShortcut::activated, this, [ = ] {
        if (m_stack->currentWidget() == m_sliderPanel)
        {
            m_sliderPanel->backToLastPanel();
        } else if (window()->isFullScreen())
        {
            toggleFullScreen();
        }
    });
    // 1:1 size
    QShortcut *adaptImage = new QShortcut(QKeySequence("Ctrl+0"), this);
    adaptImage->setContext(Qt::WindowShortcut);
    connect(adaptImage, &QShortcut::activated, this, [ = ] {
        if (QFile(m_view->path()).exists())
            m_view->fitImage();
    });

}

void ViewPanel::onMenuItemClicked(QAction *action)
{
    //判断旋转图片本体是否旋转
    if (m_view) {
        m_view->slotRotatePixCurrent();
    }
    const int id = action->property("MenuID").toInt();
    switch (MenuItemId(id)) {
    case IdFullScreen:
    case IdExitFullScreen: {
        toggleFullScreen();
        break;
    }
    case IdStartSlideShow: {
        //todo,幻灯片
        if (!m_sliderPanel) {
            initSlidePanel();
        }
        ViewInfo vinfo;
        vinfo.fullScreen = window()->isFullScreen();
        vinfo.lastPanel = this;
        vinfo.path = m_bottomToolbar->getCurrentItemInfo().path;
        vinfo.paths = m_bottomToolbar->getAllPath();
        vinfo.viewMainWindowID = 0;
        m_sliderPanel->startSlideShow(vinfo);
        m_stack->setCurrentWidget(m_sliderPanel);

        break;
    }
    case IdPrint: {
        PrintHelper::getIntance()->showPrintDialog(QStringList(m_bottomToolbar->getCurrentItemInfo().path), this);
        break;
    }
    case IdRename: {
        //todo,重命名
        QString oldPath = m_bottomToolbar->getCurrentItemInfo().path;
        RenameDialog *renamedlg =  new RenameDialog(oldPath, this);
#ifndef USE_TEST
        if (renamedlg->exec()) {
#else
        renamedlg->m_lineedt->setText("40_1");
        renamedlg->show();
        {
#endif
            QFile file(oldPath);
            QString filepath = renamedlg->GetFilePath();
            QString filename = renamedlg->GetFileName();
            bool bOk = file.rename(filepath);
            if (bOk) {
                //to文件改变后做的事情
                if (m_topToolbar) {
                    m_topToolbar->setMiddleContent(filename);
                    CommonService::instance()->reName(oldPath, filepath);
                    //重新打开该图片
                    openImg(0, filepath);
                }
            }
        }
        break;
    }
    case IdCopy: {
        //todo,复制
        utils::base::copyImageToClipboard(QStringList(m_bottomToolbar->getCurrentItemInfo().path));
        break;
    }
    case IdMoveToTrash: {
        //todo,删除
        if (m_bottomToolbar) {
            m_bottomToolbar->deleteImage();
        }
        break;
    }
    case IdShowNavigationWindow: {
        m_nav->setAlwaysHidden(false);
        break;
    }
    case IdHideNavigationWindow: {
        m_nav->setAlwaysHidden(true);
        break;
    }
    case IdRotateClockwise: {
        //todo旋转
        if (m_bottomToolbar) {
            m_bottomToolbar->onRotateLBtnClicked();
        }
        break;
    }
    case IdRotateCounterclockwise: {
        //todo旋转
        if (m_bottomToolbar) {
            m_bottomToolbar->onRotateRBtnClicked();
        }
        break;
    }
    case IdSetAsWallpaper: {
        //todo设置壁纸
        setWallpaper(m_view->image());
        break;
    }
    case IdDisplayInFileManager : {
        //todo显示在文管
        utils::base::showInFileManager(m_bottomToolbar->getCurrentItemInfo().path);
        break;
    }
    case IdImageInfo: {
        //todo,文件信息
        if (!m_info && !m_extensionPanel) {
            initExtensionPanel();
        }
        //重新刷新文件信息
        m_info->updateInfo();
        m_info->show();
        m_info->setImagePath(m_bottomToolbar->getCurrentItemInfo().path);
        m_extensionPanel->setContent(m_info);
        m_extensionPanel->show();
        if (this->window()->isFullScreen() || this->window()->isMaximized()) {
            m_extensionPanel->move(this->window()->width() - m_extensionPanel->width() - 24,
                                   TOP_TOOLBAR_HEIGHT * 2);
        } else {
            m_extensionPanel->move(this->window()->pos() +
                                   QPoint(this->window()->width() - m_extensionPanel->width() - 24,
                                          TOP_TOOLBAR_HEIGHT * 2));
        }
        break;
    }
    case IdOcr: {
        //todo,ocr
        slotOcrPicture();
        break;
    }
}

}

void ViewPanel::slotOneImgReady(QString path, imageViewerSpace::ItemInfo itemInfo)
{
    imageViewerSpace::ItemInfo ItemInfo = m_bottomToolbar->getCurrentItemInfo();
    if (path.contains(ItemInfo.path)) {
        updateMenuContent();
    }
}

void ViewPanel::resetBottomToolbarGeometry(bool visible)
{
//    m_bosetVisiblele);
    if (m_isBottomBarVisble) {
        m_bottomToolbar->setVisible(visible);
    }
    if (visible) {
        int width = qMin(m_bottomToolbar->getToolbarWidth(), (this->width() - RT_SPACING));
        int x = (this->width() - width) / 2;
        //窗口高度-工具栏高度-工具栏到底部距离
        //全屏默认隐藏
        int y = this->height();
        if (!window()->isFullScreen()) {
            y = this->height() - BOTTOM_TOOLBAR_HEIGHT - BOTTOM_SPACING;
        }
        m_bottomToolbar->setGeometry(x, y, width, BOTTOM_TOOLBAR_HEIGHT);
    }
}

void ViewPanel::openImg(int index, QString path)
{
    //展示图片
    m_view->slotRotatePixCurrent();
    m_view->setImage(path);
    m_view->resetTransform();
    QFileInfo info(path);
    m_topToolbar->setMiddleContent(info.fileName());
    updateMenuContent();
}

void ViewPanel::slotRotateImage(int angle)
{
    if (m_view) {
        m_view->slotRotatePixmap(angle);
    }
    //实时保存太卡，因此采用2s后延时保存的问题
    if (!m_tSaveImage) {
        m_tSaveImage = new QTimer(this);
        connect(m_tSaveImage, &QTimer::timeout, this, [ = ]() {
            m_view->slotRotatePixCurrent();
        });
    }

    m_tSaveImage->setSingleShot(true);
    m_tSaveImage->start(2000);
    m_view->autoFit();
}

void ViewPanel::slotResetTransform(bool bRet)
{
    if (bRet && m_view) {
        m_view->fitWindow();
    } else if (!bRet && m_view) {
        m_view->fitImage();
    }
}


void ViewPanel::resizeEvent(QResizeEvent *e)
{
    if (m_extensionPanel) {
        // 获取widget左上角坐标的全局坐标
        //lmh0826,解决bug44826
        QPoint p = this->mapToGlobal(QPoint(0, 0));
        m_extensionPanel->move(p + QPoint(this->window()->width() - m_extensionPanel->width() - 24,
                                          TOP_TOOLBAR_HEIGHT * 2));
    }
    if (this->m_topToolbar) {

        if (window()->isFullScreen()) {
            this->m_topToolbar->setVisible(false);
        } else {
            this->m_topToolbar->setVisible(true);
        }

        if (m_topToolbar->isVisible()) {
            this->m_topToolbar->resize(width(), 50);
        }
    }
    //当view处于适应窗口状态的时候,resize也会继承状态
    if (m_view->isFitImage()) {
        m_view->fitImage();
    } else if (m_view->isFitWindow()) {
        m_view->fitWindow();
    }

//    resetBottomToolbarGeometry(m_stack->currentWidget() == m_view);
    resetBottomToolbarGeometry(true);
    QFrame::resizeEvent(e);
}

void ViewPanel::showEvent(QShowEvent *e)
{
//    resetBottomToolbarGeometry(m_stack->currentWidget() == m_view);
    QFrame::showEvent(e);
}

void ViewPanel::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
//    qDebug() << "windows flgs ========= " << this->windowFlags() << "attributs = " << this->testAttribute(Qt::WA_Resized);
}


void ViewPanel::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
    event->acceptProposedAction();
    DWidget::dragEnterEvent(event);
}

void ViewPanel::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void ViewPanel::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }
    QStringList paths;
    for (QUrl url : urls) {
        paths << url.toLocalFile();
    }
    startdragImage(paths);
}
