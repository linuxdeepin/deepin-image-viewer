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
#include "gtestview.h"
#define  private public
#include <QKeySequence>
#include <QShortcut>
#include <QEnterEvent>
#include <QFile>
#include <QDir>
#include <DApplication>

#include "accessibility/ac-desktop-define.h"
#include "imageviewer.h"

#include "widgets/extensionpanel.h"
#include "widgets/formlabel.h"
#include "widgets/imagebutton.h"
#include "widgets/printhelper.h"
#include "widgets/renamedialog.h"
#include "widgets/toast.h"
#include "widgets/toptoolbar.h"

#define  private public
#include "viewpanel/viewpanel.h"
#include "viewpanel/navigationwidget.h"
#include "slideshow/slideshowpanel.h"
#include "service/imagedataservice.h"
#include "viewpanel/contents/bottomtoolbar.h"
#include "viewpanel/contents/imgviewwidget.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/imgoperate.h"
#include "unionimage/snifferimageformat.h"

gtestview::gtestview()
{

}
TEST_F(gtestview, cpFile)
{
    QFile::copy(":/gif.gif", QApplication::applicationDirPath() + "/gif.gif");
    QFile(QApplication::applicationDirPath() + "/gif.gif").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/gif2.gif", QApplication::applicationDirPath() + "/gif2.gif");
    QFile(QApplication::applicationDirPath() + "/gif2.gif").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/ico.ico", QApplication::applicationDirPath() + "/ico.ico");
    QFile(QApplication::applicationDirPath() + "/ico.ico").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/jpg.jpg", QApplication::applicationDirPath() + "/jpg.jpg");
    QFile(QApplication::applicationDirPath() + "/jpg.jpg").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/mng.mng", QApplication::applicationDirPath() + "/mng.mng");
    QFile(QApplication::applicationDirPath() + "/mng.mng").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/png.png", QApplication::applicationDirPath() + "/png.png");
    QFile(QApplication::applicationDirPath() + "/png.png").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg.svg", QApplication::applicationDirPath() + "/svg.svg");
    QFile(QApplication::applicationDirPath() + "/svg.svg").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg1.svg", QApplication::applicationDirPath() + "/svg1.svg");
    QFile(QApplication::applicationDirPath() + "/svg1.svg").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg2.svg", QApplication::applicationDirPath() + "/svg2.svg");
    QFile(QApplication::applicationDirPath() + "/svg2.svg").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg1.svg", QApplication::applicationDirPath() + "/svg3.svg");
    QFile(QApplication::applicationDirPath() + "/svg3.svg").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/tif.tif", QApplication::applicationDirPath() + "/tif.tif");
    QFile(QApplication::applicationDirPath() + "/tif.tif").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/wbmp.wbmp", QApplication::applicationDirPath() + "/wbmp.wbmp");
    QFile(QApplication::applicationDirPath() + "/wbmp.wbmp").setPermissions(\
                                                                            QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                            QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/dds.dds", QApplication::applicationDirPath() + "/dds.dds");
    QFile(QApplication::applicationDirPath() + "/dds.dds").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/tga.tga", QApplication::applicationDirPath() + "/tga.tga");
    QFile(QApplication::applicationDirPath() + "/tga.tga").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/errorPic.icns", QApplication::applicationDirPath() + "/errorPic.icns");
    QFile(QApplication::applicationDirPath() + "/errorPic.icns").setPermissions(\
                                                                                QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                                QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QDir a(QApplication::applicationDirPath());
    a.mkdir("test");
    QFile::copy(":/jpg.jpg", QApplication::applicationDirPath() + "/test/jpg.jpg");
    QFile(QApplication::applicationDirPath() + "/test/jpg.jpg").setPermissions(\
                                                                               QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                               QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);
}

//主窗体
TEST_F(gtestview, MainWindow)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeLocal, CACHE_PATH, nullptr);
    QStringList paths{QApplication::applicationDirPath() + "/svg.svg", QApplication::applicationDirPath() + "/test/jpg.jpg"};
    bool bRet = m_imageViewer->startdragImage({});
    bRet = m_imageViewer->startdragImage({"smb-share:server=bisuhfawe.png"});
    bRet = m_imageViewer->startdragImage(paths);
    if (bRet) {

        m_imageViewer->showNormal();
        m_imageViewer->resize(800, 600);
        m_imageViewer->resize(1000, 500);

        m_imageViewer->setTopBarVisible(false);
        m_imageViewer->setTopBarVisible(true);
        m_imageViewer->setBottomtoolbarVisible(false);
        m_imageViewer->setTopBarVisible(true);

        QTest::qWait(500);
    }

    m_imageViewer->deleteLater();
    QTest::qWait(500);

    //另外一种使用方式
    m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeLocal, CACHE_PATH, nullptr);
    paths = QStringList{QApplication::applicationDirPath() + "/tif.tif"};
    m_imageViewer->startImgView(paths[0], paths);
    QTest::qWait(500);
    m_imageViewer->deleteLater();
    QTest::qWait(500);
}

