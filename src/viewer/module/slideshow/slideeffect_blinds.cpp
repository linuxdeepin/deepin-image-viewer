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
    SlideEffect_Blinds: Blinds effect.
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
#include <time.h>

static const QString kBlindsBottomToTop = "blinds_bottom_to_top";
static const QString kBlindsLeftToRight = "blinds_left_to_right";
static const QString kBlindsRightToLeft = "blinds_right_to_left";
static const QString kBlindsTopToBottom = "blinds_top_to_bottom";
class SlideEffect_Blinds : public SlideEffect
{
public:
    virtual bool prepare();
    virtual EffectName effectName() const
    {
        return Blinds;
    }
    virtual QVector<EffectId> supportedTypes() const
    {
        return QVector<EffectId>() << kBlindsBottomToTop << kBlindsLeftToRight
               << kBlindsRightToLeft << kBlindsTopToBottom;
    }
protected:
    virtual bool prepareFrameAt(int frame);
private:
    //void calculateRegion_Random(qreal k); //default
    void calculateRegion_BottomToTop(qreal k);
    void calculateRegion_LeftToRight(qreal k);
    void calculateRegion_RightToLeft(qreal k);
    void calculateRegion_TopToBottom(qreal k);

    typedef void (SlideEffect_Blinds::*calculateRegion_Func)(qreal);
    calculateRegion_Func calculateRegion_ptr;

    int leafs;
    int leaf_width;
};

REGISTER_EFFECTS(SlideEffect_Blinds)

bool SlideEffect_Blinds::prepare()
{
    SlideEffect::prepare(); //Important!!!
    if (effect_type == kBlindsBottomToTop)
        calculateRegion_ptr = &SlideEffect_Blinds::calculateRegion_BottomToTop;
    else if (effect_type == kBlindsLeftToRight)
        calculateRegion_ptr = &SlideEffect_Blinds::calculateRegion_LeftToRight;
    else if (effect_type == kBlindsRightToLeft)
        calculateRegion_ptr = &SlideEffect_Blinds::calculateRegion_RightToLeft;
    else if (effect_type == kBlindsTopToBottom)
        calculateRegion_ptr = &SlideEffect_Blinds::calculateRegion_TopToBottom;
    else
        calculateRegion_ptr = &SlideEffect_Blinds::calculateRegion_TopToBottom; //calculateRegion_Random

    qsrand(time(0));
    leafs = 5;

    if (effect_type == kBlindsLeftToRight || effect_type == kBlindsRightToLeft) {
        leaf_width = width / leafs + 1; //Must plus 1! Or the final frame_image may have blanks
    } else {
        leaf_width = height / leafs + 1;
    }

    return true;
}

bool SlideEffect_Blinds::prepareFrameAt(int frame)
{
    if (isEndFrame(frame))
        return false;
    qreal k = easing_.valueForProgress(progress_);
    (this->*calculateRegion_ptr)(k);
    current_clip_region = QRegion(0, 0, width, height) - next_clip_region;
    return true;
}

void SlideEffect_Blinds::calculateRegion_BottomToTop(qreal k)
{
    next_clip_region = QRegion();
    for (int i = 0; i < leafs; next_clip_region += QRegion(0, (i++ +1.0 - k) * leaf_width, width, leaf_width * k));
}

//Have some bugs! Pictures are not show completely
void SlideEffect_Blinds::calculateRegion_LeftToRight(qreal k)
{
    next_clip_region = QRegion();
    for (int i = 0; i < leafs; next_clip_region += QRegion(i++*leaf_width, 0, leaf_width * k, height));
}

void SlideEffect_Blinds::calculateRegion_RightToLeft(qreal k)
{
    next_clip_region = QRegion();
    for (int i = 0; i < leafs; next_clip_region += QRegion((i++ +1.0 - k) * leaf_width, 0, leaf_width * k, height));
}

void SlideEffect_Blinds::calculateRegion_TopToBottom(qreal k)
{
    next_clip_region = QRegion();
    for (int i = 0; i < leafs; next_clip_region += QRegion(0, i++*leaf_width, width, leaf_width * k));
}
