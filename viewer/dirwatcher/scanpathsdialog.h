/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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

#ifndef SCANPATHSDIALOG_H
#define SCANPATHSDIALOG_H

#include <QFrame>
#include <DMainWindow>
#include <DLabel>
#include <DStackedWidget>
#include <DWidget>

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;
typedef DStackedWidget QSWToDStackedWidget;
typedef DWidget QWdToDWidget;

class QVBoxLayout;
class ScanPathsDialog : public DMainWindow
{
    Q_OBJECT
public:
    static ScanPathsDialog *instance();
    bool addPath(const QString &path, bool check=true);
    void show();

signals:
    void requestUpdateCount();

protected:
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;

private slots:
    void showSelectDialog();

private:
    explicit ScanPathsDialog(QWidget *parent = 0);
    void removePath(const QString &path);
    void initTitle();
    void initPathsArea();
    void initMessageLabel();
    void initAddButton();
    void initSinglaFileWatcher();

    void showMessage(const QString &message);

    bool isLegalPath(const QString &path) const;
    bool isContainByScanPaths(const QString &path) const;
    bool isSubPathOfScanPaths(const QString &path) const;

    QStringList scanpaths() const;
    void addToScanPaths(const QString &path);
    void removeFromScanPaths(const QString &path);

private:
    static ScanPathsDialog *m_dialog;
    int m_messageTID;
    QLbtoDLabel *m_messageLabel;
    QVBoxLayout *m_mainLayout;
    QVBoxLayout *m_pathsLayout;
    QSWToDStackedWidget *m_contentStack;
};

#endif // SCANPATHSDIALOG_H
