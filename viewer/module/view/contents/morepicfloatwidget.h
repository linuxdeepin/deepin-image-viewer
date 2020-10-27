#ifndef MOREPICFLOATWIDGET_H
#define MOREPICFLOATWIDGET_H
#include <DFloatingWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <DPushButton>
#include <DLabel>

DWIDGET_USE_NAMESPACE
class MorePicFloatWidget : public DFloatingWidget
{
public:
   MorePicFloatWidget(QWidget *parent=nullptr);
    ~MorePicFloatWidget();
   void initUI();

   DPushButton * getButtonUp();

   DPushButton * getButtonDown();

   void setLabelText(const QString& num );
private:
//ui
    QVBoxLayout *m_pLayout{nullptr};
    DLabel *m_labelNum{nullptr};
    DPushButton *m_buttonUp{nullptr};
    DPushButton *m_buttonDown{nullptr};
};

#endif // MOREPICFLOATWIDGET_H
