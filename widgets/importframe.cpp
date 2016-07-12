#include "importframe.h"
#include "controller/importer.h"
#include "controller/databasemanager.h"
#include "controller/signalmanager.h"
#include <QDropEvent>

#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>

ImportFrame::ImportFrame(QWidget *parent) : QWidget(parent)
{
    this->setAcceptDrops(true);

    QLabel* bgLabel = new QLabel();
    bgLabel->setPixmap(QPixmap(":/images/resources/images/no_picture.png"));

    QPushButton *importButton = new QPushButton(tr("Import"));
    importButton->setObjectName("ImportFrameButton");
    connect(importButton, &QPushButton::clicked, this, [=] {
        Importer::instance()->showImportDialog();
    });

    QLabel *titleLabel = new QLabel(
                tr("Import or drag image to timeline"));
    titleLabel->setObjectName("ImportFrameTooltip");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(bgLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(20);
    layout->addWidget(importButton, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addStretch(1);
}

