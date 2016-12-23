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
    initItems();

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
        QtConcurrent::run(this, &TimelineFrame::insertItems, infos);
    });
    connect(dApp->signalM, &SignalManager::imagesRemoved,
            this, [=] (const DBImgInfoList &infos) {
        for (auto info : infos) {
            removeItem(info);
        }
    });
    connect(dApp->importer, &Importer::imported, this, &TimelineFrame::initItems);

    // Top-Tip
    initTopTip();
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

void TimelineFrame::updateThumbnails()
{

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
        QtConcurrent::run(this, &TimelineFrame::insertItems, subInfos);
    }
}

void TimelineFrame::insertItems(const DBImgInfoList &infos)
{
    using namespace utils::base;
    using namespace utils::image;
    QStringList errorPaths;
    for (auto info : infos) {
        TimelineItem::ItemData data;
        data.isTitle = false;
        data.path = info.filePath;

        QBuffer inBuffer( &data.thumbArray );
        inBuffer.open( QIODevice::WriteOnly );
        // write inPixmap into inByteArray
        if ( ! cutSquareImage(getThumbnail(data.path, true)).save( &inBuffer, "JPG" )) {
             errorPaths << info.filePath;
        }
        data.timeline = timeToString(info.time, true);

        m_model.appendData(data);
    }
//    // dApp->importer thread may cause crash
//    if (errorPaths.length() > 0 && ! dApp->importer->isRunning()) {
//        qDebug() << "Remove some unsupport images: " << errorPaths.length();
//        dApp->dbM->removeImgInfos(errorPaths);
//    }

    m_view->updateView();
    m_view->update();
}

void TimelineFrame::removeItem(const DBImgInfo &info)
{
    TimelineItem::ItemData data;
    data.isTitle = false;
    data.path = info.filePath;
    data.timeline = utils::base::timeToString(info.time, true);

    m_model.removeData(data);

    m_view->updateView();
}
