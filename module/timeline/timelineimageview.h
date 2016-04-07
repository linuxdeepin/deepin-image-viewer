#ifndef TIMELINEIMAGEVIEW_H
#define TIMELINEIMAGEVIEW_H

#include <QMap>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

class TimelineViewFrame;

class TimelineImageView : public QScrollArea
{
    Q_OBJECT
public:
    explicit TimelineImageView(QWidget *parent = 0);
    void setIconSize(const QSize &iconSize);
    QStringList selectedImages();

protected:
    void resizeEvent(QResizeEvent *e);
private:
    void inserFrame(const QString &timeline);
    void removeFrame(const QString &timeline);
    void updateContentRect();
    int getMinContentsWidth();

private:
    QVBoxLayout *m_contentLayout;
    QFrame *m_contentFrame;
    QMap<QString, TimelineViewFrame *> m_frames;
    bool m_ascending;
    QSize m_iconSize;
};

#endif // TIMELINEIMAGEVIEW_H
