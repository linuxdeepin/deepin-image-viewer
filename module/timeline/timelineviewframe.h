#ifndef TIMELINEVIEWFRAME_H
#define TIMELINEVIEWFRAME_H

#include "controller/databasemanager.h"
#include "widgets/thumbnaillistview.h"
#include <QLabel>
#include <QWidget>
#include <QFrame>
#include <QListView>
#include <QStandardItem>
#include <QVBoxLayout>

class SignalManager;
class TimelineViewFrame : public QFrame
{
    Q_OBJECT
public:
    explicit TimelineViewFrame(const QString &timeline,
                               bool multiselection,
                               QWidget *parent);
    void insertItem(const DatabaseManager::ImageInfo &info);
    bool removeItem(const QString &name);
    void clearSelection() const;

    QMap<QString, QString> selectedImages() const;
    QString timeline() const;
    bool isEmpty() const;
    bool contain(const QModelIndex &index) const;

    QSize viewSize() const;
    QSize iconSize() const;
    void setIconSize(const QSize &iconSize);

signals:
    void mousePress();
    void clicked(const QModelIndex &index);

protected:
    void resizeEvent(QResizeEvent *e);

private:
    void initListView();

    DatabaseManager::ImageInfo imageInfo(const QString &name);
    QString currentSelectOne(bool isPath = true);
    QPixmap generateSelectedThumanail(const QPixmap &pixmap);
    QPixmap increaseThumbnail(const QPixmap &pixmap);

    void updateThumbnail(const QString &name);

private:
    bool m_multiselection;
    QSize m_iconSize;
    QString m_timeline;
    ThumbnailListView *m_listView;
    QStandardItemModel m_standardModel;
    DatabaseManager *m_dbManager;
    SignalManager *m_sManager;
};

#endif // TIMELINEVIEWFRAME_H
