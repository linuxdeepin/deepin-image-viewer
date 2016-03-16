#include "FiltersPreview.h"
#include "filters/FilterObj.h"
#include <QBoxLayout>

using namespace filter2d;
static const int kW = 180;
static const int kH = 90;
static const int kIndensityMax = 20;
#define QSS(x) #x

static const char kListViewQSS[] = QSS(
            QListView {
                background-color: transparent
            }
            QListView::item {
                border: 2px solid #aaaaaa;
                border-radius: 5px;
            }
            QListView::item:selected {
                border: 6px solid #3399ff;
                border-radius: 5px;
            }
            );

FiltersPreview::FiltersPreview(QWidget *parent) : QWidget(parent)
{
#ifdef USE_DLISTWIDGET
    m_list = new DListWidget();
    m_list->setItemSize(kW, kH);
    m_list->enableVerticalScroll();
#else
    m_list = new QListWidget();
    m_list->setStyleSheet(kListViewQSS);
    connect(m_list, &QListWidget::itemPressed, [=](QListWidgetItem* item) {
        int id = m_list->itemWidget(item)->property("filter_id").toInt();
        Q_EMIT filterIdSelected(id);
    });
#endif
    m_list->setSpacing(8);
    QVector<int> ids = FilterObj::filters();
    for (auto id : ids) {
        FilterObj *f = FilterObj::create(id);
        f->setProperty("brightness", 0.6);
        f->setProperty("hue", 0.6);
        f->setProperty("contrast", 0.6);
        f->setProperty("saturation", 0.6);
        m_filter.append(f);
        QLabel *label = new QLabel();
        label->resize(kW, kH);
        label->setProperty("filter_id", id);
#ifdef USE_DLISTWIDGET
        m_list->addWidget(label);
#else
        QListWidgetItem *item = new QListWidgetItem(QString::number(id));
        item->setSizeHint(QSize(kW, kH));
        m_list->insertItem(m_filter.size()-1, item);
        m_list->setItemWidget(item, label);
#endif
    }

    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_indensity = new DSlider();
    m_indensity->setRange(0, kIndensityMax);
    m_indensity->setOrientation(Qt::Horizontal);
    connect(m_indensity, &DSlider::valueChanged, this, &FiltersPreview::applyIndensity);
    m_indensity->setValue(0.8*kIndensityMax);

    QVBoxLayout *vb = new QVBoxLayout();
    setLayout(vb);
    vb->addWidget(m_list);
    vb->addWidget(m_indensity);
}

void FiltersPreview::setImage(const QImage &img)
{
    qDebug() << m_list->visualItemRect(m_list->item(0));
    m_image = img.scaled(kW, kH, Qt::KeepAspectRatioByExpanding);
    applyIndensity(m_indensity->value());
}

qreal FiltersPreview::indensity() const
{
    return qreal(m_indensity->value())/qreal(kIndensityMax);
}

void FiltersPreview::applyIndensity(int value)
{
    if (m_image.isNull())
        return;
    const qreal k = qreal(value)/qreal(kIndensityMax);
    for (int i = 0; i < m_filter.size(); ++i) {
        FilterObj* f = m_filter[i];
        f->setIndensity(k);
#ifdef USE_DLISTWIDGET
        QLabel* label = (QLabel*)m_list->getWidget(i);
#else
        QLabel* label = static_cast<QLabel*>(m_list->itemWidget(m_list->item(i)));
#endif
        label->setPixmap(QPixmap::fromImage(f->apply(m_image)));
    }
}
