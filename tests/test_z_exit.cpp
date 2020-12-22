#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
#ifdef test_z_exit

TEST_F(gtestview, ViewPanel_menu)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }

    ViewPanel *panel = m_frameMainWindow->findChild<ViewPanel *>(VIEW_PANEL_WIDGET);
    if(panel)
    {
        // 打开保存绘制的 tif
        QString TriangleItemPath = QApplication::applicationDirPath() + "/test/jpg55.jpg";
        TestApi::drogPathtoWidget(panel,TriangleItemPath);

        QTest::mouseClick(panel, Qt::RightButton,Qt::NoModifier,QPoint(50,50),50);
        emit panel->customContextMenuRequested(QPoint(300,300));

        QMenu *m_menu=panel->getMenu();
        if(m_menu)
        {
            QAction *IdCopy = new QAction(m_menu);
            IdCopy->setProperty("MenuID", ViewPanel::IdCopy);
            m_menu->addAction(IdCopy);
            IdCopy->trigger();

            QAction *IdPrint = new QAction(m_menu);
            IdPrint->setProperty("MenuID", ViewPanel::IdPrint);
            m_menu->addAction(IdPrint);
            IdPrint->trigger();

            QAction *IdRename = new QAction(m_menu);
            IdRename->setProperty("MenuID", ViewPanel::IdRename);
            m_menu->addAction(IdRename);
            IdRename->trigger();

            QAction *IdHideNavigationWindow = new QAction(m_menu);
            IdHideNavigationWindow->setProperty("MenuID", ViewPanel::IdHideNavigationWindow);
            m_menu->addAction(IdHideNavigationWindow);
            IdHideNavigationWindow->trigger();

            QAction *IdRotateClockwise = new QAction(m_menu);
            IdRotateClockwise->setProperty("MenuID", ViewPanel::IdRotateClockwise);
            m_menu->addAction(IdRotateClockwise);
            IdRotateClockwise->trigger();

            QAction *IdRotateCounterclockwise = new QAction(m_menu);
            IdRotateCounterclockwise->setProperty("MenuID", ViewPanel::IdRotateCounterclockwise);
            m_menu->addAction(IdRotateCounterclockwise);
            IdRotateCounterclockwise->trigger();

            QAction *IdSetAsWallpaper = new QAction(m_menu);
            IdSetAsWallpaper->setProperty("MenuID", ViewPanel::IdSetAsWallpaper);
            m_menu->addAction(IdSetAsWallpaper);
            IdSetAsWallpaper->trigger();

            QAction *IdShowNavigationWindow = new QAction(m_menu);
            IdShowNavigationWindow->setProperty("MenuID", ViewPanel::IdShowNavigationWindow);
            m_menu->addAction(IdShowNavigationWindow);
            IdShowNavigationWindow->trigger();

            QAction *IdDisplayInFileManager = new QAction(m_menu);
            IdDisplayInFileManager->setProperty("MenuID", ViewPanel::IdDisplayInFileManager);
            m_menu->addAction(IdDisplayInFileManager);
            IdDisplayInFileManager->trigger();

            QAction *IdDraw = new QAction(m_menu);
            IdDraw->setProperty("MenuID", ViewPanel::IdDraw);
            m_menu->addAction(IdDraw);
            IdDraw->trigger();

//            QAction *IdMoveToTrash = new QAction(m_menu);
//            IdMoveToTrash->setProperty("MenuID", ViewPanel::IdMoveToTrash);
//            m_menu->addAction(IdMoveToTrash);
//            IdMoveToTrash->trigger();
//            QTest::qWait(100);

            m_frameMainWindow->activateWindow();
        }

    }
}

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
//    QTest::qWait(100);
    if(dApp->m_imageloader)
    {
//        dApp->m_imageloader->addImageLoader(list);
        dApp->m_imageloader->stopThread();

    }
    m_frameMainWindow->close();

}

TEST_F(gtestview, exit)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
//        QTest::keyClick(m_frameMainWindow, Qt::Key_F4, Qt::AltModifier, 1000);
        m_frameMainWindow->close();
    }
//    QTest::qWait(5000);
//    QTimer::singleShot(10000,[=]{
//        return exit(0);
//    });
}
#endif
