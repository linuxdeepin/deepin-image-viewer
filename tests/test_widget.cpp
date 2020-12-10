#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
#include "viewer/src/widgets/elidedlabel.h"
#include "viewer/src/frame/mainwidget.h"
#define private public
#include "viewer/src/widgets/toast.h"
//TEST_F(gtestview, BlurFrame_1)
//{
//    QWidget *widget =new QWidget();
//    BlurFrame * blurFrame=new BlurFrame(widget);
//    blurFrame->show();
//    blurFrame->resize(200,200);
//    QTest::mousePress(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
//    QTest::mouseRelease(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(100,100),200);
//    QTest::mouseClick(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
//    QTest::mouseMove(blurFrame, QPoint(50,100),200);
//    QTest::keyClick(blurFrame, Qt::Key_Escape, Qt::ShiftModifier, 200);
//    QTest::mouseDClick(blurFrame,Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);

//    QTest::mousePress(widget, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
//    QTest::mouseRelease(widget, Qt::LeftButton,Qt::NoModifier,QPoint(100,100),200);
//    QTest::mouseClick(widget, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
//    QTest::mouseMove(widget, QPoint(50,100),200);
//    QTest::keyClick(widget, Qt::Key_Escape, Qt::ShiftModifier, 200);
//    QTest::mouseDClick(widget,Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
//}

//TEST_F(gtestview, ElidedLabel_1)
//{
//    QWidget *widget =new QWidget();
//    ElidedLabel * blurFrame=new ElidedLabel(widget);
//    blurFrame->show();
//    blurFrame->resize(200,200);
//    QTest::mousePress(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
//    QTest::mouseRelease(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(100,100),200);
//    QTest::mouseClick(blurFrame, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
//    QTest::mouseMove(blurFrame, QPoint(50,100),200);
//    QTest::keyClick(blurFrame, Qt::Key_Escape, Qt::ShiftModifier, 200);
//    QTest::mouseDClick(blurFrame,Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);

//    QTest::mousePress(widget, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
//    QTest::mouseRelease(widget, Qt::LeftButton,Qt::NoModifier,QPoint(100,100),200);
//    QTest::mouseClick(widget, Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
//    QTest::mouseMove(widget, QPoint(50,100),200);
//    QTest::keyClick(widget, Qt::Key_Escape, Qt::ShiftModifier, 200);
//    QTest::mouseDClick(widget,Qt::LeftButton,Qt::NoModifier,QPoint(50,50),200);
//}

TEST_F(gtestview, Toast_1)
{
    Toast *widget = new Toast();
    if(widget)
    {
        widget->icon();
        widget->setText("toast");
        widget->text();
        widget->setOpacity(qreal());
        widget->opacity();
    }
    widget->deleteLater();
    widget=nullptr;

}

TEST_F(gtestview, MainWidget_1)
{
    dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
    dApp->signalM->updateBottomToolbar(false);
    QTest::qWait(100);
    dApp->m_timer=0;
    MainWidget *widget=new MainWidget(false);
    widget->deleteLater();
    widget=nullptr;
}
TEST_F(gtestview, TopToolbar)
{
    m_topoolBar=new TopToolbar(false,nullptr);
    m_topoolBar->show();
    QTest::mousePress(m_topoolBar, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),500);
    QTest::mouseRelease(m_topoolBar, Qt::LeftButton,Qt::NoModifier,QPoint(30,20),500);
    QTest::mouseClick(m_topoolBar, Qt::LeftButton);
    QTest::mouseMove(m_topoolBar, QPoint(20,20),500);
    QTest::keyClick(m_topoolBar, Qt::Key_Escape, Qt::ShiftModifier, 1000);
    QTest::mouseDClick(m_topoolBar,Qt::LeftButton);

    m_topoolBar->setMiddleContent("test");
    m_topoolBar->setTitleBarTransparent(true);
    m_topoolBar->setTitleBarTransparent(false);
    QTest::mouseDClick(m_topoolBar,Qt::LeftButton);
    dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
    m_topoolBar->setTitleBarTransparent(false);
    dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Light);
    m_topoolBar->hide();
    m_topoolBar->deleteLater();
    m_topoolBar=nullptr;
}
TEST_F(gtestview, picInUSB_1)
{
    emit dApp->signalM->picInUSB(true);
}



