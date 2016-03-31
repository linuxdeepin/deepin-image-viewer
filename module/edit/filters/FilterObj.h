#ifndef FILTEROBJ_H
#define FILTEROBJ_H

#include <QObject>
#include <QVector>
#include "Filter2D.h"

namespace filter2d {

class FilterObj : public QObject, public virtual Filter2DBase // virtual: use implemention in XXXFilter
{
    Q_OBJECT
    Q_PROPERTY(qreal intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)
public:
    static FilterObj* create(int id);
    static QVector<int> filters();
Q_SIGNALS:
    void intensityChanged() override;
};

}//namespace filter2d {
#endif // FILTEROBJ_H
