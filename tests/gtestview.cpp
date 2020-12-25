#include "gtestview.h"
#define  private public
#include <QKeySequence>
#include <QShortcut>

#include <QFile>
#include <QDir>
#include "accessibility/ac-desktop-define.h"
#include "widgets/toast.h"
#include "viewer/src/widgets/elidedlabel.h"
#include "viewer/src/dirwatcher/scanpathsdialog.h"
#include "dirwatcher/volumemonitor.h"
#include "dirwatcher/scanpathsitem.h"
#include "module/view/scen/imageview.h"
gtestview::gtestview()
{

}
TEST_F(gtestview,cpFile)
{
     QFile::copy(":/gif.gif",QApplication::applicationDirPath()+"/gif.gif");
     QFile(QApplication::applicationDirPath()+"/gif.gif").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/gif2.gif",QApplication::applicationDirPath()+"/gif2.gif");
     QFile(QApplication::applicationDirPath()+"/gif2.gif").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/ico.ico",QApplication::applicationDirPath()+"/ico.ico");
     QFile(QApplication::applicationDirPath()+"/ico.ico").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/jpg.jpg",QApplication::applicationDirPath()+"/jpg.jpg");
     QFile(QApplication::applicationDirPath()+"/jpg.jpg").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/mng.mng",QApplication::applicationDirPath()+"/mng.mng");
     QFile(QApplication::applicationDirPath()+"/mng.mng").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/png.png",QApplication::applicationDirPath()+"/png.png");
     QFile(QApplication::applicationDirPath()+"/png.png").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/svg.svg",QApplication::applicationDirPath()+"/svg.svg");
     QFile(QApplication::applicationDirPath()+"/svg.svg").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/svg1.svg",QApplication::applicationDirPath()+"/svg1.svg");
     QFile(QApplication::applicationDirPath()+"/svg1.svg").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/svg2.svg",QApplication::applicationDirPath()+"/svg2.svg");
     QFile(QApplication::applicationDirPath()+"/svg2.svg").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/svg1.svg",QApplication::applicationDirPath()+"/svg3.svg");
     QFile(QApplication::applicationDirPath()+"/svg3.svg").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/tif.tif",QApplication::applicationDirPath()+"/tif.tif");
     QFile(QApplication::applicationDirPath()+"/tif.tif").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/wbmp.wbmp",QApplication::applicationDirPath()+"/wbmp.wbmp");
     QFile(QApplication::applicationDirPath()+"/wbmp.wbmp").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/dds.dds",QApplication::applicationDirPath()+"/dds.dds");
     QFile(QApplication::applicationDirPath()+"/dds.dds").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/tga.tga",QApplication::applicationDirPath()+"/tga.tga");
     QFile(QApplication::applicationDirPath()+"/tga.tga").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QFile::copy(":/errorPic.icns",QApplication::applicationDirPath()+"/errorPic.icns");
     QFile(QApplication::applicationDirPath()+"/errorPic.icns").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);

     QDir a(QApplication::applicationDirPath());
     a.mkdir("test");
     QFile::copy(":/jpg.jpg",QApplication::applicationDirPath()+"/test/jpg.jpg");
     QFile(QApplication::applicationDirPath()+"/test/jpg.jpg").setPermissions( \
                 QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                 QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);
}


//主窗体

