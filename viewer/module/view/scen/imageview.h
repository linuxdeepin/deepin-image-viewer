#ifndef SVGVIEW_H
#define SVGVIEW_H

#include <QGraphicsView>
#include <QFutureWatcher>

QT_BEGIN_NAMESPACE
class QWheelEvent;
class QPaintEvent;
class QFile;
class GraphicsMovieItem;
class QGraphicsSvgItem;
class QThreadPool;
QT_END_NAMESPACE

class ImageView : public QGraphicsView
{
    Q_OBJECT

public:
    enum RendererType { Native, OpenGL };

    ImageView(QWidget *parent = 0);

    void fitWindow();
    void fitImage();
    void rotateClockWise();
    void rotateCounterclockwise();
    void centerOn(int x, int y);
    void setImage(const QString &path);
    void setRenderer(RendererType type = Native);
    void setScaleValue(qreal v);

    const QImage image();
    qreal imageRelativeScale() const;
    qreal windowRelativeScale() const;
    const QRectF imageRect() const;
    const QString path() const;

    QPoint mapToImage(const QPoint &p) const;
    QRect mapToImage(const QRect& r) const;
    QRect visibleImageRect() const;
    bool isWholeImageVisible() const;

    bool isFitImage() const;
    bool isFitWindow() const;

signals:
    void clicked();
    void doubleClicked();
    void imageChanged(QString path);
    void mouseHoverMoved();
    void scaled(qreal perc);
    void transformChanged();

public slots:
    void setHighQualityAntialiasing(bool highQualityAntialiasing);

protected:
    void mouseDoubleClickEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    void drawBackground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;

private slots:
    void onCacheFinish();

private:
    bool m_isFitImage;
    bool m_isFitWindow;
    RendererType m_renderer;
    QFutureWatcher<QVariantList> m_watcher;
    QString m_path;
    QThreadPool *m_pool;

    QGraphicsSvgItem *m_svgItem;
    GraphicsMovieItem *m_movieItem;
    QGraphicsPixmapItem *m_pixmapItem;
};
#endif // SVGVIEW_H
