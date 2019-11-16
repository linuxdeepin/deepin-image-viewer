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
#include "ttmcontent.h"
#include "application.h"
#include "utils/baseutils.h"
#include "controller/configsetter.h"
#include "widgets/pushbutton.h"

#include <QBoxLayout>
#include <QDebug>
#include <QFileInfo>

const int MAX_LENGTH = 600;
const int LEFT_WIDGET_WIDTH = 383;

TTMContent::TTMContent(QWidget *parent)
    : QFrame(parent)
    , m_leftContentWidth(0)
{
    setObjectName("TTM");
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_fileNameLabel = new ElidedLabel(this);
    m_fileNameLabel->setMargin(0);
    m_fileNameLabel->setObjectName("ImagenameLabel");
    m_layout->addWidget(m_fileNameLabel);
    setLayout(m_layout);

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &TTMContent::onThemeChanged);
    connect(ConfigSetter::instance(), &ConfigSetter::valueChanged, this, [=](
            const QString &group, const QString &key, const QVariant &value){
            Q_UNUSED(key);
            Q_UNUSED(value);
            if (group == "MAINWINDOW")
            {
                m_contentWidth = std::max(m_windowWidth - 120 - LEFT_WIDGET_WIDTH, 1);
                setFixedWidth(m_contentWidth);
                setPath(m_path);
            }
    });
}

const QString TTMContent::getCurrentPath()
{
    return m_path;
}

void TTMContent::setPath(const QString &path)
{
    m_path = path;
    QString filename = QFileInfo(path).fileName();
    QString name;
    using namespace utils::base;
    QFont font;
    font.setPixelSize(12);
    m_fileNameLabel->setFont(font);
    QFontMetrics fm(font);
    int strWidth = fm.boundingRect(filename).width();
    int leftMargin = 0;
    if (strWidth > m_contentWidth || strWidth > MAX_LENGTH)
    {
        name = fm.elidedText(filename, Qt::ElideMiddle, std::min(m_contentWidth, MAX_LENGTH));
        if (m_contentWidth > MAX_LENGTH)
        {
            strWidth = fm.boundingRect(name).width();
            leftMargin = std::max(0, (m_windowWidth - strWidth)/2 - m_leftContentWidth) - 6;
        } else
            leftMargin = 0;
    } else {
        leftMargin = std::max(0, (m_windowWidth - strWidth)/2 - m_leftContentWidth);
        name = filename;
    }

    m_fileNameLabel->setText(name, leftMargin);
    m_layout->update();
    updateGeometry();
}

void TTMContent::onThemeChanged(ViewerThemeManager::AppTheme theme) {

}

void TTMContent::updateLayout(int ttlWidth, const QString &path)
{
    m_leftContentWidth = ttlWidth;
    m_windowWidth = ConfigSetter::instance()->value("MAINWINDOW",
                                                     "WindowWidth").toInt();
    m_contentWidth = std::max(m_windowWidth - 140 - m_leftContentWidth, 1);
    qDebug() << "TTMContent:" << ttlWidth << m_contentWidth << m_windowWidth;
    setFixedWidth(m_contentWidth);
    setPath(path);
}

