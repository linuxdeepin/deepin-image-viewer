#ifndef CREATEALBUMDIALOG_H
#define CREATEALBUMDIALOG_H
#include "widgets/bluredialog.h"

class CreateAlbumDialog : public BlureDialog
{
public:
    explicit CreateAlbumDialog(QWidget *parent, QWidget *source);

protected:
    void keyPressEvent(QKeyEvent *e) override;
};

#endif // CREATEALBUMDIALOG_H
