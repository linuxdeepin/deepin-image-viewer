// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pathviewproxymodel.h"
#include "types.h"
#include "imageinfo.h"
#include "imagesourcemodel.h"

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logImageViewer)

PathViewProxyModel::IndexInfo::IndexInfo(const IndexInfo &other)
    : url { other.url },
      index { other.index },
      frameCount { other.frameCount },
      frameIndex { other.frameIndex }
{
}

/**
   @class PathViewProxyModel
   @brief 视图代理模型，用于大图展示控件
    此模型并非纯数据模型，会同界面进行较为复杂的交互提供动画效果。
    模型构造一组类似环状链表的结构，当前视图中图片增减时，
    会自动在反方向插入数据，以达到在 PathView 中无缝切换图片的效果
 */
PathViewProxyModel::PathViewProxyModel(ImageSourceModel *srcModel, QObject *parent)
    : QAbstractListModel { parent },
      sourceModel(srcModel)
{
    Q_ASSERT_X(nullptr != sourceModel, "init proxy model", "must contain source data");

    // 设置默认的环长度
    const int defaultCount = 5;
    setQueueCount(defaultCount);
}

PathViewProxyModel::~PathViewProxyModel() { }

QHash<int, QByteArray> PathViewProxyModel::roleNames() const
{
    return { { Types::ImageUrlRole, "imageUrl" }, { Types::FrameIndexRole, "frameIndex" } };
}

QVariant PathViewProxyModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::ParentIsInvalid | CheckIndexOption::IndexIsValid)) {
        return {};
    }

    auto infoPtr = indexQueue[index.row()];
    switch (role) {
    case Types::ImageUrlRole:
        return infoPtr ? infoPtr->url : QUrl();
    case Types::FrameIndexRole:
        return infoPtr ? infoPtr->frameIndex : 0;
    default:
        break;
    }

    return {};
}

bool PathViewProxyModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!checkIndex(idx, CheckIndexOption::ParentIsInvalid | CheckIndexOption::IndexIsValid)) {
        return false;
    }

    auto infoPtr = indexQueue[idx.row()];
    switch (role) {
    case Types::ImageUrlRole:
        if (infoPtr) {
            QUrl oldUrl = infoPtr->url;
            QUrl newUrl = value.toUrl();

            infoPtr->url = newUrl;
            Q_EMIT dataChanged(idx, idx);

            // update previous and next
            for (int previous = idx.row() - 1; previous >= 0; --previous) {
                if (auto preInfo = indexQueue[previous]) {
                    if (preInfo->url == oldUrl) {
                        preInfo->url = newUrl;
                        QModelIndex notifyIdx = index(previous, idx.column());
                        Q_EMIT dataChanged(notifyIdx, notifyIdx);
                    }
                }
            }

            for (int next = idx.row() + 1; next < indexQueue.size(); ++next) {
                if (auto nextInfo = indexQueue[next]) {
                    if (nextInfo->url == oldUrl) {
                        nextInfo->url = newUrl;
                        QModelIndex notifyIdx = index(next, idx.column());
                        Q_EMIT dataChanged(notifyIdx, notifyIdx);
                    }
                }
            }
        }
        return true;
    default:
        break;
    }

    return false;
}

int PathViewProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return indexQueue.size();
}

int PathViewProxyModel::currentIndex() const
{
    return currentProxyIdx;
}

void PathViewProxyModel::setCurrentIndex(int index)
{
    if (index != currentProxyIdx) {
        currentProxyIdx = index;
        Q_EMIT currentIndexChanged(currentProxyIdx);
    }
}

/**
   @brief 跳转动画结束时，同步状态
 */
void PathViewProxyModel::syncState()
{
    if (Current != jumpFlag) {
        refreshBothSideData();
    }
}

/**
   @brief 设置当前指向的图片源索引 \a sourceIndex 和帧索引 \a frameIndex，
    若图片在当前图片左右两侧，则仅需利用 movePrevoius() 和 moveNext() 调整在数据环
    中当前指向位置; 间距超过 1 的图片会将下一张图片替换为索引指向的图片，并触发跳转动画，
    动画结束后再更新两侧的图片数据。

   @section 循环移动示例
    向前移动时，需要按环更新尾部的数据，以下示例中 < 表示索引逐步降低，> 表示索引递增增加
    当前指向代理索引为2，源数据索引为 13。

    Proxy index         0   1   2   3   4
    Index direction         <   ^   >
    Source index        11  12  13  14  15
    Proxy index         0   1   2   3   4
    Index direction     <   ^   >       *
    Source index        11  12  13  14  10
    * 此时 4 号索引被替换 * 变更为前置索引，更新为更前的源索引 10

    在 QML 组件 PathView 中，以此流程完成循环展示图片。
 */
