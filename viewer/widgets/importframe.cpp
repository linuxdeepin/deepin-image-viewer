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
#include "importframe.h"
#include "application.h"
#include "controller/importer.h"
#include "utils/baseutils.h"
#include <QDropEvent>

#include <QPushButton>
#include <QVBoxLayout>
#include <QStyle>
#include <QFileDialog>

ImportFrame::ImportFrame(QWidget *parent)
    : QWidget(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    this->setAcceptDrops(true);
    m_bgLabel = new QLbtoDLabel();
    m_bgLabel->setFixedSize(164, 104);
    m_bgLabel->setObjectName("ImportBgLabel");


    m_importButton = new QPBtnDPushButton();
    m_importButton->setFixedSize(120, 20);
    m_importButton->setObjectName("ImportFrameButton");
    connect(m_importButton, &QPBtnDPushButton::clicked, this, &ImportFrame::clicked);

    m_titleLabel = new QLbtoDLabel();
    m_titleLabel->setObjectName("ImportFrameTooltip");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(m_bgLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(18);
    layout->addWidget(m_importButton, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    layout->addStretch(1);

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &ImportFrame::onThemeChanged);

}





const QString ImportFrame::buttonText() const
{
    return m_importButton->text();
}

void ImportFrame::onThemeChanged(ViewerThemeManager::AppTheme theme)
{
    Q_UNUSED(theme);
}
