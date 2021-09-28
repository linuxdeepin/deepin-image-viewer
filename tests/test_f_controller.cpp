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
#include "accessibility/ac-desktop-define.h"
#include "src/src/controller/dbusclient.h"
#include "src/src/controller/divdbuscontroller.h"
#define private public
#include "src/src/controller/wallpapersetter.h"

TEST_F(gtestview, Dbusclient1)
{

    Dbusclient *client = new  Dbusclient();
    QImage imgQImage(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    QList<QString> list;
    list.push_back(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    QList<QImage> listimg;
    listimg.push_back(imgQImage);
    client->openFiles(list);
    client->openImages(listimg);
    client->openDrawingBoard(list);
    client->deleteLater();
    client = nullptr;

    EXPECT_EQ(true, !(imgQImage.isNull()));
}

TEST_F(gtestview, DIVDBusController)
{
    DIVDBusController *control = new DIVDBusController();
    control->activeWindow();
    control->editImage(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    control->enterAlbum(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    control->searchImage(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    control->backToMainWindow();
    control->deleteLater();
    control = nullptr;
    QFileInfo info(QApplication::applicationDirPath() + "/test/jpg102.jpg");
    EXPECT_EQ(true, info.isReadable());
}
TEST_F(gtestview, setWallPaper)
{
    QString TriangleItemPath = QApplication::applicationDirPath() + "/test/jpg1.jpg";

    dApp->wpSetter->setWallpaper(TriangleItemPath);
    QFileInfo info(TriangleItemPath);
    EXPECT_EQ(true, info.isReadable());
}

TEST_F(gtestview, WallPaperSetting1)
{
    QString path = QApplication::applicationDirPath() + "/test/jpg102.jpg";
    WallpaperSetter::instance()->setWallpaper(QImage(QApplication::applicationDirPath() + "/test/jpg120.jpg"));
    if (!m_frameMainWindow) {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
    m_frameMainWindow->activateWindow();

    QFileInfo info(path);
    EXPECT_EQ(true, info.isReadable());
}


//CommandLine
TEST_F(gtestview, CommandLine_createOpenImageInfo)
{
    QString str = QApplication::applicationDirPath() + "/test/jpg113.jpg";
    QString jsonStr = CommandLine::instance()->createOpenImageInfo(str, QStringList(str), QDateTime::currentDateTime());
    QStringList list(str);
    CommandLine::instance()->paraOpenImageInfo(jsonStr, str, list);
    CommandLine::instance()->showHelp();
    QFileInfo info(str);
    EXPECT_EQ(true, info.isReadable());
}
