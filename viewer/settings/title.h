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
#ifndef TITLELINE_H
#define TITLELINE_H

#include <QWidget>
#include <QLabel>

class Title1 : public QWidget
{
    Q_OBJECT
public:
    explicit Title1(const QString &title, QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    QString m_title;
};

class Title2 : public QLabel
{
    Q_OBJECT
public:
    explicit Title2(const QString &title, QWidget *parent = 0);
};

class Title3 : public QLabel
{
    Q_OBJECT
public:
    explicit Title3(const QString &title, QWidget *parent = 0);
};

#endif // TITLELINE_H
