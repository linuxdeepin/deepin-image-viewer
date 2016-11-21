#ifndef TIMELINEITEM_H
#define TIMELINEITEM_H

#include <QList>
#include <QVariant>
#include <QPixmap>

class TimelineItem
{
public:
    struct ItemData {
        bool isTitle;
        QString path;
        QString timeline;
        QPixmap thumbnail;
    };

    explicit TimelineItem(TimelineItem *parentItem = 0);
    ~TimelineItem();

    void appendData(const ItemData &data);
    void updateData(const ItemData &data);
    void removeData(const QString &path);


    void insertChild(int index, TimelineItem *child);
    void appendChild(TimelineItem *child);
    void removeChild(TimelineItem *child);

    TimelineItem *child(int row);
    int childCount() const;
    int columnCount() const;
    ItemData data(int column) const;
    int row() const;
    TimelineItem *parentItem();

private:
    QList<TimelineItem*> m_childItems;
    QMap<QString, ItemData> m_datas;
    TimelineItem *m_parentItem;
};

#endif // TIMELINEITEM_H
