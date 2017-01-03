#ifndef ALBUMDELEGATE_H
#define ALBUMDELEGATE_H

#include <QObject>
#include <QDateTime>
#include <QStyledItemDelegate>

class TDThumbnailThread;
class ThumbnailDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    struct ItemData {
        QString name;
        QString path;
        QPixmap thumbnail;
    };

    explicit ThumbnailDelegate(QObject *parent = nullptr);
    void setIsDataLocked(bool value);

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const Q_DECL_OVERRIDE;

signals:
    void thumbnailGenerated(const QString &path);

private:
    ItemData itemData(const QModelIndex &index) const;
    QPixmap thumbnail(const ThumbnailDelegate::ItemData &data) const;
    void startThumbnailThread(const QString &path) const;

private:
    bool m_isDataLocked;
    mutable QList<TDThumbnailThread *> m_threads;
};

#endif // ALBUMDELEGATE_H
