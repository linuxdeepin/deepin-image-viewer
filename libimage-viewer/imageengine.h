#ifndef IMAGEENGINE_H
#define IMAGEENGINE_H

#include "image-viewer_global.h"

#include <DWidget>
#include <QtCore/qglobal.h>
#include <QImage>

DWIDGET_USE_NAMESPACE

class ImageEnginePrivate;
class IMAGEVIEWERSHARED_EXPORT ImageEngine : public QObject
{
    Q_OBJECT
public:
    static ImageEngine *instance(QObject *parent = nullptr);
    explicit ImageEngine(QWidget *parent = nullptr);
    ~ImageEngine() override;

    //制作图片缩略图
    void makeImgThumbnail(QString thumbnailSavePath, QStringList paths);
signals:
    //一张缩略图制作完成
    void sigOneImgReady(QString path, QImage image);
private:

    static ImageEngine *m_ImageEngine;

    QScopedPointer<ImageEnginePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ImageEngine)
};

#endif // IMAGEENGINE_H
