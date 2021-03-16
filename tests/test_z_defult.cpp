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
//#include "gtestview.h"
//#define  private public
//#include "src/widgets/elidedlabel.h"
//#include "src/widgets/toast.h"
//#include "accessibility/ac-desktop-define.h"

////TEST_F(gtestview, m_frameMainWidget)
////{
////    m_frameMainWidget=new MainWidget(false,nullptr);
////    m_frameMainWidget->show();
////    QTest::mousePress(m_frameMainWidget, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
////    QTest::mouseRelease(m_frameMainWidget, Qt::LeftButton);
////    QTest::mouseClick(m_frameMainWidget, Qt::LeftButton);
////    QTest::mouseMove(m_frameMainWidget, QPoint(20,20),500);
////    QTest::keyClick(m_frameMainWidget, Qt::Key_Escape, Qt::ShiftModifier, 1000);
////    QTest::mouseDClick(m_frameMainWidget,Qt::LeftButton);

////    m_frameMainWidget->hide();
////}
//TEST_F(gtestview, RenameDialog)
//{
//    m_renameDialog=new RenameDialog(nullptr);
//    m_renameDialog->show();
//    QTest::mousePress(m_renameDialog, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
//    QTest::mouseRelease(m_renameDialog, Qt::LeftButton);
//    QTest::mouseClick(m_renameDialog, Qt::LeftButton);
//    QTest::mouseMove(m_renameDialog, QPoint(20,20),500);
//    QTest::keyClick(m_renameDialog, Qt::Key_Escape, Qt::ShiftModifier, 1000);
//    QTest::mouseDClick(m_renameDialog,Qt::LeftButton);

//    m_renameDialog->onThemeChanged(ViewerThemeManager::Light);

//    m_renameDialog->onThemeChanged(ViewerThemeManager::Dark);

//    m_renameDialog->hide();
//}

//TEST_F(gtestview, LockWidget)
//{
//    m_lockWidget=new LockWidget("","");
//    m_lockWidget->show();
//    QTest::mousePress(m_lockWidget, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
//    QTest::mouseRelease(m_lockWidget, Qt::LeftButton);
//    QTest::mouseClick(m_lockWidget, Qt::LeftButton);
//    QTest::mouseMove(m_lockWidget, QPoint(20,20),500);
//    QTest::keyClick(m_lockWidget, Qt::Key_Escape, Qt::ShiftModifier, 1000);
//    QTest::mouseDClick(m_lockWidget,Qt::LeftButton);

//    m_lockWidget->hide();
//}
//TEST_F(gtestview, ThumbnailWidget)
//{
//    m_thumbnailWidget=new ThumbnailWidget(m_SVGPath,m_SVGPath);
//    m_thumbnailWidget->show();
//    QTest::mousePress(m_thumbnailWidget, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
//    QTest::mouseRelease(m_thumbnailWidget, Qt::LeftButton);
//    QTest::mouseClick(m_thumbnailWidget, Qt::LeftButton);
//    QTest::mouseMove(m_thumbnailWidget, QPoint(20,20),500);
//    QTest::keyClick(m_thumbnailWidget, Qt::Key_Escape, Qt::ShiftModifier, 1000);
//    QTest::mouseDClick(m_thumbnailWidget,Qt::LeftButton);

//    m_thumbnailWidget->hide();
//}
//TEST_F(gtestview, NavigationWidget)
//{
//    m_navigationWidget=new NavigationWidget();
//    m_navigationWidget->show();
//    QTest::mousePress(m_navigationWidget, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
//    QTest::mouseRelease(m_navigationWidget, Qt::LeftButton);
//    QTest::mouseClick(m_navigationWidget, Qt::LeftButton);
//    QTest::mouseMove(m_navigationWidget, QPoint(20,20),500);
//    QTest::keyClick(m_navigationWidget, Qt::Key_Escape, Qt::ShiftModifier, 1000);
//    QTest::mouseDClick(m_navigationWidget,Qt::LeftButton);

//    m_navigationWidget->hide();
//}
//TEST_F(gtestview, ImageIconButton)
//{
//    m_ImageIconButton1=new ImageIconButton();
//    m_ImageIconButton1->setPropertyPic(m_SVGPath,QVariant(),m_SVGPath,m_SVGPath,m_SVGPath,m_SVGPath);
//    m_ImageIconButton1->show();
//    m_ImageIconButton1->resize(50,50);
//    QTest::mousePress(m_ImageIconButton1, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
//    QTest::mouseRelease(m_ImageIconButton1, Qt::LeftButton);
//    QTest::mouseClick(m_ImageIconButton1, Qt::LeftButton);
//    QTest::mouseMove(m_ImageIconButton1, QPoint(20,20),500);
//    QTest::keyClick(m_ImageIconButton1, Qt::Key_Escape, Qt::ShiftModifier, 1000);
//    QTest::mouseDClick(m_ImageIconButton1,Qt::LeftButton);

