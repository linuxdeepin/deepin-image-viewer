#ifndef THUMBNAILLISTVIEW_H
#define THUMBNAILLISTVIEW_H

#include <QListView>

class ThumbnailListView : public QListView
{
    Q_OBJECT
public:
    explicit ThumbnailListView(QWidget *parent = 0);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void fixedViewPortSize();

private:
    void initStyleSheet();

};

#endif // THUMBNAILLISTVIEW_H
