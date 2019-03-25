#ifndef PRINTOPTIONSPAGE_H
#define PRINTOPTIONSPAGE_H

#include <QDialog>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QButtonGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QSettings>

class PrintOptionsPage : public QDialog
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
    QDoubleSpinBox *m_printWidth;
    QDoubleSpinBox *m_printHeight;
    QComboBox *m_printUnit;
    QButtonGroup *m_buttonGroup;
    QButtonGroup *m_posBtnGroup;
    QSettings m_settings;
};

#endif // PRINTOPTIONSPAGE_H
