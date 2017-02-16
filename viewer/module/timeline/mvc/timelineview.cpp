#include "application.h"
#include "timelineview.h"
#include "timelineitem.h"
#include "timelinemodel.h"
#include "controller/dbmanager.h"
#include "controller/viewerthememanager.h"

#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "widgets/scrollbar.h"
#include <QDebug>
#include <QFileInfo>
#include <QHeaderView>
#include <QPaintEvent>
#include <QPainter>
#include <QtConcurrent>
#include <QtMath>

#include "denhancedwidget.h"

namespace {

const int LEFT_RIGHT_MARGIN = 24;
const QColor DARK_BACKGROUND_COLOR = QColor("#202020");
const QColor LIGHT_BACKGROUND_COLOR = QColor("#FFFFFF");
const QColor BORDER_COLOR_SELECTED = QColor("#01bdff");
const int TOP_TOOLBAR_THEIGHT = 40;
const int BOTTOM_TOOLBAR_HEIGHT = 22;
}  // namespace

TimelineView::TimelineView(QWidget *parent)
    : QAbstractItemView(parent)
    , m_needReAnchor(false)
    , m_itemSize(96)
    , m_titleHeight(44)
    , m_hItemSpacing(4)
    , m_vItemSpacing(4)
    , m_bottomMargin(30)
    , m_topMargin(44)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Scrollbar
    m_sb = new ScrollBar(this);
    DEnhancedWidget *enhanced_scrollbar = new DEnhancedWidget(m_sb, m_sb);
    connect(enhanced_scrollbar, &DEnhancedWidget::heightChanged, m_sb, [this] {
        m_sb->move(m_sb->x(), TOP_TOOLBAR_THEIGHT);
        m_sb->resize(m_sb->width(), m_sb->parentWidget()->height()
                     - TOP_TOOLBAR_THEIGHT - BOTTOM_TOOLBAR_HEIGHT);
    });
    setVerticalScrollBar(m_sb);
    m_sb->setContextMenuPolicy(Qt::PreventContextMenu);
    connect(m_sb, &QScrollBar::valueChanged, this, &TimelineView::onScrolled);
    // Theme
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::AppTheme::Dark) {
        m_backgroundColor = DARK_BACKGROUND_COLOR;
    } else {
        m_backgroundColor = LIGHT_BACKGROUND_COLOR;
    }

    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged, this, [=](
            ViewerThemeManager::AppTheme theme){
        if (theme == ViewerThemeManager::AppTheme::Dark) {
            m_backgroundColor = DARK_BACKGROUND_COLOR;
        } else {
            m_backgroundColor = LIGHT_BACKGROUND_COLOR;
        }
        QTimer::singleShot(500, [=]{
            emit enhanced_scrollbar->heightChanged(0);
        });
    });
}

void TimelineView::setItemSize(int size)
{
    if (size <=0) {
        return;
    }
    m_itemSize = size;
    updateVerticalScrollbar();
    updateVisualRects();
    this->update();
}

void TimelineView::setTitleHeight(int height)
{
    m_titleHeight = height;
    updateVerticalScrollbar();
    updateVisualRects();
    this->update();
}

bool TimelineView::isScrolling()
{
    return m_sb->isScrolling();
}

void TimelineView::updateScrollbarRange(bool keepAnchor)
{
    if (keepAnchor && m_sb->isScrolling())
        return;

    if (keepAnchor)
        m_needReAnchor = true;
    updateVerticalScrollbar();
}

void TimelineView::updateView()
{
    updateVisualRects();
    this->update();
}

void TimelineView::keyPressEvent(QKeyEvent *e)
{
    QAbstractItemView::keyPressEvent(e);

    if (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_A) {
        QItemSelection selection;
        for (auto i : m_irList) {
            QItemSelection s;
            s.select(i.index, i.index);
            selection.merge(s, QItemSelectionModel::Select);
        }
        selectionModel()->select(selection, QItemSelectionModel::Select);
    }
    this->update();
}

QSize TimelineView::viewportSizeHint() const
{
    int titleRowCount = model()->rowCount();
    int viewportHeight = 0;
    for (int trc = 0; trc < titleRowCount; trc ++) {
        QModelIndex titleIndex = model()->index(trc, 0);
        // Get height of data-row
        int colmunCount = model()->columnCount(titleIndex);
        int subRowCount = qCeil(1.0 * colmunCount / maxColumnCount());

        viewportHeight += m_titleHeight + m_vItemSpacing
                + subRowCount * (m_itemSize + m_vItemSpacing);
    }

    return QSize(this->width(), viewportHeight + m_topMargin + m_bottomMargin);
}

