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
#ifndef MOREPICFLOATWIDGET_H
#define MOREPICFLOATWIDGET_H
#include <DFloatingWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <DIconButton>
#include <QGuiApplication>
#include <DLabel>
#include <DIconButton>
#include <DWidget>
DWIDGET_USE_NAMESPACE
class MorePicFloatWidget : public DFloatingWidget
{
public:
    explicit MorePicFloatWidget(QWidget *parent = nullptr);
    ~MorePicFloatWidget();
    void initUI();

    DIconButton *getButtonUp();

    DIconButton *getButtonDown();

    void setLabelText(const QString &num);
private:
//ui
    QVBoxLayout *m_pLayout{nullptr};
    DLabel *m_labelNum{nullptr};
    DWidget *m_labelUp{nullptr};
    DWidget *m_labelDown{nullptr};
    DIconButton *m_buttonUp{nullptr};
    DIconButton *m_buttonDown{nullptr};
};

#endif // MOREPICFLOATWIDGET_H
