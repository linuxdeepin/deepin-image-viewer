#ifndef SCANPATHSITEM_H
#define SCANPATHSITEM_H

#include "utils/imageutils.h"
#include <QFrame>
#include <QThread>

class QHBoxLayout;
class QLabel;
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
    void initLeftIcon();
    void initMiddleContent();
    void initRemoveIcon();

    void updateCount();

    bool dirExist() const;
    bool mountDeviceExist() const;
    bool onMountDevice() const;

private:
    int m_countTID;
    QString m_path;
    QLabel *m_dirLabel;
    QLabel *m_pathLabel;
    QLabel *m_countLabel;
    QHBoxLayout *m_mainLayout;
};

// CountingThread
class CountingThread : public QThread {
    Q_OBJECT
public:
    CountingThread(const QString &path)
        :QThread()
        ,m_path(path) {}

    void run() Q_DECL_OVERRIDE {
        int count = utils::image::getImagesInfo(m_path, true).length();
        emit ready(QString::number(count) + " " + tr("Images"));
    }

signals:
    void ready(const QString &text);

private:
    QString m_path;
};

#endif // SCANPATHSITEM_H
