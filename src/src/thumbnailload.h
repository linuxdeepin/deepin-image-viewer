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
    //当前图片
    QImage m_Img;
};


class LoadImage : public QObject
{
    Q_OBJECT
public:
    explicit LoadImage(QObject *parent = nullptr);

    ThumbnailLoad *m_pThumbnail;

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
