#define private public
#include "src/src/module/view/contents/ttlcontent.h"
#include "widgets/pushbutton.h"
#undef private
#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
#include "src/src/module/view/contents/ttbcontent.h"

#include <QEvent>
#include <QGesture>
#include <QPinchGesture>
#include <QSwipeGesture>
#include <QTouchEvent>
#include "module/view/scen/imageview.h"

#ifdef test_module_view_contents

TEST_F(gtestview, QWheelEvent_2)
{
    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }

    ViewPanel *panel = m_frameMainWindow->findChild<ViewPanel *>(VIEW_PANEL_WIDGET);
    if(panel){
        //        QWheelEvent(const QPointF &pos, int delta,
        //                    Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
        //                    Qt::Orientation orient = Qt::Vertical);
        QWheelEvent event(QPointF(300,300),50,Qt::MidButton,Qt::NoModifier);
        qApp->sendEvent(dynamic_cast<QObject *>(panel),&event);

        QTouchDevice device;
        device.setType(QTouchDevice::TouchScreen);
        device.setCapabilities(QTouchDevice::Position);
        device.setMaximumTouchPoints(40);

        QList<QTouchEvent::TouchPoint> touchPoints;
        QTouchEvent::TouchPoint point1;
        point1.setPos(QPoint(100,100));
        point1.setRect(QRectF(100,100,100,100));
        point1.setState(Qt::TouchPointPressed);

        QTouchEvent::TouchPoint point2;
        point2.setPos(QPoint(200,200));
        point2.setRect(QRectF(200,200,200,200));
        point2.setState(Qt::TouchPointPressed);

        QTouchEvent::TouchPoint point3;
        point3.setPos(QPoint(300,300));
        point3.setRect(QRectF(300,300,300,300));
        point3.setState(Qt::TouchPointPressed);

        touchPoints.push_back(point1);
        touchPoints.push_back(point2);
        touchPoints.push_back(point3);

        QTouchEvent eventTouch(QEvent::TouchBegin ,&device,Qt::NoModifier,Qt::TouchPointPressed,touchPoints);

        qApp->sendEvent(dynamic_cast<QObject *>(panel),&eventTouch);
        //       QEvent *event1=new QEvent(QEvent::TouchBegin);
        //       qApp->sendEvent(dynamic_cast<QObject *>(panel),event1);
        //       QTest::qWait(100);
        //       QEvent *event2=new QEvent(QEvent::TouchUpdate );
        //       qApp->sendEvent(dynamic_cast<QObject *>(panel),event2);
        //       QTest::qWait(100);
        //       QEvent *event3=new QEvent(QEvent::TouchEnd );
        //       qApp->sendEvent(dynamic_cast<QObject *>(panel),event3);
        QList<QGesture *> gestures;

        QGestureEvent event4(gestures);
        qApp->sendEvent(dynamic_cast<QObject *>(panel),&event4);


        QList<QGesture *> gestures1;
        QPinchGesture testGest1;
        testGest1.setHotSpot(QPoint(300,300));
        testGest1.setTotalChangeFlags(QPinchGesture::RotationAngleChanged);
        gestures1.push_back(&testGest1);
        QGestureEvent event5(gestures1);
        qApp->sendEvent(dynamic_cast<QObject *>(panel),&event5);


        QList<QGesture *> gestures2;
        QPinchGesture testGest2;
        testGest2.setHotSpot(QPoint(300,300));
        testGest2.setTotalChangeFlags(QPinchGesture::CenterPointChanged);
        gestures2.push_back(&testGest2);
        QGestureEvent event6(gestures2);
        qApp->sendEvent(dynamic_cast<QObject *>(panel),&event6);

        QList<QGesture *> gestures3;
        QSwipeGesture testGest3;
        testGest3.setHotSpot(QPoint(300,300));
        testGest3.setSwipeAngle(qreal(2));
        gestures3.push_back(&testGest3);
        QGestureEvent event7(gestures3);
        qApp->sendEvent(dynamic_cast<QObject *>(panel),&event7);
    }
}


