/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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


#pragma once

#include <QScopedPointer>
#include <QFrame>
#include <QIcon>

#include "dtkwidget_global.h"
#include <DObject>
#include <DObjectPrivate>

#include <QLabel>
#include <DIconButton>
#include "imagebutton.h"
#include <dimagebutton.h>
#include "dthememanager.h"
#include "dgraphicsgloweffect.h"
#include <QPropertyAnimation>
DWIDGET_BEGIN_NAMESPACE

class ToastPrivate;
class LIBDTKWIDGETSHARED_EXPORT Toast : public QFrame, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    explicit Toast(QWidget *parent = Q_NULLPTR);
    ~Toast();

    QString text() const;
    QIcon icon() const;

public Q_SLOTS:
    void pop();
    void pack();

    void setText(QString text);
    void setIcon(QString icon);
    void setIcon(QIcon icon, QSize defaultSize = QSize(20, 20));

private:
    qreal opacity() const;
    void setOpacity(qreal);

    D_DECLARE_PRIVATE(Toast)
};
class ToastPrivate: public DTK_CORE_NAMESPACE::DObjectPrivate

{
public:
    ToastPrivate(Toast *qq);

    QIcon   icon;
    QLabel  *iconLabel      = Q_NULLPTR;
    QLabel  *textLabel      = Q_NULLPTR;

    ImageButton *closeBt    = Q_NULLPTR;

    QPropertyAnimation  *animation  = Q_NULLPTR;
    DGraphicsGlowEffect *effect     = Q_NULLPTR;

    void initUI();
private:
    D_DECLARE_PUBLIC(Toast)
};

DWIDGET_END_NAMESPACE
