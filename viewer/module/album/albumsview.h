#ifndef ALBUMSVIEW_H
#define ALBUMSVIEW_H

#include "controller/dbmanager.h"
#include <QListView>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItemModel>

class AlbumDelegate;
class PopupMenuManager;
class AlbumsView : public QListView
{
    Q_OBJECT
public:
    explicit AlbumsView(QWidget *parent = 0);
    QModelIndex addAlbum(const DBAlbumInfo &info);
    void createAlbum();
    void updateView();

    int indexOf(const QString &name) const;
    QSize itemSize() const;
    void setItemSizeMultiple(int multiple);
    void popupDelDialog(const QString &albumName);

signals:
    void albumCreated();
    void albumRemoved();
    void openAlbum(const QString &album);
    void startSlideShow(const QStringList &paths);

protected:
    bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    int horizontalOffset() const Q_DECL_OVERRIDE;

private:
    enum MenuItemId {
        IdCreate,
        IdView,
        IdStartSlideShow,
        IdRename,
        IdExport,
        IdCopy,
        IdDelete,
        IdSubMenu,
        IdSeparator
    };

    bool isCreateIcon(const QModelIndex &index) const;

    const QString createMenuContent(const QModelIndex &index);
    const QString getAlbumName(const QModelIndex &index) const;
    const QString getNewAlbumName() const;
    const QStringList paths(const QString &album) const;

    QJsonValue createMenuItem(const MenuItemId id,
                              const QString &text,
                              const bool isSeparator = false,
                              const QString &shortcut = "",
                              const QJsonObject &subMenu = QJsonObject());
    void onClicked(const QModelIndex &index);
    void onDoubleClicked(const QModelIndex &index);
    void onMenuItemClicked(int menuId);
    void removeCreateIcon();
    void appendCreateIcon();

private:
    AlbumDelegate *m_delegate;
    QStandardItemModel *m_model;
    QSize m_itemSize;
    PopupMenuManager *m_popupMenu;
};

#endif // ALBUMSVIEW_H
