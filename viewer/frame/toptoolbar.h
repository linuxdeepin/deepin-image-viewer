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

#ifndef TOPTOOLBAR_H
#define TOPTOOLBAR_H

#include "widgets/blureframe.h"
#include "controller/viewerthememanager.h"

#include <QJsonObject>
#include <QPointer>
#include <DTitlebar>
#include <DLabel>
#include <QGraphicsDropShadowEffect>
#include <DMenu>
#include <DShadowLine>

DWIDGET_USE_NAMESPACE

class QHBoxLayout;
class QProcess;

class TopToolbar : public DBlurEffectWidget
{
    Q_OBJECT
public:
    TopToolbar(bool manager, QWidget *parent);
    void setMiddleContent(QString path);

    void setTitleBarTransparent(bool a);

protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

signals:
    void updateMaxBtn();

private:
    enum MenuItemId {
        IdCreateAlbum,
        IdSwitchTheme,
        IdSetting,
        IdImport,
        IdHelp,
        IdAbout,
        IdQuick,
        IdSeparator
    };

    void initMenu();
    void initWidgets();
    QString  geteElidedText(QFont font, QString str, int MaxWidth);

private slots:

//    void onThemeChanged(ViewerThemeManager::AppTheme curTheme);

private:
    QColor m_coverBrush;
    QColor m_topBorderColor;
    QColor m_bottomBorderColor;

    QPointer<QProcess> m_manualPro;
    QHBoxLayout *m_layout = nullptr;
    QHBoxLayout *m_lLayout = nullptr;
    QHBoxLayout *m_mLayout = nullptr;
    QHBoxLayout *m_rLayout = nullptr;
    DTitlebar *m_titlebar = nullptr;
    DLabel *m_titletxt = nullptr;

    DShadowLine *m_shadowLine {nullptr};

    DMenu *m_menu;
    bool m_manager;
    bool m_viewChange;
    QString m_filename = "";

    QGraphicsDropShadowEffect* shadowEffect {nullptr};
};

#endif // TOPTOOLBAR_H
