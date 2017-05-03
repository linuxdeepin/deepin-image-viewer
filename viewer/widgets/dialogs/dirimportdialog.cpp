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
    :Dialog(parent)
{
    setMaximumWidth(380);

    // It may appear several windows At the same
    // and Qt::WindowModal will cause stuck
    setWindowModality(Qt::ApplicationModal);

    setIconPixmap(QPixmap(":/dialogs/images/resources/images/directory.png"));

    addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    if (! album.isEmpty()) {
        addButton(tr("OK"), false, DDialog::ButtonRecommend);
        setTitle(tr("Are you sure to add the pictures to the album?"));
    }
    else {
        addButton(tr("Sync and Create"), false, DDialog::ButtonNormal);
        addButton(tr("Sync"), true, DDialog::ButtonRecommend);
        setTitle(tr("Are you sure to add to the sync list and create the album named after this folder?"));
    }

    connect(this, &DirImportDialog::closed,
            this, &DirImportDialog::deleteLater);
    connect(this, &DirImportDialog::buttonClicked, this, [=] (int id) {
        if(id == 1){
            const QString tmpAlbum = album.isEmpty() ? QFileInfo(dir).fileName() : album;
            Importer::instance()->appendDir(dir, tmpAlbum);
            ScanPathsDialog::instance()->addPath(dir);
            // For UI update
            DBManager::instance()->insertIntoAlbum(tmpAlbum, QStringList(" "));//FIXDB
            emit albumCreated();
        }
        else if (id == 2) {
            Importer::instance()->appendDir(dir);
            ScanPathsDialog::instance()->addPath(dir);
        }
    });
}

void DirImportDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }
}
