#include "renamedialog.h"
#include "application.h"
#include "utils/baseutils.h"
#include "utils/imageutils.h"

#include <QLabel>
#include <DMessageBox>
RenameDialog::RenameDialog(QString filename,QWidget *parent)
    :DDialog (parent)
    ,m_filenamepath(filename)
{
    this->setIcon(QIcon::fromTheme("deepin-image-viewer"));

    setFixedSize(380,180);
    DWidget *widet = new DWidget(this);
    addContent(widet);
    m_vlayout = new QVBoxLayout(widet);
    m_hlayout = new QHBoxLayout();
    m_edtlayout = new QHBoxLayout();
    m_lineedt = new DLineEdit(widet);
    QFrame *line = new QFrame(widet);
    QLabel *labtitle = new QLabel();
    okbtn = new DSuggestButton(widet);
    cancelbtn = new DPushButton(widet);
    m_labformat = new QLabel(widet);
    m_vlayout->setContentsMargins(2, 0, 2, 1);
    okbtn->setText(tr("Confirm"));
    cancelbtn->setText(tr("Cancel"));
    m_hlayout->addWidget(cancelbtn);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    m_hlayout->addWidget(line);
    m_hlayout->addWidget(okbtn);
    labtitle->setText(tr("Input a new name"));
    labtitle->setAlignment(Qt::AlignCenter);
    m_vlayout->addWidget(labtitle);
    m_vlayout->addStretch();
    m_edtlayout->addWidget(m_lineedt);
    m_labformat->setEnabled(false);
    m_edtlayout->addWidget(m_labformat);
    m_vlayout->addLayout(m_edtlayout);
    m_vlayout->addStretch();
    m_vlayout->addLayout(m_hlayout);
    widet->setLayout(m_vlayout);
    InitDlg();

    m_lineedt->lineEdit()->setFocus();
    int Dirlen = m_DirPath.size() + 1 + m_labformat->text().size();
    m_lineedt->lineEdit()->setMaxLength(255-Dirlen);
    connect(m_lineedt,&DLineEdit::textChanged,this,[=](const QString& text){
        if(text.isEmpty())
            okbtn->setEnabled(false);
        else {
            okbtn->setEnabled(true);
        }
    });
    connect(okbtn,&DSuggestButton::clicked,this,[=]{
        m_filename = m_lineedt->text() + m_labformat->text();
        m_filenamepath = m_DirPath + "/" + m_filename;
        accept();
    });
    connect(cancelbtn,&DPushButton::clicked,this,[=]{
        reject();
    });
}


QString RenameDialog::GetFilePath()
{
    return m_filenamepath;
}

QString RenameDialog::GetFileName()
{
    return m_filename;
}

void RenameDialog::InitDlg()
{
    QFileInfo fileinfo(m_filenamepath);
    m_DirPath = fileinfo.path();
    m_filename = fileinfo.fileName();
    QString format = fileinfo.suffix();
    QString basename;
    basename = fileinfo.baseName();
    m_lineedt->setText(basename);
    m_labformat->setText("." + format);
}
