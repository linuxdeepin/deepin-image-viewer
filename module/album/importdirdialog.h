#ifndef IMPORTDIRDIALOG_H
#define IMPORTDIRDIALOG_H
#include "widgets/bluredialog.h"

class QLabel;
class QLineEdit;
class ImportDirDialog : public BlureDialog
{
    Q_OBJECT
public:
    explicit ImportDirDialog(QWidget *parent, QWidget *source);
    void import(const QString &dir);

signals:
    void albumCreated();

private:
    QLabel *m_icon;
    QLineEdit *m_edit;
    QString m_dir;
};

#endif // IMPORTDIRDIALOG_H
