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
#include <QGestureEvent>

#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include "application.h"
#include "accessibility/ac-desktop-define.h"

const QString ICON_PIXMAP_DARK = ":/assets/dark/images/picture damaged_dark.svg";
const QString ICON_PIXMAP_LIGHT = ":/assets/light/images/picture damaged_light.svg";
const QSize THUMBNAIL_SIZE = QSize(151, 151);
LockWidget::LockWidget(const QString &darkFile,
    const QString &lightFile, QWidget *parent)
    : ThemeWidget(darkFile, lightFile, parent) {
    m_picString = "";
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);
    grabGesture(Qt::PanGesture);
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
#ifdef OPENACCESSIBLE
    setObjectName(Lock_Widget);
    setAccessibleName(Lock_Widget);
    m_bgLabel->setAccessibleName("BgLabel");
#endif
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
    layout->addWidget(m_bgLabel, 0, Qt::AlignHCenter );
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

void LockWidget::handleGestureEvent(QGestureEvent *gesture)
{
    /*    if (QGesture *swipe = gesture->gesture(Qt::SwipeGesture))
            swipeTriggered(static_cast<QSwipeGesture *>(swipe));
        else */if (QGesture *pinch = gesture->gesture(Qt::PinchGesture))
            pinchTriggered(static_cast<QPinchGesture *>(pinch));
}

void LockWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
    if(e->source() == Qt::MouseEventSynthesizedByQt && m_maxTouchPoints == 1)
    {
        int offset = e->pos().x()-m_startx;
        if (qAbs(offset) > 200) {
            if (offset > 0) {
                emit previousRequested();
                qDebug() << "zy------ThumbnailWidget::event previousRequested";
            } else {
                emit nextRequested();
                qDebug() << "zy------ThumbnailWidget::event nextRequested";
            }
        }
    }
    m_startx = 0;
}

void LockWidget::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);
    m_startx = e->pos().x();
}

void LockWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

bool LockWidget::event(QEvent *event)
{
    QEvent::Type evType = event->type();
    if (evType == QEvent::TouchBegin || evType == QEvent::TouchUpdate ||
            evType == QEvent::TouchEnd) {
        if(evType == QEvent::TouchBegin)
        {
            qDebug() << "QEvent::TouchBegin";
            m_maxTouchPoints = 1;
        }
    } else if (event->type() == QEvent::Gesture)
        handleGestureEvent(static_cast<QGestureEvent *>(event));
    return QWidget::event(event);
}

void LockWidget::pinchTriggered(QPinchGesture *gesture)
{
    Q_UNUSED(gesture);
    m_maxTouchPoints = 2;
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
