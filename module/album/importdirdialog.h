#ifndef IMPORTDIRDIALOG_H
#define IMPORTDIRDIALOG_H

#include <ddialog.h>

DWIDGET_USE_NAMESPACE

class QLabel;
class QLineEdit;
class ImportDirDialog : public DDialog
{
    Q_OBJECT
public:
    explicit ImportDirDialog(QWidget *parent);
    void import(const QString &dir);
    void disableButton(const QString &name, bool disable);

signals:
    void albumCreated();

private:
    QLabel *m_icon;
    QLineEdit *m_edit;
    QString m_dir;

};

#endif // IMPORTDIRDIALOG_H
