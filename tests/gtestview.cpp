#include "gtestview.h"


gtestview::gtestview()
{

}

//主窗体
TEST_F(gtestview, mainwindow)
{
    DApplicationSettings saveTheme;
    CommandLine *cl = CommandLine::instance();
    //4.将时间写入QDataStream
    QDateTime wstime = QDateTime::currentDateTime();
    QString teststr = wstime.toString("yyyy-MM-dd hh:mm:ss");
    bool newflag = true;
    cl->processOption(wstime, newflag);
    dApp->m_timer=0;
//    m_frameMainWindow = new MainWindow(false);
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = new MainWindow(false);
    }
    m_frameMainWindow->setWindowRadius(18);
    m_frameMainWindow->setBorderWidth(0);
    m_frameMainWindow->show();
    emit dApp->signalM->hideBottomToolbar(true);
    emit dApp->signalM->enableMainMenu(false);
    SignalManager::ViewInfo info;
    info.album = "";
    info.lastPanel = nullptr;
    info.path = GIF2;

    info.paths << info.path;
    info.paths <<JPEGPATH;
    info.paths <<GIF2;

//    emit dApp->signalM->viewImage(info);
    m_frameMainWindow->OpenImage(GIF2);

    QTest::mousePress(m_frameMainWindow, Qt::LeftButton);
    QTest::mouseRelease(m_frameMainWindow, Qt::LeftButton);
    QTest::mouseClick(m_frameMainWindow, Qt::LeftButton);
    QTest::mouseMove(m_frameMainWindow, QPoint(190,50));
    QTest::mouseDClick(m_frameMainWindow,Qt::LeftButton);


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

    const QStringList &image_list =list;

    if (image_list.isEmpty())
        return;

    SignalManager::ViewInfo vinfo;

    vinfo.path = image_list.first();
    vinfo.paths = image_list;

    QFileInfo firstFileInfo(vinfo.path);
    dApp->setter->setValue(cfgGroupName, cfgLastOpenPath, firstFileInfo.path());

    emit dApp->signalM->enterView(true);

//    m_viewPanel->onViewImage(vinfo);
    emit dApp->signalM->viewImage(vinfo);
//    QTest::mousePress(m_viewPanel, Qt::LeftButton);
//    QTest::mouseRelease(m_viewPanel, Qt::LeftButton);
//    QTest::mouseClick(m_viewPanel, Qt::LeftButton);
//    QTest::mouseMove(m_viewPanel, QPoint(250,170));
//    //        QTest::keyClick(m_viewPanel, Qt::Key_Escape, Qt::ShiftModifier, 200);
//    QTest::mouseDClick(m_viewPanel,Qt::LeftButton);

//    QTest::mouseClick(m_frameMainWindow,Qt::LeftButton,Qt::KeyboardModifiers(),QPoint(600,300),500);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Right, Qt::NoModifier, 100);
     QTest::keyClick(m_frameMainWindow, Qt::Key_Right, Qt::NoModifier, 100);
     QTest::keyClick(m_frameMainWindow, Qt::Key_Left, Qt::NoModifier, 100);
//    QTest::keyClick(m_frameMainWindow, Qt::Key_F1, Qt::NoModifier, 20);
    QTest::keyClick(m_frameMainWindow, Qt::Key_F11, Qt::NoModifier, 50);//

    QTest::keyClick(m_frameMainWindow, Qt::Key_Escape, Qt::NoModifier, 50);
    QTest::keyClick(m_frameMainWindow, Qt::Key_F5, Qt::NoModifier, 5000);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Escape, Qt::NoModifier, 50);
//    QTest::keyClick(m_frameMainWindow, Qt::Key_Escape, Qt::NoModifier, 20);//



