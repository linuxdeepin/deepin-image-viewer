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
#ifndef BLUREINFOFRAME_H
#define BLUREINFOFRAME_H

#include "blureframe.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

/*!
 * \brief The BlureInfoFrame class
 * Use for PopupImageInfo and PopupAlbumInfo
 */
class BlureInfoFrame : public BlurFrame
{
    Q_OBJECT
public:
    explicit BlureInfoFrame(QWidget *parent);
    void setTopContent(QWidget *w);
    void addInfoPair(const QString &title, const QString &value);
    void close();

signals:
    void closed();

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    int m_leftMax = 0;
    int m_rightMax = 0;
    QFrame *m_infoFrame;
    QVBoxLayout *m_topLayout;
    QVBoxLayout *m_bottomLayout;
    QFormLayout *m_infoLayout;
};

#endif // BLUREINFOFRAME_H
