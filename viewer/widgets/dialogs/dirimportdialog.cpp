#include "application.h"
#include "dirimportdialog.h"
#include "controller/dbmanager.h"
#include "controller/importer.h"
#include "utils/baseutils.h"
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>

DirImportDialog::DirImportDialog(const QString &dir, QWidget* parent)
    :DDialog(parent)
{
    // It may appear several windows At the same
    // and Qt::WindowModal will cause stuck
    setWindowModality(Qt::ApplicationModal);

    setIconPixmap(QPixmap(":/dialogs/images/resources/images/album_bg_normal.png"));

    addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    addButton(tr("Import only"), false, DDialog::ButtonNormal);
    addButton(tr("OK"), true, DDialog::ButtonRecommend);

    // Input content
    const QString subStyle =
    utils::base::getFileContent(":/dialogs/qss/resources/qss/inputdialog.qss");
    QLabel *title = new QLabel(tr("Create an album named after the imported folder?"));
    title->setStyleSheet(subStyle);
    title->setObjectName("DialogTitle");
    title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLineEdit *edit = new QLineEdit;
    edit->setStyleSheet(subStyle);
    edit->setObjectName("DialogEdit");
    edit->setContextMenuPolicy(Qt::PreventContextMenu);
    edit->setFixedSize(240, 22);
    edit->setText(QFileInfo(dir).baseName());
    connect(this, &DirImportDialog::visibleChanged, this, [=] (bool v) {
        if (! v) return;
        edit->setFocus();
        edit->selectAll();
    });

    QWidget *w = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    layout->addStretch();
    layout->addWidget(title);
    layout->addWidget(edit);
    layout->addStretch();
    addContent(w);

    connect(this, &DirImportDialog::closed,
            this, &DirImportDialog::deleteLater);
    connect(this, &DirImportDialog::buttonClicked, this, [=] (int id) {
        if (edit->text().isEmpty()) return;

        if(id == 1){
            dApp->importer->appendDir(dir);
        }
        else if (id == 2) {
            const QString album = edit->text().trimmed();
            dApp->importer->appendDir(dir, album);
            // For UI update
            dApp->dbM->insertIntoAlbum(album, QStringList());//FIXDB
            emit albumCreated();
        }
    });
}

void DirImportDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }
}
