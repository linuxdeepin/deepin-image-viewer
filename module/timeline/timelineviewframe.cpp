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

const QString FAVORITES_ALBUM_NAME = "My favorites";
const QString SHORTCUT_SPLIT_FLAG = "@-_-@";
const int THUMBNAIL_MAX_SCALE_SIZE = 192;

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
    layout->addWidget(m_view);
    layout->addWidget(separator);
}

TimelineViewFrame::~TimelineViewFrame()
{

}

void TimelineViewFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    m_view->setFixedWidth(e->size().width());
}

void TimelineViewFrame::initListView()
{
    m_view = new ThumbnailListView();
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setIconSize(m_iconSize);
    m_view->setModel( &m_model );
    if (m_multiselection) {
        m_view->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    else {
        m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    connect(m_view, &ThumbnailListView::clicked,
            this, &TimelineViewFrame::clicked);
    connect(m_view, &ThumbnailListView::mousePress,
            this, &TimelineViewFrame::mousePress);
    connect(m_view, &ThumbnailListView::doubleClicked,
            this, [=] (const QModelIndex & index) {
        const QString path = index.data(Qt::UserRole).toString();
        emit SignalManager::instance()->viewImage(
                    path, m_dbManager->getAllImagesPath());
    });
    connect(m_view, &ThumbnailListView::customContextMenuRequested,
            this, &TimelineViewFrame::customContextMenuRequested);
}

int TimelineViewFrame::indexOf(const QString &name) const
{
    for (int i = 0; i < m_model.rowCount(); i ++) {
        if (m_model.item(i, 0)->toolTip() == name) {
            return i;
        }
    }
    return -1;
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

void TimelineViewFrame::updateThumbnail(const QString &name)
{
    for (int i = 0; i < m_model.rowCount(); i ++) {
        if (m_model.item(i, 0)->toolTip() == name) {
            m_dbManager->updateThumbnail(name);

            const QPixmap p = m_dbManager->getImageInfoByName(name).thumbnail;
            QIcon icon;
            QPixmap thumbnail = utils::image::cutSquareImage(p,
                QSize(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE));
            icon.addPixmap(thumbnail, QIcon::Normal);
            icon.addPixmap(generateSelectedThumanail(thumbnail), QIcon::Selected);
            m_model.item(i, 0)->setIcon(icon);
            return;
        }
    }
}

QSize TimelineViewFrame::iconSize() const
{
    return m_view->iconSize();
}

void TimelineViewFrame::setIconSize(const QSize &iconSize)
{
    m_view->setIconSize(iconSize);
}

void TimelineViewFrame::insertItem(const DatabaseManager::ImageInfo &info)
{
    // Diffrent thread connection cause duplicate insert
    if (indexOf(info.name) != -1)
        return;
    QStandardItem *item = new QStandardItem();
    item->setData(info.path, Qt::UserRole);
    QIcon icon;
    QPixmap thumbnail = utils::image::cutSquareImage(info.thumbnail,
        QSize(THUMBNAIL_MAX_SCALE_SIZE, THUMBNAIL_MAX_SCALE_SIZE));
    icon.addPixmap(thumbnail, QIcon::Normal);
    if (m_multiselection)
        icon.addPixmap(generateSelectedThumanail(thumbnail), QIcon::Selected);
    item->setIcon(icon);
    item->setToolTip(info.name);

    m_model.setItem(m_model.rowCount(), 0, item);
}

bool TimelineViewFrame::removeItem(const QString &name)
{
    const int i = indexOf(name);
    if (i != -1) {
        m_model.removeRow(i);
        return true;
    }

    return false;
}

void TimelineViewFrame::clearSelection() const
{
    m_view->selectionModel()->clearSelection();
}

/*!
    \fn QMap<QString, QString> TimelineViewFrame::selectedImages() const

    Return the name-path map of all selected items.
*/
QMap<QString, QString> TimelineViewFrame::selectedImages() const
{
    QMap<QString, QString> images;
    for (QModelIndex index : m_view->selectionModel()->selectedIndexes()) {
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
    return m_model.rowCount() == 0;
}

bool TimelineViewFrame::contain(const QModelIndex &index) const
{
    return index.model() == &m_model;
}

QSize TimelineViewFrame::viewSize() const
{
    return m_view->childrenRect().size();
}
