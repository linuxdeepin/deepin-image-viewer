#ifndef TIMELINEVIEWFRAME_H
#define TIMELINEVIEWFRAME_H

#include <QLabel>
#include <QWidget>
#include <QFrame>
#include <QListView>
#include <QStandardItem>
#include <QVBoxLayout>
#include <QJsonValue>
#include <QJsonObject>

#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include "widgets/thumbnaillistview.h"

class PopupMenuManager;
class TimelineViewFrame : public QFrame
{
    Q_OBJECT
public:
    explicit TimelineViewFrame(const QString &timeline,
                               bool multiselection,
                               QWidget *parent);
    void insertItem(const DatabaseManager::ImageInfo &info);
    bool removeItem(const QString &name);
    void clearSelection() const;

    QStringList selectedImagesNameList();
    QStringList selectedImagesPathList();
    QString timeline() const;
    bool isEmpty() const;

    QSize viewSize() const;
    QSize iconSize() const;
    void setIconSize(const QSize &iconSize);

protected:
    void resizeEvent(QResizeEvent *e);

private:
    enum MenuItemId {
        IdView,
        IdFullScreen,
        IdStartSlideShow,
        IdAddToAlbum,
        IdExport,
        IdCopy,
        IdDelete,
        IdEdit,
        IdAddToFavorites,
        IdRemoveFromFavorites,
        IdRotateClockwise,
        IdRotateCounterclockwise,
        IdLabel,
        IdSetAsWallpaper,
        IdDisplayInFileManager,
        IdImageInfo,
        IdSubMenu,
        IdSeparator
    };

    void initListView();

    DatabaseManager::ImageInfo imageInfo(const QString &name);
    QString currentSelectOne(bool isPath = true);
    QPixmap generateSelectedThumanail(const QPixmap &pixmap);
    QPixmap increaseThumbnail(const QPixmap &pixmap);
    QString createMenuContent();
    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    QJsonObject createAlbumMenuObj();

    void updateThumbnail(const QString &name);
    void updateMenuContents();
    void onMenuItemClicked(int menuId, const QString &text);

private:
    bool m_multiselection;
    QSize m_iconSize;
    QString m_timeline;
    ThumbnailListView *m_listView;
    QStandardItemModel m_standardModel;
    PopupMenuManager *m_popupMenu;
    DatabaseManager *m_dbManager;
    SignalManager *m_sManager;
};

#endif // TIMELINEVIEWFRAME_H
