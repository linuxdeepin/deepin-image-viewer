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
#ifndef TOPALBUMTIPS_H
#define TOPALBUMTIPS_H

#include <QFrame>

class QLabel;
class QPushButton;
class QHBoxLayout;
class TopAlbumTips : public QFrame
{
    Q_OBJECT
public:
    explicit TopAlbumTips(QWidget *parent = 0);
    void setAlbum(const QString &album);
    void setLeftMargin(int v);

private:
    const QString trName(const QString &name) const;

private:
    QHBoxLayout *m_layout;
    QLabel *m_infoLabel;
};

#endif // TOPALBUMTIPS_H
