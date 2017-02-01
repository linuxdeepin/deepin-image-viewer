#ifndef DIALOG_H
#define DIALOG_H

#include <ddialog.h>

DWIDGET_USE_NAMESPACE

class Dialog : public DDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = 0);
    void showInCenter(QWidget *w);
};

#endif // DIALOG_H
