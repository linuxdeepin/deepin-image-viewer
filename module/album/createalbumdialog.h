#ifndef CREATEALBUMDIALOG_H
#define CREATEALBUMDIALOG_H

#include "widgets/bluredialog.h"

class CreateAlbumDialog : public BlureDialog
{
    Q_OBJECT
public:
    explicit CreateAlbumDialog(QWidget *parent, QWidget *source);

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    const QString getNewAlbumName() const;
    void createAlbum(const QString &newName);
};

#endif // CREATEALBUMDIALOG_H
