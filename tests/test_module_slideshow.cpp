/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "gtestview.h"
#define protected public
#define private public
//#include "module/slideshow/slideeffectplayer.h"
//#include "module/slideshow/slideeffect.h"
//#include "module/slideshow/slideshowbottombar.h"
#include "module/view/scen/imageview.h"
TEST_F(gtestview, Sslideshowpanel1)
{
    SlideShowPanel *panel = new SlideShowPanel();
    emit dApp->signalM->sigESCKeyStopSlide();
//     panel->backToLastPanel();
    DBImgInfoList list;
    DBImgInfo info;
    info.format = "png";
    info.fileName = QApplication::applicationDirPath() + "/test/jpg100.jpg";
    info.filePath = QApplication::applicationDirPath() + "/test/jpg100.jpg";
    list.push_back(info);
    //panel->Receiveslideshowpathlst(true,list);

    QMenu *m_menu = new QMenu();

    QAction *action = new QAction(m_menu);
    action->setProperty("MenuID", 5);
    action->trigger();

    QAction *action1 = new QAction(m_menu);
    action1->setProperty("MenuID", SlideShowPanel::IdPlayOrPause);
    action1->trigger();

    QAction *action2 = new QAction(m_menu);
    action2->setProperty("MenuID", SlideShowPanel::IdStopslideshow);
    action2->trigger();
    panel->onMenuItemClicked(action);
//    panel->onMenuItemClicked(action1);
//    panel->onMenuItemClicked(action2);
    delete m_menu;
    m_menu = nullptr;

//    panel->getFitImage(QApplication::applicationDirPath() + "/test/jpg33.jpg");
    panel->onThemeChanged(ViewerThemeManager::Dark);
    panel->onThemeChanged(ViewerThemeManager::Light);

    QTest::mouseMove(panel, QPoint(1000, 1075), 500);
    QTest::mouseMove(panel, QPoint(200, 500), 500);

    delete panel;
    panel = nullptr;
}

//TEST_F(gtestview, SlideEffectPlayer_1)
//{
//    SlideEffectPlayer *player = new SlideEffectPlayer();
//    emit dApp->signalM->sigStartTimer();
//    emit dApp->signalM->updateButton();
//    QStringList list;
//    player->setFrameSize(200, 200);
//    list.push_back(QApplication::applicationDirPath() + "/test/jpg100.jpg");
//    list.push_back(QApplication::applicationDirPath() + "/test/jpg101.jpg");
//    list.push_back(QApplication::applicationDirPath() + "/test/jpg102.jpg");
//    list.push_back(QApplication::applicationDirPath() + "/test/jpg103.jpg");
//    list.push_back(QApplication::applicationDirPath() + "/test/jpg104.jpg");
//    player->SetfirstlastThunbnailpath(QApplication::applicationDirPath() + "/test/jpg100.jpg", QApplication::applicationDirPath() + "/test/jpg101.jpg");
//    player->isRunning();
//    player->setImagePaths(QStringList(QApplication::applicationDirPath() + "/test/jpg100.jpg"));
//    player->start();
//    player->startNext();
//    player->cacheNext();
//    player->GetPathList();

//    player->cachePrevious();
//    player->startPrevious();
//    player->pause();
//    player->stop();
//    player->deleteLater();
//    player = nullptr;

//}

//TEST_F(gtestview, SlideEffect_Circle_1)
//{
//    SlideEffect *m_effect = SlideEffect::create("");

////    m_effect->prepare();

////    m_effect->resizeImages();

//    delete m_effect;
//    m_effect = nullptr;
//}


TEST_F(gtestview, PanelTest)
{
    emit dApp->signalM->sigLoadTailThumbnail();
}
//SlideShowBottomBar


TEST_F(gtestview, SlideShowBottomBar1)
{

    SlideShowBottomBar *bottom = new SlideShowBottomBar();
    bottom->m_preButton->click();
    bottom->m_nextButton->click();
    bottom->m_playpauseButton->click();
    bottom->m_playpauseButton->click();
    bottom->m_cancelButton->click();

    emit bottom->showPrevious();
    emit bottom->showPause();
    emit bottom->showNext();
    emit bottom->showCancel();

    bottom->deleteLater();
    bottom = nullptr;

}



