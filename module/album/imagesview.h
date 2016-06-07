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

class DatabaseManager;
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

    QSize iconSize() const;
    void setIconSize(const QSize &iconSize);
    QStringList selectedImagesNameList() const;
    QStringList selectedImagesPathList() const;
    QString getCurrentAlbum() const;

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

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

    void updateThumbnail(const QString &name);
    void updateMenuContents();
    void onMenuItemClicked(int menuId);

    void updateContentRect();
    void updateTopTipsRect();
    int getMinContentsWidth();


private:
    QString m_currentAlbum;
    TopAlbumTips *m_topTips;
    ThumbnailListView *m_view;
    QStandardItemModel m_model;
    QVBoxLayout *m_contentLayout;
    QWidget *m_contentWidget;
    PopupMenuManager *m_popupMenu;
    DatabaseManager *m_dbManager;
    SignalManager *m_sManager;
};

#endif // IMAGESVIEW_H
