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

    QRect thumbRect(6, 6, 46, 46);
    using namespace utils::image;
    QPixmap thumb = cutSquareImage(getThumbnail(paths[1]),
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