//    m_ImageIconButton2=new ImageIconButton(m_SVGPath,m_SVGPath,m_SVGPath,m_SVGPath);
//    m_ImageIconButton2->setPropertyPic(m_SVGPath,QVariant(),m_SVGPath,m_SVGPath,m_SVGPath,m_SVGPath);
//    m_ImageIconButton2->show();
//    m_ImageIconButton2->setAutoChecked(false);
//    m_ImageIconButton2->setTransparent(false);
//    m_ImageIconButton2->resize(50,50);

//    QTest::mousePress(m_ImageIconButton2, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
//    QTest::mouseRelease(m_ImageIconButton2, Qt::LeftButton);
//    QTest::mouseClick(m_ImageIconButton2, Qt::LeftButton);
//    QTest::mouseMove(m_ImageIconButton2, QPoint(20,20),500);
//    QTest::keyClick(m_ImageIconButton2, Qt::Key_Escape, Qt::ShiftModifier, 1000);
//    QTest::mouseDClick(m_ImageIconButton2,Qt::LeftButton);

//    m_ImageIconButton1->hide();
//    m_ImageIconButton2->hide();
//}
//TEST_F(gtestview, ImageInfoWidget)
//{
//    m_ImageInfoWidget= new ImageInfoWidget("","");
//    m_ImageInfoWidget->setImagePath(m_JPGPath);
//    m_ImageInfoWidget->onExpandChanged(false);
//    m_ImageInfoWidget->contentHeight();

//    m_ImageInfoWidget->hide();
//}
//TEST_F(gtestview, ImageView)
//{
//    m_ImageView=new ImageView();
//    m_ImageView->setImage(m_JPGPath);
//    m_ImageView->show();
//    m_ImageView->cachePixmap(m_JPGPath);
//    m_ImageView->fitWindow();
//    m_ImageView->fitWindow_btnclicked();
//    m_ImageView->fitImage();
//    m_ImageView->rotateClockWise();
//    m_ImageView->rotateCounterclockwise();

//    m_ImageView->autoFit();
//    m_ImageView->titleBarControl();
//    m_ImageView->image();
//    m_ImageView->windowRelativeScale();
//    m_ImageView->windowRelativeScale_origin();
//    m_ImageView->imageRect();
//    m_ImageView->path();

//    m_ImageView->visibleImageRect();
//    m_ImageView->isWholeImageVisible();
//    m_ImageView->isFitImage();
//    m_ImageView->isFitWindow();
//    m_ImageView->rotatePixCurrent();

//    //        m_ImageView->showPixmap(m_JPGPath);
//    //        m_ImageView->loadPictureByType(m_ImageView->judgePictureType(m_JPGPath),m_JPGPath);
//    emit m_ImageView->clicked();
//    emit m_ImageView->doubleClicked();
//    emit m_ImageView->mouseHoverMoved();
//    emit m_ImageView->scaled(200);
//    emit m_ImageView->transformChanged();
//    emit m_ImageView->showScaleLabel();
//    emit m_ImageView->nextRequested();
//    emit m_ImageView->previousRequested();
//    emit m_ImageView->disCheckAdaptImageBtn();
//    emit m_ImageView->checkAdaptImageBtn();
//    emit m_ImageView->clicked();
//    m_ImageView->mapToImage(QPoint());
//    m_ImageView->mapToImage(QRect());
//    m_ImageView->centerOn(5,5);
//    m_ImageView->setRenderer();
//    m_ImageView->setScaleValue(qreal());


//    m_ImageView->hide();
//}
//TEST_F(gtestview, m_ScanPathsItem)
//{
//    VolumeMonitor::instance();
//    ScanPathsItem* m_ScanPathsItem=new ScanPathsItem(m_JPGPath);
//    m_ScanPathsItem->show();

//    QTest::qWait(100);
//    m_ScanPathsItem->hide();
//}
//TEST_F(gtestview, dapp)
//{
//    dApp->getRwLock();
//    dApp->loadInterface(m_JPGPath);
//    //    dApp->loadPixThread(QStringList(m_JPGPath));

//    dApp->signalM->emit sendPathlist(list,m_JPGPath);
//    dApp->wpSetter;
//    dApp->viewerTheme;
//    dApp->setter;
//    DBManager::instance()->insertIntoAlbum(m_JPGPath,list);
//    DBManager::instance()->getAllPaths();
//    DBManager::instance()->getInfosByTimeline(m_JPGPath);
//    DBManager::instance()->getInfoByPath(m_JPGPath);

