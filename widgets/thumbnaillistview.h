#ifndef THUMBNAILLISTVIEW_H
#define THUMBNAILLISTVIEW_H

#include <QListView>
#include <QFutureWatcher>

class QStandardItemModel;
class ThumbnailDelegate;
class QTimer;
class ThumbnailListView : public QListView
{
    Q_OBJECT
public:
    struct ItemInfo {
        QString name = QString();
        QString path = QString();
        bool tickable = false;
    };

    explicit ThumbnailListView(QWidget *parent = 0);
    ~ThumbnailListView();
    void setMultiSelection(bool multiple);
    void clearData();
    void updateViewPortSize();
    void updateThumbnail(const QString &name);
    void setIconSize(const QSize &size);
    void setTickable(bool v);
    void insertItem(const ItemInfo &info);
    bool removeItem(const QString &name);
    bool contain(const QModelIndex &index) const;
    bool isMultiSelection() const;
    int indexOf(const QString &name);
    int count() const;
    int hOffset() const;
    const ItemInfo itemInfo(const QModelIndex &index);
    const QList<ItemInfo> ItemInfos();
    const QList<ItemInfo> selectedItemInfos();

signals:
    void singleClicked(QMouseEvent *e);

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    int horizontalOffset() const Q_DECL_OVERRIDE;

private slots:
    void onThumbnailResultReady(int index);
    void fixedViewPortSize(bool proactive = false);
    int maxColumn() const;
    int contentsHMargin() const;
    int contentsVMargin() const;

private:
    void initThumbnailTimer();
    const QVariantList getVariantList(const ItemInfo &info);

private:
    // For high-quality-thumbnail generate
    QModelIndexList m_paintedIndexs;
    QStringList m_thumbnailCache;
    QFutureWatcher<QString> m_thumbnailWatcher;

    QStandardItemModel *m_model;
    ThumbnailDelegate *m_delegate;
    bool m_multiple;
};

#endif // THUMBNAILLISTVIEW_H
