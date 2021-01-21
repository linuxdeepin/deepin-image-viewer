#ifndef MOREPICFLOATWIDGET_H
#define MOREPICFLOATWIDGET_H
#include <DFloatingWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <DIconButton>
#include <QGuiApplication>
#include <DLabel>
#include <DIconButton>
#include <DWidget>
DWIDGET_USE_NAMESPACE
class MorePicFloatWidget : public DFloatingWidget
{
public:
   explicit MorePicFloatWidget(QWidget *parent=nullptr);
    ~MorePicFloatWidget();
   void initUI();

   DIconButton * getButtonUp();

   DIconButton * getButtonDown();

   void setLabelText(const QString& num );
private:
//ui
    QVBoxLayout *m_pLayout{nullptr};
    DLabel *m_labelNum{nullptr};
    DWidget *m_labelUp{nullptr};
    DWidget *m_labelDown{nullptr};
    DIconButton *m_buttonUp{nullptr};
    DIconButton *m_buttonDown{nullptr};
};

#endif // MOREPICFLOATWIDGET_H
