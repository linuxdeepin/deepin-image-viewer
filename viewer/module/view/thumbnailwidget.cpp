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
#include <QDebug>
#include <QPainter>
#include <QVBoxLayout>

#include <DSuggestButton>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DLabel>
#include <QImageReader>
#include "application.h"
#include "controller/signalmanager.h"
#include "thumbnailwidget.h"
#include "utils/baseutils.h"

namespace {
const QSize THUMBNAIL_BORDERSIZE = QSize(130, 130);
const QSize THUMBNAIL_SIZE = QSize(128, 128);
const QString ICON_IMPORT_PHOTO_DARK = ":/resources/dark/images/icon_import_photo dark.svg";
const QString ICON_IMPORT_PHOTO_LIGHT = ":/resources/light/images/icon_import_photo.svg";
}  // namespace

ThumbnailWidget::ThumbnailWidget(const QString &darkFile, const QString &lightFile, QWidget *parent)
    : ThemeWidget(darkFile, lightFile, parent)
{
    m_picString = "";
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::DarkType) {
        m_picString = ICON_IMPORT_PHOTO_DARK;
        m_theme = true;
    } else {
        m_picString = ICON_IMPORT_PHOTO_LIGHT;
        m_theme = false;
    }

    QPixmap logo_pix = utils::base::renderSVG(m_picString, THUMBNAIL_SIZE);
    m_logo = logo_pix;

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, [=]() {
                         DGuiApplicationHelper::ColorType themeType =
                             DGuiApplicationHelper::instance()->themeType();
                         m_picString = "";
                         if (themeType == DGuiApplicationHelper::DarkType) {
                             m_picString = ICON_IMPORT_PHOTO_DARK;
                             m_theme = true;
                         } else {
                             m_picString = ICON_IMPORT_PHOTO_LIGHT;
                             m_theme = false;
                         }

                         QPixmap logo_pix = utils::base::renderSVG(m_picString, THUMBNAIL_SIZE);
                         m_logo = logo_pix;
                         if (m_isDefaultThumbnail)
                             m_defaultImage = logo_pix;
                         update();
                     });

    setMouseTracking(true);
    m_thumbnailLabel = new QLabel(this);
    //    m_thumbnailLabel->setObjectName("ThumbnailLabel");
    m_thumbnailLabel->setFixedSize(THUMBNAIL_BORDERSIZE);
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());

#ifndef LITE_DIV
    m_tips = new QLabel(this);
    m_tips->setObjectName("ThumbnailTips");
    m_tips->setText(tr("No image files found"));
#else

    DLabel *tips = new DLabel(this);
    tips->setText(tr("Image file not found"));
    DFontSizeManager::instance()->bind(tips, DFontSizeManager::T6);
    tips->setForegroundRole(DPalette::TextTips);
    tips->hide();

    DSuggestButton *button = new DSuggestButton(tr("Open Image"), this);
    button->setFixedWidth(302);
    button->setFixedHeight(36);
    button->setShortcut(QKeySequence("Ctrl+O"));
    connect(button, &DSuggestButton::clicked, this, &ThumbnailWidget::openImageInDialog);

    connect(dApp->signalM, &SignalManager::usbOutIn, this, [=](bool visible) {
        if (button->isVisible())
            return;
        if (visible) {
            if (m_usb) {
                button->hide();
                tips->hide();
                m_usb = false;
            } else {
                button->show();
                tips->hide();
            }
        } else {
            button->hide();
            tips->show();
            m_usb = true;
        }
    });
    connect(dApp->signalM, &SignalManager::picNotExists, this, [=](bool visible) {
        if (visible) {
            button->hide();
            tips->show();
        } else {
            button->show();
            tips->hide();
        }
    });
#endif
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(m_thumbnailLabel, 0, Qt::AlignCenter);
    layout->addSpacing(9);
#ifndef LITE_DIV
    layout->addWidget(m_tips, 0, Qt::AlignCenter);
#else
    layout->addWidget(tips, 0, Qt::AlignCenter);
    layout->addWidget(button, 0, Qt::AlignCenter);
#endif
    layout->addStretch();
    setLayout(layout);

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &ThumbnailWidget::onThemeChanged);
}

void ThumbnailWidget::onThemeChanged(ViewerThemeManager::AppTheme theme)
{
    if (theme == ViewerThemeManager::Dark) {
        m_inBorderColor = utils::common::DARK_BORDER_COLOR;
        if (m_isDefaultThumbnail)
            m_defaultImage = m_logo;
    } else {
        m_inBorderColor = utils::common::LIGHT_BORDER_COLOR;
        if (m_isDefaultThumbnail)
            m_defaultImage = m_logo;
    }

    ThemeWidget::onThemeChanged(theme);
    update();
}

void ThumbnailWidget::setThumbnailImage(const QPixmap thumbnail)
{
    if (thumbnail.isNull()) {
        if (m_theme) {
            m_defaultImage = m_logo;
        } else {
            m_defaultImage = m_logo;
        }
        m_isDefaultThumbnail = true;
    } else {
        m_defaultImage = thumbnail;
        m_isDefaultThumbnail = false;
    }

    update();
}

bool ThumbnailWidget::isDefaultThumbnail()
{
    return m_isDefaultThumbnail;
}

void ThumbnailWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (m_defaultImage.isNull()) {
        if (m_theme) {
            m_defaultImage = m_logo;
        } else {
            m_defaultImage = m_logo;
        }
        m_isDefaultThumbnail = true;
    }

    if (m_defaultImage.size() != THUMBNAIL_SIZE) {
        m_defaultImage =
            m_defaultImage.scaled(THUMBNAIL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QPoint startPoint = mapToParent(QPoint(m_thumbnailLabel->x(), m_thumbnailLabel->y()));
    QPoint imgStartPoint = QPoint(startPoint.x() + (THUMBNAIL_SIZE.width() - 128) / 2 + 1,
                                  startPoint.y() + (THUMBNAIL_SIZE.height() - 128) / 2 + 1);
    QRect imgRect = QRect(imgStartPoint.x(), imgStartPoint.y(), 128, 128);

    QPainter painter(this);
    painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
    //    painter.drawPixmap(imgRect, m_defaultImage);
    QIcon m_icon(m_defaultImage);
    //    QIcon m_icon(m_picString);
    m_icon.paint(&painter, imgRect);
}

void ThumbnailWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);

    emit mouseHoverMoved();
}

ThumbnailWidget::~ThumbnailWidget() {}
