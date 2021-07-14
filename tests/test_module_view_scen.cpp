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
#include <QGestureEvent>
#include <QPointF>
#include <QMouseEvent>

#include <QCoreApplication>
#include "module/view/scen/imageview.h"
#ifdef test_module_view_scen
TEST_F(gtestview, showVagueImage)
{

    m_frameMainWindow = CommandLine::instance()->getMainWindow();

    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if (panel) {
        panel->showVagueImage(QPixmap(QApplication::applicationDirPath() + "/png.png"), QApplication::applicationDirPath() + "/png.png");
    }
}

TEST_F(gtestview, showFileImage)
{
    m_frameMainWindow = CommandLine::instance()->getMainWindow();


    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if (panel) {
        //        panel->showFileImage();
    }
}

TEST_F(gtestview, mouse)
{
    m_frameMainWindow = CommandLine::instance()->getMainWindow();

    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if (panel) {
        QTest::mousePress(panel, Qt::LeftButton, Qt::NoModifier, QPoint(200, 200), 500);
        QTest::mouseRelease(panel, Qt::LeftButton, Qt::NoModifier, QPoint(200, 100), 500);
        QTest::mouseClick(panel, Qt::LeftButton, Qt::NoModifier, QPoint(200, 50), 500);
        QTest::mouseMove(panel, QPoint(200, 100), 500);
        QTest::keyClick(panel, Qt::Key_Escape, Qt::ShiftModifier, 500);
        QTest::mouseDClick(panel, Qt::LeftButton, Qt::NoModifier, QPoint(200, 50), 500);
    }
}

TEST_F(gtestview, reloadSvgPix)
{
    m_frameMainWindow = CommandLine::instance()->getMainWindow();

    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if (panel) {
        panel->reloadSvgPix(QApplication::applicationDirPath() + "/svg1.svg", 90, true);
        panel->reloadSvgPix(QApplication::applicationDirPath() + "/svg2.svg", -90, false);
    }
    DGuiApplicationHelper::instance();

}

TEST_F(gtestview, setThemeType)
{
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);

    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);

    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::UnknownType);

}

TEST_F(gtestview, loadingDisplay)
{
    dApp->signalM->loadingDisplay(true);
}

TEST_F(gtestview, clear)
{
    m_frameMainWindow = CommandLine::instance()->getMainWindow();

    //    QTest::qWait(400);

    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if (panel) {
        panel->clear();
    }
}
TEST_F(gtestview, QGestureEvent)
{
    m_frameMainWindow = CommandLine::instance()->getMainWindow();


    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if (panel) {

        QMimeData mimedata;
        //        QList<QUrl> li;
        //        li.append(QUrl::fromLocalFile(TriangleItemPath));

        //        mimedata.setUrls(li);
        QList<QGesture *> gestures;
        QGestureEvent gestureEvent(gestures);
        qApp->sendEvent(panel, &gestureEvent);

    }
}

TEST_F(gtestview, QWheelEvent_1)
{
    m_frameMainWindow = CommandLine::instance()->getMainWindow();

    ImageView *view = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if (view) {

//        QTest::mouseDClick(view, Qt::LeftButton, Qt::NoModifier, QPoint(30, 30), 50);

//        QTest::mouseDClick(view, Qt::MidButton, Qt::NoModifier, QPoint(30, 30), 50);

//        QTest::mousePress(view, Qt::MidButton, Qt::NoModifier, QPoint(40, 50), 50);

//        QTest::mouseRelease(view, Qt::MidButton, Qt::NoModifier, QPoint(80, 90), 50);

//        QTest::mousePress(view, Qt::MiddleButton, Qt::NoModifier, QPoint(40, 50), 50);

//        QTest::mouseRelease(view, Qt::MiddleButton, Qt::NoModifier, QPoint(80, 90), 50);

        QMouseEvent event(QEvent::MouseButtonDblClick, QPoint(30, 30), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

        qApp->sendEvent(view, &event);

        QMouseEvent event1(QEvent::MouseButtonPress, QPoint(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

        qApp->sendEvent(view, &event1);

        QMouseEvent event2(QEvent::MouseButtonRelease, QPoint(100, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

        qApp->sendEvent(view, &event2);

        QMouseEvent event3(QEvent::MouseMove, QPoint(200, 150), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

        qApp->sendEvent(view, &event3);

        QString TriangleItemPath = "path";

        QMimeData mimedata1;
        QList<QUrl> li;
        li.append(QUrl::fromLocalFile(TriangleItemPath));

        mimedata1.setUrls(li);

        const QPoint pos = QPoint(view->pos().x() + 200, view->pos().y() + 200);
        QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata1, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(view, &eEnter);

        QDropEvent e(pos, Qt::IgnoreAction, &mimedata1, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(view, &e);

    }
}

//还没有模拟手指事件
#endif
