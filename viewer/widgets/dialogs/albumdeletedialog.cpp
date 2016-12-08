#include "albumdeletedialog.h"
#include "application.h"
#include "controller/dbmanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QPainter>
#include <QDebug>

AlbumDeleteDialog::AlbumDeleteDialog(const QStringList &paths, QWidget *parent)
    : DDialog(parent)
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

    if (paths.length() < 2)
        return bp;

    QPainter pp(&bp);

    QRect thumbRect(7, 7, 45, 45);
    using namespace utils::image;
    QPixmap thumb = cutSquareImage(getThumbnail(paths[1]),
                                   thumbRect.size());
    pp.drawPixmap(thumbRect, thumb);

    return bp;
}