TEST_F(gtestview, CommandLine)
{
    DApplicationSettings saveTheme;
    //4.将时间写入QDataStream
    QDateTime wstime = QDateTime::currentDateTime();
    QString teststr = wstime.toString("yyyy-MM-dd hh:mm:ss");
    bool newflag = true;
    CommandLine::instance()->processOption();
    QTest::qWait(500);
    CommandLine::instance()->processOption(wstime, newflag);
    QTest::qWait(500);

}
#ifdef test_main
TEST_F(gtestview, mainwindow)
{

    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(400);

    ViewPanel *panel = m_frameMainWindow->findChild<ViewPanel *>(VIEW_PANEL_WIDGET);
    if(panel){
        // 打开保存绘制的 tif
        QString TriangleItemPath = QApplication::applicationDirPath() + "/tif.tif";
        TestApi::drogPathtoWidget(panel,TriangleItemPath);
    }

    m_frameMainWindow->setWindowRadius(18);
    m_frameMainWindow->setBorderWidth(0);
    m_frameMainWindow->show();
    QTest::mousePress(m_frameMainWindow, Qt::LeftButton);
    QTest::mouseRelease(m_frameMainWindow, Qt::LeftButton);
    QTest::mouseClick(m_frameMainWindow, Qt::LeftButton);
    QTest::mouseMove(m_frameMainWindow, QPoint(190,50));
    QTest::mouseDClick(m_frameMainWindow,Qt::LeftButton);

//    m_frameMainWindow->OpenImage(m_GIFPath);






    QTest::keyClick(m_frameMainWindow, Qt::Key_Up, Qt::ControlModifier, 200);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Up, Qt::ControlModifier, 200);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Up, Qt::ControlModifier, 200);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Down, Qt::ControlModifier, 200);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Down, Qt::ControlModifier, 200);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Down, Qt::ControlModifier, 200);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Right, Qt::NoModifier, 200);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Right, Qt::NoModifier, 200);
    int index1=0;
    while(index1++<20)
    {
        QTest::keyClick(m_frameMainWindow, Qt::Key_Left, Qt::NoModifier, 100);
    }
    while(index1-->0)
    {
         QTest::keyClick(m_frameMainWindow, Qt::Key_Right, Qt::NoModifier, 100);
    }
    QTest::keyClick(m_frameMainWindow, Qt::Key_Left, Qt::NoModifier, 200);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Up, Qt::ControlModifier, 100);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Up, Qt::ControlModifier, 100);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Up, Qt::ControlModifier, 100);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Down, Qt::ControlModifier, 100);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Down, Qt::ControlModifier, 100);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Down, Qt::ControlModifier, 100);

    QTest::keyClick(m_frameMainWindow, Qt::Key_F11, Qt::NoModifier, 2000);//
    int index=0;
    while(index++<10)
        QTest::keyClick(m_frameMainWindow, Qt::Key_Plus, Qt::ControlModifier, 10);
    while(index-->0)
        QTest::keyClick(m_frameMainWindow, Qt::Key_Minus, Qt::ControlModifier, 10);
    QTest::mouseMove(m_frameMainWindow, QPoint(1000,1075),500);
    QTest::mouseMove(m_frameMainWindow, QPoint(200,500),500);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Escape, Qt::NoModifier, 500);

}

TEST_F(gtestview, sliderShow)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(100);
    m_frameMainWindow->showNormal();
    m_frameMainWindow->resize(800,600);
    QTest::keyClick(m_frameMainWindow, Qt::Key_R, Qt::ControlModifier | Qt::ShiftModifier, 100);
    QTest::keyClick(m_frameMainWindow, Qt::Key_R, Qt::ControlModifier , 500);
    QTest::keyClick(m_frameMainWindow, Qt::Key_F5, Qt::NoModifier, 3000);
    QTest::qWait(10000);
    QTest::mouseMove(m_frameMainWindow, QPoint(1000,1075),1000);
    QTest::mouseMove(m_frameMainWindow, QPoint(200,500),1000);
    SlideShowPanel * sliderShow = m_frameMainWindow->findChild<SlideShowPanel *>(SLIDE_SHOW_WIDGET);

    m_frameMainWindow->customContextMenuRequested(QPoint(300,300));
    QTest::mousePress(m_frameMainWindow, Qt::LeftButton,Qt::NoModifier,QPoint(100,1020),300);
    QTest::mouseRelease(m_frameMainWindow, Qt::LeftButton,Qt::NoModifier,QPoint(200,1020),300);
    QTest::mouseClick(m_frameMainWindow, Qt::LeftButton,Qt::NoModifier,QPoint(300,1020),300);
    QTest::mouseMove(m_frameMainWindow, QPoint(400,1020),300);
    QTest::mouseDClick(m_frameMainWindow,Qt::LeftButton,Qt::NoModifier,QPoint(500,1020),300);
    QTest::keyClick(m_frameMainWindow, Qt::Key_I, Qt::ControlModifier , 500);

    QTest::mouseClick(m_frameMainWindow, Qt::RightButton,Qt::NoModifier,QPoint(300,1020),300);

    QTest::mouseClick(m_frameMainWindow, Qt::MidButton,Qt::NoModifier,QPoint(300,1020),300);
    if(sliderShow)
    {
        sliderShow->customContextMenuRequested(QPoint(300,300));
        sliderShow->moduleName();
        sliderShow->extensionPanelContent() ;
        sliderShow->toolbarBottomContent() ;
        sliderShow->toolbarTopMiddleContent() ;
        sliderShow->toolbarTopLeftContent() ;

        DIconButton *buttonPre=dApp->findChild <DIconButton *>("PreviousButton");
        if(buttonPre)
        {
            buttonPre->click();

        }
        DIconButton * PlayPauseButton=dApp->findChild <DIconButton *>("PlayPauseButton");
        if(PlayPauseButton)
        {
            PlayPauseButton->click();
            QTest::qWait(100);
            PlayPauseButton->click();
        }

        DIconButton *nextButton=dApp->findChild <DIconButton *>("NextButton");
        if(nextButton)
        {
            nextButton->click();

        }

        DIconButton *CancelButton=dApp->findChild <DIconButton *>("CancelButton");
        if(CancelButton)
        {
            CancelButton->click();

        }
        QTest::mouseMove(sliderShow, QPoint(600,500),500);
        QTest::mouseMove(sliderShow, QPoint(800,500),500);
        QTest::mouseClick(sliderShow, Qt::LeftButton,Qt::NoModifier,QPoint(300,600),300);
        QTest::mouseDClick(sliderShow, Qt::LeftButton,Qt::NoModifier,QPoint(400,600),300);
        QMenu *menu=m_frameMainWindow->findChild<QMenu *>(SLIDER_SHOW_MENU);
        QAction* action1=   m_frameMainWindow->findChild<QAction *> ("MenuID"+QString::number(SlideShowPanel::IdPlayOrPause))  ;
        if(action1)
        {
            qDebug()<<"menu->triggered(action1);";
            menu->triggered(action1);
            QTest::qWait(300);
            menu->triggered(action1);
        }

    }

    QTest::keyClick(m_frameMainWindow, Qt::Key_Escape, Qt::NoModifier, 50);

    m_frameMainWindow->hide();
    m_frameMainWindow->showNormal();


}
TEST_F(gtestview, infoWidget)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::keyClick(m_frameMainWindow, Qt::Key_Escape, Qt::NoModifier, 100);
    m_frameMainWindow->resize(800,600);
    QTest::keyClick(m_frameMainWindow, Qt::Key_I, Qt::ControlModifier , 100);

    QTest::keyClick(m_frameMainWindow, Qt::Key_Escape, Qt::NoModifier, 50);
}

