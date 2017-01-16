#ifndef TIMELINEFRAME_H
#define TIMELINEFRAME_H

#include "controller/dbmanager.h"
#include "mvc/timelinemodel.h"
#include "anchors.h"
#include <QFrame>

DWIDGET_USE_NAMESPACE

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
    void updateThumbnail(const QString &path);
    void updateScrollRange();

    bool isEmpty() const;
    const QString currentMonth() const;
    const QStringList selectedPaths() const;

signals:
    void selectIndexChanged(const QModelIndex &current);
    void changeItemSize(bool increase);
    void showMenu();
    void viewImage(const QString &path, const QStringList &paths);

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

private:
    void initConnection();
    void initView();
    void initTopTip();
    void initItems();
    void insertItems(const TimelineItem::ItemData &data);
    void removeItem(const DBImgInfo &info);
    void removeItems(const DBImgInfoList &infos);

private:
    Anchors<TopTimelineTip> m_tip;
    DBImgInfoList           m_infos;
    QMutex                  m_mutex;
    TimelineModel           m_model;
    TimelineView            *m_view;
};

#endif // TIMELINEFRAME_H
