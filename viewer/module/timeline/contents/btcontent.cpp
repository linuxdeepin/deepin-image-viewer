#include "btcontent.h"
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
const QString SETTINGS_GROUP = "TIMEPANEL";
const QString SETTINGS_ICON_SCALE_KEY = "IconScale";

const QColor TOP_LINE1_COLOR_DARK = QColor(0, 0, 0, 0.5 * 255);
const QColor TOP_LINE2_COLOR_DARK = QColor(255, 255, 255, 0.05 * 255);
const QColor BOTTOM_LINE_COLOR_DARK = QColor(255, 255, 255, 0.02 * 255);
const QColor TOP_LINE1_COLOR_LIGHT = QColor(0, 0, 0, 25);
const QColor TOP_LINE2_COLOR_LIGHT = QColor(255, 255, 255, 0.05 * 255);
const QColor BOTTOM_LINE_COLOR_LIGHT = QColor(255, 255, 255, 0.02 * 255);

}  // namespace

BTContent::BTContent(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 0, 14, 0);
    m_layout->setSpacing(0);

    initImportBtn();
    initMiddleContent();
    initSlider();

    updateImageCount();
    updateColor();
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &BTContent::updateColor);
}

void BTContent::updateImageCount()
{
    int count = dApp->dbM->getImgsCount();
    if (count <= 1) {
        m_label->setText(tr("%1 image").arg(count));
    }
    else {
        m_label->setText(tr("%1 images").arg(count));
    }

    m_slider->setFixedWidth(count > 1 ? SLIDER_WIDTH : 0);
}

void BTContent::changeItemSize(bool increase)
{
    if (increase) {
        m_slider->setValue(qMin(m_slider->value() + 1, m_slider->maximum()));
    }
    else {
        m_slider->setValue(qMax(m_slider->value() - 1, m_slider->minimum()));
    }
}

void BTContent::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter p(this);
    // Draw border line
    p.fillRect(QRect(0, 0, width(), 1), m_tl1Color);
    p.fillRect(QRect(0, 1, width(), 1), m_tl2Color);
    p.fillRect(QRect(0, height() - 1, width(), 1), m_blColor);
}

void BTContent::initImportBtn()
{
    ImageButton *ib = new ImageButton;
    ib->setFixedWidth(SLIDER_WIDTH);
    ib->setObjectName("ImportBtn");
    ib->setToolTip(tr("Import"));

    connect(ib, &ImageButton::clicked, this, [=] {
        dApp->importer->showImportDialog();
    });

    m_layout->addWidget(ib);
    m_layout->addStretch(1);
}

void BTContent::initMiddleContent()
{
    m_label = new QLabel;
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setObjectName("CountLabel");

    QWidget *w = new QWidget;
    QHBoxLayout *hl = new QHBoxLayout(w);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(5);

    LoadingIcon *lIcon = new LoadingIcon(this);
    hl->addWidget(lIcon);
    QLabel *l = new QLabel;
    l->setFixedWidth(260);
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
            this, &BTContent::updateImageCount);
    connect(dApp->signalM, &SignalManager::imagesRemoved,
            this, &BTContent::updateImageCount);
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
        l->setText(tr("Importing: ") + fm.elidedText(path, Qt::ElideMiddle, 200));
    });

    m_layout->addLayout(layout);
    m_layout->addStretch(1);
}

void BTContent::initSlider()
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
    connect(m_slider, &Slider::valueChanged, this, [=] (int multiple) {
        int newSize = MIN_ICON_SIZE + multiple * 32;
        emit itemSizeChanged(newSize);
        dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_ICON_SCALE_KEY,
                               QVariant(m_slider->value()));
    });

    m_layout->addWidget(m_slider);
}

void BTContent::updateColor()
{
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
        m_tl1Color = TOP_LINE1_COLOR_DARK;
        m_tl2Color = TOP_LINE2_COLOR_DARK;
        m_blColor = BOTTOM_LINE_COLOR_DARK;
    }
    else {
        m_tl1Color = TOP_LINE1_COLOR_LIGHT;
        m_tl2Color = TOP_LINE2_COLOR_LIGHT;
        m_blColor = BOTTOM_LINE_COLOR_LIGHT;
    }
}
