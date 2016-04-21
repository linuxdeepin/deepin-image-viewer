#pragma once
#include "slideeffect.h"
#include <QtCore/QThread>

class SlideEffectPlayer : public QObject
{
    Q_OBJECT
public:
    SlideEffectPlayer(QObject* parent = 0);
    void setFrameSize(int width, int height);
    QSize frameSize() const { return QSize(m_w, m_h);}
    // call setCurrentImage later
    void setImagePaths(const QStringList& paths);
    // invalid path: black image+1st image
    void setCurrentImage(const QString& path = QString());
    QString currentImagePath() const;
    QString nextImagePath() const;
    bool isRunning() const;

Q_SIGNALS:
    void frameReady(const QImage& image);
    void finished();
    void currentImageChanged(const QString& path);
    void stepChanged(int steps);

public Q_SLOTS:
    void start();
    void stop();

protected:
    void timerEvent(QTimerEvent *e);

private:
    bool startNext();

    bool m_running = false;
    bool m_random = true;
    SlideEffect *m_effect = NULL;
    int m_tid;
    int m_w, m_h;
    QThread m_thread;
    QStringList::ConstIterator m_current;
    QStringList m_paths;
};
