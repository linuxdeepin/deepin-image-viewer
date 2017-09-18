/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
#include "globaleventfilter.h"
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
