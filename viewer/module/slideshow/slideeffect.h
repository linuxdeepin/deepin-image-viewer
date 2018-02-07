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

#include <functional>

#include <qimage.h>
#include <qcolor.h>
#include <QtCore/QObject>
#include <QtCore/QEasingCurve>
typedef QString EffectId;

static const EffectId kInvalid = "invalid";
static const EffectId kRandom = "random";
/*!
  How to use:
  1) Display on your widget
  call SlideEffectFactory::instance().createEffect() or SlideEffectFactory::instance().getRandomEffect() to get
  an effect, then set pixmaps using setImages() and other optinal parameter. After the foreplay,
  call prepare();

  start widget's effect timer. In it's timerEvent, add
    if (!mEffect->prepareNextFrame()) {
        return;
    }
  update();
  in widget's painterEvent, add
    QPainter painter(this);
    painter.drawPixmap(rect(), *mEffect->currentFrame());
*/

class SlideEffect : public QObject
{
    Q_OBJECT
public:
    enum EffectName {
        Blinds,
        Switcher,
        Slide,
        Circle
    };
    // default id will return an object randomly
    static SlideEffect* create(const EffectId& id = EffectId());
    template<class C> static void registerEffect(const EffectId& id) {
        Register(id, std::bind(create<C>, id));
    }

    SlideEffect();
    virtual ~SlideEffect();

    void setEasingCurve(const QEasingCurve& easing);
    void setEasingCurve(QEasingCurve::Type easing_type);
    QEasingCurve easingCurve() const;
    void setDuration(int ms);
    int duration() const;

    QImage *currentFrame();
    void setType(EffectId type);
    EffectId type() const;
/*!
  Some class may have several effect types.
*/
    virtual EffectName effectName() const = 0;
    virtual QVector<EffectId> supportedTypes() const = 0;
    void setSpeed(qreal s);
    void setFrames(int frames);
    int currentFrameNumber() const;
    int frames() const;

    void setImages(const QString& currentPath, const QString& nextPath);
    void setImages(const QImage& currentImage, const QImage& nextImage);
/*!
    set images' maximum size. DO NOT forget to call it
    TODO: setFrameSize(), frameSize()
*/
    void setSize(const QSize& s);
    QSize size() const;

    void setBackgroundColor(const QColor& color);
    QColor backgroundColor() const;

    void setAspectRatioMode(Qt::AspectRatioMode mode);
    Qt::AspectRatioMode aspectRatioMode() const;

Q_SIGNALS:
    void stopped();
    void frameReady(const QImage& image);

public Q_SLOTS:
    void start();
    void stop();
    void pause();
protected:
    virtual void timerEvent(QTimerEvent *e);
    virtual bool prepare(); //after all parameters set and before effect start
    virtual bool prepareFrameAt(int frame) = 0;
    /*!
        false: no more frames
    */
    bool prepareNextFrame();

    void resizeImages(); //resize to given size with given scale type
    virtual bool isEndFrame(int frame); //TODO: do not change progress
    virtual void renderFrame();

protected:
    bool finished;
    bool paused;
    int tid;
    int duration_ms;
    Qt::AspectRatioMode mode;
    qreal progress_; //the step, [0,1]
    qreal speed; //>1.0
    EffectId effect_type;
    int frames_total, current_frame;
    //if current_image is null, just paint next_image inside
    QImage *frame_image, *current_image, *next_image;
    int width, height;
    //clip region of currentFrame() to paint current and next frame_image
    QRegion current_clip_region, next_clip_region;

/*
    rect of current and frame_image to be paint. The size is always the frame_image's size.
    when calling drawPixmap(const QRect& target, const QPixmap& frame_image, const QRect& source),
    if rect not equals target's rect, then the frame_image will be scaled to target's rect.
    Here, target's rect is it's rect()
*/
    QRect current_rect, next_rect;
    QString  current_path, next_path;
    QColor color;
    QEasingCurve easing_;

private:
    template<class C> static SlideEffect* create(EffectId id) {
        SlideEffect *e = new C();
        e->setType(id);
        return e;
    }
    static void Register(EffectId id, std::function<SlideEffect*()> c);

    static QHash<EffectId, std::function<SlideEffect*()> > effects;
};

#define REGISTER_EFFECTS(T) \
static void register_effects() { \
    T* e = new T(); \
    foreach (EffectId id, e->supportedTypes()) { \
        SlideEffect::registerEffect<T>(id); \
    } \
    delete e; \
} \
Q_CONSTRUCTOR_FUNCTION(register_effects)

