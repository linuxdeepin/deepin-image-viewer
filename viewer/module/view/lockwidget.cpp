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
#include "lockwidget.h"

#include <QVBoxLayout>
#include <DGuiApplicationHelper>

#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include "application.h"

const QString ICON_PIXMAP_DARK = ":/resources/dark/images/picture damaged_dark.svg";
const QString ICON_PIXMAP_LIGHT = ":/resources/light/images/picture damaged_light.svg";
const QSize THUMBNAIL_SIZE = QSize(151, 151);
LockWidget::LockWidget(const QString &darkFile,
    const QString &lightFile, QWidget *parent)
    : ThemeWidget(darkFile, lightFile, parent) {
    m_picString = "";
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::DarkType) {
        m_picString = ICON_PIXMAP_DARK;
        m_theme = true;
    } else {
        m_picString = ICON_PIXMAP_LIGHT;
        m_theme = false;
    }
    m_bgLabel = new QLbtoDLabel();
    m_bgLabel->setFixedSize(151, 151);
    m_bgLabel->setObjectName("BgLabel");
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, [=]() {
                         DGuiApplicationHelper::ColorType themeType =
                             DGuiApplicationHelper::instance()->themeType();
                         m_picString = "";
                         if (themeType == DGuiApplicationHelper::DarkType) {
                             m_picString = ICON_PIXMAP_DARK;
                             m_theme = true;
                         } else {
                             m_picString = ICON_PIXMAP_LIGHT;
                             m_theme = false;
                         }

                         QPixmap logo_pix = utils::base::renderSVG(m_picString, THUMBNAIL_SIZE);
                         m_bgLabel->setPixmap(logo_pix);
                     });
    m_lockTips = new QLbtoDLabel();
    m_lockTips->setObjectName("LockTips");
    setContentText(tr("You have no permission to view the image"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    QPixmap logo_pix = utils::base::renderSVG(m_picString, THUMBNAIL_SIZE);
    m_bgLabel->setPixmap(logo_pix);
    layout->addWidget(m_bgLabel, 0, Qt::AlignHCenter | Qt::AlignHCenter);
    //layout->addSpacing(18);
    //layout->addWidget(m_lockTips, 0, Qt::AlignHCenter);
    layout->addStretch(1);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &LockWidget::onThemeChanged);
}

void LockWidget::setContentText(const QString &text) {
    m_lockTips->setText(text);
    int textHeight = utils::base::stringHeight(m_lockTips->font(),
                                               m_lockTips->text());
    m_lockTips->setMinimumHeight(textHeight + 2);
}

void LockWidget::onThemeChanged(ViewerThemeManager::AppTheme theme)
{
//    if (theme == ViewerThemeManager::Dark) {
//        m_inBorderColor = utils::common::DARK_BORDER_COLOR;
//        if (m_isDefaultThumbnail)
//            m_defaultImage = m_logo;
//    } else {
//        m_inBorderColor = utils::common::LIGHT_BORDER_COLOR;
//        if (m_isDefaultThumbnail)
//            m_defaultImage = m_logo;
//    }

    ThemeWidget::onThemeChanged(theme);
    update();
}



LockWidget::~LockWidget() {}