//    QTimer::singleShot(500,[=]{
//            QTest::keyClick(m_frameMainWindow, Qt::Key_Right, Qt::NoModifier, 0);
//    });
//    QTest::keyClick(m_frameMainWindow, Qt::Key_Left, Qt::NoModifier, 30);
//    delete m_frameMainWindow;
//    m_frameMainWindow=nullptr;

}
TEST_F(gtestview, ViewPanel)
{

    /*module*/
//    m_viewPanel=new ViewPanel(nullptr);
//    m_viewPanel->show();
//    m_viewPanel->refreshPixmap("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");

//    QString filter = tr("All images");

//    filter.append('(');
//    filter.append(utils::image::supportedImageFormats().join(" "));
//    filter.append(')');


}
TEST_F(gtestview, m_ImageLoader)
{
//    m_ImageLoader=new ImageLoader(NULL,QStringList(DDSPATH),JPEGPATH);
}
TEST_F(gtestview, m_pushbutton)
{
    m_pushbutton=new PushButton();
    m_pushbutton->show();
    m_pushbutton->normalPic();
    m_pushbutton-> hoverPic() ;
    m_pushbutton-> pressPic() ;
    m_pushbutton-> disablePic() ;
    m_pushbutton-> checkedPic() ;
    m_pushbutton-> text() ;
    m_pushbutton-> normalColor() ;
    m_pushbutton-> hoverColor() ;
    m_pushbutton-> pressColor() ;
    m_pushbutton-> disableColor() ;
    m_pushbutton-> getSpacing() ;
    m_pushbutton-> setSpacing(11);
    m_pushbutton-> getChecked() ;
    m_pushbutton-> setToolTip("TEST");
    m_pushbutton-> setChecked(false);
    m_pushbutton-> setNormalPic(JPEGPATH);
    m_pushbutton-> setHoverPic(JPEGPATH);
    m_pushbutton-> setPressPic(JPEGPATH);
    m_pushbutton-> setDisablePic(JPEGPATH);
    m_pushbutton-> setCheckedPic(JPEGPATH);
    m_pushbutton-> setText("test");
    m_pushbutton-> setNormalColor(QColor(120,111,150));
    m_pushbutton-> setHoverColor(QColor(120,180,150));
    m_pushbutton-> setPressColor(QColor(120,20,150));
    m_pushbutton-> setDisableColor(QColor(120,123,150));

    m_pushbutton-> setToolTip("TEST");
    m_pushbutton-> setChecked(true);
    m_pushbutton-> setNormalPic(JPEGPATH);
    m_pushbutton-> setHoverPic(JPEGPATH);
    m_pushbutton-> setPressPic(JPEGPATH);
    m_pushbutton-> setDisablePic(JPEGPATH);
    m_pushbutton-> setCheckedPic(JPEGPATH);
    m_pushbutton-> setText("test");
    m_pushbutton-> setNormalColor(QColor(120,111,150));
    m_pushbutton-> setHoverColor(QColor(120,180,150));
    m_pushbutton-> setPressColor(QColor(120,20,150));
    m_pushbutton-> setDisableColor(QColor(120,123,150));

    QTest::mousePress(m_pushbutton, Qt::LeftButton);
    QTest::mouseRelease(m_pushbutton, Qt::LeftButton);
    QTest::mouseClick(m_pushbutton, Qt::LeftButton);
    QTest::mouseMove(m_pushbutton, QPoint(50,50));
//    QTest::keyClick(m_pushbutton, Qt::Key_Escape, Qt::ShiftModifier, 30);
    QTest::mouseDClick(m_pushbutton,Qt::LeftButton);
}
TEST_F(gtestview, m_returnButton)
{
    m_returnButton=new ReturnButton();
    m_returnButton->show();
    m_returnButton->normalPic();
    m_returnButton-> hoverPic() ;
    m_returnButton-> pressPic() ;
    m_returnButton-> disablePic() ;
    m_returnButton-> checkedPic() ;
    m_returnButton-> text() ;
    m_returnButton-> normalColor() ;
    m_returnButton-> hoverColor() ;
    m_returnButton-> pressColor() ;
    m_returnButton-> disableColor() ;
    m_returnButton-> getSpacing() ;
    m_returnButton-> setSpacing(11);
    m_returnButton-> getChecked() ;
    m_returnButton-> setToolTip("TEST");
    m_returnButton-> setChecked(false);
    m_returnButton-> setNormalPic(JPEGPATH);
    m_returnButton-> setHoverPic(JPEGPATH);
    m_returnButton-> setPressPic(JPEGPATH);
    m_returnButton-> setDisablePic(JPEGPATH);
    m_returnButton-> setCheckedPic(JPEGPATH);
    m_returnButton-> setText("test");
    m_returnButton-> setNormalColor(QColor(120,111,150));
    m_returnButton-> setHoverColor(QColor(120,180,150));
    m_returnButton-> setPressColor(QColor(120,20,150));
    m_returnButton-> setDisableColor(QColor(120,123,150));

    m_returnButton-> setToolTip("TEST");
    m_returnButton-> setChecked(true);
    m_returnButton-> setNormalPic(JPEGPATH);
    m_returnButton-> setHoverPic(JPEGPATH);
    m_returnButton-> setPressPic(JPEGPATH);
    m_returnButton-> setDisablePic(JPEGPATH);
    m_returnButton-> setCheckedPic(JPEGPATH);
    m_returnButton-> setText("test");
    m_returnButton-> setNormalColor(QColor(120,111,150));
    m_returnButton-> setHoverColor(QColor(120,180,150));
    m_returnButton-> setPressColor(QColor(120,20,150));
    m_returnButton-> setDisableColor(QColor(120,123,150));

    QTest::mousePress(m_returnButton, Qt::LeftButton);
    QTest::mouseRelease(m_returnButton, Qt::LeftButton);
    QTest::mouseClick(m_returnButton, Qt::LeftButton);
    QTest::mouseMove(m_returnButton, QPoint(50,50));
    QTest::keyClick(m_returnButton, Qt::Key_Escape, Qt::ShiftModifier, 30);
    QTest::mouseDClick(m_returnButton,Qt::LeftButton);

}
TEST_F(gtestview, m_bottomToolbar)
{
    /*frame*/
    m_bottomToolbar =new BottomToolbar(nullptr);
    m_bottomToolbar->show();
    QTest::mousePress(m_bottomToolbar, Qt::LeftButton);
    QTest::mouseRelease(m_bottomToolbar, Qt::LeftButton);
    QTest::mouseClick(m_bottomToolbar, Qt::LeftButton);
    QTest::mouseMove(m_bottomToolbar, QPoint(50,50));
    //        QTest::keyClick(m_bottomToolbar, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_bottomToolbar,Qt::LeftButton);
}
TEST_F(gtestview, m_frameMainWidget)
{
    m_frameMainWidget=new MainWidget(false,nullptr);
    m_frameMainWidget->show();
    QTest::mousePress(m_frameMainWidget, Qt::LeftButton);
    QTest::mouseRelease(m_frameMainWidget, Qt::LeftButton);
    QTest::mouseClick(m_frameMainWidget, Qt::LeftButton);
    QTest::mouseMove(m_frameMainWidget, QPoint(230,50));
    //        QTest::keyClick(m_frameMainWidget, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_frameMainWidget,Qt::LeftButton);
}
TEST_F(gtestview, RenameDialog)
{
    m_renameDialog=new RenameDialog(nullptr);
    m_renameDialog->show();
    QTest::mousePress(m_renameDialog, Qt::LeftButton);
    QTest::mouseRelease(m_renameDialog, Qt::LeftButton);
    QTest::mouseClick(m_renameDialog, Qt::LeftButton);
    QTest::mouseMove(m_renameDialog, QPoint(190,100));
    //        QTest::keyClick(m_renameDialog, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_renameDialog,Qt::LeftButton);

    m_renameDialog->onThemeChanged(ViewerThemeManager::Light);
    //        m_frameMainWindow->OpenImage("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
    m_renameDialog->onThemeChanged(ViewerThemeManager::Dark);
}
TEST_F(gtestview, TopToolbar)
{
    m_topoolBar=new TopToolbar(false,nullptr);
    m_topoolBar->show();
    QTest::mousePress(m_topoolBar, Qt::LeftButton);
    QTest::mouseRelease(m_topoolBar, Qt::LeftButton);
    QTest::mouseClick(m_topoolBar, Qt::LeftButton);
    QTest::mouseMove(m_topoolBar, QPoint(190,160));
    //        QTest::keyClick(m_topoolBar, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_topoolBar,Qt::LeftButton);

    m_topoolBar->setMiddleContent("test");
    m_topoolBar->setTitleBarTransparent(true);
    m_topoolBar->setTitleBarTransparent(false);
    //        QTest::keyClick(m_topoolBar, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_topoolBar,Qt::LeftButton);
}
TEST_F(gtestview, LockWidget)
{
    m_lockWidget=new LockWidget("","");
    m_lockWidget->show();
    QTest::mousePress(m_lockWidget, Qt::LeftButton);
    QTest::mouseRelease(m_lockWidget, Qt::LeftButton);
    QTest::mouseClick(m_lockWidget, Qt::LeftButton);
    QTest::mouseMove(m_lockWidget, QPoint(50,160));
    //        QTest::keyClick(m_lockWidget, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_lockWidget,Qt::LeftButton);
}
TEST_F(gtestview, ThumbnailWidget)
{
    m_thumbnailWidget=new ThumbnailWidget(":/assets/dark/images/icon_import_photo dark.svg",":/assets/light/images/icon_import_photo.svg");
    m_thumbnailWidget->show();
    QTest::mousePress(m_thumbnailWidget, Qt::LeftButton);
    QTest::mouseRelease(m_thumbnailWidget, Qt::LeftButton);
    QTest::mouseClick(m_thumbnailWidget, Qt::LeftButton);
    QTest::mouseMove(m_thumbnailWidget, QPoint(130,160));
    //        QTest::keyClick(m_thumbnailWidget, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_thumbnailWidget,Qt::LeftButton);
}
TEST_F(gtestview, NavigationWidget)
{
    m_navigationWidget=new NavigationWidget();
    m_navigationWidget->show();
    QTest::mousePress(m_navigationWidget, Qt::LeftButton);
    QTest::mouseRelease(m_navigationWidget, Qt::LeftButton);
    QTest::mouseClick(m_navigationWidget, Qt::LeftButton);
    QTest::mouseMove(m_navigationWidget, QPoint(700,160));
    //        QTest::keyClick(m_navigationWidget, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_navigationWidget,Qt::LeftButton);
}
TEST_F(gtestview, ImageIconButton)
{
    m_ImageIconButton1=new ImageIconButton();
    m_ImageIconButton1->setPropertyPic(":/assets/dark/images/icon_import_photo dark.svg",QVariant(),":/assets/dark/images/icon_import_photo dark.svg",":/assets/dark/images/icon_import_photo dark.svg",":/assets/dark/images/icon_import_photo dark.svg",":/assets/dark/images/icon_import_photo dark.svg");
    m_ImageIconButton1->show();
    m_ImageIconButton2=new ImageIconButton(":/assets/dark/images/icon_import_photo dark.svg",":/assets/dark/images/icon_import_photo dark.svg",":/assets/dark/images/icon_import_photo dark.svg",":/assets/dark/images/icon_import_photo dark.svg");
    m_ImageIconButton2->setPropertyPic(":/assets/dark/images/icon_import_photo dark.svg",QVariant(),":/assets/dark/images/icon_import_photo dark.svg",":/assets/dark/images/icon_import_photo dark.svg",":/assets/dark/images/icon_import_photo dark.svg",":/assets/dark/images/icon_import_photo dark.svg");
    m_ImageIconButton2->show();
    m_ImageIconButton2->setAutoChecked(false);
    m_ImageIconButton2->setTransparent(false);
}
TEST_F(gtestview, ImageInfoWidget)
{
    m_ImageInfoWidget= new ImageInfoWidget("","");
    m_ImageInfoWidget->setImagePath(JPEGPATH);
    m_ImageInfoWidget->onExpandChanged(false);
    m_ImageInfoWidget->contentHeight();
}
TEST_F(gtestview, ImageView)
{
    m_ImageView=new ImageView();
    m_ImageView->setImage(JPEGPATH);
    m_ImageView->show();
    m_ImageView->cachePixmap(JPEGPATH);
    m_ImageView->fitWindow();
    m_ImageView->fitWindow_btnclicked();
    m_ImageView->fitImage();
    m_ImageView->rotateClockWise();
    m_ImageView->rotateCounterclockwise();

    m_ImageView->autoFit();
    m_ImageView->titleBarControl();
    m_ImageView->image();
    m_ImageView->windowRelativeScale();
    m_ImageView->windowRelativeScale_origin();
    m_ImageView->imageRect();
    m_ImageView->path();

    m_ImageView->visibleImageRect();
    m_ImageView->isWholeImageVisible();
    m_ImageView->isFitImage();
    m_ImageView->isFitWindow();
    m_ImageView->rotatePixCurrent();

    //        m_ImageView->showPixmap("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
    //        m_ImageView->loadPictureByType(m_ImageView->judgePictureType("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg"),"/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
    emit m_ImageView->clicked();
    emit m_ImageView->doubleClicked();
    emit m_ImageView->mouseHoverMoved();
    emit m_ImageView->scaled(200);
    emit m_ImageView->transformChanged();
    emit m_ImageView->showScaleLabel();
    emit m_ImageView->nextRequested();
    emit m_ImageView->previousRequested();
    emit m_ImageView->disCheckAdaptImageBtn();
    emit m_ImageView->checkAdaptImageBtn();
    emit m_ImageView->clicked();
    m_ImageView->mapToImage(QPoint());
    m_ImageView->mapToImage(QRect());
    m_ImageView->centerOn(5,5);
    m_ImageView->setRenderer();
    m_ImageView->setScaleValue(qreal());
}
TEST_F(gtestview, m_ScanPathsItem)
{
    VolumeMonitor::instance();
    ScanPathsItem* m_ScanPathsItem=new ScanPathsItem("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
    m_ScanPathsItem->show();
}
TEST_F(gtestview, dapp)
{
    dApp->getRwLock();
    dApp->loadInterface("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
//    dApp->loadPixThread(QStringList("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg"));

    dApp->signalM->emit sendPathlist(list,"/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
    dApp->wpSetter;
    dApp->viewerTheme;
    dApp->setter;
    DBManager::instance()->insertIntoAlbum("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg",list);
    DBManager::instance()->getAllPaths();
    DBManager::instance()->getInfosByTimeline("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
    DBManager::instance()->getInfoByPath("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");

    Importer::instance()->isRunning();
    Importer::instance()->appendDir("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg","/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
    Importer::instance()->appendFiles(list,"/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
    //        Importer::instance()->showImportDialog("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
}
TEST_F(gtestview, BlurFrame)
{
    m_printOptionspage=new PrintOptionsPage();
    m_printOptionspage->show();
    m_blurFrame=new BlurFrame(m_printOptionspage);
    m_blurFrame->moveWithAnimation( 1200, 1000);

    m_blurFrame->getBorderColor() ;
    m_blurFrame->getBorderRadius() ;
    m_blurFrame->getBorderWidth() ;
    m_blurFrame->show();
    m_blurFrame->setBorderColor(QColor(200,155,200));
    m_blurFrame->resize(600,500);
    m_blurFrame->setBorderRadius(100);
    m_blurFrame->setBorderWidth(50);
    m_blurFrame->setCoverBrush(QBrush());
    m_blurFrame->setPos(QPoint(200,500));
    m_blurFrame->setMoveEnable(true);
    QTest::mousePress(m_blurFrame, Qt::LeftButton);
    QTest::mouseRelease(m_blurFrame, Qt::LeftButton);
    QTest::mouseClick(m_blurFrame, Qt::LeftButton);
    QTest::mouseMove(m_blurFrame, QPoint(50,50));
    QTest::keyClick(m_blurFrame, Qt::Key_Escape, Qt::ShiftModifier, 20);
    QTest::mouseDClick(m_blurFrame,Qt::LeftButton);
}
TEST_F(gtestview, unionimage)
{
    //unionimage
    //        UnionImage_NameSpace::noneQImage();
    QString pppath="/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg";
    supportStaticFormat();
    supportMovieFormat();
    isSupportReading(pppath);
    QImage rimg(pppath);
    creatNewImage(rimg,800,600,0);
    QImage img2;
    QString errorMsg;
    loadStaticImageFromFile(DDSPATH,img2,errorMsg);
    detectImageFormat(DDSPATH);
    isNoneQImage(img2);
    rotateImage(90,img2);
    QString ddsPath=DDSPATH;
    QString svgPath=SVGPATH;
    rotateImageFIle(90,SVGPATH,errorMsg);
    rotateImageFIle(90,pppath,errorMsg);
    rotateImageFIleWithImage(90,rimg,"1.jpg",errorMsg);
    rotateImageFIleWithImage(90,rimg,"1.svg",errorMsg);
    DetectImageFormat(SVGPATH);
}
TEST_F(gtestview, imageutil)
{
    //        m_scrollBar=new ScrollBar();
    //        m_scrollBar->show();
    qDebug()<<"imageutil1";
    showInFileManager("");
//    showInFileManager("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
    qDebug()<<"imageutil22";
    copyImageToClipboard(list);
    trashFile("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg");
    wrapStr("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg",QFont(),20);
    qDebug()<<"imageutil23";
    SpliteText("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg",QFont(),20,false);
    SpliteText("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg",QFont(),20,true);

     qDebug()<<"imageutil2";
    QString ppath=JPEGPATH;
    onMountDevice(ppath);
    mountDeviceExist(ppath);

     qDebug()<<"imageutil3";
    //utils::image
    QString pppath=JPEGPATH;
    imageSupportWrite(pppath);
    rotate(pppath,90);
    cutSquareImage(QPixmap(pppath),QSize(50,50));
    utils::image::getOrientation(pppath);
    bool iRet=false;
    //        utils::image::loadTga(pppath,iRet);

     qDebug()<<"imageutil4";
    getRotatedImage(pppath);
    cachePixmap(pppath);
    getThumbnail(pppath,iRet);
    supportedImageFormats();
    imageSupportWallPaper(pppath);
    utils::image::suffixisImage(pppath);
}

TEST_F(gtestview, ExtensionPanel)
{
    m_extensionPanel=new ExtensionPanel(nullptr);
    m_extensionPanel->show();
    m_extensionPanel->setContent(new QWidget());
    m_extensionPanel->setContent(nullptr);

    QTest::mousePress(m_extensionPanel, Qt::LeftButton);
    QTest::mouseRelease(m_extensionPanel, Qt::LeftButton);
    QTest::mouseClick(m_extensionPanel, Qt::LeftButton);
    QTest::mouseMove(m_extensionPanel, QPoint(150,50));
    //        QTest::keyClick(m_extensionPanel, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_extensionPanel,Qt::LeftButton);
    m_extensionPanel->setContent(new QWidget());
    m_extensionPanel->setContent(nullptr);
    QTest::mousePress(m_extensionPanel, Qt::LeftButton);
    QTest::mouseRelease(m_extensionPanel, Qt::LeftButton);
    QTest::mouseClick(m_extensionPanel, Qt::LeftButton);
    QTest::mouseMove(m_extensionPanel, QPoint(50,50));
    //        QTest::keyClick(m_extensionPanel, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_extensionPanel,Qt::LeftButton);
}
TEST_F(gtestview, initTest2)
{



}
//延时推出程序
TEST_F(gtestview, initTest1)
{
    ScanPathsDialog::instance()->addPath(JPEGPATH);

    QTimer::singleShot(10000,[=]{
        return exit(0);
    });
}
