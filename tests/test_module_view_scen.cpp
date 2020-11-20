#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
//baseutils utils::base
TEST_F(gtestview, showVagueImage)
{

    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(1000);

    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if(panel){
        panel->showVagueImage(QPixmap(QApplication::applicationDirPath()+"/png.png"),QApplication::applicationDirPath()+"/png.png");
    }
}

TEST_F(gtestview, showFileImage)
{
    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(1000);

    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if(panel){
//        panel->showFileImage();
    }
}

TEST_F(gtestview, mouse)
{
    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(1000);

    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if(panel){
        QTest::mousePress(panel, Qt::LeftButton,Qt::NoModifier,QPoint(200,200),500);
        QTest::mouseRelease(panel, Qt::LeftButton,Qt::NoModifier,QPoint(200,100),500);
        QTest::mouseClick(panel, Qt::LeftButton,Qt::NoModifier,QPoint(200,50),500);
        QTest::mouseMove(panel, QPoint(200,100),500);
        QTest::keyClick(panel, Qt::Key_Escape, Qt::ShiftModifier, 500);
        QTest::mouseDClick(panel,Qt::LeftButton,Qt::NoModifier,QPoint(200,50),500);
    }
}

TEST_F(gtestview, reloadSvgPix)
{
    //reloadSvgPix
    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(1000);

    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if(panel){
        panel->reloadSvgPix(QApplication::applicationDirPath()+"/svg1.svg",90,true);
        panel->reloadSvgPix(QApplication::applicationDirPath()+"/svg2.svg",-90,false);
    }
    DGuiApplicationHelper::instance();

}

TEST_F(gtestview, setThemeType)
{
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
    QTest::qWait(1000);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
    QTest::qWait(1000);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::UnknownType);
    QTest::qWait(1000);
}

TEST_F(gtestview, loadingDisplay)
{
    dApp->signalM->loadingDisplay(true);
}

TEST_F(gtestview, clear)
{
    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(1000);

    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if(panel){
        panel->clear();
    }
}



//还没有模拟手指事件
