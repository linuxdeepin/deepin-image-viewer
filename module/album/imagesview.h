#ifndef IMAGESVIEW_H
#define IMAGESVIEW_H

#include "topalbumtips.h"
#include <QWidget>
#include <QScrollArea>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QObject>

class PopupMenuManager;
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
    QString selectedImagePath() const;
    QString getCurrentAlbum() const;

protected:
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

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

    void initContent();
    void initListView();
    void initTopTips();

    QPixmap increaseThumbnail(const QPixmap &pixmap);
    QString createMenuContent();

    QJsonValue createMenuItem(const MenuItemId id,
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
    PopupMenuManager *m_popupMenu;
};

#endif // IMAGESVIEW_H
