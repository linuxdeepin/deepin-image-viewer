#include "FiltersPreview.h"
#include "filters/FilterObj.h"
using namespace filter2d;
static const int kW = 160;
static const int kH = 90;
FiltersPreview::FiltersPreview(QWidget *parent) : DScrollArea(parent)
{
    QWidget *vp = new QWidget();
    QVector<int> ids = FilterObj::filters();
    vp->resize(kW, ids.size()*(kH+5));
    for (auto id : ids) {
        FilterObj *f = FilterObj::create(id);
        qDebug() << f;
        f->setProperty("brightness", 0.6);
        f->setProperty("hue", 0.6);
        f->setProperty("contrast", 0.6);
        f->setProperty("saturation", 0.6);
        m_filter.append(f);
        QLabel *label = new QLabel(vp);
        label->resize(kW, kH);
        label->move(0, m_label.size()*(kH+5));
        m_label.append(label);
    }
    setWidget(vp);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void FiltersPreview::setImage(const QImage &img)
{
    QImage thumb = img.scaled(kW, kH, Qt::KeepAspectRatioByExpanding);
    for (int i = 0; i < m_filter.size(); ++i) {
        FilterObj* f = m_filter[i];
        m_label[i]->setPixmap(QPixmap::fromImage(f->apply(thumb)));
    }
}
