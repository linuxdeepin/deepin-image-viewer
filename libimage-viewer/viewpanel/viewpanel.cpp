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
#include <QShortcut>
#include <DMenu>
#include "contents/bottomtoolbar.h"
#include "navigationwidget.h"

const int BOTTOM_TOOLBAR_HEIGHT = 80;   //底部工具看高
const int BOTTOM_SPACING = 10;          //底部工具栏与底部边缘距离

QString ss(const QString &text, const QString &defaultValue)
{
    Q_UNUSED(text);
    //采用代码中快捷键不使用配置文件快捷键
    // QString str = dApp->setter->value(SHORTCUTVIEW_GROUP, text, defaultValue).toString();
    QString str = defaultValue;
    str.replace(" ", "");
    return defaultValue;
}

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

    setContextMenuPolicy(Qt::CustomContextMenu);
    initConnect();
    initRightMenu();
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

void ViewPanel::initRightMenu()
{
    if (!m_menu) {
        m_menu = new DMenu(this);
        updateMenuContent();
    }
    QShortcut *ctrlm = new QShortcut(QKeySequence("Ctrl+M"), this);
    ctrlm->setContext(Qt::WindowShortcut);
    connect(ctrlm, &QShortcut::activated, this, [ = ] {
        this->customContextMenuRequested(cursor().pos());
    });

    m_menu = new DMenu;
    connect(this, &ViewPanel::customContextMenuRequested, this, [ = ] {

        updateMenuContent();
        m_menu->popup(QCursor::pos());
    });
    connect(m_menu, &DMenu::triggered, this, &ViewPanel::onMenuItemClicked);
}

void ViewPanel::updateMenuContent()
{
    if (m_menu) {
        m_menu->clear();
        qDeleteAll(this->actions());
        if (window()->isFullScreen()) {
            appendAction(IdExitFullScreen, QObject::tr("Exit fullscreen"), ss("Fullscreen", "F11"));
        } else {
            appendAction(IdFullScreen, QObject::tr("Fullscreen"), ss("Fullscreen", "F11"));
        }

        appendAction(IdPrint, QObject::tr("Print"), ss("Print", "Ctrl+P"));
        //ocr按钮,是否是动态图,todo
        if (true) {
            appendAction(IdOcr, QObject::tr("Extract text"), ss("Extract text", "Alt+O"));
        }

        //如果图片数量大于0才能有幻灯片
        if (true) {
            appendAction(IdStartSlideShow, QObject::tr("Slide show"), ss("Slide show", "F5"));
        }

        m_menu->addSeparator();

        appendAction(IdCopy, QObject::tr("Copy"), ss("Copy", "Ctrl+C"));

        //如果程序有可读可写的权限,才能重命名,todo
        if (true) {
            appendAction(IdRename, QObject::tr("Rename"), ss("Rename", "F2"));
        }

        //apple phone的delete没有权限,保险箱无法删除,垃圾箱也无法删除,其他需要判断可读权限,todo
        if (true) {
            appendAction(IdMoveToTrash, QObject::tr("Delete"), ss("Throw to trash", "Delete"));
        }

        m_menu->addSeparator();

        //判断导航栏隐藏,需要添加一个当前是否有图片,todo
        if (!m_view->isWholeImageVisible() && m_nav->isAlwaysHidden()) {
            appendAction(IdShowNavigationWindow, QObject::tr("Show navigation window"),
                         ss("Show navigation window", ""));
        } else if (!m_view->isWholeImageVisible() && !m_nav->isAlwaysHidden()) {
            appendAction(IdHideNavigationWindow, QObject::tr("Hide navigation window"),
                         ss("Hide navigation window", ""));
        }
        //apple手机特殊处理，不具备旋转功能,todo
        if (true) {
            appendAction(IdRotateClockwise, QObject::tr("Rotate clockwise"), ss("Rotate clockwise", "Ctrl+R"));
            appendAction(IdRotateCounterclockwise, QObject::tr("Rotate counterclockwise"),
                         ss("Rotate counterclockwise", "Ctrl+Shift+R"));
        }

        //需要判断图片是否支持设置壁纸,todo
        if (true) {
            appendAction(IdSetAsWallpaper, QObject::tr("Set as wallpaper"), ss("Set as wallpaper", "Ctrl+F9"));
        }
        appendAction(IdDisplayInFileManager, QObject::tr("Display in file manager"),
                     ss("Display in file manager", "Alt+D"));
        appendAction(IdImageInfo, QObject::tr("Image info"), ss("Image info", "Ctrl+I"));

    }
}

