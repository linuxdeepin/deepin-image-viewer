#ifndef TIMELINEFRAME_H
#define TIMELINEFRAME_H

#include "controller/dbmanager.h"
#include "mvc/timelinemodel.h"
#include <QFrame>

class TimelineView;
class TopTimelineTip;

class TimelineFrame : public QFrame
{
    Q_OBJECT
public:
    explicit TimelineFrame(QWidget *parent = 0);
    void clearSelection();
    void selectAll();
    void setIconSize(int size);
    void updateThumbnails();

    bool isEmpty() const;
    const QString currentMonth() const;
    const QStringList selectedPaths() const;

signals:
    void currentIndexChanged(const QModelIndex &current);
    void changeItemSize(bool increase);
    void showMenu();
    void viewImage(const QString &path, const QStringList &paths);

private:
    void initTopTip();
    void initItems();
    void insertItems(const DBImgInfoList &infos);
    void removeItem(const DBImgInfo &info);

private:
    DBImgInfoList       m_infos;
    TimelineModel       m_model;
    TimelineView        *m_view;
    TopTimelineTip      *m_tip;
};

#endif // TIMELINEFRAME_H
