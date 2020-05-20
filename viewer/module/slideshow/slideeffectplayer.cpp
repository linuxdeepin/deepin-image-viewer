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
#include "slideeffectplayer.h"
#include "application.h"
#include "controller/configsetter.h"
#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QTimerEvent>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QScreen>
#include <DIconButton>

namespace {

const QString DURATION_SETTING_GROUP = "SLIDESHOWDURATION";
const QString DURATION_SETTING_KEY = "Duration";
const int ANIMATION_DURATION  = 1000;
const int SLIDER_DURATION  = 3000;
const int ANIMATION_DURATION_4K  = 2300;
const int SLIDER_DURATION_4K  = 7000;

//const int ANIMATION_DURATION  = 2500;
//const int SLIDER_DURATION  = 7000;

} // namespace

SlideEffectPlayer::SlideEffectPlayer(QObject *parent)
    : QObject(parent)
{
//    QDesktopWidget *desktopWidget = QApplication::desktop();
//    m_screenrect = desktopWidget->screenGeometry();
    QScreen *screen = QGuiApplication::primaryScreen ();
    QRect m_screenrect = QRect(0, 0, 0, 0);
    qreal m_ratio = 1;
    m_screenrect = screen->availableGeometry() ;
    m_ratio = screen->devicePixelRatio();
//    qDebug() << "-----------m_screenrect:" << m_screenrect << " devicePixelRatio:" << screen->devicePixelRatio();
    if ((((qreal)m_screenrect.width())*m_ratio) > 3000 || (((qreal)m_screenrect.height())*m_ratio) > 3000) {
        b_4k = true;
    }
    connect(dApp->signalM, &SignalManager::updateButton, this, [ = ] {
        killTimer(m_tid);
        m_tid = 0;
    });
    connect(dApp->signalM, &SignalManager::sigStartTimer, this, [ = ] {
        if (!b_4k)
            m_tid = startTimer(SLIDER_DURATION);
        else
            m_tid = startTimer(SLIDER_DURATION_4K);
    });

}

SlideEffectPlayer::~SlideEffectPlayer()
{
    if (m_thread.isRunning()) {
        m_thread.quit();
    }

    if (m_effect) {
        delete m_effect;
        m_effect = nullptr;
    }
}

void SlideEffectPlayer::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != m_tid || m_pausing)
        return;
//    if(m_current == m_paths.size()-1){
//        emit dApp->signalM->updateButton();
//        emit dApp->signalM->updatePauseButton();
//    }

    if (bneedupdatepausebutton) {
        //emit dApp->signalM->updateButton();
       // emit dApp->signalM->updatePauseButton();
        //bneedupdatepausebutton = false;
        //return;
        //m_current = 0;
        bneedupdatepausebutton =false;
    }
    if (! startNext()) {
        stop();
    }
    if (bfirstrun) {
        killTimer(m_tid);
        bfirstrun = false;
        if (!b_4k)
            m_tid = startTimer(SLIDER_DURATION);
        else
            m_tid = startTimer(SLIDER_DURATION_4K);
    }
}

int SlideEffectPlayer::duration() const
{
    return dApp->setter->value(DURATION_SETTING_GROUP,
                               DURATION_SETTING_KEY).toInt() * 1000;
}

void SlideEffectPlayer::setFrameSize(int width, int height)
{
    m_w = width;
    m_h = height;
}

void SlideEffectPlayer::setImagePaths(const QStringList &paths)
{
    m_paths = paths;
    m_current = 0;
}


void SlideEffectPlayer::setCurrentImage(const QString &path)
{
    for (int i = 0; i < m_paths.length(); i++) {
        if (path == m_paths[i]) {
            m_current = i;
        }
    }
}


int SlideEffectPlayer::currentImageIndex() const
{
    return m_current;
}

QStringList SlideEffectPlayer::GetPathList()
{
    return m_paths;
}

QString SlideEffectPlayer::currentImagePath() const
{
    return m_paths[m_current];
}

bool SlideEffectPlayer::isRunning() const
{
    return m_running;
}

void SlideEffectPlayer::start()
{
    if (m_paths.isEmpty())
        return;

    bfirstrun = true;
    cacheNext();
    cachePrevious();
    m_running = true;
    if (!b_4k)
        m_tid = startTimer(ANIMATION_DURATION );
//        m_tid = startTimer(2 );
    else
        m_tid = startTimer(ANIMATION_DURATION_4K );
//        m_tid = startTimer(2 );
}

void SlideEffectPlayer::pause()
{
    if (m_effect) {
        m_pausing = !m_pausing;
        m_effect->pause();
    }
}


