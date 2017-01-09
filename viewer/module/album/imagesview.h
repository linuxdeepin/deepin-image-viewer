#ifndef IMAGESVIEW_H
#define IMAGESVIEW_H

#include "topalbumtips.h"
#include "controller/dbmanager.h"
#include "controller/viewerthememanager.h"
#include "widgets/thumbnaillistview.h"

#include <QWidget>
#include <QScrollArea>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QObject>
#include <QThread>
#include <QStackedWidget>

class ImportFrame;
class LoadThread;
class QMenu;
class QStandardItemModel;
class QStackedWidget;

class ImagesView : public QStackedWidget
{
    Q_OBJECT
public:
    explicit ImagesView(QWidget *parent = 0);

    void cancelLoadImages();
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
    void rotateFinished();
    void changeItemSize(bool increase);
    void startSlideShow(const QStringList &paths, const QString &path);
    void viewImage(const QString &path,
                   const QStringList &paths,
                   bool fullscreen = false);

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

private:
    enum MenuItemId {
        IdView,
        IdFullScreen,
        IdStartSlideShow,
        IdPrint,
        IdAddToAlbum,
        IdExport,
        IdCopy,
        IdCopyToClipboard,
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

    void initItems();
    void initPopupMenu();
    void initListView();
    void initContent();
    void initTopTips();

    void appendAction(int id, const QString &text, const QString &shortcut);
    void onMenuItemClicked(QAction *action);
    void popupDelDialog(const QStringList &paths);
    void showPrintDialog(const QStringList &paths);
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
    QMenu *m_menu;
    ImportFrame *m_importFrame;
    QList<LoadThread *> m_loadingThreads;
};

#endif // IMAGESVIEW_H
