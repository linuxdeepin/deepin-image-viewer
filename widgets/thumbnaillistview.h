#ifndef THUMBNAILLISTVIEW_H
#define THUMBNAILLISTVIEW_H

#include <QListView>

class ThumbnailListView : public QListView
{
    Q_OBJECT
public:
    explicit ThumbnailListView(QWidget *parent = 0);

signals:
    void mousePress(QMouseEvent *e);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void fixedViewPortSize();

private:
    void initStyleSheet();

};

#endif // THUMBNAILLISTVIEW_H
