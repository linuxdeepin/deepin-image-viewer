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
    SlideEffect_Switcher: Open from or close to center effect.
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

static const EffectId kHorizontalOpen = "horizontal_open";
static const EffectId kHorizontalClose = "horizontal_close";
static const EffectId kVerticalOpen = "vertical_open";
static const EffectId kVerticalClose = "vertical_close";
class SlideEffect_Switcher : public SlideEffect
{
public:
    SlideEffect_Switcher();
    virtual bool prepare() override;
    virtual EffectName effectName() const override
    {
        return Switcher;
    }
    virtual QVector<EffectId> supportedTypes() const override
    {
        return QVector<EffectId>() << kHorizontalOpen << kHorizontalClose
               << kVerticalOpen << kVerticalClose;
    }

protected:
    virtual bool prepareFrameAt(int frame) override;
    //virtual bool isEndFrame(int frame);

private:
    bool prepareFrameAt_HorizontalOpen(int frame);
    bool prepareFrameAt_HorizontalClose(int frame);
    bool prepareFrameAt_VerticalOpen(int frame);
    bool prepareFrameAt_VerticalClose(int frame);
    //bool prepareFrameAt_RectZoomOpen(int frame);

    typedef bool (SlideEffect_Switcher::*prepareFrameAt_Func)(int);
    prepareFrameAt_Func func;
};

REGISTER_EFFECTS(SlideEffect_Switcher)

SlideEffect_Switcher::SlideEffect_Switcher()
    :func()
{
#ifndef NO_EASINGCURVE
    setEasingCurve(QEasingCurve::InOutQuint);
#endif //NO_EASINGCURVE
}

bool SlideEffect_Switcher::prepare()
{
    SlideEffect::prepare();
    if (effect_type == kHorizontalOpen) {
        func = &SlideEffect_Switcher::prepareFrameAt_HorizontalOpen;
    } else if (effect_type == kHorizontalClose) {
        func = &SlideEffect_Switcher::prepareFrameAt_HorizontalClose;
    } else if (effect_type == kVerticalOpen) {
        func = &SlideEffect_Switcher::prepareFrameAt_VerticalOpen;
    } else if (effect_type == kVerticalClose) {
        func = &SlideEffect_Switcher::prepareFrameAt_VerticalClose;
    } else {
        func = &SlideEffect_Switcher::prepareFrameAt_VerticalOpen;
    }

    current_clip_region = QRegion();
    current_rect = QRect();
    //frame_image = current_image; //?
    return true;
}

bool SlideEffect_Switcher::prepareFrameAt(int frame)
{
    return (this->*func)(frame);
}
//TODO: to use easingcurve, the total steps must be fixed, k<=1;
/*
bool SlideEffect_Switcher::isEndFrame(int frame)
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

bool SlideEffect_Switcher::prepareFrameAt_HorizontalOpen(int frame)
{
    if (isEndFrame(frame))
        return false;
    qreal k = easing_.valueForProgress(progress_);
    next_clip_region = QRegion(width * (1.0 - k) * 0.5, 0, width * k, height);
    current_clip_region = QRegion(0, 0, width, height) - next_clip_region;
    return true;
}

bool SlideEffect_Switcher::prepareFrameAt_HorizontalClose(int frame)
{
    if (isEndFrame(frame))
        return false;
    qreal k = easing_.valueForProgress(progress_);
    current_clip_region = QRegion(width * k * 0.5, 0, width * (1.0 - k), height);
    next_clip_region = QRegion(0, 0, width, height) - current_clip_region;
    return true;
}

bool SlideEffect_Switcher::prepareFrameAt_VerticalOpen(int frame)
{
    if (isEndFrame(frame))
        return false;
    qreal k = easing_.valueForProgress(progress_);
    next_clip_region = QRegion(0, height * (1.0 - k) * 0.5, width, height * k);
    current_clip_region = QRegion(0, 0, width, height) - next_clip_region;
    return true;
}

bool SlideEffect_Switcher::prepareFrameAt_VerticalClose(int frame)
{
    if (isEndFrame(frame))
        return false;
    qreal k = easing_.valueForProgress(progress_);
    current_clip_region = QRegion(0, height * k * 0.5, width, height * (1.0 - k));
    next_clip_region = QRegion(0, 0, width, height) - current_clip_region;
    return true;
}
