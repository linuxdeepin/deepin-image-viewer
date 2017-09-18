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
/*
    treemodel.cpp

    Provides a simple tree model to show how to create and use hierarchical
    models.
*/

#include "timelineitem.h"
#include "timelinemodel.h"
#include "utils/baseutils.h"

#include <QDateTime>
#include <QDebug>
#include <QStringList>

TimelineModel::TimelineModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new TimelineItem();
}

TimelineModel::~TimelineModel()
{
    delete rootItem;
}

void TimelineModel::appendData(const TimelineItem::ItemData &data)
{
    // If timeline not exists, create one
    TimelineItem *titleItem = timelineItem(data.timeline);
    TimelineItem *dataItem = titleItem->child(titleItem->childCount() - 1); // Last child

    dataItem->appendData(data);
}

void TimelineModel::removeData(const TimelineItem::ItemData &data)
{
    TimelineItem *titleItem = timelineItem(data.timeline);
    TimelineItem *dataItem = titleItem->child(titleItem->childCount() - 1); // Last child

    if (dataItem->columnCount() <= 1) {
        rootItem->removeChild(titleItem);
    }
    else {
        dataItem->removeData(data.path);
    }
}

void TimelineModel::updateData(const TimelineItem::ItemData &data)
{
    // If timeline not exists, create one
    TimelineItem *titleItem = timelineItem(data.timeline);
    TimelineItem *dataItem = titleItem->child(titleItem->childCount() - 1); // Last child

    dataItem->updateData(data);
}

QVariant TimelineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TimelineItem *item = static_cast<TimelineItem*>(index.internalPointer());

    auto data = item->data(index.column());
    QVariantList vl;
    vl << data.isTitle;
    vl << data.path;
    vl << data.timeline;
    vl << data.thumbArray;

    return QVariant(vl);
}

Qt::ItemFlags TimelineModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant TimelineModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return rootItem->data(section).path;
    }

    return QVariant();
}

QModelIndex TimelineModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TimelineItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TimelineItem*>(parent.internalPointer());

    TimelineItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TimelineModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TimelineItem *childItem = static_cast<TimelineItem*>(index.internalPointer());
    TimelineItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    // FIXME: parentItem of same childItem will change and cause crash
    // don't know why
    return createIndex(/*parentItem->row()*/0, 0, parentItem);
}

/*!
 * \brief TreeModel::rowCount
 * Return all row including title-row and data-row
 * \param parent
 * \return
 */
int TimelineModel::rowCount(const QModelIndex &parent) const
{
    TimelineItem *parentItem;
//    if (parent.column() > 0)
//        return 0;

    if (!parent.isValid()) {
        parentItem = rootItem;
    }
    else
        parentItem = static_cast<TimelineItem*>(parent.internalPointer());

    return parentItem->childCount();

//    // TODO read the timeline count from database directory
//    Q_UNUSED(parent)
//    // Every title row should contain single data row, so the row count
//    // is double of title row count
//    return rootItem->childCount() * 2;
}


int TimelineModel::columnCount(const QModelIndex &parent) const
{
    // TODO read image count from database directory
    if (parent.isValid()) {
        TimelineItem *titleItem = static_cast<TimelineItem*>(parent.internalPointer());
        return titleItem->child(0)->columnCount();
    }
    else
        return 1/*rootItem->columnCount()*/;
}

TimelineItem *TimelineModel::timelineItem(const QString &timeline)
{
    QMap<QString, QString> m; // For sort
    for (int row = 0; row < rootItem->childCount(); row ++) {
        // Title row should contain single column
        TimelineItem *ti = rootItem->child(row);
        if (ti->columnCount() == 1) {
            const QString tl = ti->data(0).timeline;
            m.insert(tl, timeline);
            if (tl == timeline) {
                return ti;
            }
        }
    }

    // Can not find specify timeline, create it
    //Get the position
    m.insert(timeline, timeline);
    int insertIndex = m.count() - m.keys().indexOf(timeline) - 1;

    TimelineItem *timelineItem = new TimelineItem(rootItem);
    rootItem->insertChild(insertIndex, timelineItem);
    TimelineItem::ItemData data;
    data.isTitle = true;
    data.timeline = timeline;
    timelineItem->appendData(data);

    TimelineItem *si = new TimelineItem(timelineItem);
    timelineItem->appendChild(si);

    return timelineItem;
}
