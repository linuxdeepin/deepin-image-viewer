#ifndef ALBUMDELEGATE_H
#define ALBUMDELEGATE_H

#include <QObject>
#include <QDateTime>
#include <QStyledItemDelegate>

class ThumbnailDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit ThumbnailDelegate(QObject *parent = nullptr);
    void clearPaintingList();
    const QModelIndexList paintingIndexList();
    const QStringList paintingNameList();

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const Q_DECL_OVERRIDE;

private:
    void renderThumbnail(const QString &name, QPixmap &thumbnail) const;

private:
    mutable QModelIndexList m_indexs;
    mutable QStringList m_names;
};

#endif // ALBUMDELEGATE_H
