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
#pragma once
#include "utils/imageutils.h"
#include "slideeffect.h"
#include <QThread>
#include <QMap>
#include "application.h"
#include "controller/signalmanager.h"

class CacheThread : public QThread
{
    Q_OBJECT
public:
    CacheThread(const QString &path)
        : QThread(NULL)
        , m_path(path) {}

signals:
    void cached(QString, QImage);

protected:
    void run() Q_DECL_OVERRIDE {
        QImage img = utils::image::getRotatedImage(m_path);
//        QImage img = dApp->m_imagemap.value(m_path).toImage();
        qDebug() << "QImage img = utils::image::getRotatedImage(m_path)" << m_path;
        emit cached(m_path, img);
    }

private:
    QString m_path;
};

class SlideEffectPlayer : public QObject
{
    Q_OBJECT
public:
    SlideEffectPlayer(QObject *parent = 0);
    ~SlideEffectPlayer();
    void setFrameSize(int width, int height);
    QSize frameSize() const
    {
        return QSize(m_w, m_h);
    }
    // call setCurrentImage later
    void setImagePaths(const QStringList &paths);
    // invalid path: black image+1st image
    void setCurrentImage(const QString &path = QString());
    QString currentImagePath() const;
    bool isRunning() const;
    int currentImageIndex() const;
    QStringList GetPathList();
Q_SIGNALS:
    void frameReady(const QImage &image);
    void finished();
    void currentImageChanged(const QString &path);
    void stepChanged(int steps);
    void updateButton();
    void sigLoadslideshowpathlst(bool bflag);
public Q_SLOTS:
    void start();
    void stop();
    void pause();

protected:
    void timerEvent(QTimerEvent *e);

public:
    int duration() const;
    bool startNext();
    void cacheNext();
    bool startPrevious();
    void cachePrevious();
    void setStartNextFlag(bool flag);

private:
    bool m_running = false;
    bool m_pausing = false;
    bool m_random = true;
    int m_tid;
    int m_newtid;
    int m_w, m_h;
    QMap<QString, QImage> m_cacheImages;
    QStringList m_paths;
//    QStringList::ConstIterator m_current;
    int m_current;

    QThread m_thread;
    SlideEffect *m_effect = NULL;
    bool b_4k = false;
    bool bfirstrun = true;
    bool bneedupdatepausebutton = false;

    //maozhengyu 点击下一张标志
    bool bstartnext = false;
};
