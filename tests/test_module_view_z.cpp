#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
#include "viewer/src/module/modulepanel.h"
#include "viewer/src/widgets/imagebutton.h"
#include "viewer/src/module/view/navigationwidget.h"
#include "viewer/src/module/view/viewpanel.h"
#include <QPixmap>
#include <QImage>
#ifdef test_module_view_z
TEST_F(gtestview, moduleName)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(300);

    ViewPanel *panel = m_frameMainWindow->findChild<ViewPanel *>(VIEW_PANEL_WIDGET);
    if(panel)
    {
        panel->moduleName();
        emit dApp->endThread();
        emit dApp->signalM->usbOutIn(true);
        emit dApp->signalM->usbOutIn(false);
        emit dApp->signalM->sigUpdateThunbnail(QApplication::applicationDirPath()+"/test/jpg40.jpg");
        emit dApp->signalM->UpdateNavImg();
    }
}


TEST_F(gtestview, RenameDialog)
{
    m_renameDialog=new RenameDialog(QApplication::applicationDirPath()+"/test/jpg52.jpg" );
    m_renameDialog->show();
    m_renameDialog->GetFilePath();
    m_renameDialog->GetFileName();
    QTest::qWait(100);
    dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
    QTest::qWait(100);
    dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Light);
    QTest::qWait(100);
    dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);


    m_renameDialog-> InitDlg();

    QTest::mouseMove(m_renameDialog->m_lineedt, QPoint(10,10), 200);
    m_renameDialog->m_lineedt->setText("");
    QTest::keyClicks(m_renameDialog->m_lineedt,QString("jpg520.jpg"),Qt::NoModifier, 10);
    m_renameDialog->m_lineedt->setText("jpg520.jpg");

    QTest::mouseMove(m_renameDialog->okbtn, QPoint(), 500);
    QTest::mouseClick(m_renameDialog->okbtn, Qt::LeftButton, Qt::NoModifier,QPoint(), 1000);

    m_renameDialog->okbtn->click();



    RenameDialog a("");
    a.show();
    a.m_lineedt->textEdited(QApplication::applicationDirPath()+"/test/jpg5200谢谢谢谢谢谢谢谢谢谢谢0.jpg"+QApplication::applicationDirPath()+"/test/jpg52000.jpg"+QApplication::applicationDirPath()+"/test/jpg52000.jpg");
    a.cancelbtn->click();

}

//NavigationWidget
TEST_F(gtestview, NavigationWidget_find)
{
    if(!m_frameMainWindow)
    {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    QTest::qWait(500);
    dApp->viewerTheme->setCurrentTheme(ViewerThemeManager::Dark);
    NavigationWidget *widget= m_frameMainWindow->findChild <NavigationWidget *>();
    if(widget)
    {
        widget->resize(200,200);
        widget->show();
        widget->resize(300,300);
        QImage *a=new QImage(QApplication::applicationDirPath()+"/png.png");

        widget->setImage(*a);
        widget->resize(400,400);
        QTest::mouseMove(widget, QPoint(10,10), 150);
        QTest::mouseMove(widget, QPoint(10,10), 100);
        widget->update();
        widget->hide();
        ImageButton *imgbutton= widget->findChild<ImageButton *>();
        imgbutton->clicked();

    }
}


#endif