TEST_F(gtestview,loadBack)
{

    for(int i=0;i<200;i++)
    {
        QFile::copy(":/jpg.jpg",QApplication::applicationDirPath()+"/test/jpg" +QString::number(i)+".jpg");
        QFile(QApplication::applicationDirPath()+"/test/jpg" +QString::number(i)+".jpg").setPermissions( \
                    QFile::WriteUser | QFile::ReadUser |QFile::WriteOther |\
                    QFile::ReadOther |QFile::ReadGroup|QFile::WriteGroup);
    }

    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }

    ViewPanel *panel = m_frameMainWindow->findChild<ViewPanel *>(VIEW_PANEL_WIDGET);
    if(panel){
        // 打开保存绘制的 tif
        QString TriangleItemPath = QApplication::applicationDirPath() + "/test/jpg100.jpg";
        TestApi::drogPathtoWidget(panel,TriangleItemPath);
    }

    m_frameMainWindow->setWindowRadius(18);
    m_frameMainWindow->setBorderWidth(0);
    m_frameMainWindow->show();

    ImageItem *Item1=m_frameMainWindow->findChild<ImageItem *>(QApplication::applicationDirPath()+"/test/jpg55.jpg");
    if(Item1)
        Item1->emitClickSig(QApplication::applicationDirPath()+"/test/jpg55.jpg");
    QTest::qWait(100);
    if(Item1)
        Item1->emitClickEndSig();

    ImageItem *Item3=m_frameMainWindow->findChild<ImageItem *>(QApplication::applicationDirPath()+"/test/jpg52.jpg");
    if(Item3)
        Item3->emitClickEndSig();
    int index1=0;
    while(index1++<10)
    {
        QTest::keyClick(m_frameMainWindow, Qt::Key_Left, Qt::NoModifier, 100);
    }

    ImageItem *Item2=m_frameMainWindow->findChild<ImageItem *>(QApplication::applicationDirPath()+"/test/jpg145.jpg");
    if(Item2)
        Item2->emitClickSig(QApplication::applicationDirPath()+"/test/jpg145.jpg");
    QTest::qWait(100);
    if(Item2)
        Item2->emitClickEndSig();
    while(index1++<20)
    {
        QTest::keyClick(m_frameMainWindow, Qt::Key_Right, Qt::NoModifier, 100);
    }
}
TEST_F(gtestview,TTLCONTENTS)
{
    //TTL_CONTENTS
    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }

    TTLContent *panel = m_frameMainWindow->findChild<TTLContent *>(TTL_CONTENTS);
    if(panel){
        //            panel->show();
        panel->resize(300,300);
        //            panel->hide();
    }
    TTLContent *content;
    SignalManager::ViewInfo m_vinfo;
    content=new TTLContent(m_vinfo.inDatabase);
    content->show();
    content->m_adaptImageBtn->clicked();
    content->m_adaptScreenBtn->clicked();
    content->m_clBT->clicked();
    content->m_rotateLBtn->clicked();
    content->m_rotateRBtn->clicked();
    content->m_trashBtn->clicked();
    content->resize(200,200);
    content->m_clBT->deleteLater();
    content->m_clBT=nullptr;
    content->updateCollectButton();
    content->setCurrentDir("My favorite");
    content->hide();
    content->deleteLater();
    content=nullptr;

}


TEST_F(gtestview,getImageType)
{
    //TTL_CONTENTS
    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }

    TTBContent *panel = m_frameMainWindow->findChild<TTBContent *>(TTBCONTENT_WIDGET);
    if(panel){
        panel->getImageType(QApplication::applicationDirPath()+"/jpg.jpg");
        panel->getImageType(QApplication::applicationDirPath()+"/png.png");
        panel->getImageType(QApplication::applicationDirPath()+"/gif.gif");
        panel->getImageType(QApplication::applicationDirPath()+"/tif.tif");
        panel->getImageType(QApplication::applicationDirPath()+"/jpg.jpg");
        panel->getImageType(QApplication::applicationDirPath()+"/wbmp.wbmp");
        panel->getImageType(QApplication::applicationDirPath()+"/jpg.jpg");
        //            panel->hide();

        emit panel->resetTransform(false);

        emit panel->resetTransform(true);

        emit panel->sigsetcurrent(QApplication::applicationDirPath()+"/test/jpg81.jpg");
    }
}

TEST_F(gtestview,ImageItemT)
{
    ImageItem*  item=new ImageItem(1000,"test");
    item->SetPath("test");
    item->getIndex();
    item->getIndexNow();
    item->getPixmap();
    item->resize(200,200);
    item->show();
    QTest::mousePress(item, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),200);
    QTest::mouseRelease(item, Qt::LeftButton,Qt::NoModifier,QPoint(25,25),200);
    QTest::mouseClick(item, Qt::LeftButton,Qt::NoModifier,QPoint(30,30),200);
    QTest::mouseMove(item, QPoint(35,35),300);
    QTest::mouseDClick(item,Qt::LeftButton,Qt::NoModifier,QPoint(40,40),200);
    QTest::keyClick(item, Qt::Key_I, Qt::ControlModifier , 100);
}

