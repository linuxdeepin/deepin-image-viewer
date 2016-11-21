#ifndef TIMELINE_DELEGATE_H
#define TIMELINE_DELEGATE_H

#include <QObject>
#include <QDateTime>
#include <QStyledItemDelegate>
#include "timelineitem.h"

class TimelineDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit TimelineDelegate(QObject *parent = NULL);
    void clearPaintingList();
    const QStringList paintingPaths() const;

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const Q_DECL_OVERRIDE;

private:
    TimelineItem::ItemData itemData(const QModelIndex &index) const;

private:
    mutable QStringList m_paintingPaths;
};

#endif // TIMELINE_DELEGATE_H
