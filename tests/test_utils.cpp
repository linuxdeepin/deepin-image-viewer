#include "gtestview.h"

//baseutils utils::base
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
