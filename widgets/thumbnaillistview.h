#ifndef THUMBNAILLISTVIEW_H
#define THUMBNAILLISTVIEW_H

#include <QListView>

class QStandardItemModel;
class ThumbnailListView : public QListView
{
    Q_OBJECT
public:
    struct ItemInfo {
        QString name = QString();
        QString path = QString();
        bool ticked = false;
    };

    explicit ThumbnailListView(QWidget *parent = 0);
    void clearData();
    void updateViewPortSize();
    void updateThumbnail(const QString &name);
    void setIconSize(const QSize &size);
    void insertItem(const ItemInfo &info);
    bool removeItem(const QString &name);
    bool contain(const QModelIndex &index) const;
    int indexOf(const QString &name);
    int count() const;
    int hOffset() const;
    const ItemInfo itemInfo(const QModelIndex &index);
    const QList<ItemInfo> ItemInfos();
    const QList<ItemInfo> selectedItemInfos();

signals:
    void mousePress(QMouseEvent *e);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    int horizontalOffset() const Q_DECL_OVERRIDE;

private slots:
    void fixedViewPortSize(bool proactive = false);
    int maxColumn() const;
    int contentsHMargin() const;
    int contentsVMargin() const;

private:
    const QPixmap getThumbnailByName(const QString &name) const;
    const QVariantList getVariantList(const ItemInfo &info);

private:
    QStandardItemModel *m_model;
};

#endif // THUMBNAILLISTVIEW_H
