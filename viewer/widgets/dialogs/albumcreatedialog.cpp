#include "application.h"
#include "albumcreatedialog.h"
#include "controller/dbmanager.h"
#include "utils/baseutils.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>

AlbumCreateDialog::AlbumCreateDialog(QWidget* parent)
    :DDialog(parent)
{
    setModal(true);

    setIconPixmap(QPixmap(":/dialogs/images/resources/images/album_bg_normal.png"));

    addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    addButton(tr("OK"), true, DDialog::ButtonRecommend);

    // Input content
    const QString subStyle =
    utils::base::getFileContent(":/dialogs/qss/resources/qss/inputdialog.qss");
    QLabel *title = new QLabel(tr("New album"));
    title->setStyleSheet(subStyle);
    title->setObjectName("DialogTitle");
    title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLineEdit *edit = new QLineEdit;
    edit->setStyleSheet(subStyle);
    edit->setObjectName("DialogEdit");
    edit->setText(getNewAlbumName());
    edit->setContextMenuPolicy(Qt::PreventContextMenu);
    edit->setFixedSize(240, 22);
    connect(this, &AlbumCreateDialog::visibleChanged, this, [=] (bool v) {
        if (! v) return;
        edit->setFocus();
        edit->selectAll();
    });
    connect(edit, &QLineEdit::returnPressed, this, [=] {
        const QString album = edit->text().trimmed();
        if (! album.isEmpty()) {
            createAlbum(album);
            this->close();
        }
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

    connect(this, &AlbumCreateDialog::closed,
            this, &AlbumCreateDialog::deleteLater);
    connect(this, &AlbumCreateDialog::buttonClicked, this, [=] (int id) {
        if (id == 1) {
            if (edit->text().simplified().length()!= 0)
                createAlbum(edit->text().trimmed());
            else
                createAlbum(tr("Unnamed"));
        }
    });
}

void AlbumCreateDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }
}

/*!
 * \brief AlbumCreateDialog::getNewAlbumName
 * \return Return a string like "Unnamed3", &etc
 */
const QString AlbumCreateDialog::getNewAlbumName() const
{
    const QString nan = tr("Unnamed");
    const QStringList albums = dApp->dbM->getAllAlbumNames();
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

const QString AlbumCreateDialog::getCreateAlbumName() const
{
    return m_createAlbumName;
}

void AlbumCreateDialog::createAlbum(const QString &newName)
{
    if (! dApp->dbM->getAllAlbumNames().contains(newName)) {
        m_createAlbumName = newName;
        dApp->dbM->insertIntoAlbum(newName, QStringList(" "));
    }
    else {
        m_createAlbumName = getNewAlbumName();
        dApp->dbM->insertIntoAlbum(getNewAlbumName(), QStringList(" "));
    }

    emit albumAdded();
}
