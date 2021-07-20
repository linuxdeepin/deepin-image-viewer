#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include "image-viewer_global.h"

#include <DWidget>

DWIDGET_USE_NAMESPACE

class IMAGEVIEWERSHARED_EXPORT ImageViewer : public DWidget
{
    Q_OBJECT
public:
    explicit ImageViewer(QWidget *parent = nullptr);
    ~ImageViewer() override;
};

#endif // IMAGEVIEWER_H
