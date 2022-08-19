#ifndef THUMBNAILLOAD_H
#define THUMBNAILLOAD_H

#include <QQuickImageProvider>
#include <QQuickWindow>
#include <QImageReader>
#include <QImage>
#include <QCache>
#include <QMutex>

class ThumbnailLoad : public QQuickImageProvider
{
public:
    explicit ThumbnailLoad();
    //获取缩略图
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);  //预留
    bool imageIsNull(const QString &path);

    QMutex m_mutex;
    QImage m_Img;                       // 当前图片
    QMap<QString, QImage> m_imgMap;     // 缩略图缓存
};

class ViewLoad : public QQuickImageProvider
{
public:
    explicit ViewLoad();
    //获取缩略图
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);  //预留

    //获得当前图片的宽和高
    int getImageWidth(const QString &path);
    int getImageHeight(const QString &path);
    double getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight);

    QMutex                  m_mutex;
    QImage                  m_Img;          // 当前图片
    QString                 m_currentPath;  // 加载路径
    QMap<QString, QSize>    m_imgSizes;     // 图片大小
};

/**
 * @brief 提供用于 *.tif 等多页图的单独图像加载处理类
 *      通过分割传入的 id ，判断当前读取的文件的行数和图片索引。
 *      在 QML 中注册的标识为 "multiimage"
 * @warning QQuickImageProvider 派生的接口可能多线程调用，必须保证实现函数是可重入的。
 */
class MultiImageLoad : public QQuickImageProvider
{
public:
    explicit MultiImageLoad();

    // 请求加载图片，获取图片加载信息
    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

    // 获得当前图片的宽和高
    int getImageWidth(const QString &path, int frameIndex);
    int getImageHeight(const QString &path, int frameIndex);
    // 获取当前图片和适应窗口的缩放比值
    double getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight, int frameIndex);

private:
    QMutex              m_mutex;
    QString             m_lastReadPath;     // 之前读取的图像路径
    QImageReader        m_imageReader;      // 图像读取类

    // 缓存图片信息
    struct CacheImage {
        QImage  imgThumbnail;   // 图片缩略图
        QSize   originSize;     // 原始图片大小

        explicit CacheImage(const QImage &img);
    };
    QCache<QPair<QString, int>, CacheImage> m_imageCache;   // 缩略图缓存(默认最多缓存256组图像)
};

class LoadImage : public QObject
{
    Q_OBJECT
public:
    // 多页图帧号类型，Invalid 表示非多页图
    enum FrameType { Invalid = -1 };

    explicit LoadImage(QObject *parent = nullptr);

    ThumbnailLoad   *m_pThumbnail{nullptr};
    ViewLoad        *m_viewLoad{nullptr};
    MultiImageLoad  *m_multiLoad{nullptr};

    Q_INVOKABLE double getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight);
    Q_INVOKABLE bool imageIsNull(const QString &path);
    //获得当前图片的宽和高
    Q_INVOKABLE int getImageWidth(const QString &path);
    Q_INVOKABLE int getImageHeight(const QString &path);
    //获得宽高比例
    Q_INVOKABLE double getrealWidthHeightRatio(const QString &path);

    // 设置当前的多页图索引，-1表示当前非多页图
    Q_INVOKABLE void setMultiFrameIndex(int index = Invalid);

public slots:
    //加载多张
    void loadThumbnails(const QStringList list);
    //加载一张
    void loadThumbnail(const QString path);
    //缩略图裁切接口-预留
    void catThumbnail(const QStringList &list);

signals:
    //通知QML刷新
    void callQmlRefeshImg();

private:
    int     m_FrameIndex = Invalid;
};

#endif // THUMBNAILLOAD_H
