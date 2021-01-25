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
/******************************************************************************
    SlideEffect_Circle: Open from or close to center effect.
    Copyright (C) 2011-2013 Wang Bin <wbsecg1@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
******************************************************************************/

#include "slideeffect.h"
#include <cmath>
#include <QtGui/QMatrix>

static const EffectId kEllipseOpen = "ellipse_open";
static const EffectId kEllipseClose = "ellipse_close";
class SlideEffect_Circle : public SlideEffect
{
public:
    SlideEffect_Circle();
    virtual bool prepare() override;
    virtual EffectName effectName() const
    {
        return Circle;
    }
    virtual QVector<EffectId> supportedTypes() const
    {
        return QVector<EffectId>() << kEllipseClose << kEllipseOpen;
    }

protected:
    virtual bool prepareFrameAt(int frame);
    //virtual bool isEndFrame(int frame);

private:
    bool prepareFrameAt_EllipseOpen(int frame);
    bool prepareFrameAt_EllipseClose(int frame);
    //bool prepareFrameAt_RectZoomOpen(int frame);

    typedef bool (SlideEffect_Circle::*prepareFrameAt_Func)(int);
    prepareFrameAt_Func func;
};

REGISTER_EFFECTS(SlideEffect_Circle)

SlideEffect_Circle::SlideEffect_Circle()
    :func(nullptr)
{
#ifndef NO_EASINGCURVE
    setEasingCurve(QEasingCurve::InOutQuint);
#endif //NO_EASINGCURVE
}

bool SlideEffect_Circle::prepare()
{
    SlideEffect::prepare(); //Important!!!
    if (effect_type == kEllipseOpen) {
        func = &SlideEffect_Circle::prepareFrameAt_EllipseOpen;
    } else {
        func = &SlideEffect_Circle::prepareFrameAt_EllipseClose;
    }

    current_clip_region = QRegion();
    current_rect = QRect();
    //frame_image = current_image; //?
    return true;
}

bool SlideEffect_Circle::prepareFrameAt(int frame)
{
    return (this->*func)(frame);
}
//TODO: to use easingcurve, the total steps must be fixed, k<=1;
/*
bool SlideEffect_Circle::isEndFrame(int frame)
{
  //current_clip_region.isEmpty() is true at first frame
  if (current_clip_region.isEmpty() && current_frame>1)
  //if (frame>frames_total)
      return true;
  current_frame = frame;
  progress_ = (qreal)current_frame/(qreal)frames_total*1*speed; //1.618 to speed up
  return false;
}
*/
bool SlideEffect_Circle::prepareFrameAt_EllipseOpen(int frame)
{
    if (isEndFrame(frame))
        return false;
    qreal k = easing_.valueForProgress(progress_);
    /*
    //next_clip_region = QRegion(width*(1.0-k)*0.5, height*(1.0-k)*0.5, width*k, height*k, QRegion::Ellipse);
    //current_clip_region = QRegion(0, 0, width, height) - next_clip_region;
    QRegion region = QRegion(width*(1.0-k)*0.5, height*(1.0-k)*0.5, width*k, height*k, QRegion::Ellipse);
    next_clip_region = region - next_clip_region; //the changed region
    current_clip_region = QRegion(0, 0, width, height) - region;
    */
    static qreal ab = qreal(width) / qreal(height); //a/b or b/a
    static int r = sqrt(qreal(width * width + height * height)) * 0.5;
    int rk = r * k;
    QRegion E(0.5 * width - rk, 0.5 * height - rk, 2 * rk, 2 * rk, QRegion::Ellipse); //(x, y, w, h)=>center(x+w/2,y+h/2)
    //transform to an ellipse
    QMatrix m;
    m.translate(width * 0.5, height * 0.5);
    m.scale(ab, 1.0);
    m.translate(-width * 0.5, -height * 0.5);
    E = m.map(E);
    current_clip_region = QRegion(0, 0, width, height) - E;
    next_clip_region = E & QRegion(0, 0, width, height);
    return true;
}

//Circle -> ellipse
bool SlideEffect_Circle::prepareFrameAt_EllipseClose(int frame)
{
    if (isEndFrame(frame))
        return false;
    qreal k = easing_.valueForProgress(progress_);
    static qreal ab = qreal(width) / qreal(height); //a/b or b/a
    static int r = sqrt(qreal(width * width + height * height)) * 0.5;
    int rk = r * (1.0 - k);
    QRegion E(0.5 * width - rk, 0.5 * height - rk, 2 * rk, 2 * rk, QRegion::Ellipse); //(x, y, w, h)=>center(x+w/2,y+h/2)
    //transform to an ellipse
    QMatrix m;
    m.translate(width * 0.5, height * 0.5);
    m.scale(ab, 1.0);
    m.translate(-width * 0.5, -height * 0.5);
    E = m.map(E);
    current_clip_region = E & QRegion(0, 0, width, height);//the initial current_clip_region is null
    next_clip_region = QRegion(0, 0, width, height) - current_clip_region;
    return true;
}
