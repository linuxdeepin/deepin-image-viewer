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
#include "application.h"
#include "controller/configsetter.h"
#include "navigationwidget.h"
#include "widgets/imagebutton.h"
#include "utils/baseutils.h"

#include <QPainter>
#include <dwindowclosebutton.h>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QtDebug>

#include <dthememanager.h>

namespace {

const QString SETTINGS_GROUP = "VIEWPANEL";
const QString SETTINGS_ALWAYSHIDDEN_KEY = "NavigationAlwaysHidden";
const int IMAGE_MARGIN = 5;
const int IMAGE_MARGIN_BOTTOM = 5;

}  // namespace

using namespace Dtk::Widget;

NavigationWidget::NavigationWidget(QWidget *parent)
    : QWidget(parent)
{
    hide();
    resize(150, 112);
    onThemeChanged(dApp->viewerTheme->getCurrentTheme());
    ImageButton *closeBtn = new ImageButton(":/resources/common/close_normal.png",
                                            ":/resources/common/close_hover.png",
                                            ":/resources/common/close_press.png",
                                            ":/resources/common/close_normal.png", this);
    closeBtn->setTooltipVisible(true);
    closeBtn->setFixedSize(27, 23);
    closeBtn->move(QPoint(this->x() + this->width() - 27 - 6,
                   rect().topRight().y() + 4));
    closeBtn->show();
    connect(closeBtn, &ImageButton::clicked, [this](){
        setAlwaysHidden(true);
    });

    m_mainRect = QRect(rect().x() + IMAGE_MARGIN,
                       rect().y() + IMAGE_MARGIN_BOTTOM,
                       rect().width() - IMAGE_MARGIN*2,
                       rect().height() - IMAGE_MARGIN_BOTTOM*2);

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            &NavigationWidget::onThemeChanged);
}

void NavigationWidget::setAlwaysHidden(bool value)
{
    dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_ALWAYSHIDDEN_KEY,
                           QVariant(value));
    if (isAlwaysHidden())
        hide();
    else
        show();
}

bool NavigationWidget::isAlwaysHidden() const
{
    return dApp->setter->value(SETTINGS_GROUP, SETTINGS_ALWAYSHIDDEN_KEY,
                              QVariant(false)).toBool();
}

void NavigationWidget::setImage(const QImage &img)
{
    const qreal ratio = devicePixelRatioF();

    QRect tmpImageRect = QRect(m_mainRect.x(), m_mainRect.y(),
                               qRound(m_mainRect.width() * ratio),
                               qRound(m_mainRect.height() * ratio));

    m_originRect = img.rect();

    // 只在图片比可显示区域大时才缩放
    if (tmpImageRect.width() < m_originRect.width() || tmpImageRect.height() < m_originRect.height()) {
        m_img = img.scaled(tmpImageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        m_img = img;
    }

    m_pix = QPixmap::fromImage(m_img);
    m_pix.setDevicePixelRatio(ratio);
    m_imageScale = qMax(1.0, qMax(qreal(img.width()) / qreal(m_img.width()), qreal(img.height()) / qreal(m_img.height())));
    m_r = QRectF(0, 0, m_img.width() / ratio, m_img.height() / ratio);

    update();
}

void NavigationWidget::setRectInImage(const QRect &r)
{
    if (m_img.isNull())
        return;
    m_r.setX((qreal)r.x() / m_imageScale);
    m_r.setY((qreal)r.y() / m_imageScale);
    m_r.setWidth((qreal)r.width() / m_imageScale);
    m_r.setHeight((qreal)r.height() / m_imageScale);

    update();
}

void NavigationWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        tryMoveRect(e->pos());
}

void NavigationWidget::mouseMoveEvent(QMouseEvent *e)
{
    tryMoveRect(e->pos());
}

void NavigationWidget::tryMoveRect(const QPoint &p)
{
    const int x0 = (m_mainRect.width()-m_img.width())/2;
    const int y0 = (m_mainRect.height()-m_img.height())/2;
    const QRect imageRect(x0, y0, m_img.width(), m_img.height());
    if (! imageRect.contains(p))
        return;
    const qreal x = 1.0 * (p.x() - x0) / m_img.width() * m_originRect.width();
    const qreal y = 1.0 * (p.y() - y0) / m_img.height() * m_originRect.height();

    Q_EMIT requestMove(x, y);
}

void NavigationWidget::paintEvent(QPaintEvent *)
{
    QImage img(m_img);
    if (m_img.isNull()) {
        QPainter p(this);
        p.fillRect(m_r, m_BgColor);
        return;
    }

    const qreal ratio = devicePixelRatioF();

    QPainter p(&img);
    p.fillRect(m_r, m_mrBgColor);
    p.setPen(m_mrBorderColor);
    p.drawRect(m_r);
    p.end();
    p.begin(this);

    QImage background(m_bgImgUrl);

    p.drawImage(this->rect(), background);
    QRect imageDrawRect =  QRect((m_mainRect.width() - m_img.width() / ratio)/2 + IMAGE_MARGIN,
                (m_mainRect.height() - m_img.height() / ratio)/2 + utils::common::BORDER_WIDTH,
                                 m_img.width() / ratio, m_img.height() / ratio);
    //**draw transparent background
//    QPixmap pm(12, 12);
//    QPainter pmp(&pm);
//    //TODO: the transparent box
//    //should not be scaled with the image
//    pmp.fillRect(0, 0, 6, 6, LIGHT_CHECKER_COLOR);
//    pmp.fillRect(6, 6, 6, 6, LIGHT_CHECKER_COLOR);
//    pmp.fillRect(0, 6, 6, 6, DARK_CHECKER_COLOR);
//    pmp.fillRect(6, 0, 6, 6, DARK_CHECKER_COLOR);
//    pmp.end();

//    p.fillRect(imageDrawRect, QBrush(pm));
    p.drawImage(imageDrawRect, img);
    QRect borderRect = QRect(imageDrawRect.x(), imageDrawRect.y(), imageDrawRect.width()
          - utils::common::BORDER_WIDTH, imageDrawRect.height() - utils::common::BORDER_WIDTH);
    p.setPen(m_imgRBorderColor);
    p.drawRect(borderRect);
    p.end();
}

void NavigationWidget::onThemeChanged(ViewerThemeManager::AppTheme theme) {
    if (theme == ViewerThemeManager::Dark) {
        m_bgImgUrl = utils::view::naviwindow::DARK_BG_IMG ;
        m_BgColor = utils::view::naviwindow::DARK_BG_COLOR;
        m_mrBgColor = utils::view::naviwindow::DARK_MR_BG_COLOR;
        m_mrBorderColor = utils::view::naviwindow::DARK_MR_BORDER_Color;
        m_imgRBorderColor =utils::view::naviwindow:: DARK_IMG_R_BORDER_COLOR;
//        Dtk::Widget::DThemeManager::instance()->setTheme("dark");
    } else {
        m_bgImgUrl = utils::view::naviwindow::LIGHT_BG_IMG ;
        m_BgColor = utils::view::naviwindow::LIGHT_BG_COLOR;
        m_mrBgColor = utils::view::naviwindow::LIGHT_MR_BG_COLOR;
        m_mrBorderColor = utils::view::naviwindow::LIGHT_MR_BORDER_Color;
        m_imgRBorderColor = utils::view::naviwindow::LIGHT_IMG_R_BORDER_COLOR;
//        Dtk::Widget::DThemeManager::instance()->setTheme("light");
    }
    update();
}
