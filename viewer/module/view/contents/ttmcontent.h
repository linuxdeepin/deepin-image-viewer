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
#ifndef TTMCONTENT_H
#define TTMCONTENT_H

#include "controller/viewerthememanager.h"
#include "widgets/elidedlabel.h"

#include <QFrame>
#include <QWidget>
#include <QLabel>

class PushButton;
class QHBoxLayout;

class TTMContent : public QFrame
{
    Q_OBJECT
public:
    explicit TTMContent(QWidget *parent = 0);

    const QString getCurrentPath();

public slots:
    void setPath(const QString &path);
    void updateLayout(int ttlWidth, const QString &path);

private slots:
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

private:
    QHBoxLayout* m_layout;
    QLabel* m_emptyLabel;
    ElidedLabel* m_fileNameLabel;

    QString m_path;
    int m_contentWidth;
    int m_windowWidth;
    int m_leftContentWidth;

};

#endif // TTMCONTENT_H