TEST_F(gtestview, iconRotatePic)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    DIconButton *m_adaptImageBtn=m_frameMainWindow->findChild<DIconButton *>(ADAPT_BUTTON);
    DIconButton *m_adaptScreenBtn=m_frameMainWindow->findChild<DIconButton *>(ADAPT_SCREEN_BUTTON);
    DIconButton *m_preButton=m_frameMainWindow->findChild<DIconButton *>(PRE_BUTTON);
    DIconButton *m_nextButton=m_frameMainWindow->findChild<DIconButton *>(NEXT_BUTTON);
    DIconButton *m_rotateRBtn=m_frameMainWindow->findChild<DIconButton *>(CLOCKWISE_ROTATION);
    DIconButton *m_rotateLBtn=m_frameMainWindow->findChild<DIconButton *>(COUNTER_CLOCKWISE_ROTATION);
    DIconButton *m_trashBtn=m_frameMainWindow->findChild<DIconButton *>(TRASH_BUTTON);
    ViewPanel *panel = m_frameMainWindow->findChild<ViewPanel *>(VIEW_PANEL_WIDGET);
    if(panel){
        m_adaptImageBtn->click();
        QTest::qWait(100);
        m_adaptScreenBtn->click();
        QTest::qWait(100);
        m_preButton->click();
        QTest::qWait(100);
        m_nextButton->click();
        QTest::qWait(100);
        m_rotateRBtn->click();
        QTest::qWait(100);
        m_rotateLBtn->click();
        QTest::qWait(100);
        m_nextButton->click();
        QTest::qWait(100);
        m_adaptScreenBtn->click();
        QTest::qWait(100);
        m_preButton->click();
        QTest::qWait(100);
        m_nextButton->click();
        QTest::qWait(100);
        m_rotateRBtn->click();
        QTest::qWait(100);
        m_rotateLBtn->click();
        QTest::qWait(100);
        m_nextButton->click();
        QTest::qWait(100);
        m_adaptScreenBtn->click();
        QTest::qWait(100);
        m_preButton->click();
        QTest::qWait(100);
        m_nextButton->click();
        QTest::qWait(100);
        m_rotateRBtn->click();
        QTest::qWait(100);
        m_rotateLBtn->click();
        QTest::qWait(100);
        m_nextButton->click();
        QTest::qWait(100);
        m_adaptScreenBtn->click();
        QTest::qWait(100);
        m_preButton->click();
        QTest::qWait(100);
        m_nextButton->click();
        QTest::qWait(100);
        m_rotateRBtn->click();
        QTest::qWait(100);
        m_rotateLBtn->click();
        QTest::qWait(100);
        m_nextButton->click();
        QTest::qWait(100);
        m_adaptScreenBtn->click();
        QTest::qWait(100);
        m_preButton->click();
        QTest::qWait(100);
        m_nextButton->click();
        QTest::qWait(100);
        m_rotateRBtn->click();
        QTest::qWait(100);
        m_rotateLBtn->click();
        QTest::qWait(100);
        m_nextButton->click();
        QTest::qWait(100);
        m_adaptScreenBtn->click();
        QTest::qWait(100);
        m_preButton->click();
        QTest::qWait(100);
        m_nextButton->click();
        QTest::qWait(100);
        m_rotateRBtn->click();
        QTest::qWait(100);
        m_rotateLBtn->click();
        QTest::qWait(1000);
        m_nextButton->click();
        QTest::qWait(100);



    }

}

