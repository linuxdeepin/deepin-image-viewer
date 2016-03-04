#ifndef TIMELINEVIEWFRAME_H
#define TIMELINEVIEWFRAME_H

#include <QLabel>
#include <QWidget>
#include <QFrame>
#include <QListView>
#include <QStandardItem>
#include <QVBoxLayout>
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include "widgets/thumbnaillistview.h"

class TimelineViewFrame : public QFrame
{
    Q_OBJECT
public:
    explicit TimelineViewFrame(const QString &timeline, QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent *e);

private:
    void initListView();
    void insertItem(const DatabaseManager::ImageInfo &info);
    void removeItem(const QString &name);

private:
    QString m_timeline;
    ThumbnailListView *m_listView;
    QStandardItemModel m_standardModel;
};

#endif // TIMELINEVIEWFRAME_H
