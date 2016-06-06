#include "timelineviewframe.h"
#include "controller/wallpapersetter.h"
#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QResizeEvent>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QJsonArray>
#include <QJsonDocument>

namespace {

const int THUMBNAIL_MAX_SCALE_SIZE = 384;
const QString FAVORITES_ALBUM_NAME = "My favorites";
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";

}  //namespace

TimelineViewFrame::TimelineViewFrame(const QString &timeline,
                                     bool multiselection,
                                     QWidget *parent)
    : QFrame(parent),
      m_multiselection(multiselection),
      m_iconSize(96, 96),
      m_timeline(timeline),
      m_dbManager(DatabaseManager::instance()),
      m_sManager(SignalManager::instance())
{
    QLabel *title = new QLabel(timeline);
    title->setObjectName("TimelineFrameTitle");
    QLabel *separator = new QLabel();
    separator->setObjectName("TimelineSeparator");
    separator->setFixedHeight(1);

    initListView();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(title);
    layout->addWidget(m_listView);
    layout->addWidget(separator);

    connect(m_sManager, &SignalManager::updateThumbnail,
            this, &TimelineViewFrame::updateThumbnail);
}

void TimelineViewFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    m_listView->setFixedWidth(e->size().width());
}

void TimelineViewFrame::initListView()
{
    m_listView = new ThumbnailListView();
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listView->setIconSize(m_iconSize);
    m_listView->setModel( &m_standardModel );
    if (m_multiselection) {
        m_listView->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    else {
        m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    connect(m_listView, &ThumbnailListView::clicked,
            this, &TimelineViewFrame::clicked);
    connect(m_listView, &ThumbnailListView::mousePress,
            this, &TimelineViewFrame::mousePress);
    connect(m_listView, &ThumbnailListView::doubleClicked,
            this, [=] (const QModelIndex & index) {
        emit m_sManager->viewImage(index.data(Qt::UserRole).toString());
    });
    connect(m_listView, &ThumbnailListView::customContextMenuRequested,
            this, &TimelineViewFrame::customContextMenuRequested);

    // Ddd data
    QList<DatabaseManager::ImageInfo> list
            = DatabaseManager::instance()->getImageInfosByTime(
                utils::base::stringToDateTime(m_timeline));
    for (DatabaseManager::ImageInfo info : list) {
        insertItem(info);
    }
}

DatabaseManager::ImageInfo TimelineViewFrame::imageInfo(const QString &name)
{
    return m_dbManager->getImageInfoByName(name);
}

QString TimelineViewFrame::currentSelectOne(bool isPath)
{
    const QStringList nl = selectedImages().keys();
    const QStringList pl = selectedImages().values();

    if (isPath) {
        if (pl.isEmpty())
            return QString();
        else
            return pl.first();
    }
    else {
        if (nl.isEmpty())
            return QString();
        else
            return nl.first();
    }
}

QPixmap TimelineViewFrame::generateSelectedThumanail(const QPixmap &pixmap)
{
    if (m_multiselection) {
        QPixmap target = pixmap;
        QPainter painter(&target);
        QPixmap icon(":/images/resources/images/item_selected.png");
        int selectIconSize = 80;
        painter.drawPixmap((target.width() - selectIconSize) / 2,
                           (target.height() - selectIconSize) / 2,
                           selectIconSize, selectIconSize, icon);

        return target;
    }
    else {
        return pixmap;
    }
}

QPixmap TimelineViewFrame::increaseThumbnail(const QPixmap &pixmap)
{
    QSize targetSize;
    if (pixmap.width() > pixmap.height()) {
        targetSize = QSize(THUMBNAIL_MAX_SCALE_SIZE,
                           (double)THUMBNAIL_MAX_SCALE_SIZE / pixmap.width() *
                           pixmap.height());
    }
    else {
        targetSize = QSize((double)THUMBNAIL_MAX_SCALE_SIZE / pixmap.height() *
                           pixmap.width(), THUMBNAIL_MAX_SCALE_SIZE);
    }
    return pixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void TimelineViewFrame::updateThumbnail(const QString &name)
{
    for (int i = 0; i < m_standardModel.rowCount(); i ++) {
        if (m_standardModel.item(i, 0)->toolTip() == name) {
            DatabaseManager::ImageInfo info =
                    m_dbManager->getImageInfoByName(name);
            const QPixmap p = utils::image::getThumbnail(info.path);
            info.thumbnail = p;
            m_dbManager->updateImageInfo(info);

            QIcon icon;
            QPixmap thumbnail = increaseThumbnail(p);
            icon.addPixmap(thumbnail, QIcon::Normal);
            icon.addPixmap(generateSelectedThumanail(thumbnail), QIcon::Selected);
            m_standardModel.item(i, 0)->setIcon(icon);
            return;
        }
    }
}

QSize TimelineViewFrame::iconSize() const
{
    return m_listView->iconSize();
}

void TimelineViewFrame::setIconSize(const QSize &iconSize)
{
    m_listView->setIconSize(iconSize);
}

void TimelineViewFrame::insertItem(const DatabaseManager::ImageInfo &info)
{
    QStandardItem *item = new QStandardItem();
    item->setData(info.path, Qt::UserRole);
    QIcon icon;
    QPixmap thumbnail = increaseThumbnail(info.thumbnail);
    icon.addPixmap(thumbnail, QIcon::Normal);
    icon.addPixmap(generateSelectedThumanail(thumbnail), QIcon::Selected);
    item->setIcon(icon);
    item->setToolTip(info.name);

    m_standardModel.setItem(m_standardModel.rowCount(), 0, item);
}

bool TimelineViewFrame::removeItem(const QString &name)
{
    for (int i = 0; i < m_standardModel.rowCount(); i ++) {
        if (m_standardModel.item(i, 0)->toolTip() == name) {
            m_standardModel.removeRow(i);
            return true;
        }
    }

    return false;
}

void TimelineViewFrame::clearSelection() const
{
    m_listView->selectionModel()->clearSelection();
}

/*!
    \fn QMap<QString, QString> TimelineViewFrame::selectedImages() const

    Return the name-path map of all selected items.
*/
QMap<QString, QString> TimelineViewFrame::selectedImages() const
{
    QMap<QString, QString> images;
    for (QModelIndex index : m_listView->selectionModel()->selectedIndexes()) {
        QString path = index.data(Qt::UserRole).toString();
        images.insert(QFileInfo(path).fileName(),
                      index.data(Qt::UserRole).toString());
    }

    return images;
}

QString TimelineViewFrame::timeline() const
{
    return m_timeline;
}

bool TimelineViewFrame::isEmpty() const
{
    return m_standardModel.rowCount() == 0;
}

bool TimelineViewFrame::contain(const QModelIndex &index) const
{
    return index.model() == &m_standardModel;
}

QSize TimelineViewFrame::viewSize() const
{
    return m_listView->childrenRect().size();
}
