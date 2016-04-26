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

class TimelineViewFrame : public QFrame
{
    Q_OBJECT
public:
    explicit TimelineViewFrame(const QString &timeline, bool multiselection = false, QWidget *parent = 0);
    void insertItem(const DatabaseManager::ImageInfo &info);
    void removeItem(const QString &name);
    QStringList selectedImagesNameList();
    QStringList selectedImagesPathList();
    QString timeline() const;

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
        IdCopy,
        IdDelete,
        IdEdit,
        IdAddToFavorites,
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
    QPixmap generateSelectedThumanail(const QPixmap &pixmap);
    QPixmap increaseThumbnail(const QPixmap &pixmap);
    QString createMenuContent();
    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    void onMenuItemClicked(int menuId);

private:
    bool m_multiselection;
    QSize m_iconSize;
    QString m_timeline;
    ThumbnailListView *m_listView;
    QStandardItemModel m_standardModel;
};

#endif // TIMELINEVIEWFRAME_H
