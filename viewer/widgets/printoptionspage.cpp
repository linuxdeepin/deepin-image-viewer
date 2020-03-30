#include "printoptionspage.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QDebug>

PrintOptionsPage::PrintOptionsPage(QWidget *parent)
    : QDialog(parent),
      m_settings("deepin", "print-image-option")
{
    m_noScaleBtn = new QRadioButton(tr("No scaling"));
    m_fitToImageBtn = new QRadioButton(tr("Fit page to image"));
    m_fitToPageBtn = new QRadioButton(tr("Fit image to page"));
    m_scaleBtn = new QRadioButton(tr("Scale to:"));
    m_printWidth = new QDoubleSpinBox;
    m_printHeight = new QDoubleSpinBox;
    m_printUnit = new QComboBox;
    m_buttonGroup = new QButtonGroup;
    m_posBtnGroup = new QButtonGroup;

    m_printWidth->setValue(m_settings.value("scale_width", 15.0).toDouble());
    m_printHeight->setValue(m_settings.value("scale_height", 10.0).toDouble());

    m_buttonGroup->addButton(m_noScaleBtn, 0);
    m_buttonGroup->addButton(m_fitToImageBtn, 1);
    m_buttonGroup->addButton(m_fitToPageBtn, 2);
    m_buttonGroup->addButton(m_scaleBtn, 3);

    m_printUnit->addItem(tr("Millimeters"));
    m_printUnit->addItem(tr("Centimeters"));
    m_printUnit->addItem(tr("Inches"));

    QHBoxLayout *layout = new QHBoxLayout(this);

    QGroupBox *posBtnBox = new QGroupBox(tr("Image Position"));

    QVBoxLayout *vLayout = new QVBoxLayout;
    QWidget *btnsWidget = new QWidget;
    QGridLayout *btnsLayout = new QGridLayout(btnsWidget);
    btnsLayout->setMargin(50);
    btnsLayout->setSpacing(1);

    vLayout->addWidget(btnsWidget, Qt::AlignCenter);
    posBtnBox->setLayout(vLayout);

    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            QToolButton *btn = new QToolButton(posBtnBox);
            btn->setFixedSize(40, 40);
            btn->setCheckable(true);
            btnsLayout->addWidget(btn, row, column);

            Qt::Alignment alignment;
            if (row == 0) {
                alignment = Qt::AlignTop;
            } else if (row == 1) {
                    alignment = Qt::AlignVCenter;
                } else {
                    alignment = Qt::AlignBottom;
                }
                if (column == 0) {
                    alignment |= Qt::AlignLeft;
                } else if (column == 1) {
                    alignment |= Qt::AlignHCenter;
                } else {
                    alignment |= Qt::AlignRight;
                }

                m_posBtnGroup->addButton(btn, (int) alignment);
            }
        }

    QGroupBox *groupBox = new QGroupBox(tr("Scaling"));
    QVBoxLayout *groupLayout = new QVBoxLayout;
    groupBox->setLayout(groupLayout);

    QHBoxLayout *scaleToLayout = new QHBoxLayout;
    scaleToLayout->addWidget(m_printWidth);
    scaleToLayout->addWidget(m_printHeight);
    scaleToLayout->addWidget(m_printUnit);

    groupLayout->addWidget(m_noScaleBtn);
    groupLayout->addWidget(m_fitToImageBtn);
    groupLayout->addWidget(m_fitToPageBtn);
    groupLayout->addWidget(m_scaleBtn);
    groupLayout->addLayout(scaleToLayout);

    layout->addWidget(posBtnBox);
    layout->addWidget(groupBox);

    setWindowTitle(tr("Image Settings"));

    connect(m_noScaleBtn, &QRadioButton::toggled, this,
            [=] {
                updateStatus();
                m_settings.setValue("button_index", 0);
            });

    connect(m_fitToImageBtn, &QRadioButton::toggled, this,
            [=] {
                updateStatus();
                m_settings.setValue("button_index", 1);
            });

    connect(m_fitToPageBtn, &QRadioButton::toggled, this,
            [=] {
                updateStatus();
                m_settings.setValue("button_index", 2);
            });

    connect(m_scaleBtn, &QRadioButton::toggled, this,
            [=] (bool) {
                updateStatus();
                m_settings.setValue("button_index", 3);
            });

    connect(m_posBtnGroup, static_cast<void (QButtonGroup::*)(QAbstractButton *)>(&QButtonGroup::buttonClicked), this, [=] (QAbstractButton *) {
//        m_settings.setValue("pos", m_posBtnGroup->checkedId());
        emit valueChanged();
    });

    connect(m_printWidth, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &PrintOptionsPage::valueChanged);
    connect(m_printHeight, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &PrintOptionsPage::valueChanged);
    connect(m_printUnit, &QComboBox::currentTextChanged, this, &PrintOptionsPage::valueChanged);

    init();
}

void PrintOptionsPage::init()
{
    Qt::Alignment defaultAlignment;
    defaultAlignment = Qt::AlignVCenter | Qt::AlignHCenter;
    m_posBtnGroup->button(defaultAlignment)->setChecked(true);

    switch (m_settings.value("button_index", 2).toInt()) {
    case 0:
        m_noScaleBtn->click();
        break;
    case 1:
        m_fitToImageBtn->click();
        break;
    case 2:
        m_fitToPageBtn->click();
        break;
    case 3:
        m_scaleBtn->click();
        break;
    }

//    setStyleSheet("QToolButton {"
//                  "border: 1px solid #A0A0A4;"
//                  "background: #FBFBFB;"
//                  "}"
//                  "QToolButton:hover {"
//                  "border: 1px solid #2CA7F8;"
//                  "}"
//                  "QToolButton:checked {"
//                  "border: none;"
//                  "background: #2CA7F8;"
//                  "}");
}

void PrintOptionsPage::updateStatus()
{
    bool enabled = m_scaleBtn->isChecked();

    m_printWidth->setEnabled(enabled);
    m_printHeight->setEnabled(enabled);
    m_printUnit->setEnabled(enabled);

    emit valueChanged();
}

PrintOptionsPage::ScaleMode PrintOptionsPage::scaleMode()
{
    return ScaleMode(m_buttonGroup->checkedId());
}

PrintOptionsPage::Unit PrintOptionsPage::scaleUnit()
{
    return Unit(m_printUnit->currentIndex());
}

double PrintOptionsPage::scaleWidth()
{
    return m_printWidth->value() * unitToInches(this->scaleUnit());
}

double PrintOptionsPage::scaleHeight()
{
    return m_printHeight->value() * unitToInches(scaleUnit());
}

Qt::Alignment PrintOptionsPage::alignment()
{
    return Qt::Alignment(m_posBtnGroup->checkedId());
}

double PrintOptionsPage::unitToInches(PrintOptionsPage::Unit unit)
{
    switch (unit) {
    case PrintOptionsPage::Inches:
        return 1;
    case PrintOptionsPage::Centimeters:
        return 1 / 2.54;
    case PrintOptionsPage::Millimeters:
        return 1 / 25.4;
    }
}
