#include "importdirdialog.h"
#include "application.h"
#include "controller/databasemanager.h"
#include "controller/importer.h"
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

ImportDirDialog::ImportDirDialog(QWidget *parent, QWidget *source)
    :BlureDialog(parent, source)
{
    // It may appear several windows At the same
    // and Qt::WindowModal will cause stuck
    setWindowModality(Qt::ApplicationModal);
    setMinimumWidth(450);

    QWidget *w = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(25, 0, 36, 0);
    layout->setSpacing(10);

    m_icon = new QLabel;
    m_icon->setPixmap(QPixmap(":/images/resources/images/import_dir.png"));
    layout->addWidget(m_icon);

    QLabel *title = new QLabel();
    title->setText(tr("Create an album named after the imported folder?"));
    title->setObjectName("CreateTitleLabel");
    title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_edit = new QLineEdit;
    m_edit->setObjectName("CreateEdit");
    connect(m_edit, &QLineEdit::textChanged, this, [=] (const QString &t) {
        disableButton(tr("OK"), t.isEmpty());
    });
    QVBoxLayout *rl = new QVBoxLayout;
    rl->setContentsMargins(0, 0, 0, 0);
    rl->addSpacing(5);
    rl->addWidget(title);
    rl->addSpacing(5);
    rl->addWidget(m_edit);
    rl->addStretch(1);
    layout->addLayout(rl);

    setContent(w);

    addButton(tr("Cancel"), 0);
    addButton(tr("Import only"), 1);
    addButton(tr("OK"), 2);

    connect(this, &ImportDirDialog::clicked, this, [=] (int id) {
        this->close();
        if(id == 1){
            dApp->importer->importDir(m_dir);
        }
        else if (id == 2) {
            const QString album = m_edit->text().trimmed();
            dApp->importer->importDir(m_dir, album);
            // For UI update
            dApp->databaseM->insertImageIntoAlbum(album, "", "");
            emit albumCreated();
        }
    });
}

void ImportDirDialog::import(const QString &dir)
{
    QFileInfo info(dir);
    if (! info.isDir()) {
        return;
    }

    m_dir = dir;
    m_edit->setText(QDir(dir).dirName());
    m_edit->selectAll();
    this->show();
    // TODO set icon compond pixmap
}
