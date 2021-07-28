/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     LiuMingHang <liuminghang@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

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
    RenameDialog(const QString &filename, QWidget *parent = nullptr);
//    void onThemeChanged(ViewerThemeManager::AppTheme theme);
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
