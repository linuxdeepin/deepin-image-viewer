#include "createalbumdialog.h"
#include "controller/databasemanager.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>

CreateAlbumDialog::CreateAlbumDialog(QWidget *parent, QWidget *source)
    :BlureDialog(parent, source)
{
    setMinimumWidth(450);

    QWidget *w = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(15, 0, 36, 0);
    layout->setSpacing(10);

    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(":/images/resources/images/new_album.png"));
    layout->addWidget(icon);

    QLabel *title = new QLabel(tr("New album"));
    title->setObjectName("CreateTitleLabel");
    title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QLineEdit *edit = new QLineEdit;
    edit->setObjectName("CreateEdit");
    edit->setText(tr("Unnamed"));
    connect(edit, &QLineEdit::returnPressed, this, [=] {
        DatabaseManager::instance()->insertImageIntoAlbum(
                    edit->text().trimmed(), "", "");
        this->close();
    });
    QVBoxLayout *rl = new QVBoxLayout;
    rl->setContentsMargins(0, 0, 0, 0);
    rl->addSpacing(5);
    rl->addWidget(title);
    rl->addSpacing(5);
    rl->addWidget(edit);
    rl->addStretch(1);
    layout->addLayout(rl);

    addButton(tr("Cancel"), 0);
    addButton(tr("OK"), 1);

    connect(this, &CreateAlbumDialog::clicked, this, [=] (int id) {
        if (id == 0) {
            this->close();
        }
        else {
            DatabaseManager::instance()->insertImageIntoAlbum(
                        edit->text().trimmed(), "", "");
            this->close();
        }
    });

    setContent(w);
}

void CreateAlbumDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }
}
