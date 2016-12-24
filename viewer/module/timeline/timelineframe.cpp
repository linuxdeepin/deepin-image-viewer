#include "timelineframe.h"
#include "anchors.h"
#include "application.h"
#include "controller/importer.h"
#include "controller/signalmanager.h"
#include "mvc/timelinemodel.h"
#include "mvc/timelinedelegate.h"
#include "mvc/timelineitem.h"
#include "mvc/timelineview.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QtConcurrent>

namespace {

const int TOP_TOOLBAR_HEIGHT = 40;
const int BOTTOM_TOOLBAR_HEIGHT = 22;

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
    connect(m_view, &TimelineView::currentIndexChanged,
            this, &TimelineFrame::currentIndexChanged);
    connect(m_view, &TimelineView::changeItemSize,
            this, &TimelineFrame::changeItemSize);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);

    // Item append and remove
    connect(dApp->signalM, &SignalManager::imagesInserted,
            this, [=] (const DBImgInfoList infos){
        insertItems(infos);
//        QtConcurrent::run(this, &TimelineFrame::insertItems, infos);
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

void TimelineFrame::initTopTip()
{
    // Top-Tips
    Anchors<TopTimelineTip> tip = new TopTimelineTip(this);
    tip.setAnchor(Qt::AnchorTop, this, Qt::AnchorTop);
    // The preButton is anchored to the left of this
    tip.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);
    // NOTE: this is a bug of Anchors,the button should be resize after set anchor
    tip->setFixedWidth(this->width());
    tip.setTopMargin(TOP_TOOLBAR_HEIGHT);
    tip->setLeftMargin(m_view->horizontalOffset());
    connect(m_view, &TimelineView::paintingIndexsChanged, this, [=] {
        tip->setText(currentMonth());
        tip->setLeftMargin(m_view->horizontalOffset());
    });
}

void TimelineFrame::initItems()
{
    auto infos = dApp->dbM->getAllInfos();

    for (int i = 0; i < infos.length(); i += 500) {
        i = qMin(i, infos.length() - 1);
        auto subInfos = infos.mid(i, 500);
        // Use thread will cause painting crash
        // And QMutex not working, don't know why
        TIMER_SINGLESHOT(4 * (i + 500), {insertItems(subInfos);}, this, subInfos)
    }
}

void TimelineFrame::insertItems(const DBImgInfoList &infos)
{
    using namespace utils::base;
    using namespace utils::image;
    for (auto info : infos) {
        TimelineItem::ItemData data;
        data.isTitle = false;
        data.path = info.filePath;

        QBuffer inBuffer( &data.thumbArray );
        inBuffer.open( QIODevice::WriteOnly );
        // write inPixmap into inByteArray
        if ( ! cutSquareImage(getThumbnail(data.path, true)).save( &inBuffer, "JPG" )) {
//             errorPaths << info.filePath;
        }
        data.timeline = timeToString(info.time, true);

        m_model.appendData(data);
    }
    m_infos << infos;

    // Make sure the firse screen will fill with images while importing
    if (m_infos.length() > 300 && dApp->importer->isRunning()) {
        m_view->updateView(false);
    }
    else {
        m_view->updateView();
    }
}

void TimelineFrame::removeItem(const DBImgInfo &info)
{
    TimelineItem::ItemData data;
    data.isTitle = false;
    data.path = info.filePath;
    data.timeline = utils::base::timeToString(info.time, true);

    m_model.removeData(data);
    m_infos.removeAll(info);

    m_view->updateView();
}
