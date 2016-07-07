#include "thumbnaillistview.h"
#include "utils/baseutils.h"
#include <QPaintEvent>
#include <QEvent>
#include <QDebug>
#include <QFile>
#include <QTimer>

namespace {

const int ITEM_SPACING = 4;

}  //namespace


ThumbnailListView::ThumbnailListView(QWidget *parent)
    : QListView(parent)
{
    setStyleSheet(utils::base::getFileContent(
                      ":/qss/resources/qss/ThumbnailListView.qss"));

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

void ThumbnailListView::updateViewPortSize()
{
    // For expand all items
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, [=] {
        fixedViewPortSize(true);
        t->deleteLater();
    });
    t->start(100);
}

bool ThumbnailListView::eventFilter(QObject *obj, QEvent *event)
{
    if ( obj == viewport() && event->type() == QEvent::Paint) {
        fixedViewPortSize();
    }

    return false;
}

void ThumbnailListView::mousePressEvent(QMouseEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        setSelectionMode(QAbstractItemView::MultiSelection);
    }
    else if (e->button() == Qt::LeftButton){
        setSelectionMode(QAbstractItemView::SingleSelection);
    }
    emit mousePress(e);
    QListView::mousePressEvent(e);
}

void ThumbnailListView::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

void ThumbnailListView::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}

void ThumbnailListView::fixedViewPortSize(bool proactive)
{
    int horizontalMargin = contentsMargins().left() + contentsMargins().right();
    int verticalMargin = contentsMargins().top() + contentsMargins().bottom();
    if (! proactive && width() - horizontalMargin == contentsRect().width()
            && height() - verticalMargin == contentsSize().height())
        return;

    if (contentsSize().isValid()) {
        //    setFixedWidth(contentsRect().width());
        setFixedHeight(contentsSize().height());
    }
}
