#ifndef DIRIMPORTDIALOG_H
#define DIRIMPORTDIALOG_H

#include "dialog.h"

class DirImportDialog : public Dialog
{
    Q_OBJECT
public:
    explicit DirImportDialog(const QString &dir, const QString &album, QWidget *parent = 0);

signals:
    void albumCreated();

protected:
    void keyPressEvent(QKeyEvent *e) override;
};

#endif // DIRIMPORTDIALOG_H
