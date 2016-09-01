#include "bottomtoolbar.h"

BottomToolbar::BottomToolbar(QWidget *parent)
    : BlurFrame(parent)
{
    setCoverBrush(QBrush(QColor(24, 24, 24, 230)));

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
}

void BottomToolbar::setContent(QWidget *content)
{
    QLayoutItem *child;
    while ((child = m_mainLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    m_mainLayout->addWidget(content);
}

void BottomToolbar::mouseMoveEvent(QMouseEvent *)
{

}
