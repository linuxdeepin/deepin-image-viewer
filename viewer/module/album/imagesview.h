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

class ImportFrame;
class PopupMenuManager;
class QStandardItemModel;
class QStackedWidget;
class ThumbnailListView;
class ImagesView : public QScrollArea
{
    Q_OBJECT
public:
    explicit ImagesView(QWidget *parent = 0);

    void insertItem(const DatabaseManager::ImageInfo &info, bool update = true);
    void setAlbum(const QString &album);
    void setIconSize(const QSize &iconSize);

    int count() const;
    bool removeItem(const QString &name);
    void removeItems(const QStringList &names);

    QString getCurrentAlbum() const;
    QSize iconSize() const;
    QStringList selectedImagesNameList() const;
    QStringList selectedImagesPathList() const;

signals:
    void rotated();
    void startSlideShow(const QStringList &paths, const QString &path);
    void viewImage(const QString &path,
                   const QStringList &paths,
                   bool fullscreen = false);

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;

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
    void initContent();
    void initTopTips();

    void onMenuItemClicked(int menuId, const QString &text);
    void rotateImage(const QString &path, int degree);
    void updateMenuContents();
    void updateContent();
    void updateTopTipsRect();
    bool allInAlbum(const QStringList &names, const QString &album);
    void popupDelDialog(const QStringList paths, const QStringList names);

    QJsonObject createAlbumMenuObj();
    QString createMenuContent();
    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    const QStringList paths();

private:
    QStringList m_rotateList;
    QString m_album;
    TopAlbumTips *m_topTips;
    ThumbnailListView *m_view;
    QVBoxLayout *m_contentLayout;
    PopupMenuManager *m_popupMenu;
    QWidget *m_contentWidget;
    ImportFrame *m_importFrame;
};

#endif // IMAGESVIEW_H
