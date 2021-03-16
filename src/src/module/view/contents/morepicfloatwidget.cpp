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
#include "morepicfloatwidget.h"
#include <QColor>
#include <DGuiApplicationHelper>
MorePicFloatWidget::MorePicFloatWidget(QWidget *parent)
    : DFloatingWidget(parent)
{

}

MorePicFloatWidget::~MorePicFloatWidget()
{

}

void MorePicFloatWidget::initUI()
{
    setBlurBackgroundEnabled(true);
    m_pLayout = new QVBoxLayout(this);
    this->setLayout(m_pLayout);
    m_buttonUp = new  DIconButton(this);
    m_buttonDown = new  DIconButton(this);
    m_labelNum = new DLabel(this);
    m_pLayout->addWidget(m_labelNum);
    m_labelNum->setAlignment(Qt::AlignCenter);
    m_labelNum->setText("0/0");
    m_buttonUp->setIcon(QIcon::fromTheme("dcc_up"));
    m_buttonUp->setIconSize(QSize(40, 40));
    m_buttonDown->setIcon(QIcon::fromTheme("dcc_down"));
    m_buttonDown->setIconSize(QSize(40, 40));

    DPalette pa1 = m_buttonUp->palette();
    DPalette pa2 = m_buttonDown->palette();;
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        pa1.setColor(DPalette::Light, QColor(255, 255, 255, 255));
        pa1.setColor(DPalette::Dark, QColor(255, 255, 255, 255));

        pa2.setColor(DPalette::Light, QColor(255, 255, 255, 255));
        pa2.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
    } else {
        pa1.setColor(DPalette::Light, QColor(40, 40, 40, 255));
        pa1.setColor(DPalette::Dark, QColor(40, 40, 40, 255));

        pa2.setColor(DPalette::Light, QColor(40, 40, 40, 255));
        pa2.setColor(DPalette::Dark, QColor(40, 40, 40, 255));
    }
    m_buttonUp->setPalette(pa1);
    m_buttonDown->setPalette(pa2);

    m_pLayout->addWidget(m_buttonUp);
    m_pLayout->addWidget(m_buttonDown);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
    this, [ = ]() {
        if (!m_buttonUp && !m_buttonDown) {
            return;
        }
        DGuiApplicationHelper::ColorType themeType =
            DGuiApplicationHelper::instance()->themeType();
        DPalette pa1 = m_buttonUp->palette();
        DPalette pa2 = m_buttonDown->palette();
        if (themeType == DGuiApplicationHelper::LightType) {
            pa1.setColor(DPalette::Light, QColor(255, 255, 255, 255));
            pa2.setColor(DPalette::Light, QColor(255, 255, 255, 255));

            pa1.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
            pa2.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
        } else {
            pa1.setColor(DPalette::Light, QColor(40, 40, 40, 255));
            pa2.setColor(DPalette::Light, QColor(40, 40, 40, 255));

            pa1.setColor(DPalette::Dark, QColor(40, 40, 40, 255));
            pa2.setColor(DPalette::Dark, QColor(40, 40, 40, 255));
        }
        m_buttonUp->setPalette(pa1);
        m_buttonDown->setPalette(pa2);
    });


}

DIconButton *MorePicFloatWidget::getButtonUp()
{
    return m_buttonUp;
}

DIconButton *MorePicFloatWidget::getButtonDown()
{
    return m_buttonDown;
}

void MorePicFloatWidget::setLabelText(const QString &num)
{
    m_labelNum->setText(num);
}
