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
#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include "controller/viewerthememanager.h"
#include <DLabel>

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;


class ElidedLabel : public QLbtoDLabel
{
    Q_OBJECT
public:
    explicit ElidedLabel(QWidget *parent = nullptr);
    ~ElidedLabel();

    void setText(const QString &text, int leftMargin = 0);
    void onThemeChanged(ViewerThemeManager::AppTheme theme);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *event);

private:
    QString m_text;
    int m_leftMargin;
    QColor m_textColor;
};
#endif // ELIDEDLABEL_H
