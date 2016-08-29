#include "application.h"
#include "createalbumdialog.h"
#include "controller/databasemanager.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>

CreateAlbumDialog::CreateAlbumDialog(QWidget* parent)
    :DDialog(parent)
{
    setMinimumWidth(450);

    QWidget *w = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(25, 0, 36, 0);
    layout->setSpacing(10);

    QLabel *icon = new QLabel;
    QPixmap pix(":/images/resources/images/import_dir.png");
    icon->setPixmap(pix);
    layout->addWidget(icon);

    QLabel *title = new QLabel(tr("New album"));
    title->setObjectName("CreateTitleLabel");
    title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QLineEdit *edit = new QLineEdit;
    edit->setObjectName("CreateEdit");
    edit->setText(getNewAlbumName());
    edit->selectAll();
    connect(edit, &QLineEdit::returnPressed, this, [=] {
        const QString album = edit->text().trimmed();
        if (! album.isEmpty()) {
            createAlbum(album);
            this->close();
        }
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

    connect(this, &CreateAlbumDialog::buttonClicked, this, [=] (int id, const QString &text) {
        Q_UNUSED(text);
        if (id == 0) {
            this->close();
        }
        else {
            createAlbum(edit->text().trimmed());
            this->close();
        }
    });

    this->insertContent(0, w);
}

void CreateAlbumDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }
}


const QString CreateAlbumDialog::getNewAlbumName() const
{
    const QString nan = tr("Unnamed");
    const QStringList albums = dApp->databaseM->getAlbumNameList();
    QList<int> countList;
    for (QString album : albums) {
        if (album.startsWith(nan)) {
            countList << QString(album.split(nan).last()).toInt();
        }
    }

    if (countList.isEmpty()) {
        return nan;
    }
    else if (countList.length() == 1) {
        return nan + QString::number(2);
    }
    else {
        qSort(countList.begin(), countList.end());
        if (countList.first() != 0)
            return nan;
        for (int c : countList) {
            if (c == 0) {
                // Index star from 2.
                c = 1;
            }
            if (countList.indexOf(c + 1) == -1) {
                return nan + QString::number(c + 1);
            }
        }
        return nan;
    }
}

void CreateAlbumDialog::createAlbum(const QString &newName)
{
    if (dApp->databaseM->getAlbumNameList().indexOf(newName) == -1) {
        dApp->databaseM->insertImageIntoAlbum(newName, "", "");
    }
    else {
        dApp->databaseM->insertImageIntoAlbum(getNewAlbumName(), "", "");
    }
}
