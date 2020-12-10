#include "gtestview.h"

//baseutils utils::base
#ifdef test_utils
TEST_F(gtestview, renderSVG_error)
{
    utils::base::renderSVG("error",QSize(50,50));
}

TEST_F(gtestview, sizeToHuman_error)
{
    utils::base::sizeToHuman(1000);
}

TEST_F(gtestview, timeToString_false)
{
    utils::base::timeToString(QDateTime(),false);
}

TEST_F(gtestview, showInFileManager_ok)
{
    utils::base::showInFileManager(QApplication::applicationDirPath()+"/tif.tif");
}

TEST_F(gtestview, trashFile_error)
{
    utils::base::trashFile("error");
}

TEST_F(gtestview, mountDeviceExist)
{
    utils::base::mountDeviceExist("/media/test");

    utils::base::mountDeviceExist("/run/media/test");
}

//imageutils utils::image::

TEST_F(gtestview, scaleImage)
{
    utils::image::scaleImage("error");

    utils::image::scaleImage(QApplication::applicationDirPath()+"/png.png");
}

TEST_F(gtestview, getCreateDateTime)
{
    utils::image::getCreateDateTime("error");

    utils::image::getCreateDateTime(QApplication::applicationDirPath()+"/png.png");
}

TEST_F(gtestview, imageSupportRead)
{
    utils::image::imageSupportRead("error");

    utils::image::imageSupportRead(QApplication::applicationDirPath()+"/png.png");
}

TEST_F(gtestview, cutSquareImage)
{
    QPixmap pix(QApplication::applicationDirPath()+"/png.png");

    utils::image::cutSquareImage(pix);
}

TEST_F(gtestview, getImagesInfo)
{
    utils::image::getImagesInfo(QApplication::applicationDirPath(),false);

    utils::image::getImagesInfo(QApplication::applicationDirPath(),true);
}

TEST_F(gtestview, loadTga)
{
    bool iRet=false;
    utils::image::loadTga(QApplication::applicationDirPath()+"/tga.tga",iRet);
    bool iRet2=true;
    utils::image::loadTga(QApplication::applicationDirPath()+"/png.png",iRet2);
}

TEST_F(gtestview, generateThumbnail)
{
    utils::image::generateThumbnail("error");

    utils::image::generateThumbnail(QApplication::applicationDirPath()+"/tga.tga");

    utils::image::generateThumbnail(QApplication::applicationDirPath()+"/png.png");
}

TEST_F(gtestview, thumbnailPath)
{
    utils::image::thumbnailPath("error");

}



#include "accessibility/ac-desktop-define.h"
#include "utils/snifferimageformat.h"
TEST_F(gtestview, QShortcut)
{
if(CommandLine::instance()->getMainWindow())
{
   QShortcut *scViewShortcut = CommandLine::instance()->getMainWindow()->findChild <QShortcut *> (SC_VIEW_SHORTCUT);
   if(scViewShortcut){
       scViewShortcut->activated();
   }
}

}

TEST_F(gtestview, DetectImageFormat)
{
    DetectImageFormat(QApplication::applicationDirPath()+"/png.png");

    DetectImageFormat(QApplication::applicationDirPath()+"/icns.icns");

    DetectImageFormat(QApplication::applicationDirPath()+"/gif.gif");

    DetectImageFormat(QApplication::applicationDirPath()+"/mng.mng");

    DetectImageFormat(QApplication::applicationDirPath()+"/tif.tif");


    DetectImageFormat("error");


}
//UnionImage_NameSpace
#include "utils/unionimage.h"
TEST_F(gtestview, string2DateTime)
{
    UnionImage_NameSpace::string2DateTime(QApplication::applicationDirPath()+"/png.png");


    UnionImage_NameSpace::string2DateTime(QDateTime::currentDateTime().toString());
}

TEST_F(gtestview, QImge2FIBitMap)
{
    UnionImage_NameSpace::string2DateTime(QApplication::applicationDirPath()+"/png.png");


    UnionImage_NameSpace::string2DateTime(QDateTime::currentDateTime().toString());
}

TEST_F(gtestview, isSupportsReading)
{
    UnionImage_NameSpace::unionImageVersion();

}

TEST_F(gtestview, detectImageFormat)
{
    UnionImage_NameSpace::detectImageFormat(QApplication::applicationDirPath()+"/png.png");

    UnionImage_NameSpace::detectImageFormat(QApplication::applicationDirPath()+"/icns.icns");

    UnionImage_NameSpace::detectImageFormat(QApplication::applicationDirPath()+"/gif.gif");

    UnionImage_NameSpace::detectImageFormat(QApplication::applicationDirPath()+"/mng.mng");

    UnionImage_NameSpace::detectImageFormat(QApplication::applicationDirPath()+"/tif.tif");

    UnionImage_NameSpace::detectImageFormat("error");

}