void PathViewProxyModel::setCurrentSourceIndex(int sourceIndex, int frameIndex)
{
    qCDebug(logImageViewer) << "PathViewProxyModel::setCurrentSourceIndex() called with sourceIndex:" << sourceIndex << "frameIndex:" << frameIndex;
    if (indexQueue.isEmpty()) {
        qCWarning(logImageViewer) << "indexQueue is empty, returning.";
        return;
    }

    DistanceType type = distance(sourceIndex, frameIndex);
    qCDebug(logImageViewer) << "Calculated distance type:" << type;

    // 若跳转中途更新代理索引(打断当前动画)立即更新图片状态
    if (Current != type && Current != jumpFlag) {
        qCDebug(logImageViewer) << "Jump in progress or current, refreshing both side data.";
        refreshBothSideData();
    }

    switch (type) {
    case Previous:
        qCDebug(logImageViewer) << "Distance type is Previous, moving previous.";
        movePrevoius();
        break;
    case Next:
        qCDebug(logImageViewer) << "Distance type is Next, moving next.";
        moveNext();
        break;
    case OutOfRange:
        qCDebug(logImageViewer) << "Distance type is OutOfRange, jumping to index.";
        // 触发跳转动画设置
        jumpToIndex(sourceIndex, frameIndex);
        break;
    default:
        qCDebug(logImageViewer) << "Distance type is Current or Invalid, no action taken.";
        // Current / Invalid
        break;
    }
    qCDebug(logImageViewer) << "PathViewProxyModel::setCurrentSourceIndex() finished.";
}

/**
   @brief 移动到上一张图片，更新当前指向的环索引
    将当前图片最右侧的图片进行更新为左侧区间的数据。
 */
void PathViewProxyModel::movePrevoius()
{
    qCDebug(logImageViewer) << "PathViewProxyModel::movePrevoius() called.";
    // 同步 view 的当前位置
    setCurrentIndex(previousPorxyIdx(currentProxyIdx));
    qCDebug(logImageViewer) << "Current proxy index after moving previous:" << currentProxyIdx;

    // 移动至前一张图片，附近的索引不变，右侧区间 - 1, 左侧区间 + 1
    int changeIndex = (currentProxyIdx + radius + 1) % maxCount;
    const IndexInfoPtr &baseInfo = indexQueue[nextProxyIdx(changeIndex)];
    qCDebug(logImageViewer) << "Change index:" << changeIndex << "Base info URL:" << (baseInfo ? baseInfo->url.toString() : "null");
    updateIndexInfo(changeIndex, createPreviousIndexInfo(baseInfo));
    qCDebug(logImageViewer) << "PathViewProxyModel::movePrevoius() finished.";
}

/**
   @brief 移动到下一张图片，更新当前指向的环索引
    将当前图片最左侧的图片进行更新为右侧区间的数据。
 */
void PathViewProxyModel::moveNext()
{
    qCDebug(logImageViewer) << "PathViewProxyModel::moveNext() called.";
    setCurrentIndex(nextProxyIdx(currentProxyIdx));
    qCDebug(logImageViewer) << "Current proxy index after moving next:" << currentProxyIdx;

    int changeIndex = (currentProxyIdx + radius) % maxCount;
    const IndexInfoPtr &baseInfo = indexQueue[previousPorxyIdx(changeIndex)];
    qCDebug(logImageViewer) << "Change index:" << changeIndex << "Base info URL:" << (baseInfo ? baseInfo->url.toString() : "null");

    updateIndexInfo(changeIndex, createNextIndexInfo(baseInfo));
    qCDebug(logImageViewer) << "PathViewProxyModel::moveNext() finished.";
}

/**
   @brief 重置模型索引数据，指向源模型的数据将被重置，
   @note 当处于边界时，索引数据将写入空数据，以在切换中不会显示到环另一侧的
 */