TEST_F(gtestview,ImageItemTfind)
{
    //TTL_CONTENTS
    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }

    TTBContent *panel = m_frameMainWindow->findChild<TTBContent *>(TTBCONTENT_WIDGET);
    if(panel){
        ImageItem *Item1=m_frameMainWindow->findChild<ImageItem *>(QApplication::applicationDirPath()+"/test/jpg80.jpg");
        ImageItem *Item2=m_frameMainWindow->findChild<ImageItem *>(QApplication::applicationDirPath()+"/test/jpg90.jpg");
        ImageItem *Item3=m_frameMainWindow->findChild<ImageItem *>(QApplication::applicationDirPath()+"/test/jpg32.jpg");
        ImageItem *Item4=m_frameMainWindow->findChild<ImageItem *>(QApplication::applicationDirPath()+"/test/jpg180.jpg");
        if(Item1 &&Item2)
        {
            QTest::mousePress(Item1, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),50);
            QTest::mouseMove(Item1, QPoint(35,35),50);
            QTest::mousePress(Item2, Qt::LeftButton,Qt::NoModifier,QPoint(20,20),50);
            QTest::mouseRelease(Item2, Qt::LeftButton,Qt::NoModifier,QPoint(25,25),50);
            if(Item2)
                Item2->emitClickEndSig();

            dApp->m_bMove=true;
            if(Item3)
                Item3->emitClickSig(QApplication::applicationDirPath()+"/test/jpg32.jpg");
            QTest::qWait(100);
            if(Item4)
                Item4->emitClickSig(QApplication::applicationDirPath()+"/test/jpg180.jpg");
            QTest::qWait(100);
            if(Item2)
                Item2->emitClickSig(QApplication::applicationDirPath()+"/test/jpg100.jpg");
            QTest::qWait(100);
            if(Item2)
                Item2->emitClickEndSig();
            dApp->m_bMove=false;
        }
        DWidget* list= m_frameMainWindow->findChild<DWidget *>(IMAGE_LIST_OBJECT);
        if(list)
        {
            QTest::mousePress(list, Qt::LeftButton,Qt::NoModifier,QPoint(70,20),50);
            QTest::mouseMove(list, QPoint(200,35),50);
            QTest::mousePress(list, Qt::LeftButton,Qt::NoModifier,QPoint(300,20),50);
            QTest::mouseRelease(list, Qt::LeftButton,Qt::NoModifier,QPoint(500,25),50);

            QTestEventList e;
            e.addMouseMove(QPoint(100,30), 50);
            //    e.addKeyPress(Qt::Key_Shift, Qt::NoModifier, 50);
            e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(100,30), 50);
            e.addMouseMove(QPoint(150,30), 50);
            e.addMouseMove(QPoint(200,30), 50);
            //    e.addKeyRelease(Qt::Key_Shift, Qt::NoModifier, 200);
            e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(250,30), 50);
            e.simulate(list);

            e.clear();

        }
        MyImageListWidget* imgListView=m_frameMainWindow->findChild<MyImageListWidget *>(IMAGE_LIST_WIDGET);
        if(imgListView)
        {
            QTest::mousePress(imgListView, Qt::LeftButton,Qt::NoModifier,QPoint(70,20),50);
            QTest::mouseMove(imgListView, QPoint(200,35),50);
            QTest::mousePress(imgListView, Qt::LeftButton,Qt::NoModifier,QPoint(300,20),50);
            QTest::mouseRelease(imgListView, Qt::LeftButton,Qt::NoModifier,QPoint(500,25),50);

            QTestEventList e;
            e.addMouseMove(QPoint(100,30), 50);
            //    e.addKeyPress(Qt::Key_Shift, Qt::NoModifier, 50);
            e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(100,30), 50);
            e.addMouseMove(QPoint(150,30), 50);
            e.addMouseMove(QPoint(200,30), 50);
            //    e.addKeyRelease(Qt::Key_Shift, Qt::NoModifier, 200);
            e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(250,30), 50);
            e.simulate(imgListView);

            e.clear();
        }
    }
}

TEST_F(gtestview,MyImageListWidgetnew)
{
    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    MyImageListWidget* imgListView=m_frameMainWindow->findChild<MyImageListWidget *>(IMAGE_LIST_WIDGET);
    if(imgListView)
    {
        emit dApp->sigMouseRelease();
//        imgListView->ifMouseLeftPressed();

    }


}

TEST_F(gtestview,changeHideFlag)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }

    ViewPanel *panel = m_frameMainWindow->findChild<ViewPanel *>(VIEW_PANEL_WIDGET);
    if(panel)
    {
        emit panel->changeHideFlag(false);
        emit panel->changeHideFlag(true);
    }

}

TEST_F(gtestview,emitupdateTopToolbar)
{
    emit dApp->signalM->updateTopToolbar();
}
#endif
