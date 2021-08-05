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

#ifndef TOPTOOLBAR_H
#define TOPTOOLBAR_H

#include <QJsonObject>
#include <QPointer>
#include <DTitlebar>
#include <DLabel>
#include <QGraphicsDropShadowEffect>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DMenu>
#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class SettingsWindow;
class QHBoxLayout;
class QProcess;

class TopToolbar : public DBlurEffectWidget
{
    Q_OBJECT
public:
    TopToolbar(bool manager, QWidget *parent);
    void setMiddleContent(const QString &path);
public slots:
    void setTitleBarTransparent(bool a);
protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;
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
    void onThemeTypeChanged();
    void onUpdateFileName(const QString &filename);

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
    QString m_filename;
#ifndef LITE_DIV
    SettingsWindow *m_settingsWindow;
#endif
    DMenu *m_menu;
    bool m_manager;

    bool m_viewChange;
    QGraphicsDropShadowEffect *shadowEffect {nullptr};
};

#endif // TOPTOOLBAR_H