void PathViewProxyModel::resetModel(int sourceIndex, int frameIndex)
{
    qCDebug(logImageViewer) << "PathViewProxyModel::resetModel() called with sourceIndex:" << sourceIndex << "frameIndex:" << frameIndex;
    beginResetModel();
    qCDebug(logImageViewer) << "Begin reset model.";

    // 无需通知
    currentProxyIdx = 0;
    jumpFlag = Current;
    indexQueue.clear();
    indexQueue.append(infoFromIndex(sourceIndex, frameIndex));
    qCDebug(logImageViewer) << "Index queue initialized with first item:" << (indexQueue.first() ? indexQueue.first()->url.toString() : "null");

    QList<IndexInfoPtr> prependQueue;

    while ((indexQueue.size() + prependQueue.size()) < maxCount) {
        qCDebug(logImageViewer) << "Filling index queue. Current queue size:" << indexQueue.size() << "Prepend queue size:" << prependQueue.size();
        // 首次进入时，前面的节点不填充，而是追加到尾部，保证首张图片索引指向 0
        if (prependQueue.isEmpty()) {
            prependQueue.prepend(createPreviousIndexInfo(indexQueue.first()));
            qCDebug(logImageViewer) << "Prepended first previous index info.";
        } else {
            prependQueue.prepend(createPreviousIndexInfo(prependQueue.first()));
            qCDebug(logImageViewer) << "Prepended previous index info.";
        }

        // 区间右侧数据直接填充
        indexQueue.append(createNextIndexInfo(indexQueue.last()));
        qCDebug(logImageViewer) << "Appended next index info.";
    }

    // 追加区间左侧的数据
    indexQueue.append(prependQueue);
    qCDebug(logImageViewer) << "Appended prepend queue. Final queue size:" << indexQueue.size();

    endResetModel();
    qCDebug(logImageViewer) << "End reset model.";
    qCDebug(logImageViewer) << "PathViewProxyModel::resetModel() finished.";
}

/**
   @brief 移除当前图片，并更新图片索引信息
 */
void PathViewProxyModel::deleteCurrent()
{
    qCDebug(logImageViewer) << "PathViewProxyModel::deleteCurrent() called.";
    if (indexQueue.isEmpty()) {
        qCWarning(logImageViewer) << "indexQueue is empty, returning.";
        return;
    }

    // 特殊处理，此时源数据无文件
    if (0 == sourceModel->rowCount()) {
        qCDebug(logImageViewer) << "Source model row count is 0, resetting model.";
        beginResetModel();
        setCurrentIndex(0);
        indexQueue.clear();
        endResetModel();
        qCDebug(logImageViewer) << "Model reset after source model empty.";
        return;
    }

    const IndexInfoPtr &current = indexQueue[currentProxyIdx];
    if (!current) {
        qCWarning(logImageViewer) << "Current index info is null, returning.";
        return;
    }
    qCDebug(logImageViewer) << "Current image to delete:" << current->url.toString() << "at source index:" << current->index;

    // 源数据模型移除当前图片，默认将后续图片前移，currentIndex将不会变更，手动提示更新
    bool atEnd = (current->index >= sourceModel->rowCount());
    if (atEnd) {
        qCDebug(logImageViewer) << "Deleting image at the end of source model.";
        // 获取之前的索引
        int previousIdx = previousPorxyIdx(currentProxyIdx);
        qCDebug(logImageViewer) << "Previous proxy index:" << previousIdx;

        // 如果删除的是尾部数据，则索引会 - 1 . INT_MAX 会将帧索引指向尾帧
        auto previous = infoFromIndex(current->index - 1, INT_MAX);
        updateIndexInfo(previousIdx, previous);
        qCDebug(logImageViewer) << "Updated previous index info.";
        // 更新完成数据后再更新界面，触发动画
        setCurrentIndex(previousIdx);
        qCDebug(logImageViewer) << "Set current index to previous:" << previousIdx;

        // 更新当前索引后再更新两侧数据
        refreshBothSideData();
        qCDebug(logImageViewer) << "Refreshed both side data.";
    } else {
        qCDebug(logImageViewer) << "Deleting image not at the end of source model.";
        int nextIdx = nextProxyIdx(currentProxyIdx);
        qCDebug(logImageViewer) << "Next proxy index:" << nextIdx;
        // NOTE: 源数据模型移除当前图片后，index不会变更
        auto next = infoFromIndex(current->index, 0);
        updateIndexInfo(nextIdx, next);
        qCDebug(logImageViewer) << "Updated next index info.";

        setCurrentIndex(nextIdx);
        qCDebug(logImageViewer) << "Set current index to next:" << nextIdx;

        refreshBothSideData();
        qCDebug(logImageViewer) << "Refreshed both side data.";
    }
    qCDebug(logImageViewer) << "PathViewProxyModel::deleteCurrent() finished.";
}

