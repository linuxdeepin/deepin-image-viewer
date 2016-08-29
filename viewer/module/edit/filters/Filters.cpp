#include "Filters.h"

namespace filter2d {

static inline int mixChannel(int p1, int p2, qreal x)
{
    if (x == 1.0)
        return p1;
    return x*(qreal)p1 + (1.0-x)*(qreal)p2;
}

QRgb mix(qreal k, QRgb c, QRgb c0)
{
    return qRgba(mixChannel(qRed(c), qRed(c0), k),
                 mixChannel(qGreen(c), qGreen(c0), k),
                 mixChannel(qBlue(c), qBlue(c0), k),
                 mixChannel(qAlpha(c), qAlpha(c0), k)
                 );
}

QRgb mix(qreal k, QRgb c0, int r, int g, int b, int a)
{
    return qRgba(mixChannel(r, qRed(c0), k),
                 mixChannel(g, qGreen(c0), k),
                 mixChannel(b, qBlue(c0), k),
                 mixChannel(a, qAlpha(c0), k)
                 );
}
} //namespace filter2d

