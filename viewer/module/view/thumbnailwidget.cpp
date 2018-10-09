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
#include <QVBoxLayout>
#include <QDebug>
#include <QPainter>

#include <DSuggestButton>

#include "thumbnailwidget.h"
#include "application.h"
#include "utils/baseutils.h"

namespace  {
const QSize THUMBNAIL_BORDERSIZE = QSize(168, 168);
const QSize THUMBNAIL_SIZE = QSize(166, 166);
}

ThumbnailWidget::ThumbnailWidget(const QString &darkFile,
const QString &lightFile, QWidget *parent): ThemeWidget(darkFile, lightFile, parent)
{
    setMouseTracking(true);
    m_thumbnailLabel = new QLabel(this);
    m_thumbnailLabel->setObjectName("ThumbnailLabel");
    m_thumbnailLabel->setFixedSize(THUMBNAIL_BORDERSIZE);
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());

#ifndef LITE_DIV
    m_tips = new QLabel(this);
    m_tips->setObjectName("ThumbnailTips");
    m_tips->setText(tr("No image files found"));
#else
    DSuggestButton *button = new DSuggestButton(tr("Open Image File"), this);
    button->setShortcut(QKeySequence("Ctrl+O"));
    connect(button, &DSuggestButton::clicked, this, &ThumbnailWidget::openImageInDialog);
#endif
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(m_thumbnailLabel,  0, Qt::AlignCenter);
    layout->addSpacing(9);
#ifndef LITE_DIV
    layout->addWidget(m_tips,  0, Qt::AlignCenter);
#else
    layout->addWidget(button,  0, Qt::AlignCenter);
#endif
    layout->addStretch();
    setLayout(layout);

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &ThumbnailWidget::onThemeChanged);
}

void ThumbnailWidget::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        m_inBorderColor = utils::common::DARK_BORDER_COLOR;
        if(m_isDefaultThumbnail)
            m_defaultImage = QPixmap(utils::view::DARK_DEFAULT_THUMBNAIL);
    } else {
        m_inBorderColor = utils::common::LIGHT_BORDER_COLOR;
        if(m_isDefaultThumbnail)
            m_defaultImage = QPixmap(utils::view::LIGHT_DEFAULT_THUMBNAIL);
    }

    ThemeWidget::onThemeChanged(theme);
    update();
}

void ThumbnailWidget::setThumbnailImage(const QPixmap thumbnail) {
    if (thumbnail.isNull()) {
        if (isDeepMode()) {
            m_defaultImage = QPixmap(utils::view::DARK_DEFAULT_THUMBNAIL);
        } else {
            m_defaultImage = QPixmap(utils::view::LIGHT_DEFAULT_THUMBNAIL);
        }

        m_isDefaultThumbnail = true;
    } else {
        m_defaultImage = thumbnail;
        m_isDefaultThumbnail =  false;
    }

    update();
}

bool ThumbnailWidget::isDefaultThumbnail() {
    return m_isDefaultThumbnail;
}

void ThumbnailWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    if (m_defaultImage.isNull()) {
        if (isDeepMode()) {
            m_defaultImage = QPixmap(utils::view::DARK_DEFAULT_THUMBNAIL);
        } else {
            m_defaultImage = QPixmap(utils::view::LIGHT_DEFAULT_THUMBNAIL);
        }
        m_isDefaultThumbnail = true;
    }

    if (m_defaultImage.size() != THUMBNAIL_SIZE) {
        m_defaultImage = m_defaultImage.scaled(THUMBNAIL_SIZE,
                         Qt::KeepAspectRatio, Qt::SmoothTransformation);

    }
    QPoint startPoint = mapToParent(QPoint(m_thumbnailLabel->x(),
                                           m_thumbnailLabel->y()));
    QPoint imgStartPoint = QPoint(startPoint.x() + (THUMBNAIL_SIZE.width() -
           m_defaultImage.width())/2 + 1, startPoint.y() + (THUMBNAIL_SIZE.height()
           - m_defaultImage.height())/2 + 1);
    QRect imgRect = QRect(imgStartPoint.x(), imgStartPoint.y(),
                          m_defaultImage.width(), m_defaultImage.height());

    QPainter painter(this);
    painter.setRenderHints(QPainter::HighQualityAntialiasing |
                           QPainter::SmoothPixmapTransform);
    painter.drawPixmap(imgRect, m_defaultImage);
}

void ThumbnailWidget::mouseMoveEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    emit mouseHoverMoved();
}
ThumbnailWidget::~ThumbnailWidget(){
}
