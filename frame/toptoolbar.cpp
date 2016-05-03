#include "toptoolbar.h"
#include <QDebug>
#include <QGradient>
#include <QResizeEvent>
#include <QApplication>
#include <QStackedWidget>
#include <dcircleprogress.h>
#include "controller/importer.h"
#include "frame/mainwindow.h"

using namespace Dtk::Widget;

TopToolbar::TopToolbar(QWidget *parent, QWidget *source)
    :BlureFrame(parent, source)
{
    QLinearGradient linearGrad;
    linearGrad.setColorAt(0, QColor(0, 0, 0, 178));
    linearGrad.setColorAt(1, QColor(0, 0, 0, 204));

    setCoverBrush(QBrush(linearGrad));

    initWidgets();

    connect(this, SIGNAL(moving()), parentWidget()->parentWidget(), SLOT(startMoving()));
}

void TopToolbar::setLeftContent(QWidget *content)
{
    QLayoutItem *child;
    while ((child = m_leftLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    m_leftLayout->addWidget(content);
}

void TopToolbar::setMiddleContent(QWidget *content)
{
    QLayoutItem *child;
    while ((child = m_middleLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    m_middleLayout->addWidget(content);
}

void TopToolbar::resizeEvent(QResizeEvent *e)
{
    m_leftContent->setFixedWidth(e->size().width() / 3);
    m_middleContent->setFixedWidth(e->size().width() / 3);
    m_rightContent->setFixedWidth(e->size().width() / 3);
}

void TopToolbar::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    qDebug() << "startMoving";
    emit moving();
}

void TopToolbar::initWidgets()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    DCircleProgress *importProgress = new DCircleProgress;
    importProgress->setValue(0);
    importProgress->setFixedSize(21, 21);
    importProgress->setVisible(false);
    connect(Importer::instance(), &Importer::importProgressChanged,
            this, [=] (double progress) {
        importProgress->setVisible(progress != 1);
        importProgress->setValue(progress * 100);
    });

    DWindowOptionButton *ob = new DWindowOptionButton;
    connect(ob, &DWindowOptionButton::clicked, this, [=] {
        if (parentWidget()) {
            qDebug() << "Show option...";
        }
    });
    DWindowMinButton *minb = new DWindowMinButton;
    connect(minb, SIGNAL(clicked()), parentWidget()->parentWidget(), SLOT(showMinimized()));

    QStackedWidget *sw = new QStackedWidget;
    DWindowMaxButton *maxb = new DWindowMaxButton;

    connect(maxb, &DWindowMaxButton::clicked, this, [=] {
        if (parentWidget()) {
            parentWidget()->parentWidget()->showMaximized();
            sw->setCurrentIndex(1);
        }
    });
    DWindowRestoreButton *rb = new DWindowRestoreButton;
    connect(rb, &DWindowRestoreButton::clicked, this, [=] {
        if (parentWidget()) {
            parentWidget()->parentWidget()->showNormal();
            sw->setCurrentIndex(0);
        }
    });
    sw->addWidget(maxb);
    sw->addWidget(rb);
    DWindowCloseButton *cb = new DWindowCloseButton;
    connect(cb, &DWindowCloseButton::clicked, qApp, &QApplication::quit);

    m_rightContent = new QWidget;
    QHBoxLayout *rightLayout = new QHBoxLayout(m_rightContent);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addStretch(1);
    rightLayout->addWidget(importProgress);
    rightLayout->addSpacing(38);
    rightLayout->addWidget(ob);
    rightLayout->addWidget(minb);
    rightLayout->addWidget(sw);
    rightLayout->addWidget(cb);

    m_leftContent = new QWidget;
    m_leftLayout = new QHBoxLayout(m_leftContent);
    m_leftLayout->setContentsMargins(0, 0, 0, 0);
    m_leftLayout->setSpacing(0);

    m_middleContent = new QWidget;
    m_middleLayout = new QHBoxLayout(m_middleContent);
    m_middleLayout->setContentsMargins(0, 0, 0, 0);
    m_middleLayout->setSpacing(0);

    mainLayout->addWidget(m_leftContent);
    mainLayout->addWidget(m_middleContent);
    mainLayout->addWidget(m_rightContent);
}
