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
#include "topalbumtips.h"
#include "application.h"
#include "controller/dbmanager.h"
#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDebug>

TopAlbumTips::TopAlbumTips(QWidget *parent) : QFrame(parent)
{
    setFixedHeight(24);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_infoLabel = new QLabel();
    m_infoLabel->setObjectName("AlbumInfoTipsLabel");

    QFont font;
    font.setWeight(25);
    m_infoLabel->setFont(font);
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(13, 0, 13, 0);
    m_layout->addWidget(m_infoLabel);
    m_layout->addStretch(1);
}

void TopAlbumTips::setAlbum(const QString &album)
{
    auto info = DBManager::instance()->getAlbumInfo(album);
    if (info.count == 0) {
        m_infoLabel->setText("");
    }
    else {
        const QString beginTime = info.beginTime.toString(tr("dd MMMM yyyy"));
        const QString endTime = info.endTime.toString(tr("dd MMMM yyyy"));
        const QString l = (beginTime.isEmpty() || endTime.isEmpty())
                ? "" : beginTime + "-" + endTime;

        m_infoLabel->setText(QString("%1").arg(trName(album)) + "  " + l);
    }
}

void TopAlbumTips::setLeftMargin(int v)
{
    m_layout->setContentsMargins(v + 18, 0, 13, 0);
}

const QString TopAlbumTips::trName(const QString &name) const
{
    if (name == "My favorite") {
        return tr("My favorite");
    }
    else if (name == "Recent imported") {
        return tr("Recent imported");
    }
    else {
        QFontMetrics fm(this->font());
        return fm.elidedText(name, Qt::ElideMiddle, 255);
    }
}
