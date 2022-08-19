#include "thumbnailload.h"
#include "unionimage/unionimage.h"

ThumbnailLoad::ThumbnailLoad()
    : QQuickImageProvider(QQuickImageProvider::Image)
{

}

QImage ThumbnailLoad::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString tempPath = QUrl(id).toLocalFile();
    QImage Img;
    QString error;

    QMutexLocker _locker(&m_mutex);
    if (!m_imgMap.keys().contains(tempPath)) {
        LibUnionImage_NameSpace::loadStaticImageFromFile(tempPath, Img, error);
        QImage reImg = Img.scaled(100, 100);
        m_imgMap[tempPath] = reImg;
        return reImg;
    } else {
        return m_imgMap[tempPath];
    }
}

QPixmap ThumbnailLoad::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString tempPath = QUrl(id).toLocalFile();
    QImage Img;
    QString error;

    QMutexLocker _locker(&m_mutex);
    LibUnionImage_NameSpace::loadStaticImageFromFile(tempPath, Img, error);
    return QPixmap::fromImage(Img);
}

bool ThumbnailLoad::imageIsNull(const QString &path)
{
    QString tempPath = QUrl(path).toLocalFile();

    QMutexLocker _locker(&m_mutex);
    if (m_imgMap.find(tempPath) != m_imgMap.end()) {
        return m_imgMap[tempPath].isNull();
    }

    return false;
}

LoadImage::LoadImage(QObject *parent) :
    QObject(parent)
{
    m_pThumbnail = new ThumbnailLoad();
    m_viewLoad = new ViewLoad();
    m_multiLoad = new MultiImageLoad();
}

double LoadImage::getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight)
{
    if (Invalid != m_FrameIndex) {
        return m_multiLoad->getFitWindowScale(path, WindowWidth, WindowHeight, m_FrameIndex);
    }

    return m_viewLoad->getFitWindowScale(path, WindowWidth, WindowHeight);
}

bool LoadImage::imageIsNull(const QString &path)
{
    return m_pThumbnail->imageIsNull(path);
}

int LoadImage::getImageWidth(const QString &path)
{
    if (Invalid != m_FrameIndex) {
        return m_multiLoad->getImageWidth(path, m_FrameIndex);
    }

    return m_viewLoad->getImageWidth(path);
}

int LoadImage::getImageHeight(const QString &path)
{
    if (Invalid != m_FrameIndex) {
        return m_multiLoad->getImageHeight(path, m_FrameIndex);
    }

    return m_viewLoad->getImageHeight(path);
}

double LoadImage::getrealWidthHeightRatio(const QString &path)
{
    if (Invalid != m_FrameIndex) {
        double width = double(m_multiLoad->getImageWidth(path, m_FrameIndex));
        double height = double(m_multiLoad->getImageHeight(path, m_FrameIndex));
        return width / height;
    }

    double width = double(m_viewLoad->getImageWidth(path));
    double height = double(m_viewLoad->getImageHeight(path));
    return  width / height;
}

void LoadImage::setMultiFrameIndex(int index)
{
    m_FrameIndex = index;
}

void LoadImage::loadThumbnail(const QString path)
{
    QString tempPath = QUrl(path).toLocalFile();
    qDebug() << "----path--" << tempPath;
    QImage Img;
    QString error;
    if (LibUnionImage_NameSpace::loadStaticImageFromFile(tempPath, Img, error)) {
        m_pThumbnail->m_Img = Img;
        emit callQmlRefeshImg();
    } else {
        qDebug() << "load failded,the error is:" << error;
    }
}

