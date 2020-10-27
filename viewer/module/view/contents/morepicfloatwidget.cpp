#include "morepicfloatwidget.h"

MorePicFloatWidget::MorePicFloatWidget(QWidget *parent)
    :DFloatingWidget(parent)
{

}

MorePicFloatWidget::~MorePicFloatWidget()
{

}

void MorePicFloatWidget::initUI()
{
    m_pLayout=new QVBoxLayout(this);
    this->setLayout(m_pLayout);
    m_labelNum=new DLabel();
    m_buttonUp=new  DPushButton("-");
    m_buttonDown=new  DPushButton("+");
    m_pLayout->addWidget(m_labelNum);
    m_labelNum->setText("   0/0");
    m_pLayout->addWidget(m_buttonUp);
    m_pLayout->addWidget(m_buttonDown);
}

DPushButton *MorePicFloatWidget::getButtonUp()
{
    return m_buttonUp;
}

DPushButton *MorePicFloatWidget::getButtonDown()
{
    return m_buttonDown;
}

void MorePicFloatWidget::setLabelText(const QString &num)
{
     m_labelNum->setText("   "+num);
}
