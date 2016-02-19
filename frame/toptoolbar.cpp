#include "toptoolbar.h"
#include <QDebug>
#include <QGradient>
#include <QApplication>
#include <QStackedWidget>

TopToolbar::TopToolbar(QWidget *parent, QWidget *source)
    :BlureFrame(parent, source)
{
    QLinearGradient linearGrad;
    linearGrad.setColorAt(0, QColor(0, 0, 0, 178));
    linearGrad.setColorAt(1, QColor(0, 0, 0, 204));

    setCoverBrush(QBrush(linearGrad));

    initWidgets();
}

void TopToolbar::setLeftContent(QWidget *content)
{
    QLayoutItem *child;
    while ((child = m_leftLayout->takeAt(0)) != 0) {
        delete child;
    }

    m_leftLayout->addWidget(content);
}

void TopToolbar::initWidgets()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    DWindowOptionButton *ob = new DWindowOptionButton;
    connect(ob, &DWindowOptionButton::clicked, this, [=] {
        if (parentWidget()) {
            qDebug() << "Show option...";
        }
    });
    DWindowMinButton *minb = new DWindowMinButton;
    connect(minb, &DWindowMinButton::clicked, this, [=] {
        if (parentWidget()) {
            parentWidget()->showMinimized();
        }
    });
    QStackedWidget *sw = new QStackedWidget;
    DWindowMaxButton *maxb = new DWindowMaxButton;
    connect(maxb, &DWindowMaxButton::clicked, this, [=] {
        if (parentWidget()) {
            parentWidget()->showMaximized();
            sw->setCurrentIndex(1);
        }
    });
    DWindowRestoreButton *rb = new DWindowRestoreButton;
    connect(rb, &DWindowRestoreButton::clicked, this, [=] {
        if (parentWidget()) {
            parentWidget()->showNormal();
            sw->setCurrentIndex(0);
        }
    });
    sw->addWidget(maxb);
    sw->addWidget(rb);
    DWindowCloseButton *cb = new DWindowCloseButton;
    connect(cb, &DWindowCloseButton::clicked, qApp, &QApplication::quit);

    QHBoxLayout *rightLayout = new QHBoxLayout;
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addWidget(ob);
    rightLayout->addWidget(minb);
    rightLayout->addWidget(sw);
    rightLayout->addWidget(cb);

    m_leftLayout = new QHBoxLayout;
    m_leftLayout->setContentsMargins(0, 0, 0, 0);
    m_leftLayout->setSpacing(0);

    mainLayout->addLayout(m_leftLayout);
    mainLayout->addStretch(1);
    mainLayout->addLayout(rightLayout);
}
