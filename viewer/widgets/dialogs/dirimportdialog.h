#ifndef DIRIMPORTDIALOG_H
#define DIRIMPORTDIALOG_H

#include <ddialog.h>

DWIDGET_USE_NAMESPACE

class DirImportDialog : public DDialog
{
    Q_OBJECT
public:
    explicit DirImportDialog(const QString &dir, QWidget *parent = 0);

signals:
    void albumCreated();

protected:
    void keyPressEvent(QKeyEvent *e) override;
};

#endif // DIRIMPORTDIALOG_H
