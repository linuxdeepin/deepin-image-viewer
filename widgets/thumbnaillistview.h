#ifndef THUMBNAILLISTVIEW_H
#define THUMBNAILLISTVIEW_H

#include <QListView>

class ThumbnailListView : public QListView
{
    Q_OBJECT
public:
    explicit ThumbnailListView(QWidget *parent = 0);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void fixedViewPortSize();
};

#endif // THUMBNAILLISTVIEW_H
