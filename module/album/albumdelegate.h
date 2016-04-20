#ifndef ALBUMDELEGATE_H
#define ALBUMDELEGATE_H

#include <QObject>
#include <QDateTime>
#include <QStyledItemDelegate>

class AlbumDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit AlbumDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;
    void destroyEditor(QWidget* editor,
                       const QModelIndex& index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget* editor,
                       const QModelIndex& index) const Q_DECL_OVERRIDE;
    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const Q_DECL_OVERRIDE;
    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const Q_DECL_OVERRIDE;

private:
    void drawBG(const QStyleOptionViewItem &option, QPainter *painter) const;
    QPixmap getCompoundPixmap(const QDateTime &begin, const QDateTime &end,
                              const QPixmap &thumbnail) const;
};

#endif // ALBUMDELEGATE_H
