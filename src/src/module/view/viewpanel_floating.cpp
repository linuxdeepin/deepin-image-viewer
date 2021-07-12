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
#include "navigationwidget.h"
#include "contents/imageinfowidget.h"
#include "scen/imageview.h"
#include "dimagebutton.h"
#include "widgets/pushbutton.h"

#include "utils/baseutils.h"

#include <QTimer>
#include <DBlurEffectWidget>
#include <DFontSizeManager>
#include <DFloatingWidget>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

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
    QLbtoDLabel *label = new QLbtoDLabel();
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
    connect(hideT, &QTimer::timeout, scalePerc, &QLbtoDLabel::hide);

    connect(m_viewB, &ImageView::scaled, this, [ = ](qreal perc) {
        label->setText(QString("%1%").arg(int(perc)));
        if (perc > 100) {
            emit dApp->signalM->enterScaledMode(true);
            emit dApp->signalM->isAdapt(false);
        } else if (perc == 100.0) {
            emit dApp->signalM->enterScaledMode(false);
            emit dApp->signalM->isAdapt(true);
        } else {
            emit dApp->signalM->enterScaledMode(false);
            emit dApp->signalM->isAdapt(false);
        }
    });
    connect(m_viewB, &ImageView::showScaleLabel, this, [ = ]() {
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

    connect(this, &ViewPanel::imageChanged, this, [ = ](const QString & path, DBImgInfoList infos) {
        Q_UNUSED(infos);
        if (path.isEmpty()) m_nav->setVisible(false);
        m_nav->setImage(m_viewB->image(true));
    });
    connect(dApp->signalM, &SignalManager::UpdateNavImg, this, [ = ]() {
        m_nav->setImage(m_viewB->image(true));
        m_nav->setRectInImage(m_viewB->visibleImageRect());

        //正在滑动缩略图的时候不再显示
        if (m_nav->isVisible() && dApp->m_bMove) {
            m_nav->setVisible(false);
        }
    });
    connect(m_nav, &NavigationWidget::requestMove, [this](int x, int y) {
        m_viewB->centerOn(x, y);
    });
    connect(m_viewB, &ImageView::transformChanged, [this]() {
        //如果stackindex不为2，全屏会出现导航窗口
        //如果是正在移动的情况，将不会出现导航栏窗口
        if (m_stack->currentIndex() != 2 && !dApp->m_bMove) {
            m_nav->setVisible((! m_nav->isAlwaysHidden() && ! m_viewB->isWholeImageVisible()));
            m_nav->setRectInImage(m_viewB->visibleImageRect());
        }
    });
    connect(dApp->signalM, &SignalManager::hideNavigation, this, [ = ]() {
        m_nav->setVisible(false);
    });
}
