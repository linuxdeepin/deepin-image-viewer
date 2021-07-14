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

TEST_F(gtestview, frame_bottomtoolbar)
{
    if (!m_frameMainWindow) {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }

    BottomToolbar *panel = m_frameMainWindow->findChild<BottomToolbar *>(BUTTOM_TOOL_BAR);
    if (panel) {
        QTest::mouseMove(panel, QPoint(300, 20), 100);
        QTest::mouseMove(panel, QPoint(400, 30), 100);
    }

}
TEST_F(gtestview, frame_mainwindow)
{
    if (!m_frameMainWindow) {
        m_frameMainWindow = CommandLine::instance()->getMainWindow();
    }
//    m_frameMainWindow->m_diskManager->mountAdded("", "");
//    m_frameMainWindow->m_diskManager->diskDeviceRemoved("");

    m_frameMainWindow->windowAtEdge();

}

TEST_F(gtestview, frame_picOneClear)
{
    emit dApp->signalM->picOneClear();
}

TEST_F(gtestview, frame_sigOpenFileDialog)
{
    emit dApp->signalM->sigOpenFileDialog();
}



