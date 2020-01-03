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
#ifndef FORMLABEL_H
#define FORMLABEL_H

#include <QLabel>

class SimpleFormLabel : public QLabel
{
    Q_OBJECT
public:
    explicit SimpleFormLabel(const QString &t, QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent *event);
};

class SimpleFormField : public QLabel
{
    Q_OBJECT
public:
    explicit SimpleFormField(QWidget *parent = nullptr);
protected:
    void resizeEvent(QResizeEvent *event);
};
#endif // FORMLABEL_H
