#ifndef TIMELINE_DELEGATE_H
#define TIMELINE_DELEGATE_H

#include <QObject>
#include <QDateTime>
#include <QStyledItemDelegate>

#include "timelineitem.h"
#include "controller/viewerthememanager.h"

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
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
private:
    QColor m_borderColor;
    QColor m_dateColor;
    QColor m_seperatorColor;

    mutable QStringList m_paintingPaths;
};

#endif // TIMELINE_DELEGATE_H
