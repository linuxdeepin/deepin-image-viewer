#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H
#include "controller/viewerthememanager.h"

#include <DDialog>
#include <DWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <DLineEdit>
#include <DPushButton>
#include <DSuggestButton>
#include <DLabel>
DWIDGET_USE_NAMESPACE
class RenameDialog : public DDialog
{
    Q_OBJECT
public:
    RenameDialog(QString filename,QWidget *parent = nullptr);
    void onThemeChanged(ViewerThemeManager::AppTheme theme);
    DLineEdit *m_lineedt;
    DSuggestButton *okbtn;
    DPushButton *cancelbtn;
    QString GetFilePath();
    QString GetFileName();
    void InitDlg();
private:
    QVBoxLayout *m_vlayout;
    QHBoxLayout *m_hlayout;
    QHBoxLayout *m_edtlayout;
    DLabel *m_labformat;
    QString m_filenamepath;
    QString m_filename;
    QString m_DirPath;
    QString m_basename;
};

#endif // RENAMEDIALOG_H
