/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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
#include "settingswindow.h"
#include "titleframe.h"
#include "contentsframe.h"
#include "utils/baseutils.h"
#include <DTitlebar>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

SettingsWindow::SettingsWindow(QWidget *parent)
    :DMainWindow(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::WindowStaysOnTopHint|
                   Qt::FramelessWindowHint);

    if (titlebar()) titlebar()->setFixedHeight(0);
    setFixedSize(680, 560);

    // Main
    QFrame *f = new QFrame;
    setCentralWidget(f);
    QVBoxLayout *mainLayout = new QVBoxLayout(f);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Contents
    QHBoxLayout *cbl = new QHBoxLayout;
    cbl->setContentsMargins(0, 0, 0, 0);
    cbl->setSpacing(0);

    // TitleFrame
    TitleFrame *tf = new TitleFrame;
    cbl->addWidget(tf);
    ContentsFrame *cf = new ContentsFrame(this);
    cbl->addWidget(cf);

    connect(tf, &TitleFrame::clicked,
            cf, &ContentsFrame::setCurrentID);
    connect(cf, &ContentsFrame::currentFieldChanged,
            tf, &TitleFrame::setCurrentID);

    mainLayout->addLayout(cbl);
}

void SettingsWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }

    DMainWindow::keyPressEvent(e);
}
