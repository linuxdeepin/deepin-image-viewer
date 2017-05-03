#include "filedeletedialog.h"
#include "application.h"
#include "controller/dbmanager.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"
#include <QPainter>
#include <QDebug>

FileDeleteDialog::FileDeleteDialog(const QStringList &paths, QWidget *parent)
    : Dialog(parent)
{
    setModal(true);

    setIconPixmap(generateThumbnail(paths));
    setTitle(tr("Are you sure to throw pictures to TRASH?"));

    addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    addButton(tr("Delete"), true, DDialog::ButtonWarning);

    connect(this, &FileDeleteDialog::buttonClicked, this, [=] (int index) {
        if (index == 1) {
            qDebug() << "Delete files: " << paths.length();
            DBManager::instance()->removeImgInfos(paths);
            utils::base::trashFiles(paths);
        }
    });
    connect(this, &FileDeleteDialog::closed,
            this, &FileDeleteDialog::deleteLater);
}

QPixmap FileDeleteDialog::generateThumbnail(const QStringList &paths)
{
    QPixmap bp;
    QRect thumbRect;
    if (paths.length() > 1) {
        bp = QPixmap(":/dialogs/images/resources/images/del_multi_img.png");
        thumbRect = QRect(5, 14, 54, 40);
    }
    else {
        bp = QPixmap(":/dialogs/images/resources/images/del_single_img.png");
        thumbRect = QRect(5, 12, 54, 40);
    }

    if (paths.isEmpty())
        return bp;

    QPainter pp(&bp);

    using namespace utils::image;
    QPixmap thumb = cutSquareImage(getThumbnail(paths.first()),
                                   thumbRect.size());
    pp.drawPixmap(thumbRect, thumb);
    QPainterPath path;
    path.addRect(thumbRect);
    pp.setPen(QPen(QColor(0, 0, 0, 0.1 * 255), 1));
    pp.drawPath(path);

    return bp;
}