TEST_F(gtestview, rotateImageFIle)
{
    QString error;
    UnionImage_NameSpace::rotateImageFIle(45,QApplication::applicationDirPath()+"/png.png",error);

    UnionImage_NameSpace::rotateImageFIle(90,QApplication::applicationDirPath()+"/png.png",error);

    UnionImage_NameSpace::rotateImageFIle(90,QApplication::applicationDirPath()+"/svg.svg",error);

    UnionImage_NameSpace::rotateImageFIle(90,QApplication::applicationDirPath()+"/jpg.jpg",error);

    UnionImage_NameSpace::rotateImageFIle(90,QApplication::applicationDirPath()+"/svg1.svg",error);


}

TEST_F(gtestview, rotateImageFIleWithImage)
{
    QString error;
    QImage img1(QApplication::applicationDirPath()+"/png.png");
    UnionImage_NameSpace::rotateImageFIleWithImage(45,img1,QApplication::applicationDirPath()+"/png.png",error);
    QImage img2(QApplication::applicationDirPath()+"/png.png");
    UnionImage_NameSpace::rotateImageFIleWithImage(90,img2,QApplication::applicationDirPath()+"/png.png",error);
    QImage img3(QApplication::applicationDirPath()+"/svg.svg");
    UnionImage_NameSpace::rotateImageFIleWithImage(90,img3,QApplication::applicationDirPath()+"/svg.svg",error);
    QImage img4(QApplication::applicationDirPath()+"/jpg.jpg");
    UnionImage_NameSpace::rotateImageFIleWithImage(90,img4,QApplication::applicationDirPath()+"/jpg.jpg",error);
    QImage img5(QApplication::applicationDirPath()+"/svg2.svg");
    UnionImage_NameSpace::rotateImageFIleWithImage(90,img5,QApplication::applicationDirPath()+"/svg2.svg",error);


}

TEST_F(gtestview, unionisSupportsWriting)
{

    UnionImage_NameSpace::isSupportsWriting(QApplication::applicationDirPath()+"/png.png");

    UnionImage_NameSpace::isSupportsWriting(QApplication::applicationDirPath()+"/png.png");

    UnionImage_NameSpace::isSupportsWriting(QApplication::applicationDirPath()+"/svg.svg");

    UnionImage_NameSpace::isSupportsWriting(QApplication::applicationDirPath()+"/svg1.svg");

    UnionImage_NameSpace::isSupportsWriting(QApplication::applicationDirPath()+"/svg3.svg");

    UnionImage_NameSpace::isSupportsWriting(QApplication::applicationDirPath()+"/jpg.jpg");


}

TEST_F(gtestview, unionisSupportsReading)
{
    UnionImage_NameSpace::isSupportsReading(QApplication::applicationDirPath()+"/png.png");

    UnionImage_NameSpace::isSupportsReading(QApplication::applicationDirPath()+"/png.png");

    UnionImage_NameSpace::isSupportsReading(QApplication::applicationDirPath()+"/svg.svg");

    UnionImage_NameSpace::isSupportsReading(QApplication::applicationDirPath()+"/jpg.jpg");

    UnionImage_NameSpace::isSupportsReading(QApplication::applicationDirPath()+"/svg3.svg");

}

TEST_F(gtestview, uniongetOrientation)
{
    UnionImage_NameSpace::getOrientation(QApplication::applicationDirPath()+"/png.png");

    UnionImage_NameSpace::getOrientation(QApplication::applicationDirPath()+"/png.png");

    UnionImage_NameSpace::getOrientation(QApplication::applicationDirPath()+"/svg.svg");

    UnionImage_NameSpace::getOrientation(QApplication::applicationDirPath()+"/jpg.jpg");


    UnionImage_NameSpace::getOrientation(QApplication::applicationDirPath()+"/svg3.svg");

    UnionImage_NameSpace::getOrientation("error");


}

TEST_F(gtestview, uniongetThumbnail)
{
    QImage img;
   UnionImage_NameSpace::getThumbnail(img,QApplication::applicationDirPath()+"/jpg.jpg");

}

TEST_F(gtestview, loadStaticImageFromFile)
{
    QImage img;
    QString error;
    UnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath()+"/jpg.jpg", img, error, "jpg");
    UnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath()+"/png.png", img, error, "png");
    UnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath()+"/svg.svg", img, error, "svg");
    UnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath()+"/tga.tga", img, error, "tga");
    UnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath()+"/gif.gif", img, error, "gif");
    UnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath()+"/tif.tif", img, error, "tif");
    UnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath()+"/mng.mng", img, error, "mng");
    UnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath()+"/dds.dds", img, error, "dds");
    UnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath()+"/ico.ico", img, error, "ico");
    UnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath()+"/wbmp.wbmp", img, error, "wbmp");
    UnionImage_NameSpace::loadStaticImageFromFile("error", img, error, "svg");
    UnionImage_NameSpace::loadStaticImageFromFile("error", img, error, "png");
    UnionImage_NameSpace::loadStaticImageFromFile("error", img, error, "error");



}

//TEST_F(gtestview, canSave)
//{
//    utils::image::freeimage::canSave(QApplication::applicationDirPath()+"/png.png");

//    utils::image::freeimage::canSave(QApplication::applicationDirPath()+"/icns.icns");

//    utils::image::freeimage::canSave("error");


//}
#endif
