#include "application.h"
#include "dirimportdialog.h"
#include "controller/dbmanager.h"
#include "controller/importer.h"
#include "controller/importer.h"
#include "dirwatcher/scanpathsdialog.h"
#include "utils/baseutils.h"
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>

DirImportDialog::DirImportDialog(const QString &dir, const QString &album, QWidget* parent)
    :DDialog(parent)
{
    // It may appear several windows At the same
    // and Qt::WindowModal will cause stuck
    setWindowModality(Qt::ApplicationModal);

    setIconPixmap(QPixmap(":/dialogs/images/resources/images/album_bg_normal.png"));

    addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    if (! album.isEmpty()) {
        addButton(tr("Sync and Add"), false, DDialog::ButtonNormal);
    }
    else {
        addButton(tr("Sync and Create"), false, DDialog::ButtonNormal);
    }
    addButton(tr("Sync"), true, DDialog::ButtonRecommend);

    setTitle(tr("Are you sure to add this folder to sync list?"));
    if (! album.isEmpty()) {
        setMessage(tr("Add pictures in the folder to this album"));
    }
    else {
        setMessage(tr("You can also create this album"));
    }

    connect(this, &DirImportDialog::closed,
            this, &DirImportDialog::deleteLater);
    connect(this, &DirImportDialog::buttonClicked, this, [=] (int id) {
        if(id == 1){
            const QString tmpAlbum = album.isEmpty() ? QFileInfo(dir).fileName() : album;
            dApp->importer->appendDir(dir, tmpAlbum);
            dApp->scanDialog->addPath(dir);
            // For UI update
            dApp->dbM->insertIntoAlbum(tmpAlbum, QStringList(" "));//FIXDB
            emit albumCreated();
        }
        else if (id == 2) {
            dApp->importer->appendDir(dir);
            dApp->scanDialog->addPath(dir);
        }
    });
}

void DirImportDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }
}
