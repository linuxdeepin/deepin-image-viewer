/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
#include "ttlcontent.h"
#include "application.h"
#include "utils/baseutils.h"
#include "widgets/pushbutton.h"
#include <QHBoxLayout>
#include <QDebug>

namespace {

const int LEFT_MARGIN = 13;
const int MAX_BUTTON_WIDTH = 200;
const QString FAVORITES_ALBUM_NAME = "My favorite";
}  // namespace

TTLContent::TTLContent(bool inDB, QWidget *parent) : QWidget(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(LEFT_MARGIN, 0, 0, 0);
    hb->setSpacing(0);
    m_returnBtn = new PushButton();
    m_returnBtn->setMaximumWidth(MAX_BUTTON_WIDTH);
    m_returnBtn->setObjectName("ReturnBtn");
    m_returnBtn->setToolTip(tr("Back"));
    PushButton *folderBtn = new PushButton();
    folderBtn->setObjectName("FolderBtn");
    folderBtn->setToolTip(tr("Image management"));
    if(inDB) {
        hb->addWidget(m_returnBtn);
    } else {
       hb->addWidget(folderBtn);
    }
    hb->addStretch();

    connect(m_returnBtn, &PushButton::clicked, this, [=] {
        emit clicked();
    });
    connect(folderBtn, &PushButton::clicked, this, [=] {
        emit clicked();
    });
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &TTLContent::onThemeChanged);
}

void TTLContent::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/dark/qss/view.qss"));
    } else {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/light/qss/view.qss"));
    }
}

void TTLContent::setCurrentDir(QString text) {
    if (text == FAVORITES_ALBUM_NAME) {
        text = tr("My favorite");
    }
    m_returnBtn->setText(text);
    m_returnBtn->setMaximumWidth(this->width()/2);
    update();
}
