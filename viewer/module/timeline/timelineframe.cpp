#include "timelineframe.h"
#include "application.h"
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

const int TOP_TOOLBAR_HEIGHT = 40;
const int BOTTOM_TOOLBAR_HEIGHT = 22;

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
    }
    void setLeftMargin(int v) {setContentsMargins(v, 0, 0, 0);}
};

TimelineFrame::TimelineFrame(QWidget *parent)
    : QFrame(parent)
{
    m_view = new TimelineView(this);
    m_view->setItemDelegate(new TimelineDelegate());
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

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);

    // Item append and remove
    connect(dApp->importer, &Importer::imported,
            this, &TimelineFrame::updateView);
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
            this, [=] (const DBImgInfoList &infos) {
        for (auto info : infos) {
            removeItem(info);
        }
    });

    // Top-Tip
    initTopTip();
    initItems();
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

void TimelineFrame::updateThumbnails(const QString &path)
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
}

void TimelineFrame::updateView()
{
    m_view->updateView();
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
        QModelIndex index = m_view->paintingIndexs().first();
        QVariantList datas = m_model.data(index, Qt::DisplayRole).toList();
        if (datas.count() == 4) { // There is 4 field data inside TimelineData
            return datas[2].toString();
        }
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
    QMutexLocker locker(&m_mutex);

    auto infos = dApp->dbM->getAllInfos();

    LoadThread *t = new LoadThread(infos);
    connect(t, &LoadThread::ready,
            this, &TimelineFrame::insertItems, Qt::DirectConnection);
    connect(t, &LoadThread::finished, this, [=] {
        t->deleteLater();
        m_infos << infos;
        updateView();
    }, Qt::DirectConnection);
    t->start();
}

void TimelineFrame::insertItems(const TimelineItem::ItemData &data)
{
    m_model.appendData(data);
    m_view->updateView(dApp->importer->isRunning());
}

void TimelineFrame::removeItem(const DBImgInfo &info)
{
    TimelineItem::ItemData data;
    data.isTitle = false;
    data.path = info.filePath;
    data.timeline = utils::base::timeToString(info.time, true);

    m_model.removeData(data);
    m_infos.removeAll(info);

    updateView();
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
