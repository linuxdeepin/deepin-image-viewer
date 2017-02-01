#ifndef FILEDELETEDIALOG_H
#define FILEDELETEDIALOG_H

#include "dialog.h"

class FileDeleteDialog : public Dialog
{
    Q_OBJECT
public:
    explicit FileDeleteDialog(const QStringList &paths, QWidget *parent = 0);

private:
    QPixmap generateThumbnail(const QStringList &paths);
};

#endif // FILEDELETEDIALOG_H
