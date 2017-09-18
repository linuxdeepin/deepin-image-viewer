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

class SlideEffect_Enter : public SlideEffect
{
public:
    virtual bool prepare();
    virtual EffectName effectName() const {
        return Slide;
    }
    virtual QVector<EffectId> supportedTypes() const {
        return QVector<EffectId>() << kEnterFromBottom << kEnterFromLeft
                 << kEnterFromRight  << kEnterFromTop;
    }
protected:
    virtual bool prepareFrameAt(int frame);

private:
    //void calculateRegion_Random(qreal k); //default

    void calculateRegion_FromBottom(qreal k);
    void calculateRegion_FromLeft(qreal k);
    void calculateRegion_FromRight(qreal k);
    void calculateRegion_FromTop(qreal k);

    typedef void (SlideEffect_Enter::*calculateRegion_Func)(qreal);
    calculateRegion_Func calculateRegion_ptr;
};

REGISTER_EFFECTS(SlideEffect_Enter)

bool SlideEffect_Enter::prepare()
{
    SlideEffect::prepare(); //Important!!!
    setEasingCurve(QEasingCurve::InOutQuint);
    if (effect_type == kEnterFromBottom)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromBottom;
    else if (effect_type == kEnterFromLeft)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromLeft;
    else if (effect_type == kEnterFromRight)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromRight;
    else if (effect_type == kEnterFromTop)
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromTop;
	else
        calculateRegion_ptr = &SlideEffect_Enter::calculateRegion_FromRight; //calculateRegion_Random

	return true;
}

bool SlideEffect_Enter::prepareFrameAt(int frame)
{
    if (isEndFrame(frame)) {
        return false;
    } else {
        qreal k = easing_.valueForProgress(progress_);
        (this->*calculateRegion_ptr)(k);
        current_clip_region = QRegion(0, 0, width, height) - next_clip_region;
    }
    return true;
}

void SlideEffect_Enter::calculateRegion_FromBottom(qreal k)
{
	next_clip_region = QRegion(0, height*(1.0-k), width, height*k);
	next_rect = QRect(0, height*(k-1.0), width, height);
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
