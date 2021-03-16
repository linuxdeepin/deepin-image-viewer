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
    DeepinImageViewerDBus *dubs = new DeepinImageViewerDBus(dApp->signalM);
    dubs->backToMainWindow() ;
    dubs->activeWindow();
    dubs->enterAlbum("test");
    dubs->searchImage("test");
    dubs->editImage("test");

    dubs->deleteLater();
    dubs = nullptr;
}

TEST_F(gtestview, dbusimageview_adaptor)
{
    ImageViewAdaptor *adaptor = new ImageViewAdaptor(CommandLine::instance()->getMainWindow());

    adaptor->RaiseWindow();
    adaptor->OpenImage(QApplication::applicationDirPath() + "/png.png");

    adaptor->deleteLater();
    adaptor = nullptr;
}
#endif

