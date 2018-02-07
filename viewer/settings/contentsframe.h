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
#ifndef CONTENTSFRAME_H
#define CONTENTSFRAME_H

#include "titlebutton.h"
#include <QWidget>
#include <QScrollArea>

class QVBoxLayout;
class ContentsFrame : public QWidget
{
    Q_OBJECT
public:
    explicit ContentsFrame(QWidget *parent);
    void setCurrentID(const TitleButton::SettingID id);

signals:
    void currentFieldChanged(TitleButton::SettingID id);

private:
    void initScrollArea();

private:
    QScrollArea *m_area;
    QVBoxLayout *m_layout;
};

#endif // CONTENTSFRAME_H
