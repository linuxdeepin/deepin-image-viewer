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
#include <QObject>
#include <QSwipeGesture>
#define private public
#include "module/view/scen/imageview.h"

TEST_F(gtestview, MainWindowTabKey)
{
    m_frameMainWindow = CommandLine::instance()->getMainWindow();
    if (m_frameMainWindow) {
        m_frameMainWindow->initAllViewTabKeyOrder(nullptr);
        m_frameMainWindow->initEmptyTabOrder();
        m_frameMainWindow->initNormalPicTabOrder();
    }
}
// connect(animation, SIGNAL(finished()), this, SLOT(OnFinishPinchAnimal()));
TEST_F(gtestview, m_OnFinishPinchAnimal)
{
    m_frameMainWindow = CommandLine::instance()->getMainWindow();

    ImageView *panel = m_frameMainWindow->findChild<ImageView *>(IMAGE_VIEW);
    if (panel) {
        panel->OnFinishPinchAnimal();
        QList<QGesture *> gestures3;
        QSwipeGesture testGest3;
        testGest3.setHotSpot(QPoint(300, 300));
        testGest3.setSwipeAngle(qreal(2));
        gestures3.push_back(&testGest3);
        QGestureEvent event7(gestures3);
        qApp->sendEvent(dynamic_cast<QObject *>(panel->viewport()), &event7);

        panel->swipeTriggered(&testGest3);

        QList<QGesture *> gestures;
        QPinchGesture testGest;
        testGest.setHotSpot(QPoint(300, 300));
        testGest.setTotalChangeFlags(QPinchGesture::ScaleFactorChanged);
        gestures.push_back(&testGest);
        QGestureEvent event4(gestures);
        panel->handleGestureEvent(&event4);

    }
}
