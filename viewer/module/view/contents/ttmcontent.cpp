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
#include "ttmcontent.h"
#include "application.h"
#include "utils/baseutils.h"
#include "controller/configsetter.h"
#include "widgets/pushbutton.h"

#include <QBoxLayout>
#include <QDebug>
#include <QFileInfo>

TTMContent::TTMContent(QWidget *parent)
    : QFrame(parent)
    , m_leftSpacing(0)
{
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    m_windowWidth = ConfigSetter::instance()->value("MAINWINDOW",
                                                     "WindowWidth").toInt();
    m_contentWidth = m_windowWidth - 121 - 441;
    setFixedWidth(m_contentWidth);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addSpacing(m_leftSpacing);

    m_emptyLabel = new QLabel(this);
    m_fileNameLabel = new QLabel(this);
    m_fileNameLabel->setMargin(0);
    m_fileNameLabel->setObjectName("ImagenameLabel");
    m_layout->addWidget(m_emptyLabel);
    m_layout->addWidget(m_fileNameLabel);
    m_layout->addStretch(1);
    setLayout(m_layout);

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &TTMContent::onThemeChanged);
}

void TTMContent::setPath(const QString &path)
{
    QString filename = QFileInfo(path).fileName();
    QString name;
    using namespace utils::base;
    QFontMetrics fm(m_fileNameLabel->font());
    int strWidth = fm.boundingRect(filename).width();
    if (strWidth > m_contentWidth)
    {
        m_leftSpacing = 0;
    } else {
        m_leftSpacing = std::max(0, (m_windowWidth - strWidth)/2 - 406);
    }
    name = fm.elidedText(filename, Qt::ElideMiddle, m_contentWidth - 10);
    m_fileNameLabel->setText(name);
    m_emptyLabel->setMinimumWidth(m_leftSpacing);
    m_fileNameLabel->setFixedWidth(strWidth);
    m_layout->update();
    updateGeometry();

    qDebug() << "setPath:" << path;
}

void TTMContent::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/dark/qss/ttl.qss"));
    } else {
        this->setStyleSheet(utils::base::getFileContent(
                                ":/resources/light/qss/ttl.qss"));
    }
}


