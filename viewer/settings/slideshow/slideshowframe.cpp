/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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
#include "slideshowframe.h"
#include "slideshowpreview.h"
#include "application.h"
#include "controller/configsetter.h"
#include "../title.h"
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStyleFactory>
#include <DComboBox>

DWIDGET_USE_NAMESPACE
typedef DComboBox QCBToDComboBox;

namespace {

const QString DURATION_SETTING_GROUP = "SLIDESHOWDURATION";
const QString DURATION_SETTING_KEY = "Duration";

}

using namespace Dtk::Widget;

SlideshowFrame::SlideshowFrame(QWidget *parent)
    :QFrToDFrame(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    Title1 *tl1 = new Title1(tr("Slide Settings"));
    m_layout->addWidget(tl1);

    initPreview();
    initInterval();
}

int SlideshowFrame::defaultDuration() const
{
    QVariant v = dApp->setter->value(DURATION_SETTING_GROUP, DURATION_SETTING_KEY);
    if (v.isNull()) {
        dApp->setter->setValue(DURATION_SETTING_GROUP, DURATION_SETTING_KEY, 2);
        return 1;
    }
    else {
        return v.toInt();
    }
}

void SlideshowFrame::initPreview()
{

    // Preview
    Title2 *tlEffect = new Title2(tr("Switch effect"));
    m_layout->addSpacing(10);
    m_layout->addWidget(tlEffect);

    QHBoxLayout *previewLayout = new QHBoxLayout;
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->setSpacing(7);
    previewLayout->addStretch();
    SlideshowPreview *sp = new SlideshowPreview(SlideshowPreview::Blinds);
    connect(this, &SlideshowFrame::requestReset, sp, &SlideshowPreview::resetValue);
    previewLayout->addWidget(sp);
    sp = new SlideshowPreview(SlideshowPreview::Slide);
    connect(this, &SlideshowFrame::requestReset, sp, &SlideshowPreview::resetValue);
    previewLayout->addWidget(sp);
    sp = new SlideshowPreview(SlideshowPreview::Switcher);
    connect(this, &SlideshowFrame::requestReset, sp, &SlideshowPreview::resetValue);
    previewLayout->addWidget(sp);
    sp = new SlideshowPreview(SlideshowPreview::Circle);
    connect(this, &SlideshowFrame::requestReset, sp, &SlideshowPreview::resetValue);
    previewLayout->addWidget(sp);
    previewLayout->addStretch();
    m_layout->addSpacing(16);
    m_layout->addLayout(previewLayout);
}

void SlideshowFrame::initInterval()
{
    // Duration
    Title2 *tlDuration = new Title2(tr("Duration"));
    m_layout->addSpacing(20);
    m_layout->addWidget(tlDuration);

    QHBoxLayout *timeLayout = new QHBoxLayout;
    timeLayout->setContentsMargins(0, 0, 0, 0);
    timeLayout->setSpacing(103);
    timeLayout->addSpacing(37);
    timeLayout->addWidget(new Title3(tr("Switch duration")));

    QCBToDComboBox  *dcb = new QCBToDComboBox(this);
    dcb->setStyle(QStyleFactory::create("dlight"));
    dcb->setFixedSize(238, 26);
    QStringList intervalList;
    for (int i = 2; i < 6; i ++){
        intervalList << QString::number(i) + " " + tr("second");
    }
    dcb->addItems(intervalList);
    dcb->setEditable(false);
    dcb->setCurrentIndex(defaultDuration() - 2);
    connect(dcb, &QCBToDComboBox::currentTextChanged,
            this, [=] (const QString &text) {
        int i = QString(text.split(" ").first()).toInt();
        if (i != 0) {
            dApp->setter->setValue(DURATION_SETTING_GROUP, DURATION_SETTING_KEY, i);
        }
    });
    connect(this, &SlideshowFrame::requestReset, this, [=] {
        dApp->setter->setValue(DURATION_SETTING_GROUP, DURATION_SETTING_KEY, 2);
        dcb->setCurrentIndex(defaultDuration() - 2);
    });

    timeLayout->addWidget(dcb);
    timeLayout->addStretch();
    m_layout->addSpacing(10);
    m_layout->addLayout(timeLayout);

    m_layout->addStretch();

}
