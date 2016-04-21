/******************************************************************************
    SlideEffect_Enter: Fly in effect.
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

static const EffectId kEnterFromTop = "enter_from_top";
static const EffectId kEnterFromBottom = "enter_from_bottom";
static const EffectId kEnterFromLeft = "enter_from_left";
static const EffectId kEnterFromRight = "enter_from_right";
static const EffectId kEnterFromTopLeft = "enter_from_topleft";
static const EffectId kEnterFromBottomLeft = "enter_from_bottomleft";
static const EffectId kEnterFromTopRight = "enter_from_topright";
static const EffectId kEnterFromBottomRight = "enter_from_bottomright";

class SlideEffect_Enter : public SlideEffect
{
public:
    virtual bool prepare();
    virtual QVector<EffectId> supportedTypes() const {
        return QVector<EffectId>() << kEnterFromBottom  << kEnterFromBottomLeft
                 << kEnterFromBottomRight  << kEnterFromLeft
                 << kEnterFromRight  << kEnterFromTop
                 << kEnterFromTopLeft  << kEnterFromTopRight;
    }
protected:
    virtual bool prepareFrameAt(int frame);

private:
    //void calculateRegion_Random(qreal k); //default

    void calculateRegion_FromBottom(qreal k);
    void calculateRegion_FromBottomLeft(qreal k);
    void calculateRegion_FromBottomRight(qreal k);
    void calculateRegion_FromLeft(qreal k);
    void calculateRegion_FromRight(qreal k);
    void calculateRegion_FromTop(qreal k);
    void calculateRegion_FromTopLeft(qreal k);
    void calculateRegion_FromTopRight(qreal k);

    typedef void (SlideEffect_Enter::*calculateRegion_Func)(qreal);
    calculateRegion_Func calculateRegion_ptr;
};
REGISTER_EFFECTS(SlideEffect_Enter)

bool SlideEffect_Enter::prepare()
{
    SlideEffect::prepare(); //Important!!!
    if (effect_type == kEnterFromBottom)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromBottom;
    else if (effect_type == kEnterFromBottomLeft)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromBottomLeft;
    else if (effect_type == kEnterFromBottomRight)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromBottomRight;
    else if (effect_type == kEnterFromLeft)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromLeft;
    else if (effect_type == kEnterFromRight)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromRight;
    else if (effect_type == kEnterFromTop)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromTop;
    else if (effect_type == kEnterFromTopLeft)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromTopLeft;
    else if (effect_type == kEnterFromTopRight)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromTopRight;
	else
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromTop; //calculateRegion_Random

	return true;
}

bool SlideEffect_Enter::prepareFrameAt(int frame)
{
    if (isEndFrame(frame))
        return false;
	qreal k = easing_.valueForProgress(progress_);
    (this->*calculateRegion_ptr)(k);
    current_clip_region = QRegion(0, 0, width, height) - next_clip_region;
	return true;
}

void SlideEffect_Enter::calculateRegion_FromBottom(qreal k)
{
	next_clip_region = QRegion(0, height*(1.0-k), width, height*k);
	next_rect = QRect(0, height*(k-1.0), width, height);
}

/*
 ?...................
  .    next_rect    .
  .                 .
  +++++++............
  +     +           .
  +     +           .
  +++++++............
*/
void SlideEffect_Enter::calculateRegion_FromBottomLeft(qreal k)
{
	next_clip_region = QRegion(0, height*(1.0-k), width*k, height*k);
	next_rect = QRect(width*(1.0-k), height*(k-1.0), width, height);
}

/*
  ...................
  .    next_rect    .
  .          0+++++++
  .           +     +
  .           +     +
  ............+++++++
*/
void SlideEffect_Enter::calculateRegion_FromBottomRight(qreal k)
{
	next_clip_region = QRegion(width*(1.0-k), height*(1.0-k), width*k, height*k);
	next_rect = QRect(width*(k-1.0), height*(k-1.0), width, height);

}

void SlideEffect_Enter::calculateRegion_FromLeft(qreal k)
{
	next_clip_region = QRegion(0, 0, width*k, height);
	next_rect = QRect(width*(1.0-k), 0, width, height);
}


void SlideEffect_Enter::calculateRegion_FromRight(qreal k)
{
	next_clip_region = QRegion(width*(1.0-k), 0, width*k, height);
	next_rect = QRect(width*(k-1.0), 0, width, height);
}

void SlideEffect_Enter::calculateRegion_FromTop(qreal k)
{
	next_clip_region = QRegion(0, 0, width, height*k);
	next_rect = QRect(0, height*(1.0-k), width, height);
}

/*
  +++++++............
  +     +           .
  +     +           .
  +++++++           .
  .                 .
  .    next_rect    .
  ...................
*/
void SlideEffect_Enter::calculateRegion_FromTopLeft(qreal k)
{
	next_clip_region = QRegion(0, 0, width*k, height*k);
	next_rect = QRect(width*(1.0-k), height*(1.0-k), width, height);
}

/*
 ?............+++++++
  .           +     +
  .           +     +
  .           +++++++
  .                 .
  .    next_rect    .
  ...................
*/
void SlideEffect_Enter::calculateRegion_FromTopRight(qreal k)
{
	next_clip_region = QRegion(width*(1.0-k), 0, width*k, height*k);
	next_rect = QRect(width*(k-1.0), height*(1.0-k), width, height);
}
