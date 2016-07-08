#ifndef IMAGESVIEW_H
#define IMAGESVIEW_H

#include "topalbumtips.h"
#include "controller/databasemanager.h"
#include <QWidget>
#include <QScrollArea>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QObject>

class PopupMenuManager;
class SignalManager;
class ThumbnailListView;
class QStandardItemModel;
class ImagesView : public QScrollArea
{
    Q_OBJECT
public:
    explicit ImagesView(QWidget *parent = 0);
    void setAlbum(const QString &album);
    void updateView();
    void insertItem(const DatabaseManager::ImageInfo &info);
    bool removeItem(const QString &name);

    QSize iconSize() const;
    void setIconSize(const QSize &iconSize);
    QStringList selectedImagesNameList() const;
    QStringList selectedImagesPathList() const;
    QString getCurrentAlbum() const;

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    enum MenuItemId {
        IdView,
        IdFullScreen,
        IdStartSlideShow,
        IdAddToAlbum,
        IdExport,
        IdCopy,
        IdMoveToTrash,
        IdRemoveFromAlbum,
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

    QString currentSelectOne(bool isPath = true);
    QString createMenuContent();

    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());

    void updateThumbnail(const QString &path);
    void updateMenuContents();
    void onMenuItemClicked(int menuId);

    void updateTopTipsRect();
    int getMinContentsWidth();

private:
    QString m_album;
    TopAlbumTips *m_topTips;
    ThumbnailListView *m_view;
    QVBoxLayout *m_contentLayout;
    QWidget *m_contentWidget;
    PopupMenuManager *m_popupMenu;
    DatabaseManager *m_dbManager;
    SignalManager *m_sManager;
};

#endif // IMAGESVIEW_H
