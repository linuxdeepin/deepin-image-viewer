#ifndef STACKWIDGET_H
#define STACKWIDGET_H

#include <QWidget>
#include <QStackedWidget>

#include "thumbnailwidget.h"
#include "../libimage-viewer/imageviewer.h"

class StackWidget :public QStackedWidget
{
    Q_OBJECT
public:
    explicit StackWidget(QWidget *parent = nullptr);
    ~StackWidget() override;
public slots:
    void slotOpenImage();
private:
    ThumbnailWidget *thumbnailWidget=nullptr;
    ImageViewer *m_viewer=nullptr;
};

#endif // STACKWIDGET_H
