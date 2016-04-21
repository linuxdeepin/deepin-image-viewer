#include "slideeffect.h"
#include <qpainter.h>
#include <QtCore/QTimerEvent>

QHash<EffectId, std::function<SlideEffect*()> > SlideEffect::effects;

SlideEffect* SlideEffect::create(const EffectId &id)
{
    if (id == EffectId()) {
        srand(time(0));
        QList<std::function<SlideEffect*()>> cs = effects.values();
        const int idx = rand() % cs.size();
        std::function<SlideEffect*()> c = cs.at(idx);
        SlideEffect *e = c();
        return e;
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
{
    duration_ms = 1500;
    tid = 0;
    mode = Qt::KeepAspectRatio;
    finished = false;
    progress_ = 0.0;
    speed = 1.0;
    current_frame = 0;
    frames_total = 17;
    current_image = next_image = frame_image = 0;
    width = height = 0;
    color = Qt::transparent;
    easing_ = QEasingCurve(QEasingCurve::OutBack);
}

SlideEffect::~SlideEffect()
{
    if (current_image) {
        delete current_image;
        current_image = 0;
    }
    if (next_image) {
        delete next_image;
        next_image = 0;
    }
    if (frame_image) {
        delete frame_image;
        frame_image = 0;
    }
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

int SlideEffect::duration() const
{
    return duration_ms;
}

void SlideEffect::start()
{
    prepare();
    tid = startTimer(duration()/frames_total);
}

void SlideEffect::stop()
{
    killTimer(tid);
    tid = 0;
    Q_EMIT stopped();
}

void SlideEffect::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != tid)
        return;
    if (!prepareNextFrame()) {
        stop();
        return;
    }
    Q_EMIT frameReady(*currentFrame());
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
        renderFrame();
        return true;
    }
    return false;
}

QImage *SlideEffect::currentFrame() const
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

void SlideEffect::setSpeed(qreal s)
{
    speed = s;
}

void SlideEffect::setFrames(int frames)
{
    frames_total = frames;
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

void SlideEffect::setBackgroundColor(const QColor &color)
{
    this->color = color;
}

QColor SlideEffect::backgroundColor() const
{
    return color;
}

void SlideEffect::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    this->mode = mode;
}

Qt::AspectRatioMode SlideEffect::aspectRatioMode() const
{
    return mode;
}

void SlideEffect::setImages(const QString &currentPath, const QString &nextPath)
{
    current_path = currentPath;
    next_path = nextPath;
    qDebug("%s ==> %s", qPrintable(current_path), qPrintable(next_path));
    if (!next_image)
        next_image = new QImage(next_path);
    else
        *next_image = QImage(next_path);//.copy();
    if (current_path.isEmpty()) {
        qDebug("The first image. create blank image");
        current_image = new QImage(next_image->size(), QImage::Format_ARGB32);
        current_image->fill(0);
    } else {
        if (!current_image)
            current_image = new QImage(current_path);
        else
            *current_image = QImage(current_path);//.copy();
    }
}

void SlideEffect::setImages(const QImage &currentImage, const QImage &nextImage)
{
//QImage is refrence counted, so copy directly is ok
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
static void scaleImageToMax(QImage* image, int width, int height)
{
    Q_ASSERT(image);
    int w = qMax(image->width(), 1);
    int h = qMax(image->height(), 1);
    qreal ir = qreal(w)/qreal(h);
    qreal r = qreal(width)/qreal(height);

    //qDebug("image: %f %dx%d, target: %f %dx%d", ir, w, h, r, width, height);
    if (ir > r) { //too wide, the width is set to the given width, fit height
        h = int(qreal(width)/ir);
        if (h == 0)
            h = image->height();
    } else { //too high
        w = int(qreal(height)*ir);
        if (w == 0)
            w = image->width();
    }
    *image = image->scaled(w, h, Qt::KeepAspectRatio);
}

static void addBackground(QImage* image, int width, int height, const QColor& color )
{
    QImage bg(width, height, QImage::Format_ARGB32);
    bg.fill(0);
    QPainter p(&bg);
    p.fillRect(bg.rect(), color);

    p.setCompositionMode(QPainter::CompositionMode_SourceOver); //image part is the same as image
    p.setClipRect(bg.rect());
    //draw at center. If image size is larger than given size, draw the center part. Clip region is usefull
    int x = (width - image->width())/2;
    int y = (height - image->height())/2;

    p.drawImage(x, y, *image);
    *image = bg;
}

void SlideEffect::resizeImages()
{
    if (mode == Qt::IgnoreAspectRatio) {
        //TODO: just black background
        if (current_image && current_image->size()!=QSize(width, height)) {
            *current_image = current_image->scaled(width, height); //4, 1/4
        }
        if (next_image && next_image->size()!=QSize(width, height)) {
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

    if (frame_image && frame_image->size()!=QSize(width, height)) {
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
    //	return true;
    current_frame = frame;
    progress_ = speed*(qreal)current_frame/(qreal)frames_total;

    if (progress_>=1.0) {
        progress_ = 1.0; //It's important
        finished = true;
    }
    return false;
}

//TODO: alpha blending here
void SlideEffect::renderFrame()
{
    Q_ASSERT(frame_image);
    QPainter p(frame_image);
/*
    Tell me why!
    if draw the next frame_image first, then it will flick for FromBottom, CoverBottom effect!
*/
    //actually we can just paint the next frame_image if no opacity changes
    p.setClipRegion(current_clip_region);
    p.drawImage(QRect(0, 0, width, height), *current_image, current_rect);

    p.setClipRegion(next_clip_region);
    p.drawImage(QRect(0, 0, width, height), *next_image, next_rect);
}


