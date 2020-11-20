#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
#include "viewer/module/view/contents/ttbcontent.h"
#include "viewer/module/view/contents/ttlcontent.h"



TEST_F(gtestview,loadBack)
{
    QTest::qWait(1000);

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
    QTest::qWait(1000);

    ViewPanel *panel = m_frameMainWindow->findChild<ViewPanel *>(VIEW_PANEL_WIDGET);
    if(panel){
        // 打开保存绘制的 tif
        QString TriangleItemPath = QApplication::applicationDirPath() + "/test/jpg100.jpg";
        TestApi::drogPathtoWidget(panel,TriangleItemPath);
    }

    m_frameMainWindow->setWindowRadius(18);
    m_frameMainWindow->setBorderWidth(0);
    m_frameMainWindow->show();

    int index1=0;
    while(index1++<60)
    {
        QTest::keyClick(m_frameMainWindow, Qt::Key_Left, Qt::NoModifier, 200);
    }
    while(index1++<180)
    {
        QTest::keyClick(m_frameMainWindow, Qt::Key_Right, Qt::NoModifier, 200);
    }
}
TEST_F(gtestview,TTLCONTENTS)
{
//TTL_CONTENTS
        if(!m_frameMainWindow){
            m_frameMainWindow = CommandLine::instance()->getMainWindow();
        }
        QTest::qWait(1000);

        TTLContent *panel = m_frameMainWindow->findChild<TTLContent *>(TTL_CONTENTS);
        if(panel){
//            panel->show();
            panel->resize(300,300);
//            panel->hide();
        }
}


TEST_F(gtestview,getImageType)
{
//TTL_CONTENTS
        if(!m_frameMainWindow){
            m_frameMainWindow = CommandLine::instance()->getMainWindow();
        }
        QTest::qWait(1000);

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
        }
}
