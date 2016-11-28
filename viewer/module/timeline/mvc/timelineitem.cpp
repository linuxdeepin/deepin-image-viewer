/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#include <QStringList>
#include <QDebug>

#include "timelineitem.h"

TimelineItem::TimelineItem(TimelineItem *parent)
{
    m_parentItem = parent;
}

TimelineItem::~TimelineItem()
{
    qDeleteAll(m_childItems);
}

void TimelineItem::appendData(const ItemData &data)
{
    m_datas.insert(data.path, data);
}

void TimelineItem::updateData(const TimelineItem::ItemData &data)
{
    // QMap will just update the value for the same key
    appendData(data);
}

void TimelineItem::removeData(const QString &path)
{
    m_datas.remove(path);
}

void TimelineItem::insertChild(int index, TimelineItem *child)
{
    m_childItems.insert(index, child);
}

void TimelineItem::appendChild(TimelineItem *item)
{
    m_childItems.append(item);
}

void TimelineItem::removeChild(TimelineItem *child)
{
    m_childItems.removeAll(child);
    delete child;
}

TimelineItem *TimelineItem::child(int row)
{
    return m_childItems.value(row);
}

int TimelineItem::childCount() const
{
    return m_childItems.count();
}

int TimelineItem::columnCount() const
{
    return m_datas.count();
}

TimelineItem::ItemData TimelineItem::data(int column) const
{
    // FIXME: It may cause crash
    if (column >= m_datas.count()) {
        return TimelineItem::ItemData();
    }
    else {
        return m_datas[m_datas.keys().at(column)];
    }
}

TimelineItem *TimelineItem::parentItem()
{
    return m_parentItem;
}

int TimelineItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<TimelineItem*>(this));

    return 0;
}
