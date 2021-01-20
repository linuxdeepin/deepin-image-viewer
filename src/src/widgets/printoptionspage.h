#ifndef PRINTOPTIONSPAGE_H
#define PRINTOPTIONSPAGE_H

#include <DDialog>
#include <QRadioButton>
#include <DDoubleSpinBox>
#include <QButtonGroup>
#include <DComboBox>
#include <QCheckBox>
#include <QSettings>
class QHBoxLayout;
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

    explicit PrintOptionsPage(QWidget *parent = nullptr);

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
    QRadioButton *m_noScaleBtn{nullptr};
    QRadioButton *m_fitToImageBtn{nullptr};
    QRadioButton *m_fitToPageBtn{nullptr};
    QRadioButton *m_scaleBtn{nullptr};
    QDSBToDDoubleSpinBox *m_printWidth{nullptr};
    QDSBToDDoubleSpinBox *m_printHeight{nullptr};
    QCBToDComboBox *m_printUnit{nullptr};
    QButtonGroup *m_buttonGroup{nullptr};
    QButtonGroup *m_posBtnGroup{nullptr};
    QSettings m_settings{nullptr};
};

#endif // PRINTOPTIONSPAGE_H
