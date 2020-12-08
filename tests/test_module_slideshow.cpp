#include "gtestview.h"
#define protected public
#include "module/slideshow/slideeffectplayer.h"

#include "module/slideshow/slideeffect.h"
#include "module/slideshow/slideshowpanel.h"

#include "module/view/scen/imageview.h"
TEST_F(gtestview, Sslideshowpanel1)
{
SlideShowPanel *panel=new SlideShowPanel();
DBImgInfoList list;
DBImgInfo info;
info.format="png";
info.fileName=QApplication::applicationDirPath() + "/test/jpg100.jpg";
info.filePath=QApplication::applicationDirPath() + "/test/jpg100.jpg";
list.push_back(info);
//panel->Receiveslideshowpathlst(true,list);
}

TEST_F(gtestview, SlideEffectPlayer_1)
{
    SlideEffectPlayer *player=new SlideEffectPlayer();
    emit dApp->signalM->sigStartTimer();
    emit dApp->signalM->updateButton();
    QStringList list;
    player->setFrameSize(200,200);
    list.push_back(QApplication::applicationDirPath() + "/test/jpg100.jpg");
    list.push_back(QApplication::applicationDirPath() + "/test/jpg101.jpg");
    list.push_back(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    list.push_back(QApplication::applicationDirPath() + "/test/jpg103.jpg");
    list.push_back(QApplication::applicationDirPath() + "/test/jpg104.jpg");
    player->SetfirstlastThunbnailpath(QApplication::applicationDirPath() + "/test/jpg100.jpg",QApplication::applicationDirPath() + "/test/jpg101.jpg");
    player->isRunning();
    player->setImagePaths(QStringList(QApplication::applicationDirPath() + "/test/jpg100.jpg"));
    player->start();
    player->startNext();
    player->cacheNext();
    player->GetPathList();

    player->cachePrevious();
    player->startPrevious();
    player->pause();
    player->stop();
    player->deleteLater();
    player=nullptr;

}

TEST_F(gtestview, SlideEffect_Circle_1)
{

}



