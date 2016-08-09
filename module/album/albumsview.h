#ifndef ALBUMSVIEW_H
#define ALBUMSVIEW_H

#include "controller/databasemanager.h"
#include <QListView>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItemModel>

class PopupMenuManager;
class AlbumsView : public QListView
{
    Q_OBJECT
public:
    explicit AlbumsView(QWidget *parent = 0);
    QModelIndex addAlbum(const DatabaseManager::AlbumInfo &info);
    void createAlbum();
    void updateView();

    QSize itemSize() const;
    void setItemSizeMultiple(int multiple);

signals:
    void albumCreated();
    void albumRemoved();
    void openAlbum(const QString &album);
    void startSlideShow(const QStringList &paths);
    void paintRequest();

protected:
    bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

private:
    enum MenuItemId {
        IdCreate,
        IdView,
        IdStartSlideShow,
        IdRename,
        IdExport,
        IdCopy,
        IdDelete,
// TODO: Add albumInfo in menu
//        IdAlbumInfo,
        IdSubMenu,
        IdSeparator
    };

    const QString createMenuContent(const QModelIndex &index);
    const QString getAlbumName(const QModelIndex &index) const;
    const QString getNewAlbumName() const;
    const QStringList paths(const QString &album) const;

    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    void onMenuItemClicked(int menuId);
    void onDoubleClicked(const QModelIndex &index);

private:
    QStandardItemModel *m_model;
    QSize m_itemSize;
    PopupMenuManager *m_popupMenu;
};

#endif // ALBUMSVIEW_H
