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
#ifndef SLIDESHOWPANEL_H
#define SLIDESHOWPANEL_H

#include "imageanimation.h"
#include "controller/viewerthememanager.h"
#include "controller/signalmanager.h"

#include <DMenu>
#include <DIconButton>
#include <DLabel>
#include <QHBoxLayout>
#include <DFloatingWidget>
#include <QShortcut>
#include <QObject>

DWIDGET_USE_NAMESPACE

class SlideShowBottomBar : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit SlideShowBottomBar(QWidget *parent = nullptr);
public:
    DIconButton *m_preButton;
    DIconButton *m_nextButton;
    DIconButton *m_playpauseButton;
    DIconButton *m_cancelButton;
    int a = 0;

public slots:
    void onPreButtonClicked();
    void onPlaypauseButtonClicked();
    void onUpdatePauseButton();
    void onInitSlideShowButton();
    void onNextButtonClicked();
    void onCancelButtonClicked();

signals:
    void showPrevious();
    void showPause();
    void showContinue();
    void showNext();
    void showCancel();
};

class SlideShowPanel : public QWidget
{
    Q_OBJECT
public:
    enum MenuItemId {
        IdStopslideshow,
        IdPlayOrPause,
        IdPlay,
        IdPause
    };
    explicit SlideShowPanel(QWidget *parent = nullptr);
    void initConnections();
    void initMenu();
    void appendAction(int id, const QString &text, const QString &shortcut);
    void backToLastPanel();
    void showNormal();
    void showFullScreen();

    SlideShowBottomBar *slideshowbottombar;

public slots:
    void startSlideShow(const SignalManager::ViewInfo &vinfo, bool inDB);
    void onMenuItemClicked(QAction *action);
    void onThemeChanged(ViewerThemeManager::AppTheme dark);
    void onSingleAnimationEnd();
    void onESCKeyStopSlide();
    void onShowPause();
    void onShowContinue();
    void onShowPrevious();
    void onShowNext();
    void onCustomContextMenuRequested();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
private:
    ImageAnimation *m_animation;
    DMenu *m_menu;
    QShortcut *m_sEsc;
    SignalManager::ViewInfo m_vinfo;
    QColor m_bgColor;
    bool m_isMaximized;
    int m_hideCursorTid;
};

#endif // SLIDESHOWPANEL_H
