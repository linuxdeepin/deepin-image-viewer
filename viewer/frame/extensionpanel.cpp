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
#include "extensionpanel.h"
#include "application.h"
#include "controller/signalmanager.h"
#include "darrowbutton.h"
#include <QPainter>

using namespace Dtk::Widget;

namespace {

//const int CONTROL_BUTTON_WIDTH = 20;
//const int CONTROL_BUTTON_HEIGHT = 60;
//const int CONTROL_BUTTON_CUBIC_LENGTH = 30;
const int EXTENSION_PANEL_WIDTH = 300;
const int EXTENSION_PANEL_MAX_WIDTH = 340;

const QColor DARK_COVERBRUSH = QColor(0, 0, 0, 100);
const QColor LIGHT_COVERBRUSH = QColor(255, 255, 255, 179);
const int ANIMATION_DURATION = 500;
const QEasingCurve ANIMATION_EASING_CURVE = QEasingCurve::InOutCubic;
}  // namespace

ExtensionPanel::ExtensionPanel(QWidget *parent)
    : DBlurEffectWidget(parent)
{
//    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
//    setBorderColor(QColor(255, 255, 255, 51));
//    setMaximumWidth(EXTENSION_PANEL_MAX_WIDTH);
    setFixedWidth(EXTENSION_PANEL_WIDTH);
    setFixedHeight(540);
    setBlurRectYRadius(18);
    setBlurRectXRadius(18);
    setMaskAlpha(204);

//    setBorderRadius(18);
    m_contentLayout = new QHBoxLayout(this);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);

//    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
//            &ExtensionPanel::onThemeChanged);
//    DArrowButton *hideButton = new DArrowButton();
//    hideButton->setFixedSize(CONTROL_BUTTON_WIDTH, CONTROL_BUTTON_WIDTH);
//    hideButton->setArrowDirection(DArrowButton::ArrowLeft);
//    connect(hideButton, &DArrowButton::mouseRelease, [=] {
//        emit dApp->signalM->hideExtensionPanel();
//    });

//    QHBoxLayout *mainLayout = new QHBoxLayout(this);
//    mainLayout->setContentsMargins(0, 0, 0, 0);
//    mainLayout->setSpacing(0);

//    mainLayout->addLayout(m_contentLayout);
//    mainLayout->addWidget(hideButton);
//    mainLayout->addSpacing(5);
}

void ExtensionPanel::onThemeChanged(ViewerThemeManager::AppTheme theme) {
//    if (theme == ViewerThemeManager::Dark) {
//        m_coverBrush = DARK_COVERBRUSH;
//    } else {
//        m_coverBrush = LIGHT_COVERBRUSH;
//    }
//    setCoverBrush(m_coverBrush);
}

void ExtensionPanel::setContent(QWidget *content)
{
    if (content) {
        QLayoutItem *child;
        if ((child = m_contentLayout->takeAt(0)) != 0) {
            if (child->widget())
                child->widget()->deleteLater();
            delete child;
        }

        m_content = content;
        updateRectWithContent();
        m_contentLayout->addWidget(content);
    }
}

void ExtensionPanel::updateRectWithContent()
{
    if (m_content) {
        resize(qMax(m_content->sizeHint().width(), EXTENSION_PANEL_WIDTH),
               height());
    }
}

void ExtensionPanel::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

void ExtensionPanel::paintEvent(QPaintEvent *pe)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF bgRect;
    bgRect.setSize(size());
    const QPalette pal = QGuiApplication::palette();//this->palette();
    QColor bgColor = pal.color(QPalette::ToolTipBase);

    QPainterPath pp;
    pp.addRoundedRect(bgRect, 18, 18);
    painter.fillPath(pp, QColor(0,0,0,22));

    {
        auto view_rect = bgRect.marginsRemoved(QMargins(1, 1, 1, 1));
        QPainterPath pp;
        pp.addRoundedRect(view_rect, 18, 18);
        painter.fillPath(pp, bgColor);
    }
    QWidget::paintEvent(pe);
}
//{
//    QPainter painter(this);
//    QPainterPath path;
//    path.moveTo(0, 0);//top left
//    path.lineTo(width() - CONTROL_BUTTON_WIDTH, 0);//top right
//    int cubicStep = 5;
//    //cubic 1
//    QPoint cubic1StartPoint(width() - CONTROL_BUTTON_WIDTH,
//                            (height() - CONTROL_BUTTON_HEIGHT) / 2 - cubicStep);
//    QPoint cubic1EndPoint(width(),
//                          cubic1StartPoint.y() + CONTROL_BUTTON_CUBIC_LENGTH);
//    path.lineTo(cubic1StartPoint); //start point of cubicTo
//    path.cubicTo(QPoint(cubic1StartPoint.x(), cubic1EndPoint.y() - cubicStep),
//        QPoint(width(), cubic1StartPoint.y() + cubicStep), cubic1EndPoint);
//    //cubic 2
//    QPoint cubic2StartPoint(width(), cubic1EndPoint.y() + (CONTROL_BUTTON_HEIGHT
//        - CONTROL_BUTTON_CUBIC_LENGTH) / 2);
//    QPoint cubic2EndPoint(width() - CONTROL_BUTTON_WIDTH,
//                          cubic2StartPoint.y() + CONTROL_BUTTON_CUBIC_LENGTH);
//    path.lineTo(cubic2StartPoint);
//    path.cubicTo(QPoint(cubic2StartPoint.x(), cubic2EndPoint.y() - cubicStep),
//                 QPoint(cubic2EndPoint.x(), cubic2StartPoint.y() + cubicStep),
//                 cubic2EndPoint);
//    path.lineTo(width() - CONTROL_BUTTON_WIDTH, height()); // Right bottom
//    path.lineTo(0, height()); // Left bottom
//    path.lineTo(0, 0); // Back to the start point

//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setRenderHint(QPainter::SmoothPixmapTransform);
//    painter.setClipPath(path);

//    painter.drawPixmap(0, 0, width(), height(), getResultPixmap());
//    painter.fillRect(0, 0, width(), height(), QBrush(QColor(0, 0, 0, 153)));

//    QPen pen;
//    pen.setColor(QColor(255, 255, 255, 51));
//    pen.setWidth(1);
//    painter.setPen(pen);
//    painter.drawPath(path);
//    painter.end();
//}
void ExtensionPanel::moveWithAnimation(int x, int y)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    animation->setDuration(ANIMATION_DURATION);
    animation->setEasingCurve(ANIMATION_EASING_CURVE);
    animation->setStartValue(pos());
    animation->setEndValue(QPoint(x, y));
    animation->start();
    connect(this, &ExtensionPanel::requestStopAnimation,
            animation, &QPropertyAnimation::stop);
    connect(this, &ExtensionPanel::requestStopAnimation,
            animation, &QPropertyAnimation::deleteLater);
    connect(animation, &QPropertyAnimation::finished,
            animation, &QPropertyAnimation::deleteLater);
}
