#include "dialog.h"

Dialog::Dialog(QWidget *parent) : DDialog(parent)
{

}

void Dialog::showInCenter(QWidget *w)
{
    show();

    QPoint gp = w->mapToGlobal(QPoint(0, 0));
    move((w->width() - this->width()) / 2 + gp.x(),
               (w->height() - this->sizeHint().height()) / 2 + gp.y());
}
