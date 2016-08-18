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
    setObjectName("ImageInfoScrollArea");
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setContextMenuPolicy(Qt::PreventContextMenu);

    QFrame *content = new QFrame();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(10, 10, 10, 10);

    // Title field
    SimpleFormLabel *title = new SimpleFormLabel(tr("Image info"));
    contentLayout->addWidget(title);
    ViewSeparator *separator = new ViewSeparator();
    contentLayout->addWidget(separator);
    contentLayout->addSpacing(8);
    // Info field
    m_exifLayout_base = new QFormLayout();
    m_exifLayout_base->setSpacing(8);
    m_exifLayout_base->setContentsMargins(8, 0, 8, 0);
    m_exifLayout_base->setLabelAlignment(Qt::AlignRight);
    m_separator = new ViewSeparator();
    m_separator->setVisible(false);
    m_exifLayout_details = new QFormLayout();
    m_exifLayout_details->setSpacing(8);
    m_exifLayout_details->setContentsMargins(8, 0, 8, 0);
    m_exifLayout_details->setLabelAlignment(Qt::AlignRight);

    contentLayout->addLayout(m_exifLayout_base);
    contentLayout->addSpacing(3);
    contentLayout->addWidget(m_separator);
    contentLayout->addSpacing(8);
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
    QFormLayout *fl = static_cast<QFormLayout *>(layout);
    if (fl) {
        // FIXME fl->rowCount() will always increase
        for (int i = 0; i < fl->rowCount(); i++) {
            QLayoutItem *li = fl->itemAt(i, QFormLayout::LabelRole);
            QLayoutItem *fi = fl->itemAt(i, QFormLayout::FieldRole);
            if (li) {
                if (li->widget()) delete li->widget();
                fl->removeItem(li);
            }
            if (fi) {
                if (fi->widget()) delete fi->widget();
                fl->removeItem(fi);
            }
        }
    }
}
//QSize ImageInfoWidget::sizeHint() const
//{
//    return QSize(m_maxContentWidth, height());
//}

void ImageInfoWidget::updateInfo()
{
    using namespace utils::image;
    using namespace utils::base;
    QMap<QString, QString> bei = getExifFromPath(m_path, false);
    QMap<QString, QString> dei = getExifFromPath(m_path, true);

    // Update the m_maxTitleWidth to make all colon aligned
    m_maxTitleWidth = 0;
    QFont tf;
    tf.setPixelSize(11);
    const QStringList titles = bei.unite(dei).keys();
    for (QString title : titles) {
        // FIXME append 1px for stringWidth calculate incorrect
        m_maxTitleWidth = qMax(m_maxTitleWidth + 1,
                               stringWidth(tf, trLabel(title.toUtf8().data())));
    }

    updateBaseInfo(bei);
    updateDetailsInfo(dei);
}

void ImageInfoWidget::updateBaseInfo(const QMap<QString, QString> &infos)
{
    using namespace utils::image;
    using namespace utils::base;
    clearLayout(m_exifLayout_base);

    for (const ExifItem* i = getExifItemList(false); i->tag; ++i) {
        QString value = infos.value(i->name);
        if (! value.isEmpty()) {
            splitInfoStr(value);
            SimpleFormField *label = new SimpleFormField(value);
            label->setAlignment(Qt::AlignLeft|Qt::AlignTop);

            SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
            title->setMinimumHeight(label->minimumHeight());
            title->setFixedWidth(m_maxTitleWidth);
            title->setAlignment(Qt::AlignRight|Qt::AlignTop);

            m_exifLayout_base->addRow(title, label);
        }
    }
}

void ImageInfoWidget::updateDetailsInfo(const QMap<QString, QString> &infos)
{
    using namespace utils::image;
    using namespace utils::base;
    clearLayout(m_exifLayout_details);

    for (const ExifItem* i = getExifItemList(true); i->tag; ++i) {
        QString value = infos.value(i->name);
        if (! value.isEmpty()) {
            splitInfoStr(value);
            SimpleFormField *label = new SimpleFormField(value);
            label->setAlignment(Qt::AlignLeft|Qt::AlignTop);

            SimpleFormLabel *title = new SimpleFormLabel(trLabel(i->name) + ":");
            title->setMinimumHeight(label->minimumHeight());
            title->setFixedWidth(m_maxTitleWidth);
            title->setAlignment(Qt::AlignRight|Qt::AlignTop);

            m_exifLayout_details->addRow(title, label);
        }
    }

    m_separator->setVisible(m_exifLayout_details->count() > 0);
}
