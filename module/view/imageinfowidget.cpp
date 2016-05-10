#include "imageinfowidget.h"
#include "utils/imgutil.h"
#include "controller/signalmanager.h"
#include <QScrollBar>
#include <QLabel>
#include <QFormLayout>
#include <QBoxLayout>
#include <QFileInfo>
#include <QDateTime>
#include <QPushButton>
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
    SimpleFormLabel *title = new SimpleFormLabel(tr("EXIF Information"));
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

        emit SignalManager::instance()->updateExtensionPanelRect();
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

QSize ImageInfoWidget::sizeHint() const
{
    return QSize(m_maxContentWidth, height());
}

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

    QFontMetrics fm(font());
    auto ei = utils::GetExifFromPath(m_path, m_isDetail);
    for (const utils::ExifItem* i = utils::ExifDataDetails; i->tag; ++i) {
        const QString v = ei.value(i->name);
        if (v.isEmpty()) {
            continue;
        }
        SimpleFormField *label = new SimpleFormField(v);
        titleWidth = qMax(titleWidth, fm.width(i->name));
        fieldWidth = qMax(fieldWidth, fm.width(v));
        m_exifLayout->addRow(new SimpleFormLabel(tr(i->name) + ":"), label);
    }

    m_maxContentWidth = titleWidth + fieldWidth;
    m_maxContentWidth += verticalScrollBar()->isVisible() ? -10 : 10;
}
