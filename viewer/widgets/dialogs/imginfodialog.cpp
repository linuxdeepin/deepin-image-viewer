#include "imginfodialog.h"
#include "application.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include "../imagebutton.h"
#include "widgets/formlabel.h"

#include <dtitlebar.h>
#include <QFormLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>

namespace {

const int MAX_WIDTH = 250;
const int THUMBNAIL_WIDTH = 240;
const int THUMBNAIL_HEIGHT = 160;
const int TITLE_MAXWIDTH = 100;

struct MetaData {
    QString key;
    const char *name;
};

static MetaData MetaDatas[] = {
    {"FileName",            QT_TRANSLATE_NOOP("MetadataName", "Name")},
    {"FileFormat",          QT_TRANSLATE_NOOP("MetadataName", "Type")},
    {"DateTimeOriginal",    QT_TRANSLATE_NOOP("MetadataName", "Date photoed")},
    {"DateTimeDigitized",   QT_TRANSLATE_NOOP("MetadataName", "Date modified")},
    {"Resolution",          QT_TRANSLATE_NOOP("MetadataName", "Resolution")},
    {"FileSize",            QT_TRANSLATE_NOOP("MetadataName", "File size")},
//    {"MaxApertureValue",    QT_TRANSLATE_NOOP("MetadataName", "Max aperture")},
//    {"FocalLength",         QT_TRANSLATE_NOOP("MetadataName", "Focal length")},
    {"", ""}
};

static int maxTitleWidth()
{
    int maxWidth = 0;
    QFont tf;
    tf.setPixelSize(11);
    for (const MetaData* i = MetaDatas; ! i->key.isEmpty(); ++i) {
        maxWidth = qMax(maxWidth + 1, utils::base::stringWidth(tf, i->name));
    }
    return maxWidth;
}
}  // namespace

ImgInfoDialog::ImgInfoDialog(const QString &path, QWidget *parent)
    : DMainWindow(parent)
{
    setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
    if (titleBar()) titleBar()->setFixedHeight(0);
    setFixedWidth(MAX_WIDTH);
    setStyleSheet(utils::base::getFileContent(
                      ":/dialogs/qss/resources/qss/imginfodialog.qss"));

    QWidget *w = new QWidget;
    m_layout = new QVBoxLayout(w);
    m_layout->setContentsMargins(5, 5, 5, 0);
    m_layout->setSpacing(0);

    initThumbnail(path);
    initSeparator();
    initInfos(path);

    setCentralWidget(w);

    initCloseButton();
    //    connect(this, &ImgInfoDialog::closed, this, &ImgInfoDialog::deleteLater);
}

void ImgInfoDialog::hideEvent(QHideEvent *e)
{
    DMainWindow::hideEvent(e);

    emit closed();
}

void ImgInfoDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }
}


void ImgInfoDialog::initThumbnail(const QString &path)
{
    using namespace utils::image;
    QLabel *iconLabel = new QLabel;
    iconLabel->setObjectName("IconLabel");
    QPixmap pixmap = cutSquareImage(getThumbnail(path),
                                    QSize(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT));
    // Draw inside-border
    QPainter p(&pixmap);
    QRect br(0, 0, pixmap.width() - 1, pixmap.height() - 1);
    QPainterPath pp;
    pp.addRect(br);
    p.setPen(QPen(QColor(0, 0, 0, 0.2 * 255), 1));
    p.drawPath(pp);

    iconLabel->setPixmap(pixmap);
    m_layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
}

void ImgInfoDialog::initSeparator()
{
    QLabel *sl = new QLabel;
    sl->setObjectName("Separator");
    sl->setFixedSize(MAX_WIDTH - 5 * 2, 1);
    m_layout->addSpacing(9);
    m_layout->addWidget(sl, 0, Qt::AlignHCenter);
}

void ImgInfoDialog::initInfos(const QString &path)
{
    using namespace utils::image;
    using namespace utils::base;
    QWidget *w = new QWidget;
    QFormLayout *infoLayout = new QFormLayout(w);
    infoLayout->setSpacing(8);
    infoLayout->setContentsMargins(10, 15, 10, 26);
    infoLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);

    auto mds = getAllMetaData(path);
    for (const MetaData* i = MetaDatas; ! i->key.isEmpty(); i ++) {
        QString v = mds.value(i->key);
        if (v.isEmpty()) continue;

        SimpleFormField *field = new SimpleFormField;
        field->setObjectName("Field");
        field->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        field->setText(wrapStr(v, field->font(), TITLE_MAXWIDTH + 30));
        if (QString(i->name) == "Name")
            field->setMinimumHeight(stringHeight(field->font(),
                   field->text()) * field->text().split(" ").length());

        SimpleFormLabel *title = new SimpleFormLabel(dApp->translate("MetadataName", i->name) + ":");
        title->setObjectName("Title");
        title->setFixedHeight(field->maximumHeight());
        title->setFixedWidth(qMin(maxTitleWidth(), TITLE_MAXWIDTH));
        title->setAlignment(Qt::AlignRight | Qt::AlignTop);

        infoLayout->addRow(title, field);
    }

    m_layout->addWidget(w, 0, Qt::AlignHCenter);
}

void ImgInfoDialog::initCloseButton()
{
    ImageButton* cb = new ImageButton(this);
    cb->setTooltipVisible(true);
    cb->setNormalPic(":/images/light/images/window_close_normal.png");
    cb->setHoverPic(":/images/light/images/window_close_hover.png");
    cb->setPressPic(":/images/light/images/window_close_press.png");
    cb->setFixedSize(27, 23);
    cb->move(this->x() + this->width() - cb->width() + 1, 4);
    connect(cb, &ImageButton::clicked, this, &ImgInfoDialog::close);
}
