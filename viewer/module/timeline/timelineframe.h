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
    void updateThumbnails(const QString &path);
    void updateView();

    bool isEmpty() const;
    const QString currentMonth() const;
    const QStringList selectedPaths() const;

signals:
    void currentIndexChanged(const QModelIndex &current);
    void changeItemSize(bool increase);
    void showMenu();
    void viewImage(const QString &path, const QStringList &paths);

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

private:
    void initTopTip();
    void initItems();
    void insertItems(const DBImgInfoList &infos);
    void removeItem(const DBImgInfo &info);

private:
    Anchors<TopTimelineTip> m_tip;
    DBImgInfoList           m_infos;
    TimelineModel           m_model;
    TimelineView            *m_view;
};

#endif // TIMELINEFRAME_H
