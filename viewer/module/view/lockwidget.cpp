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
#include "lockwidget.h"

#include <QVBoxLayout>

#include "utils/baseutils.h"

LockWidget::LockWidget(const QString &darkFile,
    const QString &lightFile, QWidget *parent)
    : ThemeWidget(darkFile, lightFile, parent) {
    m_bgLabel = new QLabel();
    m_bgLabel->setFixedSize(166, 166);
    m_bgLabel->setObjectName("BgLabel");

    m_lockTips = new QLabel();
    m_lockTips->setObjectName("LockTips");
    setContentText(tr("You have no permission to view the image"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(m_bgLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(18);
    layout->addWidget(m_lockTips, 0, Qt::AlignHCenter);
    layout->addStretch(1);

}

void LockWidget::setContentText(const QString &text) {
    m_lockTips->setText(text);
    int textHeight = utils::base::stringHeight(m_lockTips->font(),
                                               m_lockTips->text());
    m_lockTips->setMinimumHeight(textHeight + 2);
}



LockWidget::~LockWidget() {}
