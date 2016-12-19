#include "cancelimportdialog.h"
#include "application.h"

CancelImportDialog::CancelImportDialog(QWidget *parent)
    : DDialog(parent)
{
    setIconPixmap(QPixmap(":/dialogs/images/resources/images/warning.png"));
    setTitle(tr("Are you sure to close?"));
    setMessage(tr("The close operation will terminate current import task"));

    addButton(tr("Cancel"), true);
    addButton(tr("Close"), false, ButtonWarning);
    connect(this, &CancelImportDialog::buttonClicked, this, [=] (int index) {
        if (index == 1) {
            dApp->quit();
        }
    });
}
