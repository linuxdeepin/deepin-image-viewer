#include "bluredialog.h"
#include "utils/baseutils.h"
#include <dwindowclosebutton.h>
#include <QBoxLayout>
#include <QFile>
#include <QPushButton>
#include <QDebug>

BlureDialog::BlureDialog(QWidget *parent, QWidget *source)
    : BlureFrame(parent, source),m_first(true)
{
    setBorderRadius(4);
    setBorderWidth(1);
    setBorderColor(QColor(255, 255, 255, 51));

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Sheet | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    Dtk::Widget::DWindowCloseButton *cbutton =
            new Dtk::Widget::DWindowCloseButton;
    connect(cbutton, &Dtk::Widget::DWindowCloseButton::clicked,
            this, &BlureDialog::close);
    layout->addWidget(cbutton, 1, Qt::AlignRight);

    m_contentLayout = new QVBoxLayout;
    m_contentLayout->setContentsMargins(0, 0, 0 , 0);
    layout->addLayout(m_contentLayout);
    layout->addSpacing(10);

    m_buttonsLayout = new QHBoxLayout;
    m_buttonsLayout->setSpacing(0);
    m_buttonsLayout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(m_buttonsLayout);

    setStyleSheet(utils::base::getFileContent(
                      ":/qss/resources/qss/BlureDialog.qss"));
}

void BlureDialog::addButton(const QString &name, int id)
{
    QPushButton *button = new QPushButton(name);
    // For draw border style
    if (m_first) {
        m_first = false;
        button->setProperty("FirstButton", true);
    }
    connect(button, &QPushButton::clicked, this, [=] {
        emit clicked(name, id);
    });
    m_buttonsLayout->addWidget(button);
}

void BlureDialog::setContent(QWidget *content)
{
    m_contentLayout->addWidget(content);
}

void BlureDialog::close()
{
    emit closed();
    BlureFrame::close();
}
