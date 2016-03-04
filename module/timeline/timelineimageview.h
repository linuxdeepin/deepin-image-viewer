#ifndef TIMELINEIMAGEVIEW_H
#define TIMELINEIMAGEVIEW_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

class TimelineImageView : public QScrollArea
{
    Q_OBJECT
public:
    explicit TimelineImageView(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent *e);
private:
    void inserFrame(const QString &timeline);
    void removeFrame(const QString &tileline);

private:
    QVBoxLayout *m_contentLayout;
    QFrame *m_contentFrame;
};

#endif // TIMELINEIMAGEVIEW_H
