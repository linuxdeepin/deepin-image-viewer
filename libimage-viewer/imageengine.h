#ifndef IMAGEENGINE_H
#define IMAGEENGINE_H

#include "image-viewer_global.h"

#include <DWidget>

DWIDGET_USE_NAMESPACE

class IMAGEVIEWERSHARED_EXPORT ImageEngine : public QObject
{
    Q_OBJECT
public:
    explicit ImageEngine(QWidget *parent = nullptr);
    ~ImageEngine() override;
};

#endif // IMAGEENGINE_H
