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
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;
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
    void drawTitle(const QStyleOptionViewItem &option,
                   const QModelIndex& index,
                   QPainter *painter) const;
    QPixmap getCompoundPixmap(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;

private:
    mutable QModelIndex m_editingIndex;
};

#endif // ALBUMDELEGATE_H
