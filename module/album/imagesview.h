#ifndef IMAGESVIEW_H
#define IMAGESVIEW_H

#include "topalbumtips.h"
#include <QWidget>
#include <QScrollArea>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QVBoxLayout>

namespace {
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
}  // namaspace

class ThumbnailListView;
class QStandardItemModel;
class ImagesView : public QScrollArea
{
    Q_OBJECT
public:
    explicit ImagesView(QWidget *parent = 0);
    void setAlbum(const QString &album);

    QSize iconSize() const;
    void setIconSize(const QSize &iconSize);

protected:
    void resizeEvent(QResizeEvent *e);

private:
    void initContent();
    void initListView();
    void initTopTips();

    QPixmap increaseThumbnail(const QPixmap &pixmap);
    QString createMenuContent();

    QJsonObject createItemObj(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    void onMenuItemClicked(int menuId);

    void updateContentRect();
    void updateTopTipsRect();
    int getMinContentsWidth();

private:
    QString m_currentAlbum;
    TopAlbumTips *m_topTips;
    ThumbnailListView *m_listView;
    QStandardItemModel m_standardModel;
    QVBoxLayout *m_contentLayout;
    QWidget *m_contentWidget;
};

#endif // IMAGESVIEW_H