QModelIndex TimelineView::indexAt(const QPoint &point) const
{
    for (auto i : m_irList) {
        if (visualRect(i.index).contains(point)) {
            return i.index;
        }
    }

    return QModelIndex();
}

void TimelineView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    const QRect rect = visualRect(index);
    if (hint == EnsureVisible && ! viewport()->rect().contains(rect)) {
        int viewportHeight = viewportSizeHint().height();
        int scrollLen = verticalScrollBar()->maximum()
                - verticalScrollBar()->minimum()
                + verticalScrollBar()->pageStep();
        int scrollValue = (1.0 * rect.y() + verticalOffset()) / viewportHeight
                * scrollLen;

        if (m_cursorAction == MoveDown
                || m_cursorAction == MovePageDown) {
            scrollValue -= this->height() - m_itemSize - m_vItemSpacing;
        }

        verticalScrollBar()->setValue(scrollValue);
    }

}

QRect TimelineView::visualRect(const QModelIndex &index) const
{
    QRect r = m_irMap.value(index);
    r.setY(r.y() - verticalOffset());
    r.setHeight(r.height() - verticalOffset());
    return r;

}

int TimelineView::horizontalOffset() const
{
    const int contentWidth = (m_itemSize + m_hItemSpacing) * maxColumnCount();
    return (this->width() - contentWidth) / 2;
}

bool TimelineView::isIndexHidden(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return false;
}

QModelIndex TimelineView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                 Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    if (! model())
        return QModelIndex();

    m_cursorAction = cursorAction;
    auto indexs = selectionModel()->selectedIndexes();
    QModelIndex current = selectionModel()->currentIndex();
    if (indexs.length() < 1) {
        current = indexAt(QPoint(m_itemSize / 2, m_itemSize / 2));
    }
    else {
        if (modifiers == Qt::ShiftModifier
                && selectionModel()->currentIndex().isValid()) {
            current = selectionModel()->currentIndex();
        }
        else {
            switch (cursorAction) {
            case MoveRight:
            case MoveDown:
                current = indexs.last();
                break;
            default:
                current = indexs.first();
                break;
            }
        }
    }

    QPoint ccp = visualRect(current).center();
    switch (cursorAction) {
    case MoveLeft:
         ccp.setX(ccp.x() - m_itemSize - m_hItemSpacing);
         break;
    case MoveRight:
        ccp.setX(ccp.x() + m_itemSize + m_hItemSpacing);
        break;
    case MoveUp:
        ccp.setY(ccp.y() - m_itemSize - m_vItemSpacing);
        break;
    case MoveDown:
        ccp.setY(ccp.y() + m_itemSize + m_vItemSpacing + m_titleHeight);
        break;
    case MovePageUp:
        return movePageUp();
    case MovePageDown:
        return movePageDown();
    default:
        break;
    }
    QModelIndex ni = indexAt(ccp);
    if (ni.isValid())
        return ni;
    else
        return current;
}

void TimelineView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
    if (flags & QItemSelectionModel::Clear) {
        selectionModel()->clear();
    }

    // For mouse draging border
    QRect vr;
    if (flags & QItemSelectionModel::Current) {
        int x = rect.width() < 0 ? rect.x() + rect.width() : rect.x();
        int y = rect.height() < 0 ? rect.y() + rect.height() : rect.y();
        vr = QRect(x, y, qAbs(rect.width()), qAbs(rect.height()));
    }
    else {
        vr = QRect();
    }

    // Do not draw drag border if Qt::ShiftModifier is pressed
    if (flags == QItemSelectionModel::SelectCurrent) {
        m_selectionRect = QRect();
    }
    else {
        m_selectionRect = vr;
    }

    if (! vr.isEmpty()) {
        QItemSelection selection;
        for (auto index : m_paintingIndexs) {
            if (! visualRect(index).intersected(vr).isEmpty()) {
                QItemSelection s;
                s.select(index, index);
                selection.merge(s, flags);
            }
        }
        selectionModel()->select(selection, flags);
    }
    else {
        QModelIndex index = indexAt(rect.topLeft());
        selectionModel()->select(index, flags);
        scrollTo(index);
    }
    this->update();
}

int TimelineView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

QRegion TimelineView::visualRegionForSelection(const QItemSelection &selection) const
{
    Q_UNUSED(selection)
    return QRegion();
}

const QModelIndexList TimelineView::paintingIndexs() const
{
    return m_paintingIndexs;
}