/**
   @brief 设置当前数据缓存的环队列长度
   @warning 环长度不小于3且必须为奇数
 */
void PathViewProxyModel::setQueueCount(int count)
{
    qCDebug(logImageViewer) << "PathViewProxyModel::setQueueCount() called with count:" << count;
    static int minLimit = 3;
    bool valid = (count >= minLimit) && !!(count % 2);
    Q_ASSERT(valid);
    if (!valid) {
        qCWarning(logImageViewer) << "Invalid queue count:" << count << ". Must be >= 3 and odd.";
    }

    maxCount = count;
    radius = qFloor(maxCount / 2);
    qCDebug(logImageViewer) << "maxCount set to:" << maxCount << ", radius set to:" << radius;
}

/**
   @brief 打印当前的队列缓存信息
 */
void PathViewProxyModel::dumpInfo()
{
    qCDebug(logImageViewer) << "[ProxyModel] Current proxy index:" << currentProxyIdx;
    for (int i = 0; i < indexQueue.size(); ++i) {
        const IndexInfoPtr &info = indexQueue.at(i);
        if (info) {
            qCDebug(logImageViewer) << QString("[ProxyModel] index: %1, count: %2, frameIndex: %3, Url: %4 %5")
                                               .arg(info->index)
                                               .arg(info->frameCount)
                                               .arg(info->frameIndex)
                                               .arg(info->url.toString())
                                               .arg((i == currentProxyIdx) ? "  <--" : "");
        } else {
            qCDebug(logImageViewer) << "[ProxyModel] Empty index info at index:" << i;
        }
    }
}

/**
   @brief 从当前索引位置跳转到指定图片源索引 \a sourceIndex 和 \a frameIndex
 */
void PathViewProxyModel::jumpToIndex(int sourceIndex, int frameIndex)
{
    qCDebug(logImageViewer) << "PathViewProxyModel::jumpToIndex() called with sourceIndex:" << sourceIndex << "frameIndex:" << frameIndex;
    const IndexInfoPtr &current = indexQueue[currentProxyIdx];
    if (!current) {
        qCWarning(logImageViewer) << "No current data at index:" << currentProxyIdx;
        dumpInfo();
        return;
    }

    qCDebug(logImageViewer) << "Jumping to index:" << sourceIndex << "frame:" << frameIndex
                            << "from current:" << current->index << "frame:" << current->frameIndex;

    int jumpIndex = currentProxyIdx;
    DistanceType flag = Current;
    if (sourceIndex >= current->index && frameIndex >= current->frameIndex) {
        flag = Next;
        jumpIndex = nextProxyIdx(currentProxyIdx);
        qCDebug(logImageViewer) << "Jump direction: Next. Jump index:" << jumpIndex;
    } else {
        flag = Previous;
        jumpIndex = previousPorxyIdx(currentProxyIdx);
        qCDebug(logImageViewer) << "Jump direction: Previous. Jump index:" << jumpIndex;
    }

    // 更新跳转图片的源数据
    IndexInfoPtr jumpInfo = infoFromIndex(sourceIndex, frameIndex);
    updateIndexInfo(jumpIndex, jumpInfo);
    qCDebug(logImageViewer) << "Updated jump index info for index:" << jumpIndex;

    // 触发跳转动画
    setCurrentIndex(jumpIndex);
    qCDebug(logImageViewer) << "Set current index to jump index:" << jumpIndex;

    // currentIndex 的变更会判断 jumpFlag 触发 jumpFinished() ，
    // 因此 jumpFlag 的变更在 currentIndexChanged() 之后触发
    jumpFlag = flag;
    qCDebug(logImageViewer) << "Jump flag set to:" << jumpFlag;
    qCDebug(logImageViewer) << "PathViewProxyModel::jumpToIndex() finished.";
}

/**
   @brief 重新刷新两侧数据，用于动画结束等场景刷新
 */
