#ifndef THUMBNAILLOAD_H
#define THUMBNAILLOAD_H

#include <QQuickImageProvider>
#include <QQuickWindow>
#include <QImage>

class ThumbnailLoad : public QQuickImageProvider
{
public:
    explicit ThumbnailLoad();
    //获取缩略图
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);  //预留
    bool imageIsNull(const QString &path);
    //当前图片
    QImage m_Img;
    QMap <QString, QImage> m_imgMap; //缩略图
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

    QMap <QString, QSize> m_imgSizes; //图片大小
    //当前图片
    QImage m_Img;
    //加载路径
    QString m_currentPath;
};


class LoadImage : public QObject
{
    Q_OBJECT
public:
    explicit LoadImage(QObject *parent = nullptr);

    ThumbnailLoad *m_pThumbnail{nullptr};
    ViewLoad *m_viewLoad{nullptr};
    Q_INVOKABLE double getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight);
    Q_INVOKABLE bool imageIsNull(const QString &path);
    //加载路径
    QString m_path;

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
};

#endif // THUMBNAILLOAD_H
