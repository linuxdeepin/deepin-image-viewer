#include "timelinebtcontent.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/dbmanager.h"
#include "controller/importer.h"
#include "controller/signalmanager.h"
#include "controller/viewerthememanager.h"
#include "widgets/imagebutton.h"
#include "widgets/loadingicon.h"
#include "widgets/slider.h"
#include <QHBoxLayout>
#include <QStackedLayout>

namespace {

const int MIN_ICON_SIZE = 96;
const int SLIDER_WIDTH = 120;
const int SLIDER_HEIGHT = 22;
const QString SETTINGS_GROUP = "TIMEPANEL";
const QString SETTINGS_ICON_SCALE_KEY = "IconScale";

const QColor TOP_LINE2_COLOR_DARK = QColor(255, 255, 255, 13);
const QColor TOP_LINE2_COLOR_LIGHT = QColor(255, 255, 255, 153);

}  // namespace

TimelineBTContent::TimelineBTContent(const QString &darkStyle, const QString &lightStyle,
                                     QWidget *parent)
    : ThemeWidget(darkStyle, lightStyle, parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(5, 0, 14, 0);
    m_layout->setSpacing(0);

    initImportBtn();
    initMiddleContent();
    initSlider();

    updateImageCount();
    updateColor();
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &TimelineBTContent::updateColor);
}

void TimelineBTContent::updateImageCount()
{
    int count = dApp->dbM->getImgsCount();
    if (count <= 1) {
        m_label->setText(tr("%1 image").arg(count));
    }
    else {
        m_label->setText(tr("%1 images").arg(count));
    }

    m_slider->setFixedHeight(count > 0 ? SLIDER_HEIGHT : 0);
}

void TimelineBTContent::changeItemSize(bool increase)
{
    if (increase) {
        m_slider->setValue(qMin(m_slider->value() + 1, m_slider->maximum()));
    }
    else {
        m_slider->setValue(qMax(m_slider->value() - 1, m_slider->minimum()));
    }
}

int TimelineBTContent::iconSize() const
{

    const int sizeScale = dApp->setter->value(SETTINGS_GROUP,
                                              SETTINGS_ICON_SCALE_KEY,
                                              QVariant(0)).toInt();
    return (MIN_ICON_SIZE + sizeScale * 32);
}

void TimelineBTContent::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter p(this);
    // Draw border line
    p.fillRect(QRect(0, 0, width(), 1), m_tl2Color);
}

void TimelineBTContent::initImportBtn()
{
    ImageButton *ib = new ImageButton;
    ib->setObjectName("ImportBtn");
    ib->setToolTip(tr("Import"));

    connect(ib, &ImageButton::clicked, this, [=] {
        dApp->importer->showImportDialog();
    });

    m_layout->addWidget(ib);
    m_layout->addStretch(1);
}

void TimelineBTContent::initMiddleContent()
{
    m_label = new QLabel;
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setObjectName("CountLabel");
    m_label->setContentsMargins(SLIDER_WIDTH, 0, 0, 0);

    QWidget *w = new QWidget;
    QHBoxLayout *hl = new QHBoxLayout(w);
    hl->setContentsMargins(SLIDER_WIDTH, 0, 0, 0);
    hl->setSpacing(5);

    LoadingIcon *lIcon = new LoadingIcon(this);
    hl->addWidget(lIcon);
    QLabel *l = new QLabel;
    l->setMaximumWidth(350);
    l->setObjectName("ImportLabel");
    hl->addWidget(l);
    hl->addStretch();

    QStackedLayout *layout = new QStackedLayout;
    layout->setSpacing(0);
    layout->addWidget(m_label);
    layout->addWidget(w);
    if (dApp->importer->isRunning()) {
        lIcon->play();
        layout->setCurrentIndex(1);
    }

    connect(dApp->signalM, &SignalManager::imagesInserted,
            this, &TimelineBTContent::updateImageCount);
    connect(dApp->signalM, &SignalManager::imagesRemoved,
            this, &TimelineBTContent::updateImageCount);
    connect(dApp->importer, &Importer::progressChanged, this, [=] {
        layout->setCurrentIndex(1);
        lIcon->play();
    });
    connect(dApp->importer, &Importer::imported, this, [=] {
        layout->setCurrentIndex(0);
        lIcon->stop();
        updateImageCount();
    });
    connect(dApp->importer, &Importer::currentImport, this, [=] (const QString &path) {
        QFontMetrics fm(l->font());
        l->setText(tr("Importing: ") + fm.elidedText(path, Qt::ElideMiddle, 230));
    });

    m_layout->addLayout(layout);
    m_layout->addStretch(1);
}

void TimelineBTContent::initSlider()
{
    const int sizeScale = dApp->setter->value(SETTINGS_GROUP,
                                              SETTINGS_ICON_SCALE_KEY,
                                              QVariant(0)).toInt();
    const int iconSize = MIN_ICON_SIZE + sizeScale * 32;
    emit itemSizeChanged(iconSize);

    m_slider = new Slider(Qt::Horizontal);
    m_slider->setMinimum(0);
    m_slider->setMaximum(3);
    m_slider->setValue(sizeScale);
    m_slider->setPageStep(1);
    m_slider->setFixedSize(SLIDER_WIDTH, SLIDER_HEIGHT);
    connect(m_slider, &Slider::valueChanged, this, [=] (int multiple) {
        int newSize = MIN_ICON_SIZE + multiple * 32;
        emit itemSizeChanged(newSize);
        dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_ICON_SCALE_KEY,
                               QVariant(m_slider->value()));
    });

    m_layout->addWidget(m_slider);
}

void TimelineBTContent::updateColor()
{
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
        m_tl2Color = TOP_LINE2_COLOR_DARK;
    }
    else {
        m_tl2Color = TOP_LINE2_COLOR_LIGHT;
    }
}
