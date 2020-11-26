#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"

TEST_F(gtestview, frame_bottomtoolbar)
{
    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(100);

    BottomToolbar *panel = m_frameMainWindow->findChild<BottomToolbar *>(BUTTOM_TOOL_BAR);
    if(panel)
    {
        QTest::mouseMove(panel, QPoint(300,20),100);
        QTest::mouseMove(panel, QPoint(400,30),100);
    }

}
TEST_F(gtestview, frame_mainwindow)
{
//    dApp->signalM->enterView(false);

//    dApp->signalM->enterView(true);

//    if(!m_frameMainWindow){
//        m_frameMainWindow = CommandLine::instance()->getMainWindow();
//    }
//    QTest::qWait(500);

//    MainWindow *panel = m_frameMainWindow->findChild<MainWindow *>();
//    if(panel)
//    {
////        panel->close();
//    }

}


