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
#include "utils/imageutils.h"

#include "widgets/pushbutton.h"
#include "widgets/returnbutton.h"
#include "controller/dbmanager.h"

#include <QTimer>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QDebug>

namespace {
const int LEFT_MARGIN = 13;
const QSize ICON_SIZE = QSize(48, 39);
const QString FAVORITES_ALBUM = "My favorite";
const int ICON_SPACING = 3;
const int RETURN_BTN_MAX = 200;
}  // namespace

TTLContent::TTLContent(bool inDB,
                       QWidget *parent) : QWidget(parent)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setContentsMargins(LEFT_MARGIN, 0, 0, 0);
    hb->setSpacing(0);
    m_inDB = inDB;
    m_returnBtn = new ReturnButton();
    m_returnBtn->setMaxWidth(RETURN_BTN_MAX);
    m_returnBtn->setMaximumWidth(RETURN_BTN_MAX);
    m_returnBtn->setObjectName("ReturnBtn");
    m_returnBtn->setToolTip(tr("Back"));


    PushButton *folderBtn = new PushButton();
    folderBtn->setObjectName("FolderBtn");
    folderBtn->setToolTip(tr("Image management"));
    if(m_inDB) {
        hb->addWidget(m_returnBtn);
    } else {
       hb->addWidget(folderBtn);
    }
    hb->addSpacing(5);

    connect(m_returnBtn, &ReturnButton::clicked, this, [=] {
        emit clicked();
    });
    connect(folderBtn, &PushButton::clicked, this, [=] {
        emit clicked();
    });

    // Adapt buttons////////////////////////////////////////////////////////////
    m_adaptImageBtn = new PushButton();
    m_adaptImageBtn->setObjectName("AdaptBtn");
    m_adaptImageBtn->setFixedSize(ICON_SIZE);

    m_adaptImageBtn->setToolTip(tr("1:1 Size"));
    hb->addWidget(m_adaptImageBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_adaptImageBtn, &PushButton::clicked, this, [=] {
        emit resetTransform(false);
    });

    m_adaptScreenBtn = new PushButton();
    m_adaptScreenBtn->setFixedSize(ICON_SIZE);
    m_adaptScreenBtn->setObjectName("AdaptScreenBtn");
    m_adaptScreenBtn->setToolTip(tr("Fit to window"));
    hb->addWidget(m_adaptScreenBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_adaptScreenBtn, &PushButton::clicked, this, [=] {
        emit resetTransform(true);
    });

    // Collection button////////////////////////////////////////////////////////
    m_clBT = new PushButton();
    m_clBT->setFixedSize(ICON_SIZE);
    m_clBT->setObjectName("CollectBtn");
    if (m_inDB) {
        hb->addWidget(m_clBT);
        connect(m_clBT, &PushButton::clicked, this, [=] {
            if (DBManager::instance()->isImgExistInAlbum(FAVORITES_ALBUM, m_imagePath)) {
                DBManager::instance()->removeFromAlbum(FAVORITES_ALBUM, QStringList(m_imagePath));
            }
            else {
                DBManager::instance()->insertIntoAlbum(FAVORITES_ALBUM, QStringList(m_imagePath));
            }
            updateCollectButton();
        });
        updateCollectButton();
    }

    m_rotateLBtn = new PushButton();
    m_rotateLBtn->setFixedSize(ICON_SIZE);
    m_rotateLBtn->setObjectName("RotateBtn");
    m_rotateLBtn->setToolTip(tr("Rotate counterclockwise"));
    hb->addWidget(m_rotateLBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_rotateLBtn, &PushButton::clicked,
            this, &TTLContent::rotateCounterClockwise);

    m_rotateRBtn = new PushButton();
    m_rotateRBtn->setFixedSize(ICON_SIZE);
    m_rotateRBtn->setObjectName("RotateCounterBtn");
    m_rotateRBtn->setToolTip(tr("Rotate clockwise"));
    hb->addWidget(m_rotateRBtn);
    hb->addSpacing(ICON_SPACING);
    connect(m_rotateRBtn, &PushButton::clicked,
            this, &TTLContent::rotateClockwise);

    m_trashBtn = new PushButton();
    m_trashBtn->setFixedSize(ICON_SIZE);
    m_trashBtn->setObjectName("TrashBtn");
    m_trashBtn->setToolTip(tr("Throw to Trash"));
    hb->addWidget(m_trashBtn);

    connect(m_trashBtn, &PushButton::clicked, this, &TTLContent::removed);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &TTLContent::onThemeChanged);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &TTLContent::onThemeChanged);
}

void TTLContent::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/dark/qss/ttl.qss"));
    } else {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/light/qss/ttl.qss"));
    }
}

void TTLContent::setCurrentDir(QString text) {
    if (text == FAVORITES_ALBUM) {
        text = tr("My favorite");
    }

    m_returnBtn->setText(text);
    qDebug() << "setCurrentDir" << m_returnBtn->buttonWidth();

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(1000);
    connect(timer, &QTimer::timeout, this, &TTLContent::updateReturnButton);
}

void TTLContent::updateReturnButton()
{
    int width = this->width();
    int contentWidth = 0;
    if (m_inDB)
    {
        contentWidth = m_returnBtn->buttonWidth() + ICON_SIZE.width()*6;
        if (width != contentWidth)
        {
            setFixedWidth(std::max(contentWidth, width));
        }
    } else
    {
        contentWidth = m_returnBtn->buttonWidth() + ICON_SIZE.width()*5;
        if (width != contentWidth)
        {
            setFixedWidth(std::max(contentWidth, width));
        }
    }

    emit contentWidthChanged(this->width());
    qDebug() << "DFFF" << contentWidth;
}

void TTLContent::setImage(const QString &path)
{
    m_imagePath = path;
    if (path.isEmpty() || !QFileInfo(path).exists()
            || !QFileInfo(path).isReadable()) {
        m_adaptImageBtn->setDisabled(true);
        m_adaptScreenBtn->setDisabled(true);
        m_rotateLBtn->setDisabled(true);
        m_rotateRBtn->setDisabled(true);
        m_trashBtn->setDisabled(true);
    } else {
        m_adaptImageBtn->setDisabled(false);
        m_adaptScreenBtn->setDisabled(false);
        if (QFileInfo(path).isReadable() &&
                !QFileInfo(path).isWritable()) {
            m_trashBtn->setDisabled(true);
            m_rotateLBtn->setDisabled(true);
            m_rotateRBtn->setDisabled(true);
        } else {
            m_trashBtn->setDisabled(false);
            if (utils::image::imageSupportSave(path)) {
                m_rotateLBtn->setDisabled(false);
                m_rotateRBtn->setDisabled(false);
            } else {
                m_rotateLBtn->setDisabled(true);
                m_rotateRBtn->setDisabled(true);
            }
        }
    }

    updateCollectButton();
}

void TTLContent::updateCollectButton()
{
    if (! m_clBT)
        return;

    if (m_imagePath.isEmpty() || !QFileInfo(m_imagePath).exists()) {
        m_clBT->setDisabled(true);
        m_clBT->setChecked(false);
    }
    else
        m_clBT->setDisabled(false);

    if (! m_clBT->isEnabled()) {
        m_clBT->setDisabled(true);
    }
    else if (DBManager::instance()->isImgExistInAlbum(FAVORITES_ALBUM,
                                                      m_imagePath)) {
        m_clBT->setToolTip(tr("Unfavorite"));
        m_clBT->setChecked(true);
    }
    else {
        m_clBT->setToolTip(tr("Favorite"));
        m_clBT->setChecked(false);
        m_clBT->setDisabled(false);
    }
}
