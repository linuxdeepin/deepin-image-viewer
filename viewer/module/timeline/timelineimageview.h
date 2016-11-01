#ifndef TIMELINEIMAGEVIEW_H
#define TIMELINEIMAGEVIEW_H

#include "controller/dbmanager.h"
#include "sliderframe.h"
#include "toptimelinetips.h"
#include "timelineviewframe.h"
#include <QMap>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QItemSelectionModel>

class TimelineImageView : public QScrollArea
{
    Q_OBJECT
public:
    explicit TimelineImageView(bool multiselection = false, QWidget *parent = 0);
    void clearSelection();
    void selectAll();
    void setIconSize(const QSize &iconSize);
    void updateThumbnails();
    bool isEmpty() const;
    QStringList selectedPaths() const;
    QString currentMonth();

public slots:
    void insertReadyFrames();
    void clearImages();

signals:
    void viewImage(const QString &path, const QStringList &paths);
    void showMenuRequested();
    void updateMenuRequested();

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    void initSliderFrame();
    void initTopTips();
    void initContents();

    void insertFrame(const QString &timeline);
    void removeFrame(const QString &timeline);
    void removeImages(const QStringList &paths);
    void updateSliderFrmaeRect();
    void updateContentRect();
    void updateTopTipsRect();
    int getMinContentsWidth();
    QString currentTimeline();
    QString getMonthByTimeline(const QString &timeline);
    double scrollingPercent();
    bool posInSelected(const QPoint &pos, TimelineViewFrame *frame);

private:
//    SliderFrame *m_sliderFrame;
    TopTimelineTips *m_topTips;
    QVBoxLayout *m_contentLayout;
    QFrame *m_contentFrame;
    QFrame *m_stretchFrame;
    QMap<QString, TimelineViewFrame *> m_frames;
    bool m_ascending;
    bool m_multiSelection;
    QSize m_iconSize;
};

#endif // TIMELINEIMAGEVIEW_H
