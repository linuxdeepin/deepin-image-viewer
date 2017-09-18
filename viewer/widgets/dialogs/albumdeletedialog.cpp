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
#include "albumdeletedialog.h"
#include "application.h"
#include "controller/dbmanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QPainter>
#include <QDebug>

AlbumDeleteDialog::AlbumDeleteDialog(const QStringList &paths, QWidget *parent)
    : Dialog(parent)
{
    setModal(true);

    setIconPixmap(generateThumbnail(paths));
    setTitle(tr("Are your sure to delete this album?"));

    addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    addButton(tr("Delete"), true, DDialog::ButtonWarning);

    connect(this, &AlbumDeleteDialog::closed,
            this, &AlbumDeleteDialog::deleteLater);
}

QPixmap AlbumDeleteDialog::generateThumbnail(const QStringList &paths)
{
    QPixmap bp(":/dialogs/images/resources/images/album_bg_normal.png");
    bp.scaled(64, 64);

    if (paths.length() <1)
        return bp;

    QPainter pp(&bp);

    QRect thumbRect(6, 6, 46, 46);
    using namespace utils::image;
    QPixmap thumb = cutSquareImage(getThumbnail(paths[0]),
                                   thumbRect.size());
    pp.drawPixmap(thumbRect, thumb);
    if (! thumb.isNull()) {
        QPainterPath path;
        path.addRect(thumbRect);
        pp.setPen(QPen(QColor(0, 0, 0, 0.1 * 255), 1));
        pp.drawPath(path);
    }

    return bp;
}
