#ifndef ALBUMDELETEDIALOG_H
#define ALBUMDELETEDIALOG_H

#include "dialog.h"

class AlbumDeleteDialog : public Dialog
{
    Q_OBJECT
public:
    explicit AlbumDeleteDialog(const QStringList &paths, QWidget *parent = 0);

private:
    QPixmap generateThumbnail(const QStringList &paths);
};

#endif // ALBUMDELETEDIALOG_H