bool SlideEffectPlayer::startNext()
{
    qDebug() << "SlideEffectPlayer::startNext()";
    if (m_paths.isEmpty())
        return false;
    QSize fSize(m_w, m_h);
    if (! fSize.isValid()) {
        qWarning() << "Invalid frame size!";
        return false;
    }

    if (1 == m_paths.length()) {
        return false;
    }
//    if(m_paths.size() > 99)
//    {
//        if(m_current == m_paths.size() -2 || m_current == m_paths.size() -1)
//            emit sigLoadslideshowpathlst(false);
//    }
    int current = m_current + 1;
    if (current == m_paths.length()) {
        //emit sigLoadslideshowpathlst(false);
        current = 0;
    }

    if (m_cacheImages.value(m_paths[current]).isNull()) {
        //return false;
    }

    if (m_effect)
        m_effect->deleteLater();
    QString oldPath,newPath;
    oldPath = m_paths[m_current];
    if(m_current+1<m_paths.size())
        newPath = m_paths[m_current+1];
    if (m_paths.length() > 1) {
        m_current++;
        if (m_current == m_paths.length()) {
            m_current = 0;
        }
    }
    QImage oldImg = m_cacheImages.value(oldPath);
//    static bool bfirst = true;
//    if (bfirst) {
    cacheNext();
//        bfirst = false;
//        QEventLoop loop;
//        QTimer::singleShot(3000, &loop, SLOT(quit()));
//        loop.exec();
//    }

    if(bLoopPlayback && m_current == 1)
    {
        oldPath = LoopPlayoldpath;
        bLoopPlayback = false;
        m_current--;
        LoopPlayoldpath = "";
        newPath = m_paths[m_current];
    }
   // if(oldImg.isNull())
        oldImg = utils::image::getRotatedImage(oldPath);
    if(newPath.isEmpty())
        newPath = m_paths[m_current];
    m_effect = SlideEffect::create("");
//    m_effect = SlideEffect::create("enter_from_right");
//    if ((m_screenrect.width()*m_ratio) < 3000 && (m_screenrect.height()*m_ratio) < 3000) {

    //maozhengyu 点击下一张图片加载延时
    if(!bstartnext){
        if (!b_4k) {
            m_effect->setDuration(ANIMATION_DURATION);
            m_effect->setAllMs(SLIDER_DURATION);
        } else {
    //        qDebug() << "------------------4K";
            m_effect->setDuration(ANIMATION_DURATION_4K);
            m_effect->setAllMs(SLIDER_DURATION_4K);
        }
    }else{
        if (!b_4k) {
            m_effect->setDuration(ANIMATION_DURATION);
    //        m_effect->setAllMs(SLIDER_DURATION);
            m_effect->setAllMs(2200);
        } else {
    //        qDebug() << "------------------4K";
            m_effect->setDuration(ANIMATION_DURATION_4K);
    //        m_effect->setAllMs(SLIDER_DURATION_4K);
            m_effect->setAllMs(3500);
        }
        bstartnext = false;
    }
    m_effect->setSize(fSize);

    using namespace utils::image;

    QImage newImg = m_cacheImages.value(newPath);
    if(newImg.isNull())
        newImg = utils::image::getRotatedImage(newPath);
// The "newPath" would be the next "oldPath", so there is no need to remove it now
//    m_cacheImages.remove(oldPath);
    m_effect->setImages(oldImg, newImg);
    if (!m_thread.isRunning())
        m_thread.start();

    m_effect->moveToThread(&m_thread);
    connect(m_effect, &SlideEffect::frameReady, this, [ = ] (const QImage & img) {
        if (m_running) {
            Q_EMIT frameReady(img);
        }
    }, Qt::DirectConnection);
    QMetaObject::invokeMethod(m_effect, "start");


    if (m_current == m_paths.length() - 1) {
//        emit dApp->signalM->updateButton();
//        emit dApp->signalM->updatePauseButton();
        bneedupdatepausebutton = true;
    }

    return true;
}

bool SlideEffectPlayer::startPrevious()
{
    if (m_paths.isEmpty()) {
        return false;
    }

    QSize fSize(m_w, m_h);
    if (! fSize.isValid()) {
        return false;
    }

    if (1 == m_paths.length()) {
        return false;
    }

    int current = m_current - 1;
    if (current == -1) {
        emit dApp->signalM->sendLoadSignal(true);
        if (m_current == -1) {
            current = m_paths.length() - 1;
        }
    }

    //if (m_cacheImages.value(m_paths[current]).isNull()) {
        //return false;
   // }

    if (m_effect)
        m_effect->deleteLater();

    const QString oldPath = m_paths[m_current];

    if (m_paths.length() > 1) {
        m_current --;
        if (m_current == -1) {
            m_current = m_paths.length() - 1;
        }
    }

    cachePrevious();

    QString newPath = m_paths[m_current];
    m_effect = SlideEffect::create("enter_from_left");
    if (!b_4k) {
        m_effect->setDuration(ANIMATION_DURATION);
//        m_effect->setAllMs(SLIDER_DURATION);

        m_effect->setAllMs(2200);
    } else {
        qDebug() << "------------------4K";
        m_effect->setDuration(ANIMATION_DURATION_4K);
//        m_effect->setAllMs(SLIDER_DURATION_4K);

        m_effect->setAllMs(3500);
    }
    m_effect->setSize(fSize);

    using namespace utils::image;
    qDebug() << "m_cacheImages.value";
   // QImage oldImg = m_cacheImages.value(oldPath);
    QImage oldImg = utils::image::getRotatedImage(oldPath);
   // QImage newImg = m_cacheImages.value(newPath);
    QImage newImg = utils::image::getRotatedImage(newPath);
    if(newImg.isNull())
    {
        if(m_current>0)
            newPath = m_paths[m_current];
        else
        {
            m_current = m_paths.length() - 1;
        }
        newImg = utils::image::getRotatedImage(newPath);
    }
    // The "newPath" would be the next "oldPath", so there is no need to remove it now

//    m_cacheImages.remove(oldPath);

    m_effect->setImages(oldImg, newImg);
    if (!m_thread.isRunning())
        m_thread.start();

    m_effect->moveToThread(&m_thread);
    connect(m_effect, &SlideEffect::frameReady, this, [ = ] (const QImage & img) {
        if (m_running) {
            Q_EMIT frameReady(img);
        }
    }, Qt::DirectConnection);
    QMetaObject::invokeMethod(m_effect, "start");
    return true;
}

