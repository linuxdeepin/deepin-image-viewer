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

DirImportDialog::DirImportDialog(const QString &dir, QWidget* parent)
    :DDialog(parent)
{
//    setMaximumWidth(380);
//    setMinimumWidth(500);

    // It may appear several windows At the same
    // and Qt::WindowModal will cause stuck
    setWindowModality(Qt::ApplicationModal);

    setIconPixmap(QPixmap(":/dialogs/images/resources/images/album_bg_normal.png"));

    addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    addButton(tr("Add"), false, DDialog::ButtonNormal);
    addButton(tr("Create and add"), true, DDialog::ButtonRecommend);

    setTitle(tr("Are you sure to create album named by this "
                  "folder and sync pictures to this album?"));

    connect(this, &DirImportDialog::closed,
            this, &DirImportDialog::deleteLater);
    connect(this, &DirImportDialog::buttonClicked, this, [=] (int id) {
        if(id == 1){
            dApp->importer->appendDir(dir);
            dApp->scanDialog->addPath(dir);
        }
        else if (id == 2) {
            const QString album = QFileInfo(dir).fileName();
            dApp->importer->appendDir(dir, album);
            dApp->scanDialog->addPath(dir);
            // For UI update
            dApp->dbM->insertIntoAlbum(album, QStringList(" "));//FIXDB
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
