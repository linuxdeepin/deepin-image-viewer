#include "gtestview.h"
#if test_z_exit
TEST_F(gtestview, remove)
{
    QProcess::execute("rm -r "+QApplication::applicationDirPath() + "/test");
}
TEST_F(gtestview, frame_mainwindowtestclose)
{
    dApp->signalM->enterView(false);

    dApp->signalM->enterView(true);

    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(100);
    m_frameMainWindow->close();

}

//TEST_F(gtestview, exit)
//{
////    if(!m_frameMainWindow)
////    {
////        m_frameMainWindow = CommandLine::instance()->getMainWindow();
////        QTest::keyClick(m_frameMainWindow, Qt::Key_F4, Qt::AltModifier, 1000);
////    }
//    QTimer::singleShot(10000,[=]{
//        return exit(0);
//    });
//}
#endif
