#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
#include <QObject>
#include <QSwipeGesture>
#define private public
#include "module/view/scen/imageview.h"
TEST_F(gtestview, m_imageloader_11)
{

}
// connect(animation, SIGNAL(finished()), this, SLOT(OnFinishPinchAnimal()));
TEST_F(gtestview, m_OnFinishPinchAnimal)
{
    m_frameMainWindow = CommandLine::instance()->getMainWindow();

    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if(panel){
        panel->OnFinishPinchAnimal();
        QList<QGesture *> gestures3;
        QSwipeGesture *testGest3=new QSwipeGesture();
        testGest3->setHotSpot(QPoint(300,300));
        testGest3->setSwipeAngle(qreal(2));
        gestures3.push_back(testGest3);
        QGestureEvent *event7=new QGestureEvent(gestures3);
        qApp->sendEvent(dynamic_cast<QObject *>(panel->viewport()),event7);

        panel->swipeTriggered(testGest3);

        QList<QGesture *> gestures;
        QPinchGesture *testGest=new QPinchGesture();
        testGest->setHotSpot(QPoint(300,300));
        testGest->setTotalChangeFlags(QPinchGesture::ScaleFactorChanged);
        gestures.push_back(testGest);
        QGestureEvent *event4=new QGestureEvent(gestures);
        panel->handleGestureEvent(event4);

    }
}
