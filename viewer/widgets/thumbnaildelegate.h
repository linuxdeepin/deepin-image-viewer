#ifndef ALBUMDELEGATE_H
#define ALBUMDELEGATE_H

#include <QObject>
#include <QDateTime>
#include <QStyledItemDelegate>

class ThumbnailDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    struct ItemData {
        QString name;
        QString path;
        QPixmap thumbnail;
    };

    explicit ThumbnailDelegate(QObject *parent = nullptr);
    void clearPaintingList();
    const QStringList paintingPaths() const;

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const Q_DECL_OVERRIDE;

private:
    ItemData itemData(const QModelIndex &index) const;

private:
    mutable QStringList m_paintingPaths;
};

#endif // ALBUMDELEGATE_H