void SlideEffectPlayer::cacheNext()
{
    qDebug() << "SlideEffectPlayer::cacheNext()";
    int current = m_current;
    current ++;
    //Load the following pictures and thumbnails when playing the last thumbnail
    if(current == m_paths.length())
    {
        emit sigLoadslideshowpathlst(false);
        //Load front pictures when playing the last file
        if (current == m_paths.length()) {
            if (bfirstrun) {
                current = m_paths.length() - 1;
                bneedupdatepausebutton = true;
                current = 0;
            } else {
                current = 0;
            }
            LoopPlayoldpath = m_paths[m_current];
            bLoopPlayback = true;
            connect(dApp->signalM,&SignalManager::sigNoneedLoadfrontslideshow,this,[=]{
                bLoopPlayback = false;
            });
            emit dApp->signalM->sigLoadfrontSlideshow();
        }
    }
    QString path = m_paths[current];
    //Load current pixmap for fix bug 21480
    if(current != 0 )
    {
        QString curpath = m_paths[current-1];
        if (m_cacheImages.value(curpath).isNull())
        {
            //QImage img = utils::image::getRotatedImage(curpath);
            //m_cacheImages.insert(curpath, img);
        }
    }
    if(current == 0)
    {
        QString curpath = m_paths[0];
        if (m_cacheImages.value(curpath).isNull())
        {
            //QImage img = utils::image::getRotatedImage(curpath);
            //m_cacheImages.insert(curpath, img);
        }
        if(m_paths.size() >1)
        {
            QString curpath = m_paths[1];
            if (m_cacheImages.value(curpath).isNull())
            {
                //QImage img = utils::image::getRotatedImage(curpath);
                //m_cacheImages.insert(curpath, img);
            }
        }
    }
    if (m_cacheImages.value(path).isNull()) {
        CacheThread *t = new CacheThread(path);
        connect(t, &CacheThread::cached,
        this, [ = ] (const QString path, const QImage img) {
//            if(m_paths.size()>3)
//            {
//                int rmindex = current-3;
//                if(rmindex == -1)
//                {
//                    rmindex = m_paths.size()-1;
//                }else if(rmindex == -2)
//                {
//                    rmindex = m_paths.size()-2;
//                }else if (rmindex == -3) {
//                    rmindex = m_paths.size()-3;
//                }
//                QString rmpath = m_paths[rmindex];
//                m_cacheImages.remove(rmpath);
//            }
//            m_cacheImages.insert(path, img);

        });
        connect(t, &CacheThread::finished, t, &CacheThread::deleteLater);
        t->start();
    }
}

void SlideEffectPlayer::cachePrevious()
{
    qDebug() << "SlideEffectPlayer::cachePrevious()";
    int current = m_current;
    current--;
    if (-1 == current) {
        //emit dApp->signalM->sendLoadSignal(false);
        current = m_paths.length() - 1;
    }

    QString path = m_paths[current];

    if (m_cacheImages.value(path).isNull()) {
        CacheThread *t = new CacheThread(path);
        connect(t, &CacheThread::cached,
        this, [ = ] (const QString path, const QImage img) {
            qDebug() << "m_cacheImagespre.insert(path, img)";
            //m_cacheImages.insert(path, img);
        });
        connect(t, &CacheThread::finished, t, &CacheThread::deleteLater);
        t->start();
    }
}

void SlideEffectPlayer::setStartNextFlag(bool flag)
{
    bstartnext = flag;
}

void SlideEffectPlayer::stop()
{
    if (!isRunning())
        return;
    if (m_pausing) {
        m_pausing = !m_pausing;
        m_effect->pause();
    }

    killTimer(m_tid);
    m_tid = 0;
    m_running = false;
    m_cacheImages.clear();
    Q_EMIT finished();
}