TEST_F(gtestview, onlyonePic)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(200);
    //    SLIDE_SHOW_WIDGET
    ViewPanel *panel = m_frameMainWindow->findChild<ViewPanel *>(VIEW_PANEL_WIDGET);
    if(panel){
        // 打开 jpg
        QString TriangleItemPath = QApplication::applicationDirPath() + "/test/jpg.jpg";
        TestApi::drogPathtoWidget(panel,TriangleItemPath);
        QTest::qWait(2000);

        DIconButton *m_adaptImageBtn=m_frameMainWindow->findChild<DIconButton *>(ADAPT_BUTTON);
        DIconButton *m_adaptScreenBtn=m_frameMainWindow->findChild<DIconButton *>(ADAPT_SCREEN_BUTTON);
        DIconButton *m_rotateRBtn=m_frameMainWindow->findChild<DIconButton *>(CLOCKWISE_ROTATION);
        DIconButton *m_rotateLBtn=m_frameMainWindow->findChild<DIconButton *>(COUNTER_CLOCKWISE_ROTATION);
        DIconButton *m_trashBtn=m_frameMainWindow->findChild<DIconButton *>(TRASH_BUTTON);

        m_adaptImageBtn->click();
        QTest::qWait(100);
        m_adaptScreenBtn->click();
        QTest::qWait(100);
        m_rotateRBtn->click();
        QTest::qWait(100);
        m_rotateLBtn->click();
        QTest::qWait(100);

        int index=0;
        while(index++<10)
            QTest::keyClick(m_frameMainWindow, Qt::Key_Plus, Qt::ControlModifier, 20);
        while(index-->0)
            QTest::keyClick(m_frameMainWindow, Qt::Key_Minus, Qt::ControlModifier, 20);

        m_frameMainWindow->showFullScreen();
        QTest::keyClick(m_frameMainWindow, Qt::Key_Escape, Qt::NoModifier, 200);
        m_frameMainWindow->showNormal();

        QTest::keyClick(m_frameMainWindow, Qt::Key_F5, Qt::NoModifier, 200);
        QTest::keyClick(m_frameMainWindow, Qt::Key_Escape, Qt::NoModifier, 200);
        m_frameMainWindow->showNormal();

        m_trashBtn->click();
        QTest::qWait(50);
    }

}

TEST_F(gtestview, sliderSvg)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    //    SLIDE_SHOW_WIDGET
    ViewPanel *panel = m_frameMainWindow->findChild<ViewPanel *>(VIEW_PANEL_WIDGET);
    if(panel){
        // 打开 svg
        QString TriangleItemPath = QApplication::applicationDirPath() + "/svg.svg";
        TestApi::drogPathtoWidget(panel,TriangleItemPath);
    }
    QTest::keyClick(m_frameMainWindow, Qt::Key_F5, Qt::NoModifier, 100);
    QTest::keyClick(m_frameMainWindow, Qt::Key_Escape, Qt::NoModifier, 200);
    m_frameMainWindow->hide();
    m_frameMainWindow->showMaximized();
    m_frameMainWindow->showNormal();
}
TEST_F(gtestview, MyImageListWidget)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    MyImageListWidget *widget = m_frameMainWindow->findChild<MyImageListWidget *>(IMAGE_LIST_WIDGET);
    ASSERT_NE(widget,nullptr);

    QTestEventList e;
    e.addMouseMove(QPoint(100,30), 200);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(100,30), 200);
    e.addMouseMove(QPoint(150,30), 200);
    e.addMouseMove(QPoint(200,30), 200);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(250,30), 200);
    e.simulate(widget);
    e.clear();


}
TEST_F(gtestview, testDrag)
{

}
//设置背景颜色
TEST_F(gtestview, ViewerThemeManager)
{

     dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
     dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Light);


}

