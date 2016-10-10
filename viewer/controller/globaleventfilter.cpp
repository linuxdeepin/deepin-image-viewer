#include "globaleventfilter.h"
#include "controller/signalmanager.h"
#include "utils/baseutils.h"
#include <QDebug>
#include <QEvent>
#include <QWindowStateChangeEvent>

GlobalEventFilter::GlobalEventFilter(QObject *parent) : QObject(parent)
{

}

bool GlobalEventFilter::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent* event = static_cast<QKeyEvent*>(e);
        if (event->key() == Qt::Key_Tab) {
            event->ignore();
            return true;
        }
    }
//    if (e->type() != QEvent::Paint &&
//            e->type() != QEvent::MetaCall &&
//            e->type() != QEvent::UpdateRequest &&
//            e->type() != QEvent::MouseMove &&
//            e->type() != QEvent::ChildAdded &&
//            e->type() != QEvent::ChildRemoved &&
//            e->type() != QEvent::LayoutRequest &&
//            e->type() != QEvent::Move &&
//            e->type() != QEvent::Timer &&
//            e->type() != QEvent::FutureCallOut &&
//            e->type() != QEvent::DeferredDelete &&
//            e->type() != QEvent::Resize) {

//        qDebug() << e->type();
//    }
//    if (e->type() == QEvent::WindowStateChange) {
//        if (QWindowStateChangeEvent *we = static_cast<QWindowStateChangeEvent *>(e)) {
//            emit SignalManager::instance()->windowStatesChanged(we->oldState());
//        }
//    }
    return false;
}
