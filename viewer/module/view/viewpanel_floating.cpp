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
    DAnchors<PushButton> preButton = new PushButton(this);
    preButton->setObjectName("PreviousButton");
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
        preButton->setStyleSheet(getFileContent(":/resources/dark/qss/floating.qss"));
    } else {
        preButton->setStyleSheet(getFileContent(":/resources/light/qss/floating.qss"));
    }
    preButton.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    // The preButton is anchored to the left of this
    preButton.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);
    // NOTE: this is a bug of Anchors,the button should be resize after set anchor
    preButton->resize(53, 53);
    preButton.setLeftMargin(20);
    preButton->hide();
    connect(preButton, &PushButton::clicked, this, &ViewPanel::showPrevious);

    DAnchors<PushButton> nextButton = new PushButton(this);
    nextButton->setObjectName("NextButton");
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
        nextButton->setStyleSheet(getFileContent(":/resources/dark/qss/floating.qss"));
    } else {
        nextButton->setStyleSheet(getFileContent(":/resources/light/qss/floating.qss"));
    }
    nextButton.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    nextButton.setAnchor(Qt::AnchorRight, this, Qt::AnchorRight);
    nextButton->setFixedSize(53, 53);
    nextButton.setRightMargin(20);
    nextButton->hide();
    connect(nextButton, &PushButton::clicked, this, &ViewPanel::showNext);
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this,
            [=](ViewerThemeManager::AppTheme theme) {
        if (theme == ViewerThemeManager::Dark) {
            preButton->setStyleSheet(getFileContent(
                                         ":/resources/dark/qss/floating.qss"));
            nextButton->setStyleSheet(getFileContent(
                                          ":/resources/dark/qss/floating.qss"));
        } else {
            preButton->setStyleSheet(getFileContent(
                                         ":/resources/light/qss/floating.qss"));
            nextButton->setStyleSheet(getFileContent(
                                          ":/resources/light/qss/floating.qss"));
        }
    });
    connect(this, &ViewPanel::mouseMoved, this, [=] {

        const int EXTEND_SPACING = 15;

        DAnchors<PushButton> pb = preButton;
        if (m_info && m_info->visibleRegion().isNull()) {
            pb.setLeftMargin(20);
        }
        else {
            pb.setLeftMargin(260);
        }

        const QPoint pp = preButton->mapToGlobal(QPoint(0, 0))
                - QPoint(EXTEND_SPACING, EXTEND_SPACING);
        QRect pr(pp, QSize(preButton->width() + EXTEND_SPACING * 2,
                           preButton->height() + EXTEND_SPACING * 2));

        const QPoint np = nextButton->mapToGlobal(QPoint(0, 0))
                - QPoint(EXTEND_SPACING, EXTEND_SPACING);
        QRect nr(np, QSize(nextButton->width() + EXTEND_SPACING * 2,
                           nextButton->height() + EXTEND_SPACING * 2));

        if (pr.contains(QCursor::pos()) || nr.contains(QCursor::pos())) {
            preButton->show();
            nextButton->show();
        }
        else {
            preButton->hide();
            nextButton->hide();
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

    m_nav->setVisible(! m_nav->isAlwaysHidden());
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