void LoadImage::catThumbnail(const QStringList &list)
{
    if (list.size() < 1) {
        return;
    }
    for (QString path : list) {
        QString imgPath = path;

        if (imgPath.startsWith("file://"))
            imgPath.remove(0, 7);
        QImage tImg(imgPath);
        //保持横纵比裁切
        if (abs((tImg.width() - tImg.height()) * 10 / tImg.width()) >= 1) {
            QRect rect = tImg.rect();
            int x = rect.x() + tImg.width() / 2;
            int y = rect.y() + tImg.height() / 2;
            if (tImg.width() > tImg.height()) {
                x = x - tImg.height() / 2;
                y = 0;
                tImg = tImg.copy(x, y, tImg.height(), tImg.height());
            } else {
                y = y - tImg.width() / 2;
                x = 0;
                tImg = tImg.copy(x, y, tImg.width(), tImg.width());
            }
        }
        //压缩画质
        if (0 != tImg.height() && 0 != tImg.width() && (tImg.height() / tImg.width()) < 10 && (tImg.width() / tImg.height()) < 10) {
            if (tImg.height() != /*m_height*/100 || tImg.width() != /*m_with*/100) {
                if (tImg.height() >= tImg.width()) {
                    tImg = tImg.scaledToWidth(/*m_with*/100,  Qt::FastTransformation);
                } else if (tImg.height() <= tImg.width()) {
                    tImg = tImg.scaledToHeight(/*m_height*/100,  Qt::FastTransformation);
                }
            }
        }
    }
}

void LoadImage::loadThumbnails(const QStringList list)
{
    QImage Img;
    QString error;
    for (QString path : list) {
        loadThumbnail(path);
    }
}


ViewLoad::ViewLoad()
    : QQuickImageProvider(QQuickImageProvider::Image)
{

}

QImage ViewLoad::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString tempPath = QUrl(id).toLocalFile();
    QImage Img;
    QString error;

    QMutexLocker _locker(&m_mutex);
    if (tempPath == m_currentPath) {
        if (m_Img.size() != requestedSize && requestedSize.width() > 0 && requestedSize.height() > 0) {
            m_Img = m_Img.scaled(requestedSize);
        }
        return m_Img;
    }
    LibUnionImage_NameSpace::loadStaticImageFromFile(tempPath, Img, error);
    m_imgSizes[tempPath] = Img.size() ;
    m_Img = Img;
    m_currentPath = tempPath;
    if (m_Img.size() != requestedSize && requestedSize.width() > 0 && requestedSize.height() > 0) {
        Img = m_Img.scaled(requestedSize);
    }
    return Img;
}

QPixmap ViewLoad::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString tempPath = QUrl(id).toLocalFile();
    QImage Img;
    QString error;

    QMutexLocker _locker(&m_mutex);
    if (tempPath == m_currentPath) {
        return QPixmap::fromImage(m_Img);
    }
    LibUnionImage_NameSpace::loadStaticImageFromFile(tempPath, Img, error);
    m_imgSizes[tempPath] = Img.size();
    m_Img = Img;
    m_currentPath = tempPath;
    return QPixmap::fromImage(Img);
}

int ViewLoad::getImageWidth(const QString &path)
{
    QString tempPath = QUrl(path).toLocalFile();

    QMutexLocker _locker(&m_mutex);
    return m_imgSizes[tempPath].width();
}

int ViewLoad::getImageHeight(const QString &path)
{
    QString tempPath = QUrl(path).toLocalFile();

    QMutexLocker _locker(&m_mutex);
    return m_imgSizes[tempPath].height();
}

double ViewLoad::getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight)
{
    double scale = 0.0;
    double width = getImageWidth(path);
    double height = getImageHeight(path);
    double scaleWidth = width / WindowWidth;
    double scaleHeight = height / WindowHeight;

    if (scaleWidth > scaleHeight) {
        scale = scaleWidth;
    } else {
        scale = scaleHeight;
    }

    return scale;
}


MultiImageLoad::MultiImageLoad()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    m_imageCache.setMaxCost(256);
}

/**
 * @brief 外部请求多页图中指定帧的图像，指定帧号通过传入的 \a id 进行区分。
 *      \a id 格式为 \b{图像路径#frame_帧号_缩略图标识} ，例如 "/home/tmp.tif#frame_3_thumbnail" ，
 *      表示 tmp.tif 图像文件的第四帧图像缩略图。_thumbnail 可移除，表示需要完整图片
 *      这个 id 在 QML 文件中组合。
 * @param id            图像索引(0 ~ frameCount - 1)
 * @param size          图像的原始大小，有需要时可传出
 * @param requestedSize 请求的图像大小
 * @return 读取的图像数据
 *
 * @note 当前需要读取多页图的图像格式仅为 *.tif ，通过默认 QImageReader 即可读取，
 *      后续其它格式考虑在 LibUnionImage_NameSpace 中添加新的接口。
 */
