#include "thumbnaillistview.h"
#include <QPaintEvent>
#include <QEvent>
#include <QDebug>

const int ITEM_SPACING = 10;
ThumbnailListView::ThumbnailListView(QWidget *parent)
    : QListView(parent)
{
    setMovement(QListView::Free);
    setFrameStyle(QFrame::NoFrame);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setFlow(QListView::LeftToRight);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setUniformItemSizes(true);
    setSpacing(ITEM_SPACING);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragEnabled(false);

    viewport()->installEventFilter(this);
}

bool ThumbnailListView::eventFilter(QObject *obj, QEvent *event)
{
    if ( obj == viewport() && event->type() == QEvent::Paint) {
        fixedViewPortSize();
    }

    return false;
}

void ThumbnailListView::fixedViewPortSize()
{
    int horizontalMargin = contentsMargins().left() + contentsMargins().right();
    int verticalMargin = contentsMargins().top() + contentsMargins().bottom();
    if (width() - horizontalMargin == contentsRect().width()
            && height() - verticalMargin == contentsSize().height())
        return;

//    setFixedWidth(contentsRect().width());
    setFixedHeight(contentsSize().height());
}

