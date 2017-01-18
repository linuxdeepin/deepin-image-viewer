#include "timelineframe.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/importer.h"
#include "controller/signalmanager.h"
#include "mvc/timelinemodel.h"
#include "mvc/timelinedelegate.h"
#include "mvc/timelineitem.h"
#include "mvc/timelineview.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMutex>
#include <QScrollBar>
#include <QtConcurrent>

namespace {

const int TOP_TOOLBAR_HEIGHT = 39;
const int BOTTOM_TOOLBAR_HEIGHT = 22;
const QString SCANPATHS_GROUP = "SCANPATHSGROUP";
const QString SCANPATHS_KEY = "SCANPATHSKEY";

class LoadThread : public QThread
{
    Q_OBJECT
public:
    explicit LoadThread(const DBImgInfoList &infos);

protected:
    void run() Q_DECL_OVERRIDE;

signals:
    void ready(TimelineItem::ItemData data);

private:
    const QStringList scanpathsHash();

private:
    DBImgInfoList m_infos;
};

}   // namespace

class TopTimelineTip : public QLabel
{
public:
    explicit TopTimelineTip(QWidget *parent) : QLabel(parent) {
        setObjectName("TopTimelineTip");
        setStyleSheet(parent->styleSheet());
        setFixedHeight(24);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    void setLeftMargin(int v) {setContentsMargins(v, 0, 0, 0);}
};

TimelineFrame::TimelineFrame(QWidget *parent)
    : QFrame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    initView();
    layout->addWidget(m_view);

    // Top-Tip
    initTopTip();
    initItems();
    initConnection();
}

void TimelineFrame::clearSelection()
{
    m_view->clearSelection();
}

void TimelineFrame::selectAll()
{
    m_view->selectAll();
}

void TimelineFrame::setIconSize(int size)
{
    m_view->setItemSize(size);
}

void TimelineFrame::updateThumbnail(const QString &path)
{
    using namespace utils::image;
    using namespace utils::base;
    auto info = dApp->dbM->getInfoByPath(path);
    TimelineItem::ItemData data;
    data.isTitle = false;
    data.path = path;

    QBuffer inBuffer( &data.thumbArray );
    inBuffer.open( QIODevice::WriteOnly );
    // write inPixmap into inByteArray
    cutSquareImage(getThumbnail(data.path, true)).save( &inBuffer, "JPG" );
    data.timeline = timeToString(info.time, true);

    m_model.updateData(data);
    m_view->update();
}

void TimelineFrame::updateScrollRange()
{
    m_view->updateScrollbarRange();
    // FIXME: the value of m_view's verticzalScrollBar won't change event if
    // it's range is changed, if the scrollbar's slider is at the top, the
    // painting_indexs won't be update, so need to scroll it here to force update
    const int vv = m_view->verticalScrollBar()->value();
    m_view->verticalScrollBar()->setValue(vv + 1);
}

bool TimelineFrame::isEmpty() const
{
    return false;
}

const QString TimelineFrame::currentMonth() const
{
    using namespace utils::base;
    if (m_view->paintingIndexs().length() > 0) {
        QPoint p(m_view->horizontalOffset() + m_view->itemSize() / 2, m_view->itemSize() / 2);
        int count = 0;
        while (count <= 3) {
            QModelIndex index = m_view->indexAt(p);
            QVariantList datas = m_model.data(index, Qt::DisplayRole).toList();
            if (datas.count() == 4) { // There is 4 field data inside TimelineData
                return datas[2].toString();
            }
            else {
                // The p may contain by title-index
                p.setY(p.y() + m_view->titleHeight());
            }
            count ++;
        };
    }

    return QString();
}

const QStringList TimelineFrame::selectedPaths() const
{
    QStringList sPaths;
    QModelIndexList il = m_view->selectionModel()->selectedIndexes();
    for (QModelIndex index : il) {
        QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();
        // There is 4 field data inside TimelineData
        if (datas.count() == 4 && !datas[0].toBool()) {
            sPaths << datas[1].toString();
        }
    }
    return sPaths;
}

void TimelineFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    m_tip->setFixedWidth(e->size().width());
}

void TimelineFrame::initConnection()
{
    // Item append and remove
    connect(dApp->importer, &Importer::imported, this, [=] {
        updateScrollRange();
        // Call initItems to reinsert those datas which miss by user scroll view
        initItems();
        TIMER_SINGLESHOT(1000, {m_view->updateView();}, this)
    });
    connect(dApp->signalM, &SignalManager::imagesInserted,
            this, [=] (const DBImgInfoList infos){
        LoadThread *t = new LoadThread(infos);
        connect(t, &LoadThread::ready,
                this, &TimelineFrame::insertItems, Qt::DirectConnection);
        connect(t, &LoadThread::finished, this, [=] {
            t->deleteLater();
            m_infos << infos;
        }, Qt::DirectConnection);
        t->start();
    });
    connect(dApp->signalM, &SignalManager::imagesRemoved,
            this, &TimelineFrame::removeItems);
}

