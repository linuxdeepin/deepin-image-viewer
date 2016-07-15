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
class QStackedWidget;
class ImagesView : public QScrollArea
{
    Q_OBJECT
public:
    explicit ImagesView(QWidget *parent = 0);
    void setAlbum(const QString &album);
    void insertItem(const DatabaseManager::ImageInfo &info);
    bool removeItem(const QString &name);

    int count() const;
    QSize iconSize() const;
    void setIconSize(const QSize &iconSize);
    QStringList selectedImagesNameList() const;
    QStringList selectedImagesPathList() const;
    QString getCurrentAlbum() const;

signals:
    void startSlideShow(const QStringList &paths, const QString &path);

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

    void initStack();
    void updateStack();
    void initListView();
    void initTopTips();

    QString createMenuContent();
    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    QString currentSelectOne(bool isPath = true);
    const QStringList paths();

    void updateThumbnail(const QString &path);
    void updateMenuContents();
    void onMenuItemClicked(int menuId);

    void updateTopTipsRect();
    int getMinContentsWidth();

private:
    QString m_album;
    QStackedWidget *m_stackWidget;
    TopAlbumTips *m_topTips;
    ThumbnailListView *m_view;
    QVBoxLayout *m_contentLayout;
    PopupMenuManager *m_popupMenu;
    DatabaseManager *m_dbManager;
    SignalManager *m_sManager;
};

#endif // IMAGESVIEW_H
