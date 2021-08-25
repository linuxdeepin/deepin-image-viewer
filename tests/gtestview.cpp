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
#define  private public
#include <QKeySequence>
#include <QShortcut>
#include <QEnterEvent>
#include <QFile>
#include <QDir>

#include "accessibility/ac-desktop-define.h"
#include "imageviewer.h"

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
}

//主窗体
TEST_F(gtestview, MainWindow)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeLocal, CACHE_PATH, nullptr);
    QStringList paths(QApplication::applicationDirPath() + "/test/jpg.jpg");
    bool bRet = m_imageViewer->startdragImage(paths);
    if (bRet) {

        m_imageViewer->showNormal();
        m_imageViewer->resize(800, 600);
        m_imageViewer->resize(1000, 500);

        m_imageViewer->setTopBarVisible(false);
        m_imageViewer->setTopBarVisible(true);
        m_imageViewer->setBottomtoolbarVisible(false);
        m_imageViewer->setTopBarVisible(true);

        QTest::qWait(500);
    }
    m_imageViewer->deleteLater();

}
