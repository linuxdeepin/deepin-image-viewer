#ifndef FILEDELETEDIALOG_H
#define FILEDELETEDIALOG_H

#include <ddialog.h>

DWIDGET_USE_NAMESPACE

class FileDeleteDialog : public DDialog
{
    Q_OBJECT
public:
    explicit FileDeleteDialog(const QStringList &paths, QWidget *parent = 0);

private:
    QPixmap generateThumbnail(const QStringList &paths);
};

#endif // FILEDELETEDIALOG_H
