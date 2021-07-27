#include "imageengine.h"

#include <QDebug>
#include <QThread>
#include <QStandardPaths>
#include <QDir>
#include <QMimeDatabase>
#include <QUrl>
#include <QCryptographicHash>

#include "service/commonservice.h"
#include "unionimage/imgoperate.h"
#include "unionimage/unionimage.h"
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
    q->connect(m_worker, &ImgOperate::sigOneImgReady, CommonService::instance(), &CommonService::slotSetImgInfoByPath);
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

void ImageEngine::makeImgThumbnail(QString thumbnailSavePath, QStringList paths, int makeCount)
{
    Q_D(ImageEngine);
    //执行子线程制作缩略图动作
    QMetaObject::invokeMethod(d->m_worker, "slotMakeImgThumbnail"
                              , Q_ARG(QString, thumbnailSavePath)
                              , Q_ARG(QStringList, paths)
                              , Q_ARG(int, makeCount)
                             );
}
//判断是否图片格式
bool ImageEngine::isImage(const QString &path)
{
    bool bRet = false;
    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForFile(path, QMimeDatabase::MatchContent);
    QMimeType mt1 = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension);
    if (mt.name().startsWith("image/") || mt.name().startsWith("video/x-mng") ||
            mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
        bRet = true;
    }
    return bRet;
}

bool ImageEngine::isRotatable(const QString &path)
{
    return UnionImage_NameSpace::isImageSupportRotate(path);
}
//根据文件路径制作md5
QString ImageEngine::makeMD5(const QString &path)
{
    const QUrl url = QUrl::fromLocalFile(path);
    return QCryptographicHash::hash(url.toString(QUrl::FullyEncoded).toLocal8Bit(), QCryptographicHash::Md5).toHex();
}