TEST_F(gtestview, ImageButton)
{
    ImageButton *button=new ImageButton();

    button->setDisablePic(QApplication::applicationDirPath()+"/png.png");
    button->setDisabled(false);
    button->setToolTip("test");
    button->getDisablePic();
    button->show();
    button->resize(100,100);
    QTest::qWait(100);
    QTest::mousePress(button, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),250);
    QTest::qWait(100);
    QTest::mouseRelease(button, Qt::LeftButton);
    QTest::mouseClick(button, Qt::LeftButton);
    QTest::mouseMove(button, QPoint(20,20),250);
    QTest::keyClick(button, Qt::Key_Escape, Qt::ShiftModifier, 1000);
    QTest::mouseDClick(button,Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);

    button->hide();

}
TEST_F(gtestview, m_pushbutton)
{
    m_pushbutton=new PushButton();

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
    m_pushbutton-> setNormalPic(m_JPGPath);
    m_pushbutton-> setHoverPic(m_JPGPath);
    m_pushbutton-> setPressPic(m_JPGPath);
    m_pushbutton-> setDisablePic(m_JPGPath);
    m_pushbutton-> setCheckedPic(m_JPGPath);
    m_pushbutton-> setText("test");
    m_pushbutton-> setNormalColor(QColor(120,111,150));
    m_pushbutton-> setHoverColor(QColor(120,180,150));
    m_pushbutton-> setPressColor(QColor(120,20,150));
    m_pushbutton-> setDisableColor(QColor(120,123,150));

    m_pushbutton-> setToolTip("TEST");
    m_pushbutton-> setChecked(true);
    m_pushbutton-> setNormalPic(m_JPGPath);
    m_pushbutton-> setHoverPic(m_JPGPath);
    m_pushbutton-> setPressPic(m_JPGPath);
    m_pushbutton-> setDisablePic(m_JPGPath);
    m_pushbutton-> setCheckedPic(m_JPGPath);
    m_pushbutton-> setText("test");
    m_pushbutton-> setNormalColor(QColor(120,111,150));
    m_pushbutton-> setHoverColor(QColor(120,180,150));
    m_pushbutton-> setPressColor(QColor(120,20,150));
    m_pushbutton-> setDisableColor(QColor(120,123,150));

    m_pushbutton->show();

    QTest::mousePress(m_pushbutton, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
    QTest::mouseRelease(m_pushbutton, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
    QTest::mouseClick(m_pushbutton, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
    QTest::mouseMove(m_pushbutton, QPoint(20,20),500);
    QTest::keyClick(m_pushbutton, Qt::Key_Escape, Qt::ShiftModifier, 1000);
    QTest::mouseDClick(m_pushbutton,Qt::LeftButton);

    m_pushbutton->hide();
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
    m_returnButton-> setNormalPic(m_JPGPath);
    m_returnButton-> setHoverPic(m_JPGPath);
    m_returnButton-> setPressPic(m_JPGPath);
    m_returnButton-> setDisablePic(m_JPGPath);
    m_returnButton-> setCheckedPic(m_JPGPath);
    m_returnButton-> setText("test");
    m_returnButton-> setNormalColor(QColor(120,111,150));
    m_returnButton-> setHoverColor(QColor(120,180,150));
    m_returnButton-> setPressColor(QColor(120,20,150));
    m_returnButton-> setDisableColor(QColor(120,123,150));

    m_returnButton-> setToolTip("TEST");
    m_returnButton-> setChecked(true);
    m_returnButton-> setNormalPic(m_JPGPath);
    m_returnButton-> setHoverPic(m_JPGPath);
    m_returnButton-> setPressPic(m_JPGPath);
    m_returnButton-> setDisablePic(m_JPGPath);
    m_returnButton-> setCheckedPic(m_JPGPath);
    m_returnButton-> setText("test");
    m_returnButton-> setNormalColor(QColor(120,111,150));
    m_returnButton-> setHoverColor(QColor(120,180,150));
    m_returnButton-> setPressColor(QColor(120,20,150));
    m_returnButton-> setDisableColor(QColor(120,123,150));
    m_returnButton->setMaxWidth(100);
    m_returnButton->buttonWidth();

    QTest::mousePress(m_returnButton, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
    QTest::mouseRelease(m_returnButton, Qt::LeftButton);
    QTest::mouseClick(m_returnButton, Qt::LeftButton);
    QTest::mouseMove(m_returnButton, QPoint(20,20),500);
    QTest::keyClick(m_returnButton, Qt::Key_Escape, Qt::ShiftModifier, 1000);
    QTest::mouseDClick(m_returnButton,Qt::LeftButton);


    m_returnButton->hide();

}

TEST_F(gtestview, LockWidget)
{
    m_lockWidget=new LockWidget("","");
    m_lockWidget->show();
    QTest::mousePress(m_lockWidget, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),200);
    QTest::mouseRelease(m_lockWidget, Qt::LeftButton);
    QTest::mouseClick(m_lockWidget, Qt::LeftButton);
    QTest::mouseMove(m_lockWidget, QPoint(20,20),200);
    QTest::keyClick(m_lockWidget, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_lockWidget,Qt::LeftButton);

    m_lockWidget->hide();
}
TEST_F(gtestview, ThumbnailWidget)
{
    m_thumbnailWidget=new ThumbnailWidget(m_SVGPath,m_SVGPath);
    m_thumbnailWidget->show();
    QTest::mousePress(m_thumbnailWidget, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),200);
    QTest::mouseRelease(m_thumbnailWidget, Qt::LeftButton);
    QTest::mouseClick(m_thumbnailWidget, Qt::LeftButton);
    QTest::mouseMove(m_thumbnailWidget, QPoint(20,20),200);
    QTest::keyClick(m_thumbnailWidget, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_thumbnailWidget,Qt::LeftButton);

    m_thumbnailWidget->hide();
}
TEST_F(gtestview, NavigationWidget)
{
    m_navigationWidget=new NavigationWidget();
    m_navigationWidget->show();
    QTest::mousePress(m_navigationWidget, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),200);
    QTest::mouseRelease(m_navigationWidget, Qt::LeftButton);
    QTest::mouseClick(m_navigationWidget, Qt::LeftButton);
    QTest::mouseMove(m_navigationWidget, QPoint(20,20),200);
    QTest::keyClick(m_navigationWidget, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_navigationWidget,Qt::LeftButton);

    m_navigationWidget->hide();
}
TEST_F(gtestview, ImageIconButton)
{
    m_ImageIconButton1=new ImageIconButton();
    m_ImageIconButton1->setPropertyPic(m_SVGPath,QVariant(),m_SVGPath,m_SVGPath,m_SVGPath,m_SVGPath);
    m_ImageIconButton1->show();
    m_ImageIconButton1->resize(50,50);
    QTest::mousePress(m_ImageIconButton1, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
    QTest::mouseRelease(m_ImageIconButton1, Qt::LeftButton);
    QTest::mouseClick(m_ImageIconButton1, Qt::LeftButton);
    QTest::mouseMove(m_ImageIconButton1, QPoint(20,20),500);
    QTest::keyClick(m_ImageIconButton1, Qt::Key_Escape, Qt::ShiftModifier, 1000);
    QTest::mouseDClick(m_ImageIconButton1,Qt::LeftButton);
    m_ImageIconButton1->setPropertyPic(m_SVGPath,m_SVGPath,m_SVGPath,m_SVGPath);
    m_ImageIconButton1->update();
    m_ImageIconButton1->setPropertyPic("","","","");
    m_ImageIconButton1->update();
    m_ImageIconButton2=new ImageIconButton(m_SVGPath,m_SVGPath,m_SVGPath,m_SVGPath);
    m_ImageIconButton2->setPropertyPic(m_SVGPath,QVariant(),m_SVGPath,m_SVGPath,m_SVGPath,m_SVGPath);
    m_ImageIconButton2->show();
    m_ImageIconButton2->setAutoChecked(false);
    m_ImageIconButton2->setTransparent(false);
    m_ImageIconButton2->resize(50,50);

    QTest::mousePress(m_ImageIconButton2, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),200);
    QTest::mouseRelease(m_ImageIconButton2, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),200);
    QTest::mouseClick(m_ImageIconButton2, Qt::LeftButton,Qt::NoModifier,QPoint(30,30),200);
    QTest::mouseMove(m_ImageIconButton2, QPoint(20,45),200);
    QTest::keyClick(m_ImageIconButton2, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_ImageIconButton2,Qt::LeftButton,Qt::NoModifier,QPoint(20,30),200);

    m_ImageIconButton1->hide();
    m_ImageIconButton2->hide();
}
TEST_F(gtestview, ImageInfoWidget)
{
    m_ImageInfoWidget= new ImageInfoWidget("","");
    m_ImageInfoWidget->setImagePath(m_JPGPath);
    m_ImageInfoWidget->onExpandChanged(false);
    m_ImageInfoWidget->contentHeight();

    m_ImageInfoWidget->hide();
}
TEST_F(gtestview, ImageView)
{
    m_ImageView=new ImageView();
    m_ImageView->setImage(m_JPGPath);
    m_ImageView->resize(500,500);
    m_ImageView->show();
    m_ImageView->cachePixmap(m_JPGPath);
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

    //        m_ImageView->showPixmap(m_JPGPath);
    //        m_ImageView->loadPictureByType(m_ImageView->judgePictureType(m_JPGPath),m_JPGPath);
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
    QTest::mousePress(m_ImageView->viewport(), Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
    QTest::mouseRelease(m_ImageView->viewport(), Qt::LeftButton,Qt::NoModifier,QPoint(100,100),200);
    QTest::mouseClick(m_ImageView->viewport(), Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
    QTest::mouseMove(m_ImageView->viewport(), QPoint(50,100),200);
    QTest::keyClick(m_ImageView->viewport(), Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(m_ImageView->viewport(),Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
    // 打开保存绘制的 tif
    QString TriangleItemPath = QApplication::applicationDirPath() + "/tif.tif";
    TestApi::drogPathtoWidget(m_ImageView->viewport(),TriangleItemPath);

    m_ImageView->hide();
}
TEST_F(gtestview, m_ScanPathsItem)
{
    VolumeMonitor::instance();
    ScanPathsItem* m_ScanPathsItem=new ScanPathsItem(m_JPGPath);
    m_ScanPathsItem->show();

    QTest::qWait(10);
    m_ScanPathsItem->hide();
}
TEST_F(gtestview, dapp)
{
    dApp->getRwLock();
    dApp->loadInterface(m_JPGPath);
    //    dApp->loadPixThread(QStringList(m_JPGPath));

    dApp->signalM->emit sendPathlist(list,m_JPGPath);
    dApp->wpSetter;
    dApp->viewerTheme;
    dApp->setter;
    DBManager::instance()->insertIntoAlbum(m_JPGPath,list);
    DBManager::instance()->getAllPaths();
    DBManager::instance()->getInfosByTimeline(m_JPGPath);
    DBManager::instance()->getInfoByPath(m_JPGPath);

    Importer::instance()->isRunning();
    Importer::instance()->appendDir(m_JPGPath,m_JPGPath);
    Importer::instance()->appendFiles(list,m_JPGPath);
    //        Importer::instance()->showImportDialog(m_JPGPath);
}
TEST_F(gtestview, BlurFrame)
{
    QWidget *testWidget=new QWidget();
    m_printOptionspage=new PrintOptionsPage(testWidget);
    m_printOptionspage->show();

    QRadioButton *noScaleBtn = m_printOptionspage->getnoScaleBtn();
    QRadioButton *fitToImageBtn = m_printOptionspage->getfitToImageBtn();
    QRadioButton *fitToPageBtn = m_printOptionspage->getfitToPageBtn();
    QRadioButton *scaleBtn = m_printOptionspage->getscaleBtn();
    if(noScaleBtn &&fitToImageBtn &&fitToPageBtn &&scaleBtn)
    {
        noScaleBtn->click();
        noScaleBtn->toggle();
        fitToImageBtn->click();
        fitToImageBtn->toggle();
        fitToPageBtn->click();
        fitToPageBtn->toggle();
        scaleBtn->click();
        scaleBtn->toggle();
    }
    m_printOptionspage->scaleMode();
    m_printOptionspage->scaleUnit();
    m_printOptionspage->scaleWidth();
    m_printOptionspage->scaleHeight();
    m_printOptionspage->alignment();

    BlurFrame *blurFrame=new BlurFrame(testWidget);
    blurFrame->resize(500,500);
    blurFrame->show();
    blurFrame->resize(300,300);
    blurFrame->update();
    blurFrame->moveWithAnimation( 150, 150);

    blurFrame->getBorderColor() ;
    blurFrame->getBorderRadius() ;
    blurFrame->getBorderWidth() ;
    blurFrame->show();
    blurFrame->setBorderColor(QColor(200,155,200));
    blurFrame->resize(200,200);
    blurFrame->setBorderRadius(100);
    blurFrame->setBorderWidth(50);
    blurFrame->setCoverBrush(QBrush());
    blurFrame->setPos(QPoint(150,150));
    blurFrame->setMoveEnable(true);

    blurFrame->update();
    blurFrame->show();
    QTest::mousePress(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
    QTest::mouseRelease(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(100,100),200);
    QTest::mouseClick(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
    QTest::mouseMove(blurFrame, QPoint(50,100),200);
    QTest::keyClick(blurFrame, Qt::Key_Escape, Qt::ShiftModifier, 200);
    QTest::mouseDClick(blurFrame,Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);

    QTest::qWait(100);
    blurFrame->hide();
    blurFrame->deleteLater();
    blurFrame=nullptr;
    testWidget->deleteLater();
    testWidget=nullptr;


}
TEST_F(gtestview, unionimage)
{
    //unionimage
    QString pppath=m_PNGPath;
    supportStaticFormat();
    supportMovieFormat();
    isSupportReading(pppath);
    QImage rimg(pppath);
    creatNewImage(rimg,800,600,0);
    QImage img2;
    QString errorMsg;
    loadStaticImageFromFile(m_DDSPath,img2,errorMsg);
    detectImageFormat(m_DDSPath);
    isNoneQImage(img2);
    rotateImage(90,img2);
    QString ddsPath=m_DDSPath;
    QString svgPath=m_SVGPath;
    rotateImageFIle(90,m_SVGPath,errorMsg);
    rotateImageFIle(90,pppath,errorMsg);
    rotateImageFIleWithImage(90,rimg,m_PNGPath,errorMsg);
    rotateImageFIleWithImage(90,rimg,m_SVGPath,errorMsg);
    DetectImageFormat(m_SVGPath);
}
TEST_F(gtestview, imageutil)
{
    qDebug()<<"imageutil1";
    showInFileManager("");
    qDebug()<<"imageutil22";
    copyImageToClipboard(list);
    trashFile(m_JPGPath);

    SpliteText("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg",QFont(),20,false);
    SpliteText("/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg/usr/share/wallpapers/deepin/Hummingbird_by_Shu_Le.jpg",QFont(),20,true);

    QString ppath=m_JPGPath;
    onMountDevice(ppath);
    mountDeviceExist(ppath);

    //utils::image
    QString pppath=m_JPGPath;
    imageSupportWrite(pppath);
    rotate(pppath,90);
    cutSquareImage(QPixmap(pppath),QSize(50,50));
    utils::image::getOrientation(pppath);
    bool iRet=false;

    getRotatedImage(pppath);
    cachePixmap(pppath);
    getThumbnail(pppath,iRet);
    supportedImageFormats();
    imageSupportWallPaper(pppath);
    utils::image::suffixisImage(pppath);

    wrapStr(m_JPGPath,QFont(),20);

}

TEST_F(gtestview, ExtensionPanel)
{
    m_extensionPanel=new ExtensionPanel(nullptr);
    m_extensionPanel->show();
    m_extensionPanel->setContent(new QWidget());
    m_extensionPanel->setContent(nullptr);
    m_extensionPanel->resize(200,200);
    QTest::mousePress(m_extensionPanel, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),100);
    QTest::mouseRelease(m_extensionPanel, Qt::LeftButton,Qt::NoModifier,QPoint(100,100),100);
    QTest::mouseClick(m_extensionPanel, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),100);
    QTest::mouseMove(m_extensionPanel, QPoint(20,20),100);
    QTest::keyClick(m_extensionPanel, Qt::Key_Escape, Qt::ShiftModifier, 100);
    QTest::mouseDClick(m_extensionPanel,Qt::LeftButton,Qt::NoModifier,QPoint(50,50),100);
    m_extensionPanel->setContent(new QWidget());
    m_extensionPanel->setContent(nullptr);
    QTest::mousePress(m_extensionPanel, Qt::LeftButton,Qt::NoModifier,QPoint(20,50),100);
    QTest::mouseRelease(m_extensionPanel, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),100);
    QTest::mouseClick(m_extensionPanel, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),100);
    QTest::mouseMove(m_extensionPanel, QPoint(50,50),50);
    QTest::mouseDClick(m_extensionPanel,Qt::LeftButton);

    m_extensionPanel->close();
    if(m_extensionPanel){
        m_extensionPanel->deleteLater();
        m_extensionPanel=nullptr;
    }
}
TEST_F(gtestview, Toast)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }

    Toast *widget = m_frameMainWindow->findChild<Toast *>(TOAST_OBJECT);
    if(widget)
    {
        widget->icon();
        widget->setText("toast");
        widget->text();
        widget->setOpacity(qreal());
        widget->opacity();
    }

}

