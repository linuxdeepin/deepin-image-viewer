#include "imageengine.h"

#include <QDebug>
#include <QThread>

#include "unionimage/imgoperate.h"

class ImageEnginePrivate : public QObject
{
public:
    explicit ImageEnginePrivate(ImageEngine *parent = nullptr);
    ~ImageEnginePrivate();

private:
    ImageEngine *const q_ptr;
    ImgOperate *m_worker = nullptr;
    Q_DECLARE_PUBLIC(ImageEngine)
};

ImageEnginePrivate::ImageEnginePrivate(ImageEngine *parent): QObject(parent), q_ptr(parent)
{
    QThread *workerThread = new QThread(this);
    m_worker = new ImgOperate(workerThread);

    m_worker->moveToThread(workerThread);
//    //开始录制
//    connect(this, &ImageEngineApi::sigLoadThumbnailsByNum, m_worker, &DBandImgOperate::sltLoadThumbnailByNum);
//    connect(this, &ImageEngineApi::sigLoadThumbnailIMG, m_worker, &DBandImgOperate::loadOneImg);
//    //加载设备中文件列表
//    connect(this, &ImageEngineApi::sigLoadMountFileList, m_worker, &DBandImgOperate::sltLoadMountFileList);

//    //收到获取全部照片信息成功信号
//    connect(m_worker, &DBandImgOperate::sig80ImgInfosReady, this, &ImageEngineApi::slt80ImgInfosReady);
//    connect(m_worker, &DBandImgOperate::sigOneImgReady, this, &ImageEngineApi::sigOneImgReady);
//    //加载设备中文件列表完成，发送到主线程
//    connect(m_worker, &DBandImgOperate::sigMountFileListLoadReady, this, &ImageEngineApi::sigMountFileListLoadReady);
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
{
    qDebug() << "test";
}

ImageEngine::~ImageEngine()
{

}
