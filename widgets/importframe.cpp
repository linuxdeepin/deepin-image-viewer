#include "importframe.h"
#include "module/importandexport/importer.h"
#include "controller/databasemanager.h"
#include <QDropEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>

ImportFrame::ImportFrame(QWidget *parent) : QWidget(parent)
{
    this->setAcceptDrops(true);

    QLabel *bgLabel = new QLabel();
    bgLabel->setPixmap(QPixmap(":/images/timeline/resources/images/timeline/timeline_import_backimg.png"));

    QPushButton *importButton = new QPushButton(tr("Import"));
    importButton->setObjectName("ImportFrameButton");
    connect(importButton, &QPushButton::clicked, this, [=] {
        importImages();
    });

    QLabel *titleLabel = new QLabel(tr("You can also drop folder here to import picture"));
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

void ImportFrame::importImages()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    Importer::instance()->importFromPath(dir);
}
