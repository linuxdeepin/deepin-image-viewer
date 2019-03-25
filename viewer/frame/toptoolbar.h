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

DWIDGET_USE_NAMESPACE

class SettingsWindow;
class QHBoxLayout;
class QProcess;
class QMenu;

class TopToolbar : public BlurFrame
{
    Q_OBJECT
public:
    TopToolbar(bool manager, QWidget *parent);
    void setLeftContent(QWidget *content);
    void setMiddleContent(QWidget *content);

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

    void initLeftContent();
    void initMiddleContent();
    void initRightContent();
    void initMenu();
    void initWidgets();

private slots:
//    void onAbout();
    void onHelp();
#ifndef LITE_DIV
    void onNewAlbum();
    void onSetting();
#endif
    void onViewShortcut();
    void onDeepColorMode();

    void onThemeChanged(ViewerThemeManager::AppTheme curTheme);

private:
    const QString newAlbumShortcut() const;

private:
    QColor m_coverBrush;
    QColor m_topBorderColor;
    QColor m_bottomBorderColor;

    QPointer<QProcess> m_manualPro;
    QHBoxLayout *m_layout;
    QHBoxLayout *m_lLayout;
    QHBoxLayout *m_mLayout;
    QHBoxLayout *m_rLayout;

#ifndef LITE_DIV
    SettingsWindow *m_settingsWindow;
#endif
    QMenu *m_menu;
    bool m_manager;
};

#endif // TOPTOOLBAR_H
