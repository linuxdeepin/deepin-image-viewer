#ifndef CREATEALBUMDIALOG_H
#define CREATEALBUMDIALOG_H

#include "widgets/bluredialog.h"
#include <ddialog.h>

using namespace Dtk::Widget;
class CreateAlbumDialog : public DDialog
{
    Q_OBJECT
public:
    explicit CreateAlbumDialog(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    const QString getNewAlbumName() const;
    void createAlbum(const QString &newName);
};

#endif // CREATEALBUMDIALOG_H
