#ifndef IMAGESVIEW_H
#define IMAGESVIEW_H

#include "topalbumtips.h"
#include "controller/dbmanager.h"
#include <QWidget>
#include <QScrollArea>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QObject>

class ImportFrame;
class QMenu;
class QStandardItemModel;
class QStackedWidget;
class ThumbnailListView;
class ImagesView : public QScrollArea
{
    Q_OBJECT
public:
    explicit ImagesView(QWidget *parent = 0);

    void insertItem(const DBImgInfo &info, bool update = true);
    void insertItems(const DBImgInfoList &infos);
    void setAlbum(const QString &album);
    void setIconSize(const QSize &iconSize);

    int count() const;
    void removeItems(const QStringList &paths);

    QString getCurrentAlbum() const;
    QSize iconSize() const;
    QStringList selectedPaths() const;

signals:
    void changeItemSize(bool increase);
    void rotated();
    void startSlideShow(const QStringList &paths, const QString &path);
    void viewImage(const QString &path,
                   const QStringList &paths,
                   bool fullscreen = false);

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;

private:
    enum MenuItemId {
        IdView,
        IdFullScreen,
        IdStartSlideShow,
        IdPrint,
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

    void initPopupMenu();
    void initListView();
    void initContent();
    void initTopTips();

    void appendAction(int id, const QString &text, const QString &shortcut);
    void onMenuItemClicked(QAction *action);
    void popupDelDialog(const QStringList &paths);
    void rotateImage(const QString &path, int degree);
    void updateMenuContents();
    void updateContent();
    void updateTopTipsRect();
    bool allInAlbum(const QStringList &paths, const QString &album);

    QMenu* createAlbumMenu();
    const QStringList albumPaths();

private:
    QStringList m_rotateList;
    QString m_album;
    TopAlbumTips *m_topTips;
    ThumbnailListView *m_view;
    QVBoxLayout *m_contentLayout;
    QMenu *m_menu;
    QWidget *m_contentWidget;
    ImportFrame *m_importFrame;
};

#endif // IMAGESVIEW_H
