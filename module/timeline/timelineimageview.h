#ifndef TIMELINEIMAGEVIEW_H
#define TIMELINEIMAGEVIEW_H

#include "sliderframe.h"
#include "toptimelinetips.h"
#include "timelineviewframe.h"
#include <QMap>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QItemSelectionModel>

class ScrollBar;
class TimelineImageView : public QScrollArea
{
    Q_OBJECT
public:
    explicit TimelineImageView(bool multiselection = false, QWidget *parent = 0);
    void clearSelection();
    void setIconSize(const QSize &iconSize);
    void updateThumbnail(const QString &name);
    bool isEmpty() const;
    QMap<QString, QString> selectedImages() const;
    QString currentMonth();

public slots:
    void insertReadyFrames();
    void clearImages();
    void onImageInserted(const DatabaseManager::ImageInfo &info);

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    void initSliderFrame();
    void initTopTips();
    void initContents();

    void inserFrame(const QString &timeline, bool multiselection = false);
    void removeFrame(const QString &timeline);
    void removeImage(const QString &name);
    void updateSliderFrmaeRect();
    void updateContentRect();
    void updateTopTipsRect();
    int getMinContentsWidth();
    QString currentTimeline();
    QString getMonthByTimeline(const QString &timeline);
    double scrollingPercent();

private:
    ScrollBar *m_vScrollBar;
    SliderFrame *m_sliderFrame;
    TopTimelineTips *m_topTips;
    QVBoxLayout *m_contentLayout;
    QFrame *m_contentFrame;
    QMap<QString, TimelineViewFrame *> m_frames;
    bool m_ascending;
    bool m_multiSelection;
    int m_scrollValue;
    QSize m_iconSize;
};

#endif // TIMELINEIMAGEVIEW_H
