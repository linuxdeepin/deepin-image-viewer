#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

#include <QAbstractItemView>
#include <QFutureWatcher>

class ScrollBar;
class TimelineView : public QAbstractItemView
{
    Q_OBJECT
public:
    struct IndexRect {
        QModelIndex index;
        QRect rect;
    };

    explicit TimelineView(QWidget *parent = 0);

    inline void scrollContentsBy(int dx, int dy) Q_DECL_OVERRIDE {
        scrollDirtyRegion(dx, dy);
        viewport()->scroll(dx, dy);
    }

    QSize viewportSizeHint() const Q_DECL_OVERRIDE;
    QModelIndex indexAt(const QPoint &point) const Q_DECL_OVERRIDE;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) Q_DECL_OVERRIDE;
    QRect visualRect(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int horizontalOffset() const Q_DECL_OVERRIDE;
    bool isIndexHidden(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) Q_DECL_OVERRIDE;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) Q_DECL_OVERRIDE;
    int verticalOffset() const Q_DECL_OVERRIDE;
    QRegion visualRegionForSelection(const QItemSelection &selection) const Q_DECL_OVERRIDE;

    const QModelIndexList paintingIndexs() const;

    int vItemSpacing() const;
    void setVItemSpacing(int vItemSpacing);

    int hItemSpacing() const;
    void setHItemSpacing(int hItemSpacing);

    int bottomMargin() const;
    void setBottomMargin(int bottomMargin);

    int topMargin() const;
    void setTopMargin(int topMargin);

    int itemSize() const;
    void setItemSize(int size);

    int titleHeight() const;
    void setTitleHeight(int height);

    bool isScrolling();

    void updateScrollbarRange(bool keepAnchor = false);
    void updateView();

signals:
    void changeItemSize(bool increase);
    void paintingIndexsChanged();
    void showMenu();

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

private:
    QModelIndexList visualIndexs() const;
    int maxColumnCount() const;
    QModelIndex movePageUp();
    QModelIndex movePageDown();
    void updateVerticalScrollbar();
    void updateVisualRects();

private slots:
    void onScrolled();

private:
    QColor m_backgroundColor;
    bool m_needReAnchor;
    int m_itemSize;
    int m_titleHeight;
    int m_hItemSpacing;
    int m_vItemSpacing;
    int m_bottomMargin;
    int m_topMargin;

    CursorAction m_cursorAction;
    ScrollBar *m_sb = nullptr;
    QMutex m_mutex;
    QRect m_selectionRect;  // 为了绘制划定选中的方框
    QMap<QModelIndex, QRect> m_irMap;  // 为了知道index的情况下加快查找速度
    QList<IndexRect> m_irList;  // 为了二分法加快查找速度
    QModelIndexList m_paintingIndexs;  // 通常，可视区域都不会改变，在点击等操作需要重绘时也没必要重新计算可视区域的indexs
};

#endif // TIMELINEVIEW_H
