#ifndef FILTEROBJ_H
#define FILTEROBJ_H

#include <QObject>
#include "Filter2D.h"

namespace filter2d {

class FilterObj : public QObject, public virtual Filter2DBase // virtual: use implemention in XXXFilter
{
    Q_OBJECT
    Q_PROPERTY(qreal indensity READ indensity WRITE setIndensity NOTIFY indensityChanged)
public:
    static FilterObj* create(int id);
Q_SIGNALS:
    void indensityChanged() override;
};

}//namespace filter2d {
#endif // FILTEROBJ_H
