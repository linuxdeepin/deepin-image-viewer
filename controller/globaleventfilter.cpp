#include "globaleventfilter.h"
#include "utils/baseutils.h"
#include <QDebug>
#include <QEvent>

GlobalEventFilter::GlobalEventFilter(QObject *parent) : QObject(parent)
{

}

bool GlobalEventFilter::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
//    if (e->type() == QEvent::MouseMove || e->type() == QEvent::Wheel) {
//        QThreadPool::globalInstance()->setMaxThreadCount(1);
//        m_threadTimer->stop();
//        m_threadTimer->start();
//    }
    return false;
}