//    Importer::instance()->isRunning();
//    Importer::instance()->appendDir(m_JPGPath,m_JPGPath);
//    Importer::instance()->appendFiles(list,m_JPGPath);
//    //        Importer::instance()->showImportDialog(m_JPGPath);
//}
//TEST_F(gtestview, BlurFrame)
//{
//    m_printOptionspage=new PrintOptionsPage();
//    m_printOptionspage->show();

//    QRadioButton *noScaleBtn = m_printOptionspage->getnoScaleBtn();
//    QRadioButton *fitToImageBtn = m_printOptionspage->getfitToImageBtn();
//    QRadioButton *fitToPageBtn = m_printOptionspage->getfitToPageBtn();
//    QRadioButton *scaleBtn = m_printOptionspage->getscaleBtn();
//    if(noScaleBtn &&fitToImageBtn &&fitToPageBtn &&scaleBtn)
//    {
//        noScaleBtn->click();
//        noScaleBtn->toggle();
//        fitToImageBtn->click();
//        fitToImageBtn->toggle();
//        fitToPageBtn->click();
//        fitToPageBtn->toggle();
//        scaleBtn->click();
//        scaleBtn->toggle();
//    }
//    m_printOptionspage->scaleMode();
//    m_printOptionspage->scaleUnit();
//    m_printOptionspage->scaleWidth();
//    m_printOptionspage->scaleHeight();
//    m_printOptionspage->alignment();

//    QTest::qWait(100);
//    if(!m_frameMainWindow)
//    {
//        m_frameMainWindow = CommandLine::instance()->getMainWindow();
//        BlurFrame *blurFrame=new BlurFrame(m_frameMainWindow);
//        blurFrame->moveWithAnimation( 150, 150);

//        blurFrame->getBorderColor() ;
//        blurFrame->getBorderRadius() ;
//        blurFrame->getBorderWidth() ;
//        blurFrame->show();
//        blurFrame->setBorderColor(QColor(200,155,200));
//        blurFrame->resize(200,200);
//        blurFrame->setBorderRadius(100);
//        blurFrame->setBorderWidth(50);
//        blurFrame->setCoverBrush(QBrush());
//        blurFrame->setPos(QPoint(150,150));
//        blurFrame->setMoveEnable(true);

//        blurFrame->update();
//        QTest::mousePress(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),500);
//        QTest::mouseRelease(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(100,100),500);
//        QTest::mouseClick(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),500);
//        QTest::mouseMove(blurFrame, QPoint(50,100),500);
//        QTest::keyClick(blurFrame, Qt::Key_Escape, Qt::ShiftModifier, 500);
//        QTest::mouseDClick(blurFrame,Qt::LeftButton,Qt::NoModifier,QPoint(50,50),500);

//        QTest::qWait(100);
//        blurFrame->deleteLater();
//        blurFrame=nullptr;
//    }

//}
//TEST_F(gtestview, unionimage)
//{
//    //unionimage
//    //        UnionImage_NameSpace::noneQImage();
//    QString pppath=m_PNGPath;
//    supportStaticFormat();
//    supportMovieFormat();
//    isSupportReading(pppath);
//    QImage rimg(pppath);
//    creatNewImage(rimg,800,600,0);
//    QImage img2;
//    QString errorMsg;
//    loadStaticImageFromFile(m_DDSPath,img2,errorMsg);
//    detectImageFormat(m_DDSPath);
//    isNoneQImage(img2);
//    rotateImage(90,img2);
//    QString ddsPath=m_DDSPath;
//    QString svgPath=m_SVGPath;
//    rotateImageFIle(90,m_SVGPath,errorMsg);
//    rotateImageFIle(90,pppath,errorMsg);
//    rotateImageFIleWithImage(90,rimg,m_PNGPath,errorMsg);
//    rotateImageFIleWithImage(90,rimg,m_SVGPath,errorMsg);
//    DetectImageFormat(m_SVGPath);
//}
//TEST_F(gtestview, imageutil)
//{
//    //        m_scrollBar=new ScrollBar();
//    //        m_scrollBar->show();
//    qDebug()<<"imageutil1";
//    showInFileManager("");
//    //    showInFileManager(m_JPGPath);
//    qDebug()<<"imageutil22";
//    copyImageToClipboard(list);
//    trashFile(m_JPGPath);

//    SpliteText("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg",QFont(),20,false);
//    SpliteText("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg",QFont(),20,true);