void TimelineView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(viewport());

    QModelIndexList sis = selectionModel()->selectedIndexes();
    QStyleOptionViewItem option = viewOptions();
    painter.fillRect(this->rect(), m_backgroundColor);

    for (auto index : m_paintingIndexs) {
        if (sis.contains(index)) {
            option.state |= QStyle::State_Selected;
        }
        else {
            option.state = QStyle::State_Enabled;
        }
        option.rect = visualRect(index);
        painter.save();
        itemDelegate()->paint(&painter, option, index);
        painter.restore();
    }

    // Draw selection box
    QPainterPath bp;
    bp.addRect(m_selectionRect);
    QPen sp(BORDER_COLOR_SELECTED, 1);
    painter.setPen(sp);
    painter.drawPath(bp);
}

void TimelineView::resizeEvent(QResizeEvent *event)
{
    QAbstractItemView::resizeEvent(event);

    updateVerticalScrollbar();
    updateVisualRects();
    this->update();
}

void TimelineView::wheelEvent(QWheelEvent *e)
{
    if (m_needReAnchor && ! m_paintingIndexs.isEmpty()) {
        m_needReAnchor = false;
        QModelIndex index = m_paintingIndexs.first();
        updateVisualRects();
        scrollTo(index);
        return;
    }

    QScrollBar *sb = verticalScrollBar();
    if (e->orientation() == Qt::Vertical &&
            sb->value() <= sb->maximum() &&
            sb->value() >= sb->minimum()) {
        if (e->modifiers() == Qt::ControlModifier) {
            emit changeItemSize(e->delta() > 0);
        }
        else {
            QApplication::sendEvent(sb, e);
        }
    }
    else {
        QApplication::sendEvent(horizontalScrollBar(), e);
    }
}

void TimelineView::mousePressEvent(QMouseEvent *e)
{
    ScrollBar *sb = dynamic_cast<ScrollBar *>(verticalScrollBar());
    if (sb) {
        sb->stopScroll();
    }

    QModelIndex index = indexAt(e->pos());
    if (index.isValid() && e->button() == Qt::RightButton) {
        bool needCallParent = selectedIndexes().length() != 1;

        if (selectedIndexes().length() <= 1) {
            selectionModel()->clear();
            selectionModel()->select(index, QItemSelectionModel::Select);
        }

        QVariantList datas = model()->data(selectedIndexes().at(0),
                                           Qt::DisplayRole).toList();
        //the selected index isn't title
        if (!datas[0].toBool() || selectedIndexes().length() >= 2)
            emit showMenu();

        if (! needCallParent)
            return;
    }

    QAbstractItemView::mousePressEvent(e);
}

/*!
 * \brief TimelineView::intersectedIndexs
 * \param rect
 * \return
 * 返回与Rect相交的所有index
 */
QModelIndexList TimelineView::visualIndexs() const
{
    QList<QModelIndex> indexs;
    if (m_irList.length() < 10) {
        QModelIndexList ils;
        for (auto i : m_irList) {
            ils << i.index;
        }
        return ils;
    }

    // 二分法
    int startIndex = m_irList.length() - 1;
    int endIndex = 0;
    int middleIndex = 1;
    while (middleIndex > 0 && middleIndex < m_irList.length() - 2) {
        middleIndex = (startIndex + endIndex) / 2;
        QModelIndex mi = m_irList[middleIndex].index;
        QRect mr = visualRect(mi);
        if (this->rect().intersected(mr).isEmpty()) {
            // 中点在可视区域上方
            if (mr.y() < y()) {
                endIndex = middleIndex;
            }
            // 中点在可视区域下方
            else {
                startIndex = middleIndex;
            }
        }
        // 如果某个Index对应的Rect跟可视区域有交集，则从该Index开始向两端扩展绘制
        else {
            // 向前
            int forwardIndex = middleIndex;
            while (forwardIndex >= 0 &&
                   ! this->rect().intersected(visualRect(m_irList[forwardIndex].index)).isEmpty()) {
                QModelIndex fi = m_irList[forwardIndex].index;
                indexs.insert(0, fi);
                forwardIndex --;
            }

            // 向后
            int backwardIndex = middleIndex;
            while (backwardIndex < m_irList.length() &&
                   ! this->rect().intersected(visualRect(m_irList[backwardIndex].index)).isEmpty()) {
                QModelIndex bi = m_irList[backwardIndex].index;
                indexs.append(bi);
                backwardIndex ++;
            }

            break;
        }
    }

    return indexs;
}

int TimelineView::maxColumnCount() const
{
    int c = (width() - LEFT_RIGHT_MARGIN*2) / (m_itemSize + m_hItemSpacing);
    if (c <=0)
        return 1;
    else
        return c;
}

