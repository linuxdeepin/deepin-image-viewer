#include "gtestview.h"
#include "viewer/module/slideshow/slideeffectplayer.h"
#include "viewer/module/slideshow/slideeffect.h"
#include "viewer/module/slideshow/slideshowpanel.h"
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

}

TEST_F(gtestview, slideeffect_1)
{
//     SlideEffect * effect=SlideEffect::create("invalid");
//     SlideEffect * effect=new  SlideEffect()
//     effect->setImages(QApplication::applicationDirPath() + "/test/jpg100.jpg",QApplication::applicationDirPath() + "/test/jpg101.jpg");
//     effect->start();
//     effect->pause();
//     effect->stop();
//     effect->setSize(QSize(200,200));
//     effect->setType(kRandom);
//     effect->setAllMs(1000);

//     effect->setDuration(1000);
//     effect->currentFrame();
//     effect->clearimagemap();
}