void PathViewProxyModel::refreshBothSideData()
{
    qCDebug(logImageViewer) << "Refreshing both sides of current index:" << currentProxyIdx;
    // 根据当前图片的位置，更新两侧半径的数据
    int previousIndex = currentProxyIdx;
    int nextIndex = currentProxyIdx;

    for (int i = 0; i < radius; ++i) {
        qCDebug(logImageViewer) << "Refreshing side data, iteration:" << i;
        // 根据上此索引位置数据更新当前数据，中心向两侧增长
        IndexInfoPtr previousInfo = createPreviousIndexInfo(indexQueue[previousIndex]);
        previousIndex = previousPorxyIdx(previousIndex);
        updateIndexInfo(previousIndex, previousInfo);
        qCDebug(logImageViewer) << "Updated previous index:" << previousIndex;

        IndexInfoPtr nextInfo = createNextIndexInfo(indexQueue[nextIndex]);
        nextIndex = nextProxyIdx(nextIndex);
        updateIndexInfo(nextIndex, nextInfo);
        qCDebug(logImageViewer) << "Updated next index:" << nextIndex;
    }

    // 取消状态
    jumpFlag = Current;
    qCDebug(logImageViewer) << "Jump flag reset to Current.";
    qCDebug(logImageViewer) << "PathViewProxyModel::refreshBothSideData() finished.";
}

/**
   @return 返回源索引为 \a soureIndex 和 \a frameIndex 与当前图片的相对距离
 */
PathViewProxyModel::DistanceType PathViewProxyModel::distance(int sourceIndex, int frameIndex)
{
    qCDebug(logImageViewer) << "PathViewProxyModel::distance() called for sourceIndex:" << sourceIndex << "frameIndex:" << frameIndex;
    if (sourceIndex < 0 || sourceIndex >= sourceModel->rowCount()) {
        qCDebug(logImageViewer) << "Source index out of range, returning Invalid.";
        return Invalid;
    }

    if (indexQueue.isEmpty()) {
        qCDebug(logImageViewer) << "Index queue is empty, returning Invalid.";
        return Invalid;
    }

    const IndexInfoPtr &current = indexQueue[currentProxyIdx];
    if (!current) {
        qCWarning(logImageViewer) << "Current index info is null, returning Invalid.";
        return Invalid;
    }
    qCDebug(logImageViewer) << "Current item in proxy:" << current->url.toString() << "at index:" << current->index << "frame:" << current->frameIndex;

    int range = OutOfRange;
    int indexDis = sourceIndex - current->index;
    qCDebug(logImageViewer) << "Index difference:" << indexDis;
    switch (indexDis) {
    case Current:
        qCDebug(logImageViewer) << "Index difference is Current.";
        range = frameIndex - current->frameIndex;
        qCDebug(logImageViewer) << "Frame difference:" << range;
        break;
    case Previous: {
        qCDebug(logImageViewer) << "Index difference is Previous.";
        const IndexInfoPtr &previous = indexQueue[previousPorxyIdx(currentProxyIdx)];
        if (previous) {
            range = -(previous->frameCount - frameIndex) - current->frameIndex;
            qCDebug(logImageViewer) << "Previous frame count:" << previous->frameCount << ", new range:" << range;
        }
    } break;
    case Next:
        range = current->frameCount - current->frameIndex + frameIndex;
        break;
    default:
        break;
    }

    // 判断范围
    if (Previous <= range && range <= Next) {
        return static_cast<DistanceType>(range);
    }
    return OutOfRange;
}

/**
   @return 返回源索引为 \a sourceIndex 指向的图片文件路径
 */
QUrl PathViewProxyModel::sourcePath(int sourceIndex)
{
    qCDebug(logImageViewer) << "PathViewProxyModel::sourcePath() called for sourceIndex:" << sourceIndex;
    if (0 <= sourceIndex && sourceIndex < sourceModel->rowCount()) {
        QUrl path = sourceModel->data(sourceModel->index(sourceIndex), Types::ImageUrlRole).toUrl();
        qCDebug(logImageViewer) << "Source path for index" << sourceIndex << ":" << path.toString();
        return path;
    }

    qCDebug(logImageViewer) << "Source index" << sourceIndex << "is out of range. Returning empty URL.";
    return {};
}

/**
   @return 返回索引 \a base 之前的索引，由于代理数据为环状结构，
    因此索引会在 0 ~ (rowCount() - 1) 之间循环
 */
int PathViewProxyModel::previousPorxyIdx(int base)
{
    int newIndex = (base - 1 + indexQueue.size()) % indexQueue.size();
    qCDebug(logImageViewer) << "PathViewProxyModel::previousPorxyIdx() called for base:" << base << ", returning:" << newIndex;
    return newIndex;
}