//    qDebug()<<"imageutil2";
//    QString ppath=m_JPGPath;
//    onMountDevice(ppath);
//    mountDeviceExist(ppath);

//    qDebug()<<"imageutil3";
//    //utils::image
//    QString pppath=m_JPGPath;
//    imageSupportWrite(pppath);
//    rotate(pppath,90);
//    cutSquareImage(QPixmap(pppath),QSize(50,50));
//    utils::image::getOrientation(pppath);
//    bool iRet=false;
//    //        utils::image::loadTga(pppath,iRet);

//    qDebug()<<"imageutil4";
//    getRotatedImage(pppath);
//    cachePixmap(pppath);
//    getThumbnail(pppath,iRet);
//    supportedImageFormats();
//    imageSupportWallPaper(pppath);
//    utils::image::suffixisImage(pppath);

//    wrapStr(m_JPGPath,QFont(),20);
//    qDebug()<<"imageutil23";
//}

//TEST_F(gtestview, ExtensionPanel)
//{
//    m_extensionPanel=new ExtensionPanel(nullptr);
//    m_extensionPanel->show();
//    m_extensionPanel->setContent(new QWidget());
//    m_extensionPanel->setContent(nullptr);
//    m_extensionPanel->resize(200,200);
//    QTest::mousePress(m_extensionPanel, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),500);
//    QTest::mouseRelease(m_extensionPanel, Qt::LeftButton,Qt::NoModifier,QPoint(100,100),500);
//    QTest::mouseClick(m_extensionPanel, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),500);
//    QTest::mouseMove(m_extensionPanel, QPoint(50,100),500);
//    QTest::keyClick(m_extensionPanel, Qt::Key_Escape, Qt::ShiftModifier, 500);
//    QTest::mouseDClick(m_extensionPanel,Qt::LeftButton,Qt::NoModifier,QPoint(50,50),500);
//    m_extensionPanel->setContent(new QWidget());
//    m_extensionPanel->setContent(nullptr);
//    QTest::mousePress(m_extensionPanel, Qt::LeftButton);
//    QTest::mouseRelease(m_extensionPanel, Qt::LeftButton);
//    QTest::mouseClick(m_extensionPanel, Qt::LeftButton);
//    QTest::mouseMove(m_extensionPanel, QPoint(50,50));
//    //        QTest::keyClick(m_extensionPanel, Qt::Key_Escape, Qt::ShiftModifier, 200);
//    QTest::mouseDClick(m_extensionPanel,Qt::LeftButton);


//    m_extensionPanel->hide();
//}
//TEST_F(gtestview, Toast)
//{
//    if(!m_frameMainWindow)
//    {
//        m_frameMainWindow = CommandLine::instance()->getMainWindow();
//    }
//    QTest::qWait(1000);

//    Toast *widget = m_frameMainWindow->findChild<Toast *>(TOAST_OBJECT);
//    if(widget)
//    {
//        widget->icon();
//        widget->setText("toast");
//        widget->text();
//        widget->setOpacity(qreal());
//        widget->opacity();
//    }

//}

//TEST_F(gtestview, ThemeWidget)
//{
//    if(!m_frameMainWindow)
//    {
//        m_frameMainWindow = CommandLine::instance()->getMainWindow();
//    }
//    QTest::qWait(1000);

//    ThemeWidget *widget = m_frameMainWindow->findChild<ThemeWidget *>(THEME_WIDGET);
//    if(widget)
//    {
//        widget->isDeepMode();
//    }
//}

//TEST_F(gtestview, Dark)
//{
//    dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
//}

//TEST_F(gtestview, Light)
//{
//    dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Light);
//}

//TEST_F(gtestview, PrintHelper)
//{
//    if(!m_frameMainWindow)
//    {
//        m_frameMainWindow = CommandLine::instance()->getMainWindow();
//        PrintHelper *testWidget=new PrintHelper();
//        DDialog * dialog=PrintHelper::showPrintDialog(list, m_frameMainWindow);
//        if(dialog)
//        {
//            dialog->show();
//            QTest::qWait(1000);
//            dialog->close();
//            dialog->deleteLater();
//            dialog=nullptr;
//        }
//    }
//}


//TEST_F(gtestview, ElidedLabel)
//{
//    if(CommandLine::instance()->getMainWindow())
//    {
//        ElidedLabel *elide=new ElidedLabel(CommandLine::instance()->getMainWindow());
//        elide->setText("test");
//        elide->show();
//        elide->update();
//        QTest::qWait(100);
//        dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
//        QTest::qWait(100);
//        dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Light);
//        QTest::qWait(100);
//        elide->deleteLater();
//        elide=nullptr;
//    }
//}



