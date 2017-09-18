/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
    :Dialog(parent)
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
       int num = 1;
       QString albumName = nan;
       while(DBManager::instance()->isAlbumExistInDB(albumName)) {
           num++;
           albumName = nan + QString::number(num);
       }
       return (const QString)(albumName);
}

const QString AlbumCreateDialog::getCreateAlbumName() const
{
    return m_createAlbumName;
}

void AlbumCreateDialog::createAlbum(const QString &newName)
{
    if (! DBManager::instance()->getAllAlbumNames().contains(newName)) {
        m_createAlbumName = newName;
        DBManager::instance()->insertIntoAlbum(newName, QStringList(" "));
    }
    else {
        m_createAlbumName = getNewAlbumName();
        DBManager::instance()->insertIntoAlbum(getNewAlbumName(), QStringList(" "));
    }

    emit albumAdded();
}