//widgets
TEST_F(gtestview, Widgets)
{
    //extension panel
    {
        ExtensionPanel panel;
        panel.updateRectWithContent(100);
        QWidget x;
        panel.setContent(&x);

        QTest::qWait(200);

        panel.show();
        QTestEventList e1;
        e1.addKeyClick(Qt::Key::Key_Escape);
        e1.simulate(&x);
        QTest::qWait(200);

        panel.show();
        QTestEventList e2;
        e2.addKeyClick(Qt::Key::Key_I, Qt::KeyboardModifier::ControlModifier);
        e2.simulate(&panel);
        QTest::qWait(200);
    }

    //form label
    {
        SimpleFormLabel label_1("12345");
        label_1.resize(100, 100);
        label_1.show();

        SimpleFormField label_2;
        label_2.resize(100, 100);
        label_2.show();

        QTest::qWait(200);
    }

    //image button
    {
        ImageButton button;
        button.setDisabled(false);
        button.show();

        QTestEventList e;
        e.addMouseMove(QPoint(1, 1));
        e.addDelay(1500);
        e.addMouseClick(Qt::MouseButton::LeftButton);
        e.addDelay(500);
        e.addMouseMove(QPoint(999, 999));
        e.addDelay(1500);
        e.simulate(&button);

        button.setToolTip("123321");
        e.simulate(&button);

        QTest::qWait(200);
    }

    //print helper
    {
        auto helper = PrintHelper::getIntance();

        helper->showPrintDialog({QApplication::applicationDirPath() + "/jpg.jpg",
                                 QApplication::applicationDirPath() + "/tif.tif"});

        helper->deleteLater();
        QTest::qWait(500);
    }

    //rename dialog
    {
        RenameDialog dialog("12345.jpg");
        dialog.GetFileName();
        dialog.GetFilePath();

        QTestEventList e;
        e.addMouseClick(Qt::MouseButton::LeftButton);
        e.addDelay(400);

        dialog.m_lineedt->setText("321.png");
        dialog.show();
        e.simulate(dialog.okbtn);

        dialog.m_lineedt->setText("");
        dialog.show();
        e.simulate(dialog.cancelbtn);

        /*e.clear();
        e.addMouseClick(Qt::MouseButton::LeftButton);
        e.addDelay(200);
        e.addKeyClick(Qt::Key_5);
        e.addDelay(200);
        e.addKeyClick(Qt::Key_Enter);
        e.addDelay(200);
        dialog.show();
        e.simulate(dialog.m_lineedt);*/

        QTest::qWait(500);
    }

    //toast
    {
        Toast toast;
        toast.text();
        toast.icon();
        toast.setText("123");
        toast.setIcon(QApplication::applicationDirPath() + "/ico.ico");
        toast.setIcon(QIcon(QApplication::applicationDirPath() + "/ico.ico"));
    }

    //top tool bar
    {
        DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
        TopToolbar toolBar(false, nullptr);
        toolBar.setTitleBarTransparent(false);

        toolBar.show();
        QTestEventList e;
        e.addMouseDClick(Qt::MouseButton::LeftButton);
        e.addDelay(500);
        e.addMouseDClick(Qt::MouseButton::LeftButton);
        e.addDelay(500);
        e.simulate(&toolBar);

        DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
        QTest::qWait(200);
    }
}