QImage MultiImageLoad::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    qWarning() << this << "xxxxxxxxxxxxxx" << id;

    Q_UNUSED(size)
    // 拆分id，获取当前读取的文件和图片索引
    static const QString s_tagFrame = "#frame_";
    static const QString s_tagThumbnail = "_thumbnail";
    QString checkId = id;
    bool useThumbnail = checkId.endsWith(s_tagThumbnail);
    if (useThumbnail) {
        checkId.chop(s_tagThumbnail.size());
    }

    // 从后向前查询索引标识
    int index = checkId.lastIndexOf(QRegExp(QString("%1\\d+$").arg(s_tagFrame)));
    if (-1 == index) {
        return QImage();
    }
    QString path = checkId.left(index);
    // 移除 "#frame_" 字段
    int frame = checkId.right(checkId.size() - index - s_tagFrame.size()).toInt();

    QString tempPath = QUrl(path).toLocalFile();
    QImage img;

    // 数据变更前加锁
    QMutexLocker _locker(&m_mutex);
    if (tempPath != m_imageReader.fileName()) {
        // 重新设置图像读取类
        m_imageReader.setFileName(tempPath);
    }

    // 获取图像数据
    auto key = qMakePair(tempPath, frame);
    bool hasThumbnail = m_imageCache.contains(key);
    if (hasThumbnail && useThumbnail) {
        // 返回缓存缩略图信息
        img = m_imageCache.object(key)->imgThumbnail;
    } else {
        if (m_imageReader.jumpToImage(frame)) {
            // 读取图像数据
            img = m_imageReader.read();
            // 判断是否正常读取
            if (img.isNull()) {
                return img;
            }

            // 不存在缩略图信息，缓存图片
            if (!hasThumbnail) {
                m_imageCache.insert(key, new CacheImage(img));
            }
        }
    }

    // 调整图像大小
    if (!img.isNull() && img.size() != requestedSize && requestedSize.width() > 0 && requestedSize.height() > 0) {
        img = img.scaled(requestedSize);
    }
    return img;
}

/**
 * @brief 调用 requestImage() 获取图形信息，返回格式为 QPixmap 。
 */
QPixmap MultiImageLoad::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    return QPixmap::fromImage(requestImage(id, size, requestedSize));
}

/**
 * @return 根据传入的图片路径 \a path 和图片帧号索引 \a frameIndex , 返回图片的宽度值
 */
int MultiImageLoad::getImageWidth(const QString &path, int frameIndex)
{
    QString tempPath = QUrl(path).toLocalFile();
    auto key = qMakePair(tempPath, frameIndex);

    QMutexLocker _locker(&m_mutex);
    CacheImage *cache = m_imageCache.object(key);
    if (cache) {
        return cache->originSize.width();
    }
    return 0;
}

/**
 * @return 根据传入的图片路径 \a path 和图片帧号索引 \a frameIndex , 返回图片的高度值
 */
int MultiImageLoad::getImageHeight(const QString &path, int frameIndex)
{
    QString tempPath = QUrl(path).toLocalFile();
    auto key = qMakePair(tempPath, frameIndex);

    QMutexLocker _locker(&m_mutex);
    CacheImage *cache = m_imageCache.object(key);
    if (cache) {
        return cache->originSize.height();
    }
    return 0;
}

/**
 * @brief 根据传入的图片路径 \a path 和图片帧号索引 \a frameIndex ,
 *      返回图片相对适应窗口的缩放比值。
 * @param path          图片路径
 * @param WindowWidth   适应窗口宽度
 * @param WindowHeight  适应窗口高度
 * @param frameIndex    图片帧号索引
 * @return 图片相对适应窗口的缩放比值
 */
double MultiImageLoad::getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight, int frameIndex)
{
    double scale = 0.0;
    double width = getImageWidth(path, frameIndex);
    double height = getImageHeight(path, frameIndex);
    double scaleWidth = width / WindowWidth;
    double scaleHeight = height / WindowHeight;

    if (scaleWidth > scaleHeight) {
        scale = scaleWidth;
    } else {
        scale = scaleHeight;
    }

    return scale;
}

MultiImageLoad::CacheImage::CacheImage(const QImage &img)
{
    // 缩略图大小
    static const QSize s_ThumbnailSize(100, 100);
    imgThumbnail = img.scaled(s_ThumbnailSize);
    originSize = img.size();
}
