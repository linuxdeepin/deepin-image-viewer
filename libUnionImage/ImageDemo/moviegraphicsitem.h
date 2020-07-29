#ifndef MOVIEGRAPHICSITEM_H
#define MOVIEGRAPHICSITEM_H

#include <unionimage.h>
#include <QGraphicsPixmapItem>

using namespace UnionImage_NameSpace;

class MovieGraphicsItem : public QGraphicsPixmapItem
{
public:
    MovieGraphicsItem();
private:
    UnionMovieImage ici;
};

#endif // MOVIEGRAPHICSITEM_H