/**
   @return 返回索引 \a base 之后的索引，由于代理数据为环状结构，
    因此索引会在 0 ~ (rowCount() - 1) 之间循环
 */
int PathViewProxyModel::nextProxyIdx(int base)
{
    int newIndex = (base + 1) % indexQueue.size();
    qCDebug(logImageViewer) << "PathViewProxyModel::nextProxyIdx() called for base:" << base << ", returning:" << newIndex;
    return newIndex;
}

/**
   @return 根据源索引 \a sourceIndex 和 \a frameIndex 返回索引信息 IndexInfo，
    索引信息包含当前指向的数据源路径 / sourceIndex / frameIndex / 帧总数信息。
 */
PathViewProxyModel::IndexInfoPtr PathViewProxyModel::infoFromIndex(int sourceIndex, int frameIndex)
{
    qCDebug(logImageViewer) << "PathViewProxyModel::infoFromIndex() called for sourceIndex:" << sourceIndex << "frameIndex:" << frameIndex;
    QUrl url = sourcePath(sourceIndex);
    if (url.isEmpty()) {
        qCWarning(logImageViewer) << "Empty URL for source index:" << sourceIndex;
        return {};
    }

    ImageInfo imageInfo(url);
    if (ImageInfo::Loading == imageInfo.status()) {
        qCDebug(logImageViewer) << "Image data not ready (status: Loading), will update asynchronously:" << url.toString();
        asyncUpdateLoadInfo(url, sourceIndex, frameIndex);
    }

    IndexInfoPtr info = IndexInfoPtr::create();
    info->url = url;
    info->index = sourceIndex;
    info->frameCount = qMax(1, imageInfo.frameCount());
    info->frameIndex = qBound(0, frameIndex, info->frameCount - 1);
    qCDebug(logImageViewer) << "Created IndexInfo: url=" << info->url.toString() << ", index=" << info->index
                            << ", frameCount=" << info->frameCount << ", frameIndex=" << info->frameIndex;

    return info;
}

/**
   @brief 异步更新加载图片路径 \a url 源索引 \a sourceIndex 帧索引 \a frameIndex 的数据
    此函数一般仅会在打开应用时触发，此时后端数据还未加载完成。
 */
void PathViewProxyModel::asyncUpdateLoadInfo(const QUrl &url, int sourceIndex, int frameIndex)
{
    qCDebug(logImageViewer) << "PathViewProxyModel::asyncUpdateLoadInfo() called for url:" << url.toString() << "sourceIndex:" << sourceIndex << "frameIndex:" << frameIndex;
    // 异步更新加载信息，一般仅初次加载存在此问题
    if (0 == frameIndex) {
        qCDebug(logImageViewer) << "Starting async update for:" << url.toString()
                                << "source index:" << sourceIndex;
        ImageInfo *delayInfo = new ImageInfo(url, this);
        connect(delayInfo, &ImageInfo::statusChanged, this, [this, delayInfo, sourceIndex]() {
            qCDebug(logImageViewer) << "AsyncUpdateLoadInfo: ImageInfo status changed for:" << delayInfo->source().toString();
            // 仅更新加载成功的多页图数据，其它类型的数据无需关注，默认的 frameIndex 都是 0
            if (ImageInfo::Ready == delayInfo->status() && Types::MultiImage == delayInfo->type()) {
                qCDebug(logImageViewer) << "Async update completed for multi-image:" << delayInfo->source().toString()
                                        << "frames:" << delayInfo->frameCount();
                auto current = indexQueue[currentProxyIdx];
                if (!current) {
                    qCWarning(logImageViewer) << "Current index info is null in async update, deleting delayInfo.";
                    delayInfo->deleteLater();
                    return;
                }

                if (sourceIndex == current->index) {
                    qCDebug(logImageViewer) << "Source index matches current index, updating frame count and refreshing both sides.";
                    // 若为当前图片，仅需更新当前图片帧总数和两侧的数据
                    current->frameCount = delayInfo->frameCount();
                    refreshBothSideData();
                } else if (sourceIndex < current->index) {
                    qCDebug(logImageViewer) << "Source index is less than current index, updating previous indices.";
                    int previousIndex = currentProxyIdx;
                    for (int i = 0; i < radius; ++i) {
                        IndexInfoPtr previousInfo = createPreviousIndexInfo(indexQueue[previousIndex]);
                        previousIndex = previousPorxyIdx(previousIndex);
                        updateIndexInfo(previousIndex, previousInfo);
                        qCDebug(logImageViewer) << "Updated previous index:" << previousIndex;
                    }
                } else {
                    qCDebug(logImageViewer) << "Source index is greater than current index, updating next indices.";
                    int nextIndex = currentProxyIdx;
                    for (int i = 0; i < radius; ++i) {
                        IndexInfoPtr nextInfo = createNextIndexInfo(indexQueue[nextIndex]);
                        nextIndex = nextProxyIdx(nextIndex);
                        updateIndexInfo(nextIndex, nextInfo);
                        qCDebug(logImageViewer) << "Updated next index:" << nextIndex;
                    }
                }
            } else {
                qCDebug(logImageViewer) << "Async update not for ready multi-image or frameIndex is not 0. Status:" << delayInfo->status() << "Type:" << delayInfo->type();
            }

            delayInfo->deleteLater();
            qCDebug(logImageViewer) << "delayInfo deleted.";
        });
    } else {
        qCDebug(logImageViewer) << "Async update skipped for frameIndex:" << frameIndex << ". Only frameIndex 0 is processed.";
    }
    qCDebug(logImageViewer) << "PathViewProxyModel::asyncUpdateLoadInfo() finished.";
}

