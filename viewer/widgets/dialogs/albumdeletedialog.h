#ifndef ALBUMDELETEDIALOG_H
#define ALBUMDELETEDIALOG_H

#include "ddialog.h"

DWIDGET_USE_NAMESPACE

class AlbumDeleteDialog : public DDialog
{
    Q_OBJECT
public:
    explicit AlbumDeleteDialog(const QStringList &paths, QWidget *parent = 0);

private:
    QPixmap generateThumbnail(const QStringList &paths);
};

#endif // ALBUMDELETEDIALOG_H
