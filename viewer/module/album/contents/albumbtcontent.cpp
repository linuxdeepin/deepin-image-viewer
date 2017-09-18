/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "albumbtcontent.h"
#include "application.h"
#include "controller/configsetter.h"
#include "controller/dbmanager.h"
#include "controller/importer.h"
#include "controller/signalmanager.h"
#include "controller/viewerthememanager.h"
#include "dirwatcher/scanpathsdialog.h"
#include "utils/baseutils.h"
#include "widgets/imagebutton.h"
#include "widgets/loadingicon.h"
#include "widgets/slider.h"
#include <QHBoxLayout>
#include <QStackedLayout>

namespace {

const int MIN_ICON_SIZE = 96;
const int SLIDER_WIDTH = 120;
const int SLIDER_HEIGHT = 22;
const QString SETTINGS_GROUP = "ALBUMPANEL";
const QString SETTINGS_ALBUM_ICON_SCALE_KEY = "AlbumIconScale";
const QString SETTINGS_IMAGE_ICON_SCALE_KEY = "ImageIconScale";

}  // namespace

AlbumBTContent::AlbumBTContent(const QString &darkStyle, const QString &lightStyle,
                               QWidget *parent)
    : ThemeWidget(darkStyle, lightStyle, parent)
    , m_inAlbumView(true)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(5, 0, 14, 0);
    m_layout->setSpacing(0);

    initSynchroBtn();
    initMiddleContent();
    initSlider();

    updateCount();
    updateColor();
    connect(dApp->viewerTheme, &ViewerThemeManager::viewerThemeChanged,
            this, &AlbumBTContent::updateColor);
}

void AlbumBTContent::updateCount()
{
    if (! inAlbumView()) {
        int count =  DBManager::instance()->getImgsCountByAlbum(m_album);
        QString text = QString::number(count) + " " +
                (count <= 1 ? tr("image") : tr("images"));
        m_label->setText(text);
        m_slider->setFixedHeight(count > 0 ? SLIDER_HEIGHT : 0);
    }
    else {
        const int count = DBManager::instance()->getAlbumsCount();
        QString text = QString::number(count) + " " +
                (count <= 1 ? tr("album") : tr("albums"));
        m_label->setText(text);
        m_slider->setFixedHeight(count > 0 ? SLIDER_HEIGHT : 0);
    }
}

void AlbumBTContent::changeItemSize(bool increase)
{
    if (increase) {
        m_slider->setValue(qMin(m_slider->value() + 1, m_slider->maximum()));
    }
    else {
        m_slider->setValue(qMax(m_slider->value() - 1, m_slider->minimum()));
    }
}

void AlbumBTContent::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter p(this);
    // Draw border line
    p.fillRect(QRect(0, 0, width(), 1), m_tl2Color);
}

void AlbumBTContent::initSynchroBtn()
{
    ImageButton *synb = new ImageButton;
    synb->setObjectName("SynchroBtn");
    synb->setToolTip(tr("Manage sync"));

    connect(synb, &ImageButton::clicked, this, [=] {
        ScanPathsDialog::instance()->show();
    });

    m_layout->addWidget(synb);
    m_layout->addStretch(1);
}

void AlbumBTContent::initMiddleContent()
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
    l->setFixedWidth(350);
    l->setObjectName("ImportLabel");
    hl->addWidget(l);
    hl->addStretch();

    QStackedLayout *layout = new QStackedLayout;
    layout->setSpacing(0);
    layout->addWidget(m_label);
    layout->addWidget(w);
    if (Importer::instance()->isRunning()) {
        lIcon->play();
        layout->setCurrentIndex(1);
    }

    connect(dApp->signalM, &SignalManager::imagesInserted,
            this, &AlbumBTContent::updateCount);
    connect(dApp->signalM, &SignalManager::imagesRemoved,
            this, &AlbumBTContent::updateCount);
    connect(Importer::instance(), &Importer::progressChanged, this, [=] {
        layout->setCurrentIndex(1);
        lIcon->play();
    });
    connect(Importer::instance(), &Importer::imported, this, [=] {
        layout->setCurrentIndex(0);
        lIcon->stop();
        updateCount();
    });
    connect(Importer::instance(), &Importer::currentImport, this, [=] (const QString &path) {
        QFontMetrics fm(l->font());
        const QString s = tr("Syncing: ") + path;
        l->setText(fm.elidedText(s, Qt::ElideMiddle, l->width()));
    });

    m_layout->addLayout(layout);
    m_layout->addStretch(1);
}

void AlbumBTContent::initSlider()
{
    m_slider = new Slider(Qt::Horizontal);
    m_slider->setMinimum(0);
    m_slider->setMaximum(3);
    m_slider->setPageStep(1);
    m_slider->setFixedSize(SLIDER_WIDTH, SLIDER_HEIGHT);
    updateSliderDefaultValue();
    connect(m_slider, &Slider::valueChanged, this, [=] (int multiple) {
        int newSize = MIN_ICON_SIZE + multiple * 32;
        emit itemSizeChanged(newSize);
        emit multipleChanged(multiple);
        if (! inAlbumView()) {
            dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_IMAGE_ICON_SCALE_KEY,
                                  QVariant(m_slider->value()));
        }
        else {
            dApp->setter->setValue(SETTINGS_GROUP, SETTINGS_ALBUM_ICON_SCALE_KEY,
                                  QVariant(m_slider->value()));
        }
    });

    m_layout->addWidget(m_slider);
}

void AlbumBTContent::updateColor()
{
    if (dApp->viewerTheme->getCurrentTheme() == ViewerThemeManager::Dark) {
        m_tl2Color = utils::common::TOP_LINE2_COLOR_DARK;
    }
    else {
        m_tl2Color = utils::common::TOP_LINE2_COLOR_LIGHT;
    }
}

QString AlbumBTContent::album() const
{
    return m_album;
}

void AlbumBTContent::setAlbum(const QString &album)
{
    m_album = album;
}

void AlbumBTContent::updateSliderDefaultValue()
{
    int multiple;
    if (! inAlbumView()) {
        multiple = dApp->setter->value(SETTINGS_GROUP,
                                   SETTINGS_IMAGE_ICON_SCALE_KEY, 0).toInt();
    }
    else {
        multiple = dApp->setter->value(SETTINGS_GROUP,
                                   SETTINGS_ALBUM_ICON_SCALE_KEY, 0).toInt();
    }
    emit itemSizeChanged(MIN_ICON_SIZE + multiple * 32);
    emit multipleChanged(multiple);
    m_slider->setValue(multiple);
}

bool AlbumBTContent::inAlbumView() const
{
    return m_inAlbumView;
}

void AlbumBTContent::setInAlbumView(bool inAlbumView)
{
    m_inAlbumView = inAlbumView;
    updateSliderDefaultValue();
}
