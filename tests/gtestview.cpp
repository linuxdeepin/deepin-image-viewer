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
#include <QKeySequence>
#include <QShortcut>
#include <QEnterEvent>
#include <QFile>
#include <QDir>
#include <DApplication>
#include "mainwindow/mainwindow.h"
#include "module/view/homepagewidget.h"
#include <libimageviewer/imageengine.h>
#include <QDropEvent>

gtestview::gtestview()
{

}
TEST_F(gtestview, cpFile)
{
    QFile::copy(":/gif.gif", QApplication::applicationDirPath() + "/gif.gif");
    QFile(QApplication::applicationDirPath() + "/gif.gif").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/gif2.gif", QApplication::applicationDirPath() + "/gif2.gif");
    QFile(QApplication::applicationDirPath() + "/gif2.gif").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/ico.ico", QApplication::applicationDirPath() + "/ico.ico");
    QFile(QApplication::applicationDirPath() + "/ico.ico").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/jpg.jpg", QApplication::applicationDirPath() + "/jpg.jpg");
    QFile(QApplication::applicationDirPath() + "/jpg.jpg").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/mng.mng", QApplication::applicationDirPath() + "/mng.mng");
    QFile(QApplication::applicationDirPath() + "/mng.mng").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/png.png", QApplication::applicationDirPath() + "/png.png");
    QFile(QApplication::applicationDirPath() + "/png.png").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg.svg", QApplication::applicationDirPath() + "/svg.svg");
    QFile(QApplication::applicationDirPath() + "/svg.svg").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg1.svg", QApplication::applicationDirPath() + "/svg1.svg");
    QFile(QApplication::applicationDirPath() + "/svg1.svg").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg2.svg", QApplication::applicationDirPath() + "/svg2.svg");
    QFile(QApplication::applicationDirPath() + "/svg2.svg").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg1.svg", QApplication::applicationDirPath() + "/svg3.svg");
    QFile(QApplication::applicationDirPath() + "/svg3.svg").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/tif.tif", QApplication::applicationDirPath() + "/tif.tif");
    QFile(QApplication::applicationDirPath() + "/tif.tif").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/wbmp.wbmp", QApplication::applicationDirPath() + "/wbmp.wbmp");
    QFile(QApplication::applicationDirPath() + "/wbmp.wbmp").setPermissions(\
                                                                            QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                            QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/dds.dds", QApplication::applicationDirPath() + "/dds.dds");
    QFile(QApplication::applicationDirPath() + "/dds.dds").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/tga.tga", QApplication::applicationDirPath() + "/tga.tga");
    QFile(QApplication::applicationDirPath() + "/tga.tga").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/errorPic.icns", QApplication::applicationDirPath() + "/errorPic.icns");
    QFile(QApplication::applicationDirPath() + "/errorPic.icns").setPermissions(\
                                                                                QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                                QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QDir a(QApplication::applicationDirPath());
    a.mkdir("test");
    QFile::copy(":/jpg.jpg", QApplication::applicationDirPath() + "/test/jpg.jpg");
    QFile(QApplication::applicationDirPath() + "/test/jpg.jpg").setPermissions(\
                                                                               QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                               QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QString path = QApplication::applicationDirPath() + "/gif.gif";
    EXPECT_EQ(true, QFileInfo(path).isFile());
}

//主窗体
TEST_F(gtestview, MainWindow)
{
    DMainWindow *dw = new DMainWindow();
    MainWindow *w = new MainWindow();
    w->setDMainWindow(dw);
    w->initSize();
    w->setValue("", "", "");
    w->value("", "", "");

    //home page
    w->resize(800, 600);
    QTest::qWait(300);
    w->show();
    w->resize(1080, 600);
    QTest::qWait(300);

    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);

    //view panel
    w->findChild<DSuggestButton *>("Open Image")->click(); //需要去ac-desktop-define.h查找这个值
    QTest::qWait(300);

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(QApplication::applicationDirPath() + "/test/jpg.jpg"));
    mimedata.setUrls(li);

    //drop image to homepage
    auto homepage = w->findChild<HomePageWidget *>("ThumbnailWidget"); //需要去ac-desktop-define.h查找这个值
    auto dropPos = homepage->rect().center();
    QDragEnterEvent eEnter(dropPos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(homepage, &eEnter);

    QDragMoveEvent emove(dropPos + QPoint(10, 10), Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(w, &emove);

    QDropEvent e(dropPos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(homepage, &e);

    QTest::qWait(300);
    emit ImageEngine::instance()->sigPicCountIsNull();

    //MainWindow drag/drop event
    QMimeData mimedata_2;
    QList<QUrl> li_2;
    li_2.append(QUrl(QApplication::applicationDirPath() + "/test/svg.svg"));
    mimedata_2.setUrls(li_2);

    auto dropPos_2 = w->rect().center();
    QDragEnterEvent eEnter_2(dropPos_2, Qt::IgnoreAction, &mimedata_2, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(w, &eEnter_2);

    QDragMoveEvent emove_2(dropPos_2 + QPoint(10, 10), Qt::IgnoreAction, &mimedata_2, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(w, &emove_2);

    QDropEvent e_2(dropPos_2, Qt::IgnoreAction, &mimedata_2, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(w, &e_2);

    QTest::qWait(300);

    w->close();
    QTest::qWait(300);

    w->deleteLater();
    w = nullptr;
    dw->deleteLater();
    dw = nullptr;
}
TEST_F(gtestview, checkMinePaths)
{
    bool bRet = false;
    HomePageWidget *widget = new HomePageWidget();

    QStringList s;
    s << QApplication::applicationDirPath() + "/test/jpgxxxx.jpg";
    bRet = widget->checkMinePaths(s);

    s << QApplication::applicationDirPath() + "/test/jpg.jpg";
    s << QApplication::applicationDirPath() + "/test/png.png";
    bRet = widget->checkMinePaths(s);

    widget->deleteLater();
    widget = nullptr;

    EXPECT_EQ(true, bRet);
}

TEST_F(gtestview, showShortCut)
{
    MainWindow *w = new MainWindow();
    w->showShortCut();
    w->deleteLater();
    w = nullptr;
}

