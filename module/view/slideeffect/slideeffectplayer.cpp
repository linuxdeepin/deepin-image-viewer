#include "slideeffectplayer.h"
#include <QtCore/QTimerEvent>
#include <QtDebug>

SlideEffectPlayer::SlideEffectPlayer(QObject *parent)
    : QObject(parent)
{

}

void SlideEffectPlayer::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != m_tid)
        return;
    if (m_effect)
        m_effect->deleteLater();
    if (!startNext()) {
        stop();
    }
}

void SlideEffectPlayer::setFrameSize(int width, int height)
{
    m_w = width;
    m_h = height;
}

void SlideEffectPlayer::setImagePaths(const QStringList &paths)
{
    m_paths = paths;
    m_current = m_paths.constEnd();
}

void SlideEffectPlayer::setCurrentImage(const QString &path)
{
    m_current = std::find(m_paths.cbegin(), m_paths.cend(),  path);
}

QString SlideEffectPlayer::currentImagePath() const
{
    if (m_current == m_paths.constEnd())
        return QString();
    return *m_current;
}

QString SlideEffectPlayer::nextImagePath() const
{
    if (m_current == m_paths.constEnd())
        return *m_paths.constBegin();
    if (m_current+1 == m_paths.constEnd())
        return QString();
    return *(m_current+1);
}

bool SlideEffectPlayer::isRunning() const
{
    return m_running;
}

void SlideEffectPlayer::start()
{
    if (!startNext())
        return;
    m_running = true;
    m_tid = startTimer(4000);
}

bool SlideEffectPlayer::startNext()
{
    if (m_paths.isEmpty())
        return false;
    if (m_current+1 == m_paths.constEnd()) {
        return false;
    }
    m_effect = SlideEffect::create();
    m_effect->setSize(QSize(m_w, m_h));
    m_effect->setImages(currentImagePath(), nextImagePath());
    qDebug() << currentImagePath() << nextImagePath();
    ++m_current;
    Q_EMIT stepChanged(1);
    Q_EMIT currentImageChanged(m_current == m_paths.constEnd() ? QString() : *m_current);
    if (!m_thread.isRunning())
        m_thread.start();

    m_effect->moveToThread(&m_thread);
    connect(m_effect, &SlideEffect::frameReady, this, &SlideEffectPlayer::frameReady, Qt::DirectConnection);
    QMetaObject::invokeMethod(m_effect, "start");
    return true;
}

void SlideEffectPlayer::stop()
{
    if (!isRunning())
        return;
    killTimer(m_tid);
    m_tid = 0;
    m_running = false;
    Q_EMIT finished();
}
