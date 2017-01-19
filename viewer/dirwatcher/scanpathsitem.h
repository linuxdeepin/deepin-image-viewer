#ifndef SCANPATHSITEM_H
#define SCANPATHSITEM_H

#include "utils/imageutils.h"
#include <QFrame>
#include <QThread>

class QHBoxLayout;
class QLabel;

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
    QLabel *m_dirLabel;
    QLabel *m_pathLabel;
    QLabel *m_countLabel;
    QHBoxLayout *m_mainLayout;
};

#endif // SCANPATHSITEM_H
