/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangYong <zhangyong@uniontech.com>
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
#include "toptoolbar.h"
//#include "unionimage/baseutils.h"
//#include "utils/shortcut.h"

#include <dwindowminbutton.h>
#include <dwindowmaxbutton.h>
#include <dwindowclosebutton.h>
#include <dwindowoptionbutton.h>
#include <dtitlebar.h>

#include <QDebug>
#include <QHBoxLayout>
#include <QPainter>
#include <QProcess>
#include <QResizeEvent>
#include <QShortcut>
#include <QStyleFactory>
#include <QImageReader>
#include <QLabel>
#include <QPainterPath>
#include <DFontSizeManager>
#include <DApplicationHelper>
DWIDGET_USE_NAMESPACE

namespace {

//const int TOP_TOOLBAR_HEIGHT = 50;
//const int ICON_MARGIN = 6;

const QColor DARK_TOP_BORDERCOLOR = QColor(255, 255, 255, 0);
const QColor LIGHT_TOP_BORDERCOLOR = QColor(255, 255, 255, 0);

const QColor DARK_BOTTOM_BORDERCOLOR = QColor(0, 0, 0, 51);
const QColor LIGHT_BOTTOM_BORDERCOLOR = QColor(0, 0, 0, 26);
}  // namespace

TopToolbar::TopToolbar(bool manager, QWidget *parent)
    : DBlurEffectWidget(parent)
{
    m_manager = manager;
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(0, 0, 0, 0)); // 最后一项为透明度
    setPalette(palette);
    initMenu();
    initWidgets();
}

void TopToolbar::setMiddleContent(const QString &path)
{
    //保存当前名称
    m_filename = path;
    //修复名字过长显示不完整bug
    QString a = geteElidedText(DFontSizeManager::instance()->get(DFontSizeManager::T7),
                               path, width() - 500);
    m_titletxt->setText(a);
    m_titletxt->setObjectName(a);
    m_titletxt->setAccessibleName(a);
}

void TopToolbar::setTitleBarTransparent(bool a)
{
    m_viewChange = a;

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    QPalette pa1, pa2;
    if (a) {
        m_titlebar->setBackgroundTransparent(true);
        shadowEffect->setOffset(0, 1);
        shadowEffect->setBlurRadius(1);
        m_titletxt->setGraphicsEffect(shadowEffect);
        //        if (themeType == DGuiApplicationHelper::LightType) {
        pa1.setColor(QPalette::ButtonText, QColor(255, 255, 255, 204));
        pa2.setColor(QPalette::WindowText, QColor(255, 255, 255, 204));
        m_titlebar->setPalette(pa1);
        m_titletxt->setPalette(pa2);

    } else {
        m_titlebar->setBackgroundTransparent(false);
        shadowEffect->setOffset(0, 0);
        shadowEffect->setBlurRadius(0);
        m_titletxt->setGraphicsEffect(shadowEffect);
        if (themeType == DGuiApplicationHelper::LightType) {
            pa1.setColor(QPalette::ButtonText, QColor(98, 110, 136, 225));
            pa2.setColor(QPalette::WindowText, QColor(98, 110, 136, 225));
            m_titlebar->setPalette(pa1);
            m_titletxt->setPalette(pa2);
        } else {
            pa1.setColor(QPalette::ButtonText, QColor(255, 255, 255, 204));
            pa2.setColor(QPalette::WindowText, QColor(255, 255, 255, 204));
            m_titlebar->setPalette(pa1);
            m_titletxt->setPalette(pa2);
        }
    }
}

void TopToolbar::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (window()->isMaximized())
            window()->showNormal();
        else if (! window()->isFullScreen())  // It would be normal state
            window()->showMaximized();
    }
    DBlurEffectWidget::mouseDoubleClickEvent(e);
}

void TopToolbar::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter p(this);
    QPixmap pixmap(":/icons/deepin/builtin/actions/imgView_titlebar.svg");
    const QPalette pal = QGuiApplication::palette();
    QBrush bgColor = QBrush(pixmap.scaled(size().width(), 74));
    QRectF bgRect;
    bgRect.setSize(size());
    QPainterPath pp;
    pp.addRoundedRect(QRectF(bgRect.x(), bgRect.y(), bgRect.width(), 60), 0, 0);
    p.fillPath(pp, bgColor);
}

