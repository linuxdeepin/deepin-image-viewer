#include "timelinepanel.h"
#include <QPushButton>
#include <QLabel>
#include <QDebug>

TimelinePanel::TimelinePanel(QWidget *parent)
    : ModulePanel(parent)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    initImportFrame();
}

QWidget *TimelinePanel::toolbarBottomContent()
{
    return NULL;
}

QWidget *TimelinePanel::toolbarTopLeftContent()
{
    return NULL;
}

QWidget *TimelinePanel::toolbarTopMiddleContent()
{
    return NULL;
}

QWidget *TimelinePanel::extensionPanelContent()
{
    return NULL;
}

void TimelinePanel::initImportFrame()
{
    //check database befor show import frame

    QLabel *il = new QLabel();
    il->setPixmap(QPixmap(":/images/resources/images/timeline_import_backimg.png"));

    QPushButton *ib = new QPushButton("Import Picture");
    ib->setObjectName("ImportFrameButton");
    connect(ib, &QPushButton::clicked, this, [=] {
        //TODO, open file select dialog
        qDebug() << "Select import folder...";
    });

    QLabel *tl = new QLabel(tr("You can also drop folder here to import picture"));
    tl->setObjectName("ImportFrameTooltip");

    QWidget *importWidget = new QWidget;
    QFontMetrics fm(tl->font());
    importWidget->setMaximumWidth(fm.width(tl->text()));

    QVBoxLayout *layout = new QVBoxLayout(importWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    layout->addStretch(1);
    layout->addWidget(il, 0, Qt::AlignHCenter);
    layout->addWidget(ib, 0, Qt::AlignHCenter);
    layout->addWidget(tl, 0, Qt::AlignHCenter);
    layout->addStretch(1);

    m_mainLayout->addWidget(importWidget, 1, Qt::AlignHCenter);
}
