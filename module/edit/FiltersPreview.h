#ifndef FILTERSPREVIEW_H
#define FILTERSPREVIEW_H

#include <QWidget>
#include <QLabel>
#include <dscrollarea.h>
using namespace Dtk::Widget;
namespace filter2d {
class FilterObj;
}
class FiltersPreview : public DScrollArea
{
    Q_OBJECT
public:
    explicit FiltersPreview(QWidget *parent = 0);
    void setImage(const QImage& img);
    void setIndensity(qreal value);

signals:

public slots:

private:
    qreal m_indensity;
    QVector<QLabel*> m_label;
    QVector<filter2d::FilterObj*> m_filter;
};

#endif // FILTERSPREVIEW_H
