#include "imageinfowidget.h"
#include "application.h"
#include "controller/signalmanager.h"
#include "utils/imageutils.h"
#include <QApplication>
#include <QBoxLayout>
#include <QDateTime>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QPushButton>
#include <QScrollBar>
#include <QtDebug>

class ViewSeparator : public QLabel {
    Q_OBJECT
public:
    explicit ViewSeparator(QWidget *parent = 0) : QLabel(parent) {
        setFixedHeight(1);
    }
};
class SimpleFormLabel : public QLabel {
    Q_OBJECT
public:
    explicit SimpleFormLabel(const QString &t, QWidget *parent = 0)
        : QLabel(t, parent) {}
};

class SimpleFormField : public QLabel {
    Q_OBJECT
public:
    explicit SimpleFormField(const QString &t, QWidget *parent = 0)
        : QLabel(t, parent)
    {
        setWordWrap(true);
        setMinimumHeight(utils::base::stringHeight(font(), t));
    }
};

#include "imageinfowidget.moc"

namespace {

const int MAX_INFO_LENGTH = 6;  // info string limit to 6 character

}  // namespace

ImageInfoWidget::ImageInfoWidget(QWidget *parent)
    : QScrollArea(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFrame *content = new QFrame();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(10, 10, 10, 10);

    // Title field
    SimpleFormLabel *title = new SimpleFormLabel(tr("Image info"));
    contentLayout->addWidget(title);
    ViewSeparator *separator = new ViewSeparator();
    contentLayout->addWidget(separator);

    // Info field
    m_exifLayout_base = new QFormLayout();
    m_exifLayout_base->setSpacing(5);
    m_exifLayout_base->setContentsMargins(8, 0, 8, 0);
    m_exifLayout_base->setLabelAlignment(Qt::AlignRight);
    m_separator = new ViewSeparator();
    m_separator->setVisible(false);
    m_exifLayout_details = new QFormLayout();
    m_exifLayout_details->setSpacing(5);
    m_exifLayout_details->setContentsMargins(8, 0, 8, 0);
    m_exifLayout_details->setLabelAlignment(Qt::AlignRight);

    contentLayout->addLayout(m_exifLayout_base);
    contentLayout->addWidget(m_separator);
    contentLayout->addLayout(m_exifLayout_details);

    contentLayout->addSpacing(15);
    contentLayout->addStretch();

    setWidget(content);
}

void ImageInfoWidget::setImagePath(const QString &path)
{
    m_path = path;

    updateInfo();
}

const QString ImageInfoWidget::trLabel(const char *str)
{
    return qApp->translate("ExifItemName", str);
}

/*!
 * \brief ImageInfoWidget::cutInfoStr
 * Split info string by Space
 * \return
 */
void ImageInfoWidget::splitInfoStr(QString &str) const
{
    const int dl = str.length();
    if (dl > MAX_INFO_LENGTH) {
        for(int i = 1; i < dl / MAX_INFO_LENGTH; i++) {
             int n = i * MAX_INFO_LENGTH;
             str.insert(n, QLatin1String(" "));
         }
    }
}

void ImageInfoWidget::clearLayout(QLayout *layout) {
    QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

}
//QSize ImageInfoWidget::sizeHint() const
//{
//    return QSize(m_maxContentWidth, height());
//}

void ImageInfoWidget::updateInfo()
{
    updateBaseInfo();
    updateDetailsInfo();
}

void ImageInfoWidget::updateBaseInfo()
{
    using namespace utils::image;
    using namespace utils::base;
    clearLayout(m_exifLayout_base);

    auto ei = getExifFromPath(m_path, false);
    for (const ExifItem* i = getExifItemList(false); i->tag; ++i) {
        QString value = ei.value(i->name);
        if (! value.isEmpty()) {
            splitInfoStr(value);
            SimpleFormField *label = new SimpleFormField(value);

            SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
            title->setMinimumHeight(label->minimumHeight());
            m_exifLayout_base->addRow(title, label);
        }
    }
}

void ImageInfoWidget::updateDetailsInfo()
{
    using namespace utils::image;
    using namespace utils::base;
    clearLayout(m_exifLayout_details);

    auto ei = getExifFromPath(m_path, true);
    for (const ExifItem* i = getExifItemList(true); i->tag; ++i) {
        QString value = ei.value(i->name);
        if (! value.isEmpty()) {
            splitInfoStr(value);
            SimpleFormField *label = new SimpleFormField(value);

            SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
            title->setMinimumHeight(label->minimumHeight());
            m_exifLayout_details->addRow(title, label);
        }
    }

    m_separator->setVisible(m_exifLayout_details->count() > 0);
}
