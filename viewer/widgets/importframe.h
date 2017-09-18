/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
#ifndef IMPORTFRAME_H
#define IMPORTFRAME_H

#include <QWidget>

#include "controller/viewerthememanager.h"

class QPushButton;
class QLabel;
class ImportFrame : public QWidget
{
    Q_OBJECT
public:
    explicit ImportFrame(QWidget *parent = 0);
    void setTitle(const QString &title);
    void setButtonText(const QString &text);
    const QString buttonText() const;
private slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
signals:
    void clicked();
private:
    QPushButton *m_importButton;
    QLabel *m_bgLabel;
    QLabel *m_titleLabel;
};

#endif // IMPORTFRAME_H
