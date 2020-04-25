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
#ifndef SCANPATHSITEM_H
#define SCANPATHSITEM_H

#include "utils/imageutils.h"
#include <QFrame>
#include <QThread>
#include <DLabel>

class QHBoxLayout;

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;

// CountingThread
class CountingThread : public QThread {
    Q_OBJECT
public:
    CountingThread(const QString &path);

    void run() Q_DECL_OVERRIDE;

signals:
    void ready(const QString &text);

private:
    QString m_path;
};

class ScanPathsItem : public QFrame {
    Q_OBJECT
public:
    ScanPathsItem(const QString &path);

protected:
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE {
        QFrame::enterEvent(e);
        emit showRemoveIconChanged(true);
    }
    void leaveEvent(QEvent *e) Q_DECL_OVERRIDE {
        QFrame::leaveEvent(e);
        emit showRemoveIconChanged(false);
    }

signals:
    void remove(QString path);
    void requestUpdateCount();
    void showRemoveIconChanged(bool show);

protected:
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;

private:
    void initCountThread();
    void initLeftIcon();
    void initMiddleContent();
    void initRemoveIcon();

    void updateCount();

    bool dirExist() const;

private:
    int m_countTID;
    CountingThread *m_thread = nullptr;
    QString m_path;
    QLbtoDLabel *m_dirLabel;
    QLbtoDLabel *m_pathLabel;
    QLbtoDLabel *m_countLabel;
    QHBoxLayout *m_mainLayout;
};

#endif // SCANPATHSITEM_H