TEST_F(gtestview, ThemeWidget)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }

    ThemeWidget *widget = m_frameMainWindow->findChild<ThemeWidget *>(THEME_WIDGET);
    if(widget)
    {
        widget->isDeepMode();
    }
}

TEST_F(gtestview, Dark)
{
    dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
}

TEST_F(gtestview, Light)
{
    dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Light);
}

//TEST_F(gtestview, PrintHelper)
//{
//    if(!m_frameMainWindow)
//    {
//        m_frameMainWindow = CommandLine::instance()->getMainWindow();
//        PrintHelper *testWidget=new PrintHelper();
//        PrintHelper::showPrintDialog(list, m_frameMainWindow);
//        QTest::qWait(1000);

//    }
//}


TEST_F(gtestview, ElidedLabel)
{
    if(CommandLine::instance()->getMainWindow())
    {
        ElidedLabel *elide=new ElidedLabel(new QWidget());
        elide->setText("test");
        elide->show();
        elide->resize(50,50);
        elide->update();
        QTest::qWait(50);
        dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
        QTest::qWait(50);
        dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Light);
        QTest::qWait(50);
        elide->deleteLater();
        elide=nullptr;
    }
}


TEST_F(gtestview, initTest1)
{
    ScanPathsDialog::instance()->addPath(m_PNGPath);
}


#endif