void ViewPanel::toggleFullScreen()
{
    m_view->setFitState(false, false);
    if (window()->isFullScreen()) {
        showNormal();
        m_view->viewport()->setCursor(Qt::ArrowCursor);
    } else {
        showFullScreen();
        if (!m_menu || !m_menu->isVisible()) {
            m_view->viewport()->setCursor(Qt::BlankCursor);
        }
    }
}

void ViewPanel::showFullScreen()
{
    m_isMaximized = window()->isMaximized();
    // Full screen then hide bars because hide animation depends on height()
    //加入动画效果，掩盖左上角展开的视觉效果，以透明度0-1显示。,时间为50ms

    QPropertyAnimation *pAn = new QPropertyAnimation(window(), "windowOpacity");
    pAn->setDuration(50);
    pAn->setEasingCurve(QEasingCurve::Linear);
    pAn->setEndValue(1);
    pAn->setStartValue(0);
    pAn->start(QAbstractAnimation::DeleteWhenStopped);


    window()->showFullScreen();

}

void ViewPanel::showNormal()
{
    //加入动画效果，掩盖左上角展开的视觉效果，以透明度0-1显示。

    QPropertyAnimation *pAn = new QPropertyAnimation(window(), "windowOpacity");
    pAn->setDuration(50);
    pAn->setEasingCurve(QEasingCurve::Linear);
    pAn->setEndValue(1);
    pAn->setStartValue(0);
    pAn->start(QAbstractAnimation::DeleteWhenStopped);
    if (m_isMaximized) {
        window()->showNormal();
        window()->showMaximized();
    } else {
        window()->showNormal();
    }
}


void ViewPanel::appendAction(int id, const QString &text, const QString &shortcut)
{
    if (m_menu) {
        QAction *ac = new QAction(m_menu);
        addAction(ac);
        ac->setText(text);
        ac->setProperty("MenuID", id);
        ac->setShortcut(QKeySequence(shortcut));
        m_menu->addAction(ac);
    }
}

void ViewPanel::onMenuItemClicked(QAction *action)
{
    const int id = action->property("MenuID").toInt();
    switch (MenuItemId(id)) {
    case IdFullScreen:
    case IdExitFullScreen: {
        toggleFullScreen();
        break;
    }
    case IdStartSlideShow: {
        //todo,幻灯片
        break;
    }
    case IdPrint: {
        //todo,打印
        break;
    }
    case IdRename: {
        //todo,重命名
        break;
    }
    case IdCopy: {
        //todo,复制
        break;
    }
    case IdMoveToTrash: {
        //todo,删除
        break;
    }
    case IdShowNavigationWindow: {
        m_nav->setAlwaysHidden(false);
        break;
    }
    case IdHideNavigationWindow: {
        m_nav->setAlwaysHidden(true);
        break;
    }
    case IdRotateClockwise: {
        //todo旋转
        break;
    }
    case IdRotateCounterclockwise: {
        //todo旋转
        break;
    }
    case IdSetAsWallpaper: {
        //todo设置壁纸
        break;
    }
    case IdDisplayInFileManager : {
        //todo显示在文管
        break;
    }
    case IdImageInfo: {
        //todo,文件信息
        break;
    }
    case IdOcr: {
        //todo,ocr
        break;
    }
    }

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
