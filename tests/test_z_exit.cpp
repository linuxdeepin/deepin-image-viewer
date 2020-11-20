#include "gtestview.h"

TEST_F(gtestview, remove)
{
    QProcess::execute("rm -r "+QApplication::applicationDirPath() + "/test");
}
TEST_F(gtestview, exit)
{
//    if(!m_frameMainWindow)
//    {
//        m_frameMainWindow = CommandLine::instance()->getMainWindow();
//        QTest::keyClick(m_frameMainWindow, Qt::Key_F4, Qt::AltModifier, 1000);
//    }
    QTimer::singleShot(10000,[=]{
        return exit(0);
    });
}
