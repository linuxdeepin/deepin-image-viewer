#ifndef ALBUMSVIEW_H
#define ALBUMSVIEW_H

#include "controller/databasemanager.h"
#include <QListView>
#include <QStandardItemModel>

class AlbumsView : public QListView
{
    Q_OBJECT
public:
    explicit AlbumsView(QWidget *parent = 0);
    void addAlbum(const DatabaseManager::AlbumInfo &info);

    QSize itemSize() const;
    void setItemSize(const QSize &itemSize);

signals:
    void openAlbum(const QString &album);

private:
    void onDoubleClicked(const QModelIndex &index);

private:
    QStandardItemModel *m_itemModel;
    QSize m_itemSize;
};

#endif // ALBUMSVIEW_H
