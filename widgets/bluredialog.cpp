#include "bluredialog.h"
#include "utils/baseutils.h"
#include <dwindowclosebutton.h>
#include <QBoxLayout>
#include <QFile>
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>
#include <QFont>

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

void BlureDialog::disableButton(const QString &name, bool disable)
{
    for (int i = 0; i < m_buttonsLayout->count(); i ++) {
        QPushButton *button =
            qobject_cast<QPushButton *>(m_buttonsLayout->itemAt(i)->widget());
        if (button) {
            if (name == button->text()) {
                button->setDisabled(disable);
            }
        }
    }
}

void BlureDialog::addButton(const QString &name, int id)
{
    QPushButton *button = new QPushButton(name);
    QFont btnFont;
    btnFont.setPixelSize(12);
    button->setFont(btnFont);
    // For draw border style
    if (m_first) {
        m_first = false;
        button->setProperty("FirstButton", true);
    }
    connect(button, &QPushButton::clicked, this, [=] {
        emit clicked(id);
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
