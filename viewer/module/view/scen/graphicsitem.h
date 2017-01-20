#ifndef GRAPHICSMOVIEITEM_H
#define GRAPHICSMOVIEITEM_H

#include <QGraphicsPixmapItem>
#include <QPointer>

class QMovie;
class GraphicsMovieItem : public QGraphicsPixmapItem, QObject
{
public:
    explicit GraphicsMovieItem(const QString &fileName, QGraphicsItem *parent = 0);
    ~GraphicsMovieItem();
    bool isValid() const;
    void start();
    void stop();

private:
    QPointer<QMovie> m_movie;
};

class GraphicsPixmapItem : public QGraphicsPixmapItem
{
public:
    explicit GraphicsPixmapItem(const QPixmap &pixmap);
    ~GraphicsPixmapItem();
};

#endif // GRAPHICSMOVIEITEM_H
