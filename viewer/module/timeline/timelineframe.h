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
#ifndef TIMELINEFRAME_H
#define TIMELINEFRAME_H

#include "controller/dbmanager.h"
#include "mvc/timelinemodel.h"
#include "danchors.h"
#include <QFrame>

DWIDGET_USE_NAMESPACE

class TimelineView;
class TopTimelineTip;

class TimelineFrame : public QFrame
{
    Q_OBJECT
public:
    explicit TimelineFrame(QWidget *parent = 0);
    void clearSelection();
    void selectAll();
    void setIconSize(int size);
    void updateThumbnail(const QString &path);
    void updateScrollRange();

    bool isEmpty() const;
    const QString currentMonth() const;
    const QStringList selectedPaths() const;

signals:
    void selectIndexChanged(const QModelIndex &current);
    void changeItemSize(bool increase);
    void showMenu();
    void viewImage(const QString &path, const QStringList &paths);

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

private:
    void initConnection();
    void initView();
    void initTopTip();
    void initItems();
    void insertItems(const TimelineItem::ItemData &data);
    void removeItem(const DBImgInfo &info);
    void removeItems(const DBImgInfoList &infos);

private:
    DAnchors<TopTimelineTip> m_tip;
    DBImgInfoList           m_infos;
    QMutex                  m_mutex;
    TimelineModel           m_model;
    TimelineView            *m_view;
};

#endif // TIMELINEFRAME_H
