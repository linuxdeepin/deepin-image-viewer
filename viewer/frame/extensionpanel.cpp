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
#include <QPainter>
#include "application.h"
#include "controller/signalmanager.h"
#include "darrowbutton.h"
#include <DFontSizeManager>

using namespace Dtk::Widget;

namespace {

// const int CONTROL_BUTTON_WIDTH = 20;
// const int CONTROL_BUTTON_HEIGHT = 60;
// const int CONTROL_BUTTON_CUBIC_LENGTH = 30;
const int EXTENSION_PANEL_WIDTH = 300 + 10;
// const int EXTENSION_PANEL_MAX_WIDTH = 340;

const QColor DARK_COVERBRUSH = QColor(0, 0, 0, 100);
const QColor LIGHT_COVERBRUSH = QColor(255, 255, 255, 179);
const int ANIMATION_DURATION = 500;
const QEasingCurve ANIMATION_EASING_CURVE = QEasingCurve::InOutCubic;
}  // namespace

ExtensionPanel::ExtensionPanel(QWidget *parent)
    //    : DFloatingWidget(parent)
    : DDialog(parent)
{
    //    onThemeChanged(dApp->viewerTheme->getCurrentTheme());

    this->setWindowTitle(tr("Image info"));
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T6, QFont::Medium);

    m_contentLayout = new QVBoxLayout(this);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);

    setFixedWidth(EXTENSION_PANEL_WIDTH);
    //    setFixedHeight(540);
}

void ExtensionPanel::setContent(QWidget *content)
{
    if (content) {
#if 1
        QLayoutItem *child;
        if ((child = m_contentLayout->takeAt(0)) != 0) {
            if (child->widget())
                child->widget()->deleteLater();
            delete child;
        }
#else
        QLayoutItem *child;
        if ((child = m_contentLayout->takeAt(0)) != nullptr) {
            if (child->widget())
                child->widget()->setParent(nullptr);
        }
        delete child;
#endif
        m_content = content;
        updateRectWithContent();
        //        m_contentLayout->addWidget(content);
        this->addContent(content);
    }
}

void ExtensionPanel::updateRectWithContent()
{
    connect(dApp->signalM, &SignalManager::extensionPanelHeight, this,
            [=](int height) {
                    setFixedHeight(height+5);//tmp for imageinfo
            });

    if (m_content) {
        //        resize(qMax(m_content->sizeHint().width(), EXTENSION_PANEL_WIDTH), height());
    }
}

void ExtensionPanel::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    DDialog::mouseMoveEvent(e);
}

void ExtensionPanel::paintEvent(QPaintEvent *pe)
{
    //    QPainter painter(this);
    //    painter.setRenderHint(QPainter::Antialiasing);
    //    QRectF bgRect;
    //    bgRect.setSize(size());
    //    const QPalette pal = QGuiApplication::palette();//this->palette();
    //    QColor bgColor = pal.color(QPalette::ToolTipBase);

    //    QPainterPath pp;
    //    pp.addRoundedRect(bgRect, 18, 18);
    //    painter.fillPath(pp, QColor(0,0,0,22));

    //    {
    //        auto view_rect = bgRect.marginsRemoved(QMargins(1, 1, 1, 1));
    //        QPainterPath pp;
    //        pp.addRoundedRect(view_rect, 18, 18);
    //        painter.fillPath(pp, bgColor);
    //    }
    //    QWidget::paintEvent(pe);
    DDialog::paintEvent(pe);
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
    //    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    //    animation->setDuration(ANIMATION_DURATION);
    //    animation->setEasingCurve(ANIMATION_EASING_CURVE);
    //    animation->setStartValue(pos());
    //    animation->setEndValue(QPoint(x, y));
    //    animation->start();
    //    connect(this, &ExtensionPanel::requestStopAnimation, animation,
    //    &QPropertyAnimation::stop); connect(this, &ExtensionPanel::requestStopAnimation,
    //    animation,
    //            &QPropertyAnimation::deleteLater);
    //    connect(animation, &QPropertyAnimation::finished, animation,
    //    &QPropertyAnimation::deleteLater);
}