void TopToolbar::resizeEvent(QResizeEvent *event)
{
    //在resize的时候,需要重新计算大小
    if (m_filename != "") {
        QString b = geteElidedText(DFontSizeManager::instance()->get(DFontSizeManager::T7),
                                   m_filename, width() - 500);
        m_titletxt->setText(b);
        m_titletxt->setObjectName(b);
        m_titletxt->setAccessibleName(b);
    }
    DBlurEffectWidget::resizeEvent(event);
}

void TopToolbar::initWidgets()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_titlebar = new DTitlebar(this);
    m_titlebar->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint |
                               Qt::WindowCloseButtonHint);
    m_titlebar->setMenu(m_menu);
    m_titlebar->setIcon(QIcon::fromTheme("deepin-image-viewer"));
    QPalette pa;
    pa.setColor(QPalette::WindowText, QColor(255, 255, 255, 255));
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
    this, [ = ]() {
        DGuiApplicationHelper::ColorType themeType =
            DGuiApplicationHelper::instance()->themeType();
        QPalette pa1, pa2;
        if (!m_viewChange) {
            if (themeType == DGuiApplicationHelper::DarkType) {
                pa1.setColor(QPalette::ButtonText, QColor(255, 255, 255, 204));
                pa2.setColor(QPalette::WindowText, QColor(255, 255, 255, 204));
            } else {
                pa1.setColor(QPalette::ButtonText, QColor(98, 110, 136, 225));
                pa2.setColor(QPalette::WindowText, QColor(98, 110, 136, 225));
            }
            m_titlebar->setPalette(pa1);
            m_titletxt->setPalette(pa2);
        } else {
        }
    });


    m_titlebar->setPalette(pa);
    m_titlebar->setTitle("");
    m_titletxt = new DLabel;
    m_titletxt->setText("");
    m_titletxt->setObjectName("");
    m_titletxt->setAccessibleName("");
    DFontSizeManager::instance()->bind(m_titletxt, DFontSizeManager::T7 /*,QFont::DemiBold*/);


    // add 12.13 by lz.
    shadowEffect = new QGraphicsDropShadowEffect(m_titletxt);

    m_titlebar->addWidget(m_titletxt, Qt::AlignCenter);

    m_layout->addWidget(m_titlebar);

//    connect(dApp->signalM, &SignalManager::updateFileName, this, &TopToolbar::onUpdateFileName);
}

QString TopToolbar::geteElidedText(QFont font, QString str, int MaxWidth)
{
    QFontMetrics fontWidth(font);
    int width = fontWidth.horizontalAdvance(str);
    if (width >= MaxWidth) {
        str = fontWidth.elidedText(str, Qt::ElideRight, MaxWidth);
    }
    return str;
}

/*void TopToolbar::onThemeTypeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    Q_UNUSED(themeType);
    QPalette pa1;
    pa1.setColor(QPalette::ButtonText, QColor(255, 255, 255, 204));
    m_titlebar->setPalette(pa1);
}*/

/*void TopToolbar::onUpdateFileName(const QString &filename)
{
    QString a = geteElidedText(DFontSizeManager::instance()->get(DFontSizeManager::T7), filename, width() - 500);
    m_titletxt->setText(a);
    m_titletxt->setObjectName(a);
    m_titletxt->setAccessibleName(a);
//    connect(dApp->signalM, &SignalManager::resizeFileName, this, [ = ]() {
//        QString b = geteElidedText(DFontSizeManager::instance()->get(DFontSizeManager::T7), filename, width() - 500);
//        m_titletxt->setText(b);
//        m_titletxt->setObjectName(b);
//        m_titletxt->setAccessibleName(b);
//    });
}*/

void TopToolbar::initMenu()
{
    m_menu = new DMenu(this);
    m_menu->addSeparator();
}

//void TopToolbar::onHelp()
//{
//    if (m_manualPro.isNull()) {
//        const QString pro = "dman";
//        const QStringList args("deepin-image-viewer");
//        m_manualPro = new QProcess(this);
//        connect(m_manualPro.data(), SIGNAL(finished(int)),
//                m_manualPro.data(), SLOT(deleteLater()));
//        m_manualPro->start(pro, args);
//    }
//}

//void TopToolbar::onDeepColorMode()
//{
//    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
//        dApp->viewerTheme->setCurrentTheme(
//            ViewerThemeManager::Light);
//    } else {
//        dApp->viewerTheme->setCurrentTheme(
//            ViewerThemeManager::Dark);
//    }
//}
