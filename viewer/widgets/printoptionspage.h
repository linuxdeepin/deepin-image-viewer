#ifndef PRINTOPTIONSPAGE_H
#define PRINTOPTIONSPAGE_H

#include <DDialog>
#include <QRadioButton>
#include <DDoubleSpinBox>
#include <QButtonGroup>
#include <DComboBox>
#include <QCheckBox>
#include <QSettings>

DWIDGET_USE_NAMESPACE
typedef DDialog QDlgToDialog;
typedef DDoubleSpinBox QDSBToDDoubleSpinBox;
typedef DComboBox QCBToDComboBox;

class PrintOptionsPage : public QDlgToDialog
{
    Q_OBJECT

public:

    enum Unit {
        Millimeters = 0,
        Centimeters,
        Inches
    };

    enum ScaleMode {
        NoScale = 0,
        ScaleToExpanding,
        ScaleToPage,
        ScaleToCustomSize
    };

    PrintOptionsPage(QWidget *parent = nullptr);

    ScaleMode scaleMode();
    Unit scaleUnit();
    double scaleWidth();
    double scaleHeight();
    Qt::Alignment alignment();

    QRadioButton* getnoScaleBtn();
    QRadioButton* getfitToImageBtn();
    QRadioButton* getfitToPageBtn();
    QRadioButton* getscaleBtn();
signals:
    void valueChanged();

private:
    void init();
    void updateStatus();
    double unitToInches(Unit unit);

private:
    QRadioButton *m_noScaleBtn;
    QRadioButton *m_fitToImageBtn;
    QRadioButton *m_fitToPageBtn;
    QRadioButton *m_scaleBtn;
    QDSBToDDoubleSpinBox *m_printWidth;
    QDSBToDDoubleSpinBox *m_printHeight;
    QCBToDComboBox *m_printUnit;
    QButtonGroup *m_buttonGroup;
    QButtonGroup *m_posBtnGroup;
    QSettings m_settings;
};

#endif // PRINTOPTIONSPAGE_H
