#include "gtestview.h"
#include "service/defaultimageviewer.h"
#include "service/deepinimageviewerdbus.h"
#include "service/dbusimageview_adaptor.h"
//baseutils utils::base

#ifdef test_service
TEST_F(gtestview, defaultimageviewer)
{
    service::isDefaultImageViewer();
    service::setDefaultImageViewer(false);
    service::setDefaultImageViewer(true);
}

TEST_F(gtestview, deepinimageviewerdbus)
{
    DeepinImageViewerDBus* dubs=new DeepinImageViewerDBus(dApp->signalM);
    dubs->backToMainWindow() ;
    dubs->activeWindow();
    dubs->enterAlbum("test");
    dubs->searchImage("test");
    dubs->editImage("test");

    dubs->deleteLater();
    dubs=nullptr;
}

TEST_F(gtestview, dbusimageview_adaptor)
{
    ImageViewAdaptor *adaptor=new ImageViewAdaptor(CommandLine::instance()->getMainWindow());

    adaptor->RaiseWindow();
    adaptor->OpenImage(QApplication::applicationDirPath()+"/png.png");

    adaptor->deleteLater();
    adaptor=nullptr;
}
#endif

