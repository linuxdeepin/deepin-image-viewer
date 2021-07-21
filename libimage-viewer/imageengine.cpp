#include "imageengine.h"

#include <QDebug>
#include <QThread>
#include <QStandardPaths>
#include <QDir>

#include "unionimage/imgoperate.h"

class ImageEnginePrivate
{
public:
    explicit ImageEnginePrivate(ImageEngine *parent = nullptr);
    ~ImageEnginePrivate();

public:
    ImageEngine *const q_ptr;
    ImgOperate *m_worker = nullptr;
//    QString m_thumbnailSavePath;//缩略图保存路径
    Q_DECLARE_PUBLIC(ImageEngine)
};

ImageEnginePrivate::ImageEnginePrivate(ImageEngine *parent): q_ptr(parent)
{
    Q_Q(ImageEngine);

    QThread *workerThread = new QThread(q_ptr);
    m_worker = new ImgOperate(workerThread);
    m_worker->moveToThread(workerThread);
    //一张缩略图制作完成，发送到主线程
    q->connect(m_worker, &ImgOperate::sigOneImgReady, q, &ImageEngine::sigOneImgReady);
    workerThread->start();
}

ImageEnginePrivate::~ImageEnginePrivate()
{

}

ImageEngine *ImageEngine::m_ImageEngine = nullptr;

ImageEngine *ImageEngine::instance(QObject *parent)
{
    Q_UNUSED(parent);
    if (!m_ImageEngine) {
        m_ImageEngine = new ImageEngine();
    }
    return m_ImageEngine;
}

ImageEngine::ImageEngine(QWidget *parent)
    : QObject(parent)
    , d_ptr(new ImageEnginePrivate(this))
{
//    Q_D(ImageEngine);
}

ImageEngine::~ImageEngine()
{

}

const QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                           + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin"/* + QDir::separator()*/;

void ImageEngine::makeImgThumbnail(QString thumbnailSavePath, QStringList paths)
{
    Q_D(ImageEngine);
//    d->m_thumbnailSavePath = thumbnailSavePath;
    //执行子线程制作缩略图动作
    QStringList list;
    list << "/home/zouya/Desktop/test/001.jpg";
    list << "/home/zouya/Desktop/test/002.jpg";
    QMetaObject::invokeMethod(d->m_worker, "slotMakeImgThumbnail", Q_ARG(QString, CACHE_PATH), Q_ARG(QStringList, list));
}
