/******************************************************************************
    SlideEffect_Center: Open from or close to center effect.
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

static const EffectId kEllipseOpen = "ellipse_open"; static const EffectId kEllipseClose = "ellipse_close";
static const EffectId kHorizontalOpen = "horizontal_open";
static const EffectId kHorizontalClose = "horizontal_close";
static const EffectId kVerticalOpen = "vertical_open";
static const EffectId kVerticalClose = "vertical_close";
class SlideEffect_Center : public SlideEffect
{
public:
    SlideEffect_Center();
    virtual bool prepare();
    virtual QVector<EffectId> supportedTypes() const {
        return QVector<EffectId>() << kEllipseClose << kEllipseOpen
                << kHorizontalOpen << kHorizontalClose << kVerticalOpen << kVerticalClose;
    }

protected:
    virtual bool prepareFrameAt(int frame);
    //virtual bool isEndFrame(int frame);

private:
    bool prepareFrameAt_EllipseOpen(int frame);
    bool prepareFrameAt_EllipseClose(int frame);
    bool prepareFrameAt_HorizontalOpen(int frame);
    bool prepareFrameAt_HorizontalClose(int frame);
    bool prepareFrameAt_VerticalOpen(int frame);
    bool prepareFrameAt_VerticalClose(int frame);
    //bool prepareFrameAt_RectZoomOpen(int frame);

    typedef bool (SlideEffect_Center::*prepareFrameAt_Func)(int);
    prepareFrameAt_Func func;
};
REGISTER_EFFECTS(SlideEffect_Center)

SlideEffect_Center::SlideEffect_Center()
{
#ifndef NO_EASINGCURVE
	setEasingCurve(QEasingCurve::InOutQuint);
#endif //NO_EASINGCURVE
}

bool SlideEffect_Center::prepare()
{
    SlideEffect::prepare(); //Important!!!
    if (effect_type == kEllipseOpen) {
        func = &SlideEffect_Center::prepareFrameAt_EllipseOpen;
    } else if (effect_type == kEllipseClose) {
        func = &SlideEffect_Center::prepareFrameAt_EllipseClose;
    } else if (effect_type == kHorizontalOpen) {
        func = &SlideEffect_Center::prepareFrameAt_HorizontalOpen;
    } else if (effect_type == kHorizontalClose) {
        func = &SlideEffect_Center::prepareFrameAt_HorizontalClose;
    } else if (effect_type == kVerticalOpen) {
        func = &SlideEffect_Center::prepareFrameAt_VerticalOpen;
    } else if (effect_type == kVerticalClose) {
        func = &SlideEffect_Center::prepareFrameAt_VerticalClose;
    } else {
        func = &SlideEffect_Center::prepareFrameAt_VerticalOpen;
    }

	current_clip_region = QRegion();
	current_rect = QRect();
	//frame_image = current_image; //?
	return true;
}

bool SlideEffect_Center::prepareFrameAt(int frame)
{
    return (this->*func)(frame);
}
//TODO: to use easingcurve, the total steps must be fixed, k<=1;
/*
bool SlideEffect_Center::isEndFrame(int frame)
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
bool SlideEffect_Center::prepareFrameAt_EllipseOpen(int frame)
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
	static qreal ab = qreal(width)/qreal(height); //a/b or b/a
	static int r = sqrt(qreal(width*width+height*height))*0.5;
	int rk = r*k;
	QRegion E(0.5*width-rk, 0.5*height-rk, 2*rk, 2*rk, QRegion::Ellipse); //(x, y, w, h)=>center(x+w/2,y+h/2)
	//transform to an ellipse
	QMatrix m;
	m.translate(width*0.5, height*0.5);
	m.scale(ab, 1.0);
	m.translate(-width*0.5, -height*0.5);
	E = m.map(E);
	current_clip_region = QRegion(0, 0, width, height) - E;
	next_clip_region = E & QRegion(0, 0, width, height);
	return true;
}

//Circle -> ellipse
bool SlideEffect_Center::prepareFrameAt_EllipseClose(int frame)
{
	if (isEndFrame(frame))
		return false;
	qreal k = easing_.valueForProgress(progress_);
	static qreal ab = qreal(width)/qreal(height); //a/b or b/a
	static int r = sqrt(qreal(width*width+height*height))*0.5;
	int rk = r*(1.0-k);
	QRegion E(0.5*width-rk, 0.5*height-rk, 2*rk, 2*rk, QRegion::Ellipse); //(x, y, w, h)=>center(x+w/2,y+h/2)
	//transform to an ellipse
	QMatrix m;
	m.translate(width*0.5, height*0.5);
	m.scale(ab, 1.0);
	m.translate(-width*0.5, -height*0.5);
	E = m.map(E);
	current_clip_region = E & QRegion(0, 0, width, height);//the initial current_clip_region is null
	next_clip_region = QRegion(0, 0, width, height) - current_clip_region;
	return true;
}

bool SlideEffect_Center::prepareFrameAt_HorizontalOpen(int frame)
{
	if (isEndFrame(frame))
		return false;
	qreal k = easing_.valueForProgress(progress_);
	next_clip_region = QRegion(width*(1.0-k)*0.5, 0, width*k, height);
	current_clip_region = QRegion(0, 0, width, height) - next_clip_region;
	return true;
}

bool SlideEffect_Center::prepareFrameAt_HorizontalClose(int frame)
{
	if (isEndFrame(frame))
		return false;
	qreal k = easing_.valueForProgress(progress_);
	current_clip_region = QRegion(width*k*0.5, 0, width*(1.0-k), height);
	next_clip_region = QRegion(0, 0, width, height) - current_clip_region;
	return true;
}

bool SlideEffect_Center::prepareFrameAt_VerticalOpen(int frame)
{
	if (isEndFrame(frame))
		return false;
	qreal k = easing_.valueForProgress(progress_);
	next_clip_region = QRegion(0, height*(1.0-k)*0.5, width, height*k);
	current_clip_region = QRegion(0, 0, width, height) - next_clip_region;
	return true;
}

bool SlideEffect_Center::prepareFrameAt_VerticalClose(int frame)
{
	if (isEndFrame(frame))
		return false;
	qreal k = easing_.valueForProgress(progress_);
	current_clip_region = QRegion(0, height*k*0.5, width, height*(1.0-k));
	next_clip_region = QRegion(0, 0, width, height) - current_clip_region;
	return true;
}
