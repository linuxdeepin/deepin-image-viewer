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
#ifndef IMGINFODIALOG_H
#define IMGINFODIALOG_H

#include <ddialog.h>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class QVBoxLayout;
class ImgInfoDialog : public DMainWindow
{
    Q_OBJECT
public:
    explicit ImgInfoDialog(const QString &path, QWidget *parent = 0);

signals:
    void closed();

protected:
    void hideEvent(QHideEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

private:
    void initThumbnail(const QString &path);
    void initSeparator();
    void initInfos(const QString &path);
    void initCloseButton();

private:
    QVBoxLayout *m_layout;
};

#endif // IMGINFODIALOG_H
