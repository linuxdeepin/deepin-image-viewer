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
#include "slideeffect.h"
#include "application.h"
#include "controller/configsetter.h"
#include "utils/imageutils.h"
#include <QPainter>
#include <QtCore/QTimerEvent>
#include <malloc.h>
#include <QDebug>

namespace {

const QString EFFECT_SETTING_GROUP = "SLIDESHOWEFFECT";

} // namespace

QHash<EffectId, std::function<SlideEffect*()> > SlideEffect::effects;


ThreadRenderFrame::ThreadRenderFrame()
    :m_data(SlideEffectThreadData())
{
    setAutoDelete(true);
}

ThreadRenderFrame::~ThreadRenderFrame()
{
   //清空
   m_data.mimage=QImage();
   m_data.next_image=QImage();
   m_data.current_image=QImage();
   m_data.current_region=QRegion();
   m_data.next_region=QRegion();
   m_data.current_rect=QRect();
   m_data.next_rect=QRect();

}

void ThreadRenderFrame::stop()
{
    bstop = true;
}

void ThreadRenderFrame::setData(SlideEffectThreadData &data)
{
//    qDebug() << "ThreadRenderImage::setPage" << width << height;
    m_data = data;
}

void ThreadRenderFrame::run()
{
    if (bstop)
        return;
    SlideEffectThreadData mdata = m_data;
//    qDebug() << "-------renderFrame start num:" << mdata.num;
    QImage image = mdata.mimage;
    QPainter p(&image);
    image.fill(Qt::transparent);

    if (bstop){
        return;
    }
    p.setClipRegion(mdata.current_region);
    p.drawImage(QRect(0, 0, mdata.width, mdata.height), /**current_image*/mdata.current_image, mdata.current_rect);

    if (bstop){
        return;
    }
    p.setClipRegion(mdata.next_region);
    p.drawImage(QRect(0, 0, mdata.width, mdata.height), /**next_image*/mdata.next_image, mdata.next_rect);
    if (bstop)
        return;
    emit signal_RenderFinish(mdata.num, image);

//    qDebug() << "-------renderFrame end num:" << mdata.num;
}



SlideEffect *SlideEffect::create(const EffectId &id)
{
    if (id == EffectId()) {
        srand(time(0));
        int count = 0; // To avoid find no match effect
        while (true || count < 100) {
            QList<std::function<SlideEffect*()>> cs = effects.values();
            const int idx = rand() % cs.size();
            std::function<SlideEffect*()> c = cs.at(idx);


            SlideEffect *e = c();
            // Check if effect should show
            if (dApp->setter->value(EFFECT_SETTING_GROUP,
                                    QString::number(e->effectName()),
                                    true).toBool()) {
                return e;
            }
            count ++;
        }
        return NULL;
    }
    if (effects.contains(id))
        return effects.value(id)();
    return NULL;
}

void SlideEffect::Register(EffectId id, std::function<SlideEffect*()> c)
{
    if (effects.contains(id)) {
        effects.remove(id);
    }

    effects.insert(id, c);
}

SlideEffect::SlideEffect()
    :paused(false)
    ,m_nNum(0)
    ,duration_ms(800)
    ,all_ms(3000)
    ,tid(0)
    ,mode(Qt::KeepAspectRatio)
    ,finished(false)
    ,progress_(0.0)
    ,speed(1.0)
    ,current_frame(0)
    ,frames_total(17)
    ,width(0)
    ,height(0)
    ,color(Qt::transparent)
    ,easing_(QEasingCurve(QEasingCurve::OutBack))
{
    QThreadPool::globalInstance()->setMaxThreadCount(3);
//    m.setMaxThreadCount(20);
//    connect(this, &SlideEffect::renderFrameFinish, this, &SlideEffect::slotrenderFrameFinish);
}

void SlideEffect::slotrenderFrameFinish(int num, QImage image)
{
    allImage[num] = image;
    if(image.isNull()){
        qDebug() << "image.isNull";
    }
    //定时器改线程同步,add by hujianbo
    if (num == m_nNum) {
        m_nNum = -1;
        int nCount = 0;
        QMap<int, QImage>::iterator iter = allImage.begin();
        while (iter != allImage.end()) {
            if(nCount > allImage.size()+100){
                break;//如果延迟500ms还没有结果，先退出，时间后续根据测试情况再确定
            }
            nCount ++;
            if (iter.value().isNull()){
                QThread::msleep(5);
                iter --;
            }
            iter ++;
        }
        tid = startTimer(1);
    }
}

SlideEffect::~SlideEffect()
{
//    qDebug() << "------------SlideEffect start release";
    disconnect(this, &SlideEffect::renderFrameFinish, this, &SlideEffect::slotrenderFrameFinish);

    if (current_image) {
        delete current_image;
        current_image = nullptr;
    }
    if (next_image) {
        delete next_image;
        next_image = nullptr;
    }
    if (frame_image) {
        delete frame_image;
        frame_image = nullptr;
    }
    for(QImage image:allImage)
    {
        image=QImage();
    }
    allImage.clear();
    malloc_trim(0);
    qDebug() << "-------------SlideEffect end release";
}

