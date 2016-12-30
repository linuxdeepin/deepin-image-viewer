#ifndef ALBUMSVIEW_H
#define ALBUMSVIEW_H

#include "controller/dbmanager.h"
#include <QListView>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItemModel>

class AlbumDelegate;
class QMenu;
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
    void changeItemSize(bool increase);
    void openAlbum(const QString &album);
    void startSlideShow(const QStringList &paths);

protected:
    int horizontalOffset() const Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;

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

    void appendAction(int id, const QString &text, const QString &shortcut, bool enable = true);
    void appendCreateIcon();
    void destroyEditor(const QModelIndex &index);
    bool isCreateIcon(const QModelIndex &index) const;
    void onClicked(const QModelIndex &index);
    void onDoubleClicked(const QModelIndex &index);
    void onMenuItemClicked(QAction *action);
    const QString getAlbumName(const QModelIndex &index) const;
    const QString getNewAlbumName() const;
    const QStringList paths(const QString &album) const;
    void removeCreateIcon();
    void updateMenuContent(const QModelIndex &index);

private:
    AlbumDelegate *m_delegate;
    QStandardItemModel *m_model;
    QSize m_itemSize;
    QMenu *m_menu;
};

#endif // ALBUMSVIEW_H
