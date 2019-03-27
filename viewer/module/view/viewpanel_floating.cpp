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
#include "viewpanel.h"
#include "navigationwidget.h"
#include "contents/imageinfowidget.h"
#include "scen/imageview.h"
#include "dimagebutton.h"
#include "widgets/pushbutton.h"

#include "utils/baseutils.h"

#include <QTimer>

DWIDGET_USE_NAMESPACE

void ViewPanel::initFloatingComponent()
{
    initSwitchButtons();
    initScaleLabel();
    initNavigation();
}

void ViewPanel::initSwitchButtons()
{
    using namespace utils::base;

    DAnchors<DImageButton> pre_button = new DImageButton(this);
    DAnchors<DImageButton> next_button = new DImageButton(this);

    pre_button->setObjectName("PreviousButton");
    next_button->setObjectName("NextButton");

    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
        pre_button->setStyleSheet(getFileContent(":/resources/dark/qss/floating.qss"));
        next_button->setStyleSheet(getFileContent(":/resources/dark/qss/floating.qss"));
    } else {
        pre_button->setStyleSheet(getFileContent(":/resources/light/qss/floating.qss"));
        next_button->setStyleSheet(getFileContent(":/resources/light/qss/floating.qss"));
    }

    pre_button.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    pre_button.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);

    next_button.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    next_button.setAnchor(Qt::AnchorRight, this, Qt::AnchorRight);

    pre_button->setFixedSize(100, 200);
    next_button->setFixedSize(100, 200);

    pre_button->hide();
    next_button->hide();

    // pre_button.setLeftMargin(20);
    // next_button.setRightMargin(20);

    connect(pre_button, &DImageButton::clicked, this, &ViewPanel::showPrevious);
    connect(next_button, &DImageButton::clicked, this, &ViewPanel::showNext);

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            [=](ViewerThemeManager::AppTheme theme) {
        if (theme == ViewerThemeManager::Dark) {
            pre_button->setStyleSheet(getFileContent(
                                         ":/resources/dark/qss/floating.qss"));
            next_button->setStyleSheet(getFileContent(
                                          ":/resources/dark/qss/floating.qss"));
        } else {
            pre_button->setStyleSheet(getFileContent(
                                         ":/resources/light/qss/floating.qss"));
            next_button->setStyleSheet(getFileContent(
                                          ":/resources/light/qss/floating.qss"));
        }
    });

    connect(this, &ViewPanel::mouseMoved, this, [=] {
        DAnchors<DImageButton> pb = pre_button;
        if (m_info && m_info->visibleRegion().isNull()) {
            pb.setLeftMargin(0);
        } else {
            pb.setLeftMargin(240);
        }

        QPoint pos = mapFromGlobal(QCursor::pos());
        QRect left_rect = pre_button->geometry();
        QRect right_rect = next_button->geometry();

        if (left_rect.contains(pos, true) || right_rect.contains(pos)) {
            pre_button->show();
            next_button->show();
        } else {
            pre_button->hide();
            next_button->hide();
        }
    });
}

void ViewPanel::initScaleLabel()
{
    using namespace utils::base;
    DAnchors<QLabel> scalePerc = new QLabel(this);
    scalePerc->setObjectName("ScaleLabel");
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
        scalePerc->setStyleSheet(getFileContent(":/resources/dark/qss/floating.qss"));
    } else {
        scalePerc->setStyleSheet(getFileContent(":/resources/light/qss/floating.qss"));
    }

    scalePerc->setAttribute(Qt::WA_TransparentForMouseEvents);
    scalePerc.setAnchor(Qt::AnchorHorizontalCenter, this, Qt::AnchorHorizontalCenter);
    scalePerc.setAnchor(Qt::AnchorBottom, this, Qt::AnchorBottom);
    scalePerc.setBottomMargin(54);
    scalePerc->setAlignment(Qt::AlignCenter);
    scalePerc->setFixedSize(82, 48);
    scalePerc->setText("100%");
    scalePerc->hide();

    QTimer *hideT = new QTimer(this);
    hideT->setSingleShot(true);
    connect(hideT, &QTimer::timeout, scalePerc, &QLabel::hide);

    connect(m_viewB, &ImageView::scaled, this, [=](qreal perc) {
        scalePerc->setText(QString("%1%").arg(int(perc)));
    });
    connect(m_viewB, &ImageView::showScaleLabel, this, [=](){
        scalePerc->show();
        hideT->start(1000);
    });
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            [=](ViewerThemeManager::AppTheme theme) {
        if (theme == ViewerThemeManager::Dark) {
            scalePerc->setStyleSheet(getFileContent(":/resources/dark/qss/floating.qss"));
        } else {
            scalePerc->setStyleSheet(getFileContent(":/resources/light/qss/floating.qss"));
        }
    });
}

void ViewPanel::initNavigation()
{
    m_nav = new NavigationWidget(this);
    m_nav.setAnchor(Qt::AnchorRight, this, Qt::AnchorRight);
    m_nav.setAnchor(Qt::AnchorBottom, this, Qt::AnchorBottom);

    connect(this, &ViewPanel::imageChanged, this, [=] (const QString &path) {
        if (path.isEmpty()) m_nav->setVisible(false);
        m_nav->setImage(m_viewB->image());
    });
    connect(m_nav, &NavigationWidget::requestMove, [this](int x, int y){
        m_viewB->centerOn(x, y);
    });
    connect(m_viewB, &ImageView::transformChanged, [this](){
        m_nav->setVisible(! m_nav->isAlwaysHidden() && ! m_viewB->isWholeImageVisible());
        m_nav->setRectInImage(m_viewB->visibleImageRect());
    });
    connect(m_viewB, &ImageView::hideNavigation, this, [=]() {
        m_nav->setVisible(false);
    });
}
