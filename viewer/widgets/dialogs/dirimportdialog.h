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
#ifndef DIRIMPORTDIALOG_H
#define DIRIMPORTDIALOG_H

#include "dialog.h"

class DirImportDialog : public Dialog
{
    Q_OBJECT
public:
    explicit DirImportDialog(const QString &dir, const QString &album, QWidget *parent = 0);

signals:
    void albumCreated();

protected:
    void keyPressEvent(QKeyEvent *e) override;
};

#endif // DIRIMPORTDIALOG_H
