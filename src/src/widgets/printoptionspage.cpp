//#include "printoptionspage.h"
//#include <QHBoxLayout>
//#include <DToolButton>
//#include <DGroupBox>
//#include <QRadioButton>
//#include <QPushButton>
//#include <QDebug>
//#include "accessibility/ac-desktop-define.h"
//DWIDGET_USE_NAMESPACE
//typedef DToolButton QTBToDToolButton;
//typedef DGroupBox QGBToDGroupBox;

//PrintOptionsPage::PrintOptionsPage(QWidget *parent)
//    : QDlgToDialog(parent),
//      m_settings("deepin", "print-image-option")
//{
//    m_noScaleBtn = new QRadioButton(tr("No scaling"));
//    m_noScaleBtn->setObjectName(NO_SCALE_RADIOBUTTON);
//    m_fitToImageBtn = new QRadioButton(tr("Fit page to image"));
//    m_fitToImageBtn->setObjectName(FITTOIMAGE_RADIOBUTTON);
//    m_fitToPageBtn = new QRadioButton(tr("Fit image to page"));
//    m_fitToPageBtn->setObjectName(FITTOPAGE_RADIOBUTTON);
//    m_scaleBtn = new QRadioButton(tr("Scale to:"));
//    m_scaleBtn->setObjectName(SCALE_RADIOBUTTON);

//    m_printWidth = new QDSBToDDoubleSpinBox;
//    m_printHeight = new QDSBToDDoubleSpinBox;
//    m_printUnit = new QCBToDComboBox;
//    m_buttonGroup = new QButtonGroup;
//    m_posBtnGroup = new QButtonGroup;

//    m_printWidth->setValue(m_settings.value("scale_width", 15.0).toDouble());
//    m_printHeight->setValue(m_settings.value("scale_height", 10.0).toDouble());

//    m_buttonGroup->addButton(m_noScaleBtn, 0);
//    m_buttonGroup->addButton(m_fitToImageBtn, 1);
//    m_buttonGroup->addButton(m_fitToPageBtn, 2);
//    m_buttonGroup->addButton(m_scaleBtn, 3);

//    m_printUnit->addItem(tr("Millimeters"));
//    m_printUnit->addItem(tr("Centimeters"));
//    m_printUnit->addItem(tr("Inches"));


//    QGBToDGroupBox *posBtnBox = new QGBToDGroupBox(tr("Image Position"));

//    QVBoxLayout *vLayout = new QVBoxLayout;
//    QWidget *btnsWidget = new QWidget;
//    QGridLayout *btnsLayout = new QGridLayout(btnsWidget);
//    btnsLayout->setMargin(50);
//    btnsLayout->setSpacing(1);

//    vLayout->addWidget(btnsWidget, Qt::AlignCenter);
//    posBtnBox->setLayout(vLayout);

//    for (int row = 0; row < 3; ++row) {
//        for (int column = 0; column < 3; ++column) {
//            QTBToDToolButton *btn = new QTBToDToolButton(posBtnBox);
//            btn->setFixedSize(40, 40);
//            btn->setCheckable(true);
//            btnsLayout->addWidget(btn, row, column);

//            Qt::Alignment alignment;
//            if (row == 0) {
//                alignment = Qt::AlignTop;
//            } else if (row == 1) {
//                alignment = Qt::AlignVCenter;
//            } else {
//                alignment = Qt::AlignBottom;
//            }
//            if (column == 0) {
//                alignment |= Qt::AlignLeft;
//            } else if (column == 1) {
//                alignment |= Qt::AlignHCenter;
//            } else {
//                alignment |= Qt::AlignRight;
//            }

//            m_posBtnGroup->addButton(btn, (int) alignment);
//        }
//    }

//    QGBToDGroupBox *groupBox = new QGBToDGroupBox(tr("Scaling"));
//    QVBoxLayout *groupLayout = new QVBoxLayout;
//    groupBox->setLayout(groupLayout);

//    QHBoxLayout *scaleToLayout = new QHBoxLayout;
//    scaleToLayout->addWidget(m_printWidth);
//    scaleToLayout->addWidget(m_printHeight);
//    scaleToLayout->addWidget(m_printUnit);

//    groupLayout->addWidget(m_noScaleBtn);
//    groupLayout->addWidget(m_fitToImageBtn);
//    groupLayout->addWidget(m_fitToPageBtn);
//    groupLayout->addWidget(m_scaleBtn);
//    groupLayout->addLayout(scaleToLayout);


//    setWindowTitle(tr("Image Settings"));

//    connect(m_noScaleBtn, &QRadioButton::toggled, this,
//    [ = ] {
//        updateStatus();
//        m_settings.setValue("button_index", 0);
//    });

//    connect(m_fitToImageBtn, &QRadioButton::toggled, this,
//    [ = ] {
//        updateStatus();
//        m_settings.setValue("button_index", 1);
//    });

//    connect(m_fitToPageBtn, &QRadioButton::toggled, this,
//    [ = ] {
//        updateStatus();
//        m_settings.setValue("button_index", 2);
//    });

//    connect(m_scaleBtn, &QRadioButton::toggled, this,
//    [ = ] (bool) {
//        updateStatus();
//        m_settings.setValue("button_index", 3);
//    });

//    connect(m_posBtnGroup, static_cast<void (QButtonGroup::*)(QAbstractButton *)>(&QButtonGroup::buttonClicked), this, [ = ] (QAbstractButton *) {
////        m_settings.setValue("pos", m_posBtnGroup->checkedId());
//        emit valueChanged();
//    });

//    connect(m_printWidth, static_cast<void(QDSBToDDoubleSpinBox::*)(double)>(&QDSBToDDoubleSpinBox::valueChanged), this, &PrintOptionsPage::valueChanged);
//    connect(m_printHeight, static_cast<void(QDSBToDDoubleSpinBox::*)(double)>(&QDSBToDDoubleSpinBox::valueChanged), this, &PrintOptionsPage::valueChanged);
//    connect(m_printUnit, &QCBToDComboBox::currentTextChanged, this, &PrintOptionsPage::valueChanged);

//    init();
//}

//void PrintOptionsPage::init()
//{
//    Qt::Alignment defaultAlignment;
//    defaultAlignment = Qt::AlignVCenter | Qt::AlignHCenter;
//    m_posBtnGroup->button(defaultAlignment)->setChecked(true);

//    switch (m_settings.value("button_index", 2).toInt()) {
//    case 0:
//        m_noScaleBtn->click();
//        break;
//    case 1:
//        m_fitToImageBtn->click();
//        break;
//    case 2:
//        m_fitToPageBtn->click();
//        break;
//    case 3:
//        m_scaleBtn->click();
//        break;
//    }

////    setStyleSheet("QToolButton {"
////                  "border: 1px solid #A0A0A4;"
////                  "background: #FBFBFB;"
////                  "}"
////                  "QToolButton:hover {"
////                  "border: 1px solid #2CA7F8;"
////                  "}"
////                  "QToolButton:checked {"
////                  "border: none;"
////                  "background: #2CA7F8;"
////                  "}");
//}

//void PrintOptionsPage::updateStatus()
//{
//    bool enabled = m_scaleBtn->isChecked();

//    m_printWidth->setEnabled(enabled);
//    m_printHeight->setEnabled(enabled);
//    m_printUnit->setEnabled(enabled);

//    emit valueChanged();
//}

