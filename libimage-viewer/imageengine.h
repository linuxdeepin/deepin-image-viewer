#ifndef IMAGEENGINE_H
#define IMAGEENGINE_H

#include "image-viewer_global.h"

#include <DWidget>
#include <QtCore/qglobal.h>

DWIDGET_USE_NAMESPACE

class ImageEnginePrivate;
class IMAGEVIEWERSHARED_EXPORT ImageEngine : public QObject
{
    Q_OBJECT
public:
    static ImageEngine *instance(QObject *parent = nullptr);
    explicit ImageEngine(QWidget *parent = nullptr);
    ~ImageEngine() override;

    QImage getImg(QStringList paths, bool synchronous = true);
private:

    static ImageEngine *m_ImageEngine;

    Q_DECLARE_PRIVATE(ImageEngine)
    Q_DISABLE_COPY(ImageEngine)
};

#endif // IMAGEENGINE_H
