#include "createalbumdialog.h"
#include "controller/databasemanager.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>

CreateAlbumDialog::CreateAlbumDialog(QWidget *parent, QWidget *source)
    :BlureDialog(parent, source),
      m_dbManager(DatabaseManager::instance())
{
    setMinimumWidth(450);

    QWidget *w = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(25, 0, 36, 0);
    layout->setSpacing(10);

    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(":/images/resources/images/new_album.png"));
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
    connect(edit, &QLineEdit::textChanged, this, [=] (const QString &t) {
        disableButton(tr("OK"), t.isEmpty());
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
            createAlbum(edit->text().trimmed());
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


const QString CreateAlbumDialog::getNewAlbumName() const
{
    const QString nan = tr("Unnamed");
    const QStringList albums = m_dbManager->getAlbumNameList();
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
        return nan + QString::number(1);
    }
    else {
        qSort(countList.begin(), countList.end());
        for (int c : countList) {
            if (countList.indexOf(c + 1) == -1) {
                return nan + QString::number(c + 1);
            }
        }

        return nan;
    }
}

void CreateAlbumDialog::createAlbum(const QString &newName)
{
    if (m_dbManager->getAlbumNameList().indexOf(newName) == -1) {
        m_dbManager->insertImageIntoAlbum(newName, "", "");
    }
    else {
        m_dbManager->insertImageIntoAlbum(getNewAlbumName(), "", "");
    }
}
