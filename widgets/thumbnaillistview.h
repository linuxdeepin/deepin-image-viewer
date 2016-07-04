#ifndef THUMBNAILLISTVIEW_H
#define THUMBNAILLISTVIEW_H

#include <QListView>

class ThumbnailListView : public QListView
{
    Q_OBJECT
public:
    explicit ThumbnailListView(QWidget *parent = 0);
    void updateViewPortSize();

signals:
    void mousePress(QMouseEvent *e);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void fixedViewPortSize(bool proactive = false);

};

#endif // THUMBNAILLISTVIEW_H