void SlideEffect::setEasingCurve(const QEasingCurve &easing)
{
    easing_ = easing;
}

void SlideEffect::setEasingCurve(QEasingCurve::Type easing_type)
{
    easing_ = QEasingCurve(easing_type);
}

QEasingCurve SlideEffect::easingCurve() const
{
    return easing_;
}

void SlideEffect::setDuration(int ms)
{
    duration_ms = ms;
}

void SlideEffect::setAllMs(int ms)
{
    all_ms = ms;
}

int SlideEffect::allMs() const
{
    return all_ms;
}

int SlideEffect::duration() const
{
    return duration_ms;
}

void SlideEffect::start()
{
    for(auto image:allImage){
        image=QImage();
    }
    allImage.clear();
    allImage[frames_total] = *next_image;
    prepare();
    current_frame = 0;

    m_readlock.lockForRead();
    for (int i = 0; i < frames_total; i++) {
        if (i == frames_total-1) {
            m_nNum = frames_total;
        }
        if (!prepareNextFrame()) {
            stop();
            m_readlock.unlock();
            return;
        }
    }
    m_readlock.unlock();
//    m_qf = QtConcurrent::run([this]() {
//        for (int i = 0; i < frames_total; i++) {
//            if (!prepareNextFrame()) {
//                stop();
//                return;
//            }
//        }
//    });

//    QEventLoop loop;
//    QTimer::singleShot(all_ms - duration_ms, &loop, SLOT(quit()));
//    loop.exec();
    scurrent = 0;
    bfirsttimeout = true;
    //tid = startTimer(all_ms - duration_ms - 1000);

//    tid = startTimer(duration() / frames_total);
}

void SlideEffect::stop()
{
    killTimer(tid);
    tid = 0;
    Q_EMIT stopped();
}

void SlideEffect::pause()
{
    paused = !paused;
}

void SlideEffect::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != tid || paused)
        return;
//    if (!prepareNextFrame()) {
//        stop();
//        return;
//    }
//    Q_EMIT frameReady(*currentFrame());
    if (bfirsttimeout) {
        stop();
        tid = startTimer(duration() / (frames_total + 1));
//        tid = startTimer(20);
        bfirsttimeout = false;
    } else {
        while (true) {
            if (scurrent > frames_total)
                return;
            QMap<int, QImage>::iterator it;
            it = allImage.find(scurrent);
            scurrent++;
            if (it != allImage.end()) {
                Q_EMIT frameReady(it.value());
                return;
            }
        }
    }
}

bool SlideEffect::prepare()
{
    if (!frame_image) {
        frame_image = new QImage(width, height, QImage::Format_ARGB32);
        frame_image->fill(0);
    }
    resizeImages();
    tid = 0;
    finished = false;
    paused = false;
    progress_ = 0.0;
    current_frame = 0;
    next_rect = QRect(0, 0, width, height);
    current_rect = QRect(0, 0, width, height);
    return true;
}

bool SlideEffect::prepareNextFrame()
{
    //current_frame++? do not paint frame 0?
    if (prepareFrameAt(++current_frame)) {
//        renderFrame(current_frame, current_clip_region, next_clip_region);

//        virtual void renderFrame(QImage mimage, int num, QRegion current_region, QRegion next_region, int width, int height
//                                 , QImage current_image, QImage next_image, QRect current_rect, QRect next_rect);
        SlideEffectThreadData data;
        data.mimage = *frame_image;
        data.num = current_frame;
        data.current_region = current_clip_region;
        data.next_region = next_clip_region;
        data.width = width;
        data.height = height;
        data.current_image = *current_image;
        data.next_image = *next_image;
        data.current_rect = current_rect;
        data.next_rect = next_rect;
        ThreadRenderFrame *threadf = new ThreadRenderFrame();
        connect(threadf, &ThreadRenderFrame::signal_RenderFinish, this, &SlideEffect::slotrenderFrameFinish);
        connect(this, &SlideEffect::deleteLater, threadf, &ThreadRenderFrame::stop);
        threadf->setData(data);
        QThreadPool::globalInstance()->start(threadf);
//        m.start(threadf);
//        QFuture<void> res = QtConcurrent::run(this, &SlideEffect::renderFrame, data);
        //        QFuture<void> res = QtConcurrent::run(this, &SlideEffect::renderFrame, *frame_image, current_frame, current_clip_region, next_clip_region,
//                                              width, height, *current_image, *next_image, current_rect, next_rect);

//        QFuture<void> res = QtConcurrent::run([this]() {
//            renderFrame(*frame_image, current_frame, current_clip_region, next_clip_region,
//                        width, height, *current_image, *next_image, current_rect, next_rect);
//        });
//        m_qflist.append(res);
        return true;
    }
    return false;
}

QImage *SlideEffect::currentFrame()
{
    return frame_image;
}

void SlideEffect::setType(EffectId type)
{
    effect_type = type;
}

EffectId SlideEffect::type() const
{
    return effect_type;
}


int SlideEffect::currentFrameNumber() const
{
    return current_frame;
}

int SlideEffect::frames() const
{
    return frames_total;
}

