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
        : QLabel(t, parent) {}
};

#include "imageinfowidget.moc"

ImageInfoWidget::ImageInfoWidget(QWidget *parent)
    : QScrollArea(parent),
      m_isDetail(false)
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
    m_exifLayout = new QFormLayout();
    m_exifLayout->setSpacing(13);
    m_exifLayout->setContentsMargins(8, 0, 8, 0);
    m_exifLayout->setLabelAlignment(Qt::AlignRight);
    contentLayout->addLayout(m_exifLayout);

    QPushButton *button = new QPushButton(tr("Show details"));
    button->setObjectName("ShowExtendInfoButton");
    connect(button, &QPushButton::clicked, this, [=] {
        if (m_isDetail) {
            m_isDetail = false;
            button->setText(tr("Show details"));
            updateInfo();
        }
        else {
            m_isDetail = true;
            button->setText(tr("Show basics"));
            updateInfo();
        }

        emit dApp->signalM->updateExtensionPanelRect();
    });
    contentLayout->addSpacing(15);
    contentLayout->addWidget(button);

    separator = new ViewSeparator();
    contentLayout->addWidget(separator);

    contentLayout->addStretch();

    setWidget(content);
}

void ImageInfoWidget::setImagePath(const QString &path)
{
    m_path = path;

    updateInfo();
}

//QSize ImageInfoWidget::sizeHint() const
//{
//    return QSize(m_maxContentWidth, height());
//}

void ImageInfoWidget::updateInfo()
{
    // Clear layout
    QLayoutItem *item;
    while((item = m_exifLayout->takeAt(0))) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    m_maxContentWidth = 0;
    int titleWidth = 0;
    int fieldWidth = 0;
    auto ei = utils::image::GetExifFromPath(m_path, m_isDetail);
    for (const utils::image::ExifItem* i =
         utils::image::getExifItemList(m_isDetail); i->tag; ++i) {

        QString v = ei.value(i->name);

        if (v.isEmpty()) {
            continue;
        }


        const int infoLength = v.length();
        const int itemWidth = 6;
        if (infoLength > itemWidth) {

           for(int i = 1; i < infoLength / itemWidth; i++) {
                int n = i * 6;
                v.insert(n, QLatin1String(" "));
            }
        }

        SimpleFormField *label = new SimpleFormField(v);
        label->setWordWrap(true);
        const QString tn = qApp->translate("ExifItemName", i->name);

        titleWidth = qMax(titleWidth, utils::base::stringWidth(font(), tn));
        fieldWidth = qMax(fieldWidth, utils::base::stringWidth(label->font(),label->text()));

        label->setMinimumHeight(utils::base::stringHeight(label->font(),label->text()) + 15);
        m_exifLayout->addRow(new SimpleFormLabel(tn + ":"), label);
    }

    m_maxContentWidth = titleWidth + fieldWidth + 10;
    m_maxContentWidth += verticalScrollBar()->isVisible() ? -10 : 10;

}
