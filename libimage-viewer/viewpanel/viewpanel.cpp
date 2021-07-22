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
#include "viewpanel.h"

#include <QVBoxLayout>

#include "contents/bottomtoolbar.h"
#include "navigationwidget.h"

const int BOTTOM_TOOLBAR_HEIGHT = 80;   //底部工具看高
const int BOTTOM_SPACING = 10;          //底部工具栏与底部边缘距离

ViewPanel::ViewPanel(QWidget *parent)
    : QFrame(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
    m_stack = new DStackedWidget(this);
    layout->addWidget(m_stack, 0, Qt::AlignCenter);

    m_view = new ImageView(this);
    m_stack->addWidget(m_view);

    m_bottomToolbar = new BottomToolbar(this);

    initConnect();
    initFloatingComponent();
}

ViewPanel::~ViewPanel()
{

}

void ViewPanel::loadImage(const QString &path)
{
    if (m_view) {
        m_view->setImage(path);
        m_view->resetTransform();
        m_stack->setCurrentWidget(m_view);
        resetBottomToolbarGeometry(true);
    }
}

void ViewPanel::initConnect()
{
    connect(m_view, &ImageView::imageChanged, this, [ = ](QString path) {
        emit imageChanged(path);
        // Pixmap is cache in thread, make sure the size would correct after
        // cache is finish
        m_view->autoFit();
    });
}

void ViewPanel::initFloatingComponent()
{
    initScaleLabel();
    initNavigation();
}

void ViewPanel::initScaleLabel()
{
    using namespace utils::base;
    DAnchors<DFloatingWidget> scalePerc = new DFloatingWidget(this);
    scalePerc->setBlurBackgroundEnabled(true);

    QHBoxLayout *layout = new QHBoxLayout();
    scalePerc->setLayout(layout);
    DLabel *label = new DLabel();
    layout->addWidget(label);
    scalePerc->setAttribute(Qt::WA_TransparentForMouseEvents);
    scalePerc.setAnchor(Qt::AnchorHorizontalCenter, this, Qt::AnchorHorizontalCenter);
    scalePerc.setAnchor(Qt::AnchorBottom, this, Qt::AnchorBottom);
    scalePerc.setBottomMargin(75 + 14);
    label->setAlignment(Qt::AlignCenter);
//    scalePerc->setFixedSize(82, 48);
    scalePerc->setFixedWidth(90 + 10);
    scalePerc->setFixedHeight(40 + 10);
    scalePerc->adjustSize();
    label->setText("100%");
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T6);
    scalePerc->hide();

    QTimer *hideT = new QTimer(this);
    hideT->setSingleShot(true);
    connect(hideT, &QTimer::timeout, scalePerc, &DLabel::hide);

    connect(m_view, &ImageView::scaled, this, [ = ](qreal perc) {
        label->setText(QString("%1%").arg(int(perc)));
        if (perc > 100) {

        } else if (perc == 100.0) {

        } else {

        }
    });
    connect(m_view, &ImageView::showScaleLabel, this, [ = ]() {
        scalePerc->show();
        hideT->start(1000);
    });
}

void ViewPanel::initNavigation()
{
    m_nav = new NavigationWidget(this);
    m_nav.setBottomMargin(100);
    m_nav.setLeftMargin(10);
    m_nav.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);
    m_nav.setAnchor(Qt::AnchorBottom, this, Qt::AnchorBottom);

    connect(this, &ViewPanel::imageChanged, this, [ = ](const QString & path) {
        if (path.isEmpty()) m_nav->setVisible(false);
        m_nav->setImage(m_view->image(true));
    });

    connect(m_nav, &NavigationWidget::requestMove, [this](int x, int y) {
        m_view->centerOn(x, y);
    });
    connect(m_view, &ImageView::transformChanged, [this]() {
        //如果stackindex不为2，全屏会出现导航窗口
        //如果是正在移动的情况，将不会出现导航栏窗口
        if (m_stack->currentIndex() != 2) {
            m_nav->setVisible((! m_nav->isAlwaysHidden() && ! m_view->isWholeImageVisible()));
            m_nav->setRectInImage(m_view->visibleImageRect());
        }
    });
    m_nav->show();
}

void ViewPanel::resetBottomToolbarGeometry(bool visible)
{
    m_bottomToolbar->setVisible(visible);
    if (visible) {
        int width = m_bottomToolbar->width();
        int x = (this->width() - m_bottomToolbar->width()) / 2;
        int y = this->height() - BOTTOM_TOOLBAR_HEIGHT - BOTTOM_SPACING;
        m_bottomToolbar->setGeometry(x, y, width, BOTTOM_TOOLBAR_HEIGHT);
    }
}

void ViewPanel::resizeEvent(QResizeEvent *e)
{
    resetBottomToolbarGeometry(m_stack->currentWidget() == m_view);
    QFrame::resizeEvent(e);
}