void SlideEffect::setSize(const QSize &s)
{
    width = s.width();
    height = s.height();
}

QSize SlideEffect::size() const
{
    return QSize(width, height);
}



void SlideEffect::setImages(const QString &currentPath, const QString &nextPath)
{
    current_path = currentPath;
    next_path = nextPath;
    if (!next_image)
        next_image = new QImage();
    *next_image = utils::image::getRotatedImage(next_path);//.copy();

    if (current_path.isEmpty()) {
        qDebug("The first image. create blank image");
        current_image = new QImage(next_image->size(), QImage::Format_ARGB32);
        current_image->fill(0);
    } else {
        if (!current_image)
            current_image = new QImage();
        *current_image = utils::image::getRotatedImage(current_path);//.copy();


    }
}

void SlideEffect::setImages(const QImage &currentImage, const QImage &nextImage)
{
//QImage is reference counted, so copy directly is ok
    if (!current_image)
        current_image = new QImage(currentImage);
    else
        *current_image = currentImage;//.copy();
    if (!next_image)
        next_image = new QImage(nextImage);
    else
        *next_image = nextImage;//.copy();

}

//small image zoom failed. why?
static void scaleImageToMax(QImage *image, int width, int height)
{
    Q_ASSERT(image);
//    int w = qMax(image->width(), 1);
//    int h = qMax(image->height(), 1);
//    qreal ir = qreal(w)/qreal(h);
//    qreal r = qreal(width)/qreal(height);

    //qDebug("image: %f %dx%d, target: %f %dx%d", ir, w, h, r, width, height);
//    if (ir > r) { //too wide, the width is set to the given width, fit height
//        h = int(qreal(width)/ir);
//        if (h == 0)
//            h = image->height();
//    } else { //too high
//        w = int(qreal(height)*ir);
//        if (w == 0)
//            w = image->width();
//    }
    *image = image->scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

static void addBackground(QImage *image, int width, int height, const QColor &color)
{
    QImage bg(width, height, QImage::Format_ARGB32);
    bg.fill(0);
    QPainter p(&bg);
    p.fillRect(bg.rect(), color);

    p.setCompositionMode(QPainter::CompositionMode_SourceOver); //image part is the same as image
    p.setClipRect(bg.rect());
    //draw at center. If image size is larger than given size, draw the center part. Clip region is useful
    int x = (width - image->width()) / 2;
    int y = (height - image->height()) / 2;

    p.drawImage(x, y, *image);
    *image = bg;
}

void SlideEffect::resizeImages()
{
    if (mode == Qt::IgnoreAspectRatio) {
        //TODO: just black background
        if (current_image && current_image->size() != QSize(width, height)) {
            *current_image = current_image->scaled(width, height); //4, 1/4
        }
        if (next_image && next_image->size() != QSize(width, height)) {
            *next_image = next_image->scaled(width, height);
        }
    } else {
        //calculate the size then scale
        scaleImageToMax(current_image, width, height);
        scaleImageToMax(next_image, width, height);
        if (mode == Qt::KeepAspectRatio) { //draw to a background with size width*height
            addBackground(current_image, width, height, color);
            addBackground(next_image, width, height, color);
        }
    }

    if (frame_image && frame_image->size() != QSize(width, height)) {
        *frame_image = frame_image->scaled(width, height);
    }
}

bool SlideEffect::isEndFrame(int frame)
{
    /*
        progress_>=1.0 may result in the speed is too large. If we stop immediately, the final frame will never show.
        At this time ,we show the last frame, and tell the program that effect is finished;
    */
    if (finished)
        return true;
    //if (frame>frames_total)
    //  return true;
    current_frame = frame;
    progress_ = speed * (qreal)current_frame / (qreal)frames_total;

    if (progress_ >= 1.0) {
        progress_ = 1.0; //It's important
        finished = true;
    }
    return false;
}

//TODO: alpha blending here
void SlideEffect::renderFrame(SlideEffectThreadData &data)
{
//    Q_ASSERT(frame_image);
//    QPainter p(frame_image);
//    frame_image->fill(Qt::transparent);
    SlideEffectThreadData mdata = data;
//    qDebug() << "-------renderFrame start num:" << data.num;
    QImage image = mdata.mimage;
    QPainter p(&image);
    image.fill(Qt::transparent);
    /*
        Tell me why!
        if draw the next frame_image first, then it will flick for FromBottom, CoverBottom effect!
    */
    //actually we can just paint the next frame_image if no opacity changes
    p.setClipRegion(mdata.current_region);
    p.drawImage(QRect(0, 0, mdata.width, mdata.height), /**current_image*/mdata.current_image, mdata.current_rect);

    p.setClipRegion(mdata.next_region);
    p.drawImage(QRect(0, 0, mdata.width, mdata.height), /**next_image*/mdata.next_image, mdata.next_rect);
    emit renderFrameFinish(mdata.num, image);

    //    qDebug() << "-------renderFrame end num:" << data.num;
}

void SlideEffect::clearimagemap()
{
    for(auto image :allImage)
    {
        image=QImage();
    }
    allImage.clear();
}