//view panel
TEST_F(gtestview, ViewPanel)
{
    //初始化
    ViewPanel panel;
    panel.loadImage(QApplication::applicationDirPath() + "/gif.gif", {QApplication::applicationDirPath() + "/gif.gif",
                                                                      QApplication::applicationDirPath() + "/tif.tif",
                                                                      QApplication::applicationDirPath() + "/jpg.jpg",
                                                                      QApplication::applicationDirPath() + "/jpg.jpg",
                                                                      QApplication::applicationDirPath() + "/svg.svg"
                                                                     });
    panel.initFloatingComponent();
    panel.show();

    //键盘与鼠标事件
    QTestEventList e;

    //动态图
    e.addKeyClick(Qt::Key_Right, Qt::KeyboardModifier::NoModifier, 200);
    e.addKeyClick(Qt::Key_Left, Qt::KeyboardModifier::NoModifier, 200);
    e.addKeyClick(Qt::Key_Up, Qt::KeyboardModifier::NoModifier, 200);
    e.addKeyClick(Qt::Key_Plus, Qt::KeyboardModifier::ControlModifier, 200);
    e.addKeyClick(Qt::Key_0, Qt::KeyboardModifier::ControlModifier, 200);
    e.addKeyClick(Qt::Key_Minus, Qt::KeyboardModifier::ControlModifier, 200);
    e.addKeyClick(Qt::Key_Down, Qt::KeyboardModifier::NoModifier, 200);
    e.addKeyClick(Qt::Key_Escape, Qt::KeyboardModifier::NoModifier, 200);
    e.addDelay(500);

    //多页图
    e.addKeyClick(Qt::Key_Right, Qt::KeyboardModifier::NoModifier, 200);
    e.addKeyClick(Qt::Key_Up, Qt::KeyboardModifier::NoModifier, 200);
    e.addKeyClick(Qt::Key_Plus, Qt::KeyboardModifier::ControlModifier, 200);
    e.addKeyClick(Qt::Key_0, Qt::KeyboardModifier::ControlModifier, 200);
    e.addKeyClick(Qt::Key_Minus, Qt::KeyboardModifier::ControlModifier, 200);
    e.addKeyClick(Qt::Key_Down, Qt::KeyboardModifier::NoModifier, 200);
    e.addKeyClick(Qt::Key_Escape, Qt::KeyboardModifier::NoModifier, 200);
    e.addDelay(500);

    e.simulate(&panel);

    //基本函数遍历
    panel.toggleFullScreen();
    panel.slotBottomMove();
    panel.toggleFullScreen();
    panel.slotBottomMove();
    panel.startChooseFileDialog();
    panel.getBottomtoolbarButton(imageViewerSpace::ButtonType::ButtonTypeOcr);
    panel.slotOcrPicture();
    panel.backImageView(QApplication::applicationDirPath() + "/svg.svg");
    panel.initSlidePanel();
    panel.resetBottomToolbarGeometry(true);
    panel.resetBottomToolbarGeometry(false);
    panel.slotRotateImage(90);
    QTest::qWait(3000);
    panel.slotRotateImage(-90);
    QTest::qWait(3000);

    //菜单
    //还剩IdPrint会崩
    QAction menuAction;

    menuAction.setProperty("MenuID", ViewPanel::IdFullScreen);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);

    menuAction.setProperty("MenuID", ViewPanel::IdRename);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);

    menuAction.setProperty("MenuID", ViewPanel::IdStartSlideShow);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);
    e.clear();
    e.addKeyClick(Qt::Key_Escape, Qt::KeyboardModifier::NoModifier, 200);
    e.simulate(panel.m_sliderPanel);

    menuAction.setProperty("MenuID", ViewPanel::IdCopy);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);

    menuAction.setProperty("MenuID", ViewPanel::IdMoveToTrash);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);
    menuAction.setProperty("MenuID", ViewPanel::IdMoveToTrash);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);

    menuAction.setProperty("MenuID", ViewPanel::IdShowNavigationWindow);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);

    menuAction.setProperty("MenuID", ViewPanel::IdHideNavigationWindow);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
    QTest::qWait(500);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
    QTest::qWait(500);
    e.clear();
    e.addMousePress(Qt::MouseButton::LeftButton);
    e.addMouseMove(QPoint(20, 20), 200);
    e.addMouseRelease(Qt::MouseButton::LeftButton);
    e.addDelay(200);
    e.simulate(panel.m_nav.widget());

    menuAction.setProperty("MenuID", ViewPanel::IdRotateClockwise);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);

    menuAction.setProperty("MenuID", ViewPanel::IdRotateCounterclockwise);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);

    menuAction.setProperty("MenuID", ViewPanel::IdSetAsWallpaper);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);

    menuAction.setProperty("MenuID", ViewPanel::IdDisplayInFileManager);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);

    menuAction.setProperty("MenuID", ViewPanel::IdImageInfo);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);

    menuAction.setProperty("MenuID", ViewPanel::IdOcr);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(500);

    menuAction.setProperty("MenuID", ViewPanel::IdMoveToTrash);
    panel.onMenuItemClicked(&menuAction);
    QTest::qWait(200);

    //ImageGraphicsView
    panel.loadImage("", {});

    auto view = panel.m_view;
    view->clear();
    view->setImage(QApplication::applicationDirPath() + "/svg2.svg", QImage());

    QTest::qWait(500);
}

TEST_F(gtestview, ImageDataService)
{
    auto testPath = QApplication::applicationDirPath() + "/jpg.jpg";
    ImageDataService::instance()->imageIsLoaded(testPath);
    ImageDataService::instance()->addImage(testPath, ImageDataService::instance()->getThumnailImageByPath(testPath));
    ImageDataService::instance()->getMovieDurationStrByPath(testPath);
    ImageDataService::instance()->getVisualIndex();
    ImageDataService::instance()->getCount();
    ImageDataService::instance()->setVisualIndex(0);
    ImageDataService::instance()->addMovieDurationStr("", "");
    ImageDataService::instance()->deleteLater();

    QTest::qWait(500);
}

