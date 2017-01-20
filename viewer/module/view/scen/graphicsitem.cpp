#include "graphicsitem.h"
#include <QMovie>

GraphicsMovieItem::GraphicsMovieItem(const QString &fileName, QGraphicsItem *parent)
    : QGraphicsPixmapItem(fileName, parent)
{
    m_movie = new QMovie(fileName);
    QObject::connect(m_movie, &QMovie::frameChanged, this, [=] {
        if (m_movie.isNull()) return;
        setPixmap(m_movie->currentPixmap());
    });
}

GraphicsMovieItem::~GraphicsMovieItem()
{
    // Prepares the item for a geometry change. Call this function
    // before changing the bounding rect of an item to keep
    // QGraphicsScene's index up to date.
    // If not doing this, it may crash
    prepareGeometryChange();

    m_movie->stop();
    m_movie->deleteLater();
    m_movie = nullptr;
}

/*!
 * \brief GraphicsMovieItem::isValid
 * There is a bug with QMovie::isValid() that is event if file's format not
 * supported this function still return true.
 * \return
 */
bool GraphicsMovieItem::isValid() const
{
    return m_movie->frameCount() > 1;
}

void GraphicsMovieItem::start()
{
    m_movie->start();
}

void GraphicsMovieItem::stop()
{
    m_movie->stop();
}


GraphicsPixmapItem::GraphicsPixmapItem(const QPixmap &pixmap)
    : QGraphicsPixmapItem(pixmap, NULL)
{

}

GraphicsPixmapItem::~GraphicsPixmapItem()
{
    prepareGeometryChange();
}
