#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
#include "src/src/controller/dbusclient.h"
#include "src/src/controller/divdbuscontroller.h"
#define private public
#include "src/src/controller/wallpapersetter.h"

TEST_F(gtestview, Dbusclient1)
{

    Dbusclient *client=new  Dbusclient();
    QImage imgQImage(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    QList<QString> list;
    list.push_back(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    QList<QImage> listimg;
    listimg.push_back(imgQImage);
    client->openFiles(list);
    client->openImages(listimg);
    client->openDrawingBoard(list);
//    client->propertyChanged(QDBusMessage());
    client->deleteLater();
    client=nullptr;
}

TEST_F(gtestview, DIVDBusController)
{
    DIVDBusController *control=new DIVDBusController();
    control->activeWindow();
    control->editImage(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    control->enterAlbum(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    control->searchImage(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    control->backToMainWindow();
    control->deleteLater();
    control=nullptr;
}
TEST_F(gtestview, setWallPaper)
{
    QString TriangleItemPath = QApplication::applicationDirPath() + "/test/jpg1.jpg";

    dApp->wpSetter->setWallpaper(TriangleItemPath);

}

TEST_F(gtestview, WallPaperSetting1)
{
    QString path=QApplication::applicationDirPath() + "/test/jpg102.jpg";
//    WallpaperSetter::instance()->setDeepinWallpaper(path);
     WallpaperSetter::instance()->setWallpaper(QImage(QApplication::applicationDirPath() + "/test/jpg120.jpg"));
//    WallpaperSetter::instance()->setKDEWallpaper(path);
//    WallpaperSetter::instance()->setGNOMEShellWallpaper(path);
//    WallpaperSetter::instance()->setGNOMEWallpaper(path);
//    WallpaperSetter::instance()->setLXDEWallpaper(path);
//    WallpaperSetter::instance()->setXfaceWallpaper(path);
//    WallpaperSetter::instance()->testDE(path);
//    WallpaperSetter::instance()->getDE();

    if(!m_frameMainWindow){
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    m_frameMainWindow->activateWindow();
}
/*TEST_F(gtestview, collectSubDirs)
{
    DirCollectThread tt(QApplication::applicationDirPath(),"test");
    Importer::instance()->stopDirCollect(QApplication::applicationDirPath());
    tt.setStop(false);
    tt.dir();
//    tt.start();
    FilesCollectThread ct(QStringList(QApplication::applicationDirPath() + "/test/jpg133.jpg"),"");

    ct.currentImport(QApplication::applicationDirPath() + "/test/jpg133.jpg");
    ct.resultReady(DBImgInfoList());
    ct.insertAlbumRequest("",QStringList(QApplication::applicationDirPath() + "/test/jpg133.jpg"));
//    ct.start();
//    tt.wait(2000);
//    ct.wait(2000);
}*/
