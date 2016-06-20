#ifndef ALBUMSVIEW_H
#define ALBUMSVIEW_H

#include "controller/databasemanager.h"
#include <QListView>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItemModel>

class PopupMenuManager;
class SignalManager;
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
    void openAlbum(const QString &album);

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
        IdAlbumInfo,
        IdSubMenu,
        IdSeparator
    };

    QString getAlbumName(const QModelIndex &index);
    QString getNewAlbumName() const;
    QString createMenuContent(const QModelIndex &index);

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
    DatabaseManager *m_dbManager;
    SignalManager *m_sManager;
    PopupMenuManager *m_popupMenu;
};

#endif // ALBUMSVIEW_H