void TimelineFrame::initView()
{
    m_view = new TimelineView(this);
    TimelineDelegate *delegate = new TimelineDelegate();
    connect(delegate, &TimelineDelegate::thumbnailGenerated,
            this, &TimelineFrame::updateThumbnail);
    m_view->setItemDelegate(delegate);
    m_view->setModel(&m_model);
    connect(m_view, &TimelineView::doubleClicked, this, [=] (const QModelIndex &index){
        QVariantList datas = index.model()->data(index, Qt::DisplayRole).toList();
        if (datas.count() == 4) { // There is 4 field data inside TimelineData
            const QString path = datas[1].toString();
            if (!path.isEmpty())
                emit viewImage(path, dApp->dbM->getAllPaths());
        }
    });
    connect(m_view, &TimelineView::showMenu, this, &TimelineFrame::showMenu);
    connect(m_view, &TimelineView::changeItemSize,
            this, &TimelineFrame::changeItemSize);

    // When user use cursor to drag to select area
    // The signal will be triggered frequently
    // Use timer to reset it
    QTimer *t = new QTimer(this);
    t->setSingleShot(true);
    connect(t, &QTimer::timeout, this, [=] {
        emit selectIndexChanged(m_view->selectionModel()->currentIndex());
    });
    connect(m_view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [=] {
        t->start(200);
    });
}

void TimelineFrame::initTopTip()
{
    // Top-Tips
    m_tip = new TopTimelineTip(this);
    m_tip.setAnchor(Qt::AnchorTop, this, Qt::AnchorTop);
    // The preButton is anchored to the left of this
    m_tip.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);
    // NOTE: this is a bug of Anchors,the button should be resize after set anchor
    m_tip->setFixedWidth(this->width());
    m_tip.setTopMargin(TOP_TOOLBAR_HEIGHT);
    m_tip->setLeftMargin(m_view->horizontalOffset());
    connect(m_view, &TimelineView::paintingIndexsChanged, this, [=] {
        if (m_view->verticalOffset() > 10)
            m_tip->setText(currentMonth());
        else
            m_tip->setText("");
        m_tip->setLeftMargin(m_view->horizontalOffset());
    });
}

void TimelineFrame::initItems()
{
    auto infos = dApp->dbM->getAllInfos();

    LoadThread *t = new LoadThread(infos);
    connect(t, &LoadThread::ready,
            this, &TimelineFrame::insertItems, Qt::DirectConnection);
    connect(t, &LoadThread::finished, this, [=] {
        t->deleteLater();
        m_infos << infos;
        updateScrollRange();
    }, Qt::DirectConnection);
    t->start();
}

void TimelineFrame::insertItems(const TimelineItem::ItemData &data)
{
    // Do not update model if user is scroll and importing, the missing datas
    // will be insert again after import thread finished by call initItems
    if (dApp->importer->isRunning() && m_view->isScrolling()) {
        return;
    }
    m_model.appendData(data);
    m_view->updateScrollbarRange();
}

void TimelineFrame::removeItem(const DBImgInfo &info)
{
    TimelineItem::ItemData data;
    data.isTitle = false;
    data.path = info.filePath;
    data.timeline = utils::base::timeToString(info.time, true);

    // NOTE: THIS IS IMPORTANT
    // clear the selection to avoid call selectedPaths read some invalid data
    clearSelection();

    m_model.removeData(data);
    m_infos.removeAll(info);

    m_view->updateScrollbarRange();
    m_view->updateView();
}

void TimelineFrame::removeItems(const DBImgInfoList &infos)
{
    // NOTE: THIS IS IMPORTANT
    // clear the selection to avoid call selectedPaths read some invalid data
    clearSelection();

    for (auto info : infos) {
        TimelineItem::ItemData data;
        data.isTitle = false;
        data.path = info.filePath;
        data.timeline = utils::base::timeToString(info.time, true);


        m_model.removeData(data);
        m_infos.removeAll(info);

    }
    m_view->updateScrollbarRange();
    m_view->updateView();
}

#include "timelineframe.moc"
LoadThread::LoadThread(const DBImgInfoList &infos)
    : QThread(nullptr)
    , m_infos(infos)
{

}

void LoadThread::run()
{
    using namespace utils::base;
    using namespace utils::image;


    for (auto info : m_infos) {
        const QStringList hashs = scanpathsHash();
        if (! QFileInfo(info.filePath).exists() || ! hashs.contains(info.dirHash)) {
            continue;
        }
        TimelineItem::ItemData data;
        data.isTitle = false;
        data.path = info.filePath;

        QBuffer inBuffer( &data.thumbArray );
        inBuffer.open( QIODevice::WriteOnly );
        // write inPixmap into inByteArray
        if ( ! cutSquareImage(getThumbnail(data.path, true)).save( &inBuffer, "JPG", 100 )) {
//             errorPaths << info.filePath;
        }
        data.timeline = timeToString(info.time, true);

        emit ready(data);
    }
}

const QStringList LoadThread::scanpathsHash()
{
    QStringList paths = dApp->setter->value(SCANPATHS_GROUP, SCANPATHS_KEY)
            .toString().split(",");
    paths.removeAll("");
    QStringList hashs;
    for (auto path : paths) {
        hashs << utils::base::hash(path);
    }
    hashs << utils::base::hash(QString());
    return hashs;
}
