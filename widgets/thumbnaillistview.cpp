#include "thumbnaillistview.h"
#include <QPaintEvent>
#include <QEvent>
#include <QDebug>
#include <QFile>
#include <QTimer>

const int ITEM_SPACING = 4;
ThumbnailListView::ThumbnailListView(QWidget *parent)
    : QListView(parent)
{
    initStyleSheet();

    setMovement(QListView::Free);
    setFrameStyle(QFrame::NoFrame);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setFlow(QListView::LeftToRight);
    setSelectionMode(QAbstractItemView::SingleSelection);
//    setUniformItemSizes(true);
    setSpacing(ITEM_SPACING);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragEnabled(false);

    viewport()->installEventFilter(this);

    // For expand all items
    QMetaObject::invokeMethod(this, "fixedViewPortSize", Qt::QueuedConnection);
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
        emit mousePress();
        setSelectionMode(QAbstractItemView::SingleSelection);
    }
    QListView::mousePressEvent(e);
}

void ThumbnailListView::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}

void ThumbnailListView::fixedViewPortSize()
{
    int horizontalMargin = contentsMargins().left() + contentsMargins().right();
    int verticalMargin = contentsMargins().top() + contentsMargins().bottom();
    if (width() - horizontalMargin == contentsRect().width()
            && height() - verticalMargin == contentsSize().height())
        return;

    if (contentsSize().isValid()) {
        //    setFixedWidth(contentsRect().width());
        setFixedHeight(contentsSize().height());
    }
}

void ThumbnailListView::initStyleSheet()
{
    QFile sf(":/qss/resources/qss/ThumbnailListView.qss");
    if (!sf.open(QIODevice::ReadOnly)) {
        qWarning() << "Open style-sheet file error:" << sf.errorString();
        return;
    }

    this->setStyleSheet(QString(sf.readAll()));
    sf.close();
}