/**
   @return 返回索引信息 \a baseInfo 之前图片的索引信息，若 \a baseInfo 为空，返回的也是 nullptr
 */
PathViewProxyModel::IndexInfoPtr PathViewProxyModel::createPreviousIndexInfo(const IndexInfoPtr &baseInfo)
{
    qCDebug(logImageViewer) << "PathViewProxyModel::createPreviousIndexInfo() called.";
    if (!baseInfo) {
        qCWarning(logImageViewer) << "Base info is null, returning empty IndexInfoPtr.";
        return {};
    }

    if (baseInfo->frameIndex > 0) {
        IndexInfoPtr previousFrame = IndexInfoPtr::create(*baseInfo);
        previousFrame->frameIndex--;
        qCDebug(logImageViewer) << "Creating previous frame info. New frame index:" << previousFrame->frameIndex;
        return previousFrame;
    }

    IndexInfoPtr previous = infoFromIndex(baseInfo->index - 1);
    if (previous) {
        previous->frameIndex = previous->frameCount - 1;
        qCDebug(logImageViewer) << "Creating previous image info. New frame index:" << previous->frameIndex;
    } else {
        qCDebug(logImageViewer) << "No previous image found for base index:" << baseInfo->index - 1;
    }
    return previous;
}

/**
   @return 返回索引信息 \a baseInfo 之后图片的索引信息，若 \a baseInfo 为空，返回的也是 nullptr
 */
PathViewProxyModel::IndexInfoPtr PathViewProxyModel::createNextIndexInfo(const IndexInfoPtr &baseInfo)
{
    qCDebug(logImageViewer) << "PathViewProxyModel::createNextIndexInfo() called.";
    if (!baseInfo) {
        qCWarning(logImageViewer) << "Base info is null, returning empty IndexInfoPtr.";
        return {};
    }

    if (baseInfo->frameCount - 1 > baseInfo->frameIndex) {
        IndexInfoPtr nextFrame = IndexInfoPtr::create(*baseInfo);
        nextFrame->frameIndex++;
        qCDebug(logImageViewer) << "Creating next frame info. New frame index:" << nextFrame->frameIndex;
        return nextFrame;
    }

    IndexInfoPtr next = infoFromIndex(baseInfo->index + 1);
    return next;
}

/**
   @brief 更新索引 \a proxyInfo 指向的信息为 \a info ，同时抛出 dataChanged() 信号
    以通知 view 更新
 */
void PathViewProxyModel::updateIndexInfo(int proxyIndex, const IndexInfoPtr &info)
{
    Q_ASSERT(0 <= proxyIndex && proxyIndex < indexQueue.size());
    indexQueue[proxyIndex] = info;
    qCDebug(logImageViewer) << "Updated indexQueue at proxy index:" << proxyIndex;

    QModelIndex changeModelIndex = index(proxyIndex);
    Q_EMIT dataChanged(changeModelIndex, changeModelIndex, { Types::ImageUrlRole, Types::FrameIndexRole });
}