TEST_F(gtestview, BottomToolbar)
{
    BottomToolbar toolBar;
    toolBar.setAllFile(QApplication::applicationDirPath() + "/gif.gif", {QApplication::applicationDirPath() + "/gif.gif",
                                                                         QApplication::applicationDirPath() + "/tif.tif",
                                                                         QApplication::applicationDirPath() + "/jpg.jpg",
                                                                         QApplication::applicationDirPath() + "/jpg.jpg",
                                                                         QApplication::applicationDirPath() + "/svg.svg"
                                                                        });
    toolBar.getAllPath();

    QTestEventList e;
    e.addMousePress(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(10, 10), 100);
    e.addMouseMove(QPoint(100, 100), 50);
    e.addMouseRelease(Qt::MouseButton::LeftButton);
    e.addDelay(500);
    e.simulate(toolBar.m_imgListWidget);
}

TEST_F(gtestview, UnionImage)
{
    //base utils
    utils::base::hash("12345");
    utils::base::mountDeviceExist("/media/uos/");
    utils::base::mountDeviceExist("/run/media/uos/");
    utils::base::onMountDevice("12345");
    utils::base::showInFileManager(QApplication::applicationDirPath() + "/jpg.jpg");
    utils::base::stringToDateTime("1970-01-01 00:00:00");
    utils::base::timeToString(QDateTime::fromString("1970-01-01 00:00:00"), true);
    utils::base::timeToString(QDateTime::fromString("1970-01-01 00:00:00"), false);

    //image utils
    utils::image::scaleImage(QApplication::applicationDirPath() + "/f827t3r9qwheo.icns");
    utils::image::scaleImage(QApplication::applicationDirPath() + "/jpg.jpg");
    utils::image::getCreateDateTime(QApplication::applicationDirPath() + "/jpg.jpg");
    utils::image::imageSupportSave(QApplication::applicationDirPath() + "/jpg.jpg");

    auto pix = utils::image::cachePixmap(QApplication::applicationDirPath() + "/jpg.jpg");
    utils::image::cutSquareImage(pix);
    utils::image::cutSquareImage(pix, {200, 200});

    utils::image::getImagesInfo(QApplication::applicationDirPath(), false);
    utils::image::getImagesInfo(QApplication::applicationDirPath(), true);
    utils::image::getOrientation(QApplication::applicationDirPath() + "/jpg.jpg");
    utils::image::getRotatedImage(QApplication::applicationDirPath() + "/jpg.jpg");

    utils::image::generateThumbnail(QApplication::applicationDirPath() + "/jpg.jpg");
    utils::image::generateThumbnail(QApplication::applicationDirPath() + "/errorPic.icns");
    utils::image::getThumbnail(QApplication::applicationDirPath() + "/jpg.jpg", true);
    utils::image::getThumbnail(QApplication::applicationDirPath() + "/jpg.jpg", false);

    utils::image::isCanRemove(QApplication::applicationDirPath() + "/jpg.jpg");
    utils::image::isCanRemove("ngw8uhrt8owjfpowsj");

    //Img Operate
    ImgOperate worker;
    worker.slotMakeImgThumbnail(QApplication::applicationDirPath(), {QApplication::applicationDirPath() + "/jpg.jpg"}, 1, true);
    worker.slotMakeImgThumbnail(QApplication::applicationDirPath(), {QApplication::applicationDirPath() + "/jpg.jpg"}, 1, false);

    //snifferimageformat
    DetectImageFormat("nfoiehrf2oq3hjrfowhnefoi");
    DetectImageFormat(QApplication::applicationDirPath() + "/jpg.jpg");
    DetectImageFormat(QApplication::applicationDirPath() + "/gif.gif");
    DetectImageFormat(QApplication::applicationDirPath() + "/svg.svg");
    DetectImageFormat(QApplication::applicationDirPath() + "/mng.mng");
    DetectImageFormat(QApplication::applicationDirPath() + "/tif.tif");
    DetectImageFormat(QApplication::applicationDirPath() + "/wbmp.wbmp");
    DetectImageFormat(QApplication::applicationDirPath() + "/dds.dds");
    DetectImageFormat(QApplication::applicationDirPath() + "/ico.ico");
    DetectImageFormat(QApplication::applicationDirPath() + "/errorPic.icns");
    DetectImageFormat(QApplication::applicationDirPath() + "/tga.tga");
}