QModelIndex TimelineView::movePageUp()
{
    auto indexs = selectionModel()->selectedIndexes();
    if (indexs.isEmpty()) {
        indexs = m_paintingIndexs;
        if (indexs.isEmpty()) {
            return QModelIndex();
        }
    }
    QModelIndex index = indexs.first();
    QPoint p = visualRect(index).topLeft();
    p.setY(p.y()  - verticalScrollBar()->pageStep() + m_itemSize + m_vItemSpacing);
    QModelIndex ti = indexAt(p);
    if (! ti.isValid()) {
        // Return the first ThumbnailItem
        QModelIndex titleIndex = model()->index(0, 0);
        return model()->index(0, 0, titleIndex);
    }
    else {
        return ti;
    }
}

QModelIndex TimelineView::movePageDown()
{
    auto indexs = selectionModel()->selectedIndexes();
    if (indexs.isEmpty()) {
        indexs = m_paintingIndexs;
        if (indexs.isEmpty()) {
            return QModelIndex();
        }
    }
    QModelIndex index = indexs.first();
    QPoint p = visualRect(index).topLeft();
    p.setY(p.y()  + verticalScrollBar()->pageStep()*2 - m_itemSize - m_vItemSpacing);
    QModelIndex ti = indexAt(p);
    if (! ti.isValid()) {
        // Return the last ThumbnailItem
        QModelIndex titleIndex = model()->index(model()->rowCount() - 1, 0);
        return model()->index(0, model()->columnCount(titleIndex) - 1, titleIndex);
    }
    else {
        return ti;
    }
}

void TimelineView::updateVerticalScrollbar()
{
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0, viewportSizeHint().height() - viewport()->height());
}

/*!
 * \brief TimelineView::updateVisualRects
 * The VisualRects should update after resized, item-changed or iconsize-changed
 * Note: it sholud consider about verticalOffset()
 */
void TimelineView::updateVisualRects()
{
    QMutexLocker locker(&m_mutex);

    m_irList.clear();
    m_irMap.clear();

    int titleRowCount = model()->rowCount();

    int viewportHeight = /*-verticalOffset() + */m_topMargin;
    for (int trc = 0; trc < titleRowCount; trc ++) {
        // Title row
        QModelIndex titleIndex = model()->index(trc, 0);
        QRect tR(horizontalOffset(), viewportHeight,
                 this->width() - horizontalOffset() * 2, m_titleHeight);
        IndexRect ir;
        ir.index = titleIndex;
        ir.rect = tR;
        m_irList.append(ir);
        m_irMap.insert(titleIndex, tR);
        viewportHeight += m_titleHeight + m_vItemSpacing;

        // Data row
        const int columnCount = model()->columnCount(titleIndex);
        for (int c = 0; c < columnCount; c++) {
            QModelIndex dataIndex = titleIndex.child(0, c);
            int subRow = c / maxColumnCount();
            int subColumn = c % maxColumnCount();
            int y = viewportHeight
                    + subRow * (m_itemSize + m_vItemSpacing);
            int x = horizontalOffset()
                    + subColumn * (m_itemSize + m_hItemSpacing);
            QRect dR(x, y, m_itemSize, m_itemSize);
            IndexRect ir;
            ir.index = dataIndex;
            ir.rect = dR;
            m_irList.append(ir);
            m_irMap.insert(dataIndex, dR);
        }
        // Append the height of data-row to viewportHeight
        int subRowCount = qCeil(1.0 * columnCount / maxColumnCount());
        viewportHeight += subRowCount * (m_itemSize + m_vItemSpacing);
    }

    m_paintingIndexs = visualIndexs();
    emit paintingIndexsChanged();
}

void TimelineView::onScrolled()
{
    updateVisualRects();
}

void TimelineView::mouseReleaseEvent(QMouseEvent *e)
{
    m_selectionRect = QRect();
    this->update();
    QAbstractItemView::mouseReleaseEvent(e);
}

int TimelineView::titleHeight() const
{
    return m_titleHeight;
}

int TimelineView::itemSize() const
{
    return m_itemSize;
}

int TimelineView::topMargin() const
{
    return m_topMargin;
}

void TimelineView::setTopMargin(int topMargin)
{
    m_topMargin = topMargin;
    updateVisualRects();
}

int TimelineView::bottomMargin() const
{
    return m_bottomMargin;
}

void TimelineView::setBottomMargin(int bottomMargin)
{
    m_bottomMargin = bottomMargin;
    updateVisualRects();
}

int TimelineView::vItemSpacing() const
{
    return m_vItemSpacing;
}

void TimelineView::setVItemSpacing(int vItemSpacing)
{
    m_vItemSpacing = vItemSpacing;
    updateVisualRects();
}

int TimelineView::hItemSpacing() const
{
    return m_hItemSpacing;
}

void TimelineView::setHItemSpacing(int hItemSpacing)
{
    m_hItemSpacing = hItemSpacing;
    updateVisualRects();
}
