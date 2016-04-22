#include "slideeffect.h"
#include <qpainter.h>

static const QString kFade = "fade";

class SlideEffect_Fade : public SlideEffect
{
public:
    SlideEffect_Fade();
    virtual QVector<EffectId> supportedTypes() const {
        return QVector<EffectId>() << kFade;
    }

protected:
    virtual bool prepare();
    virtual bool prepareFrameAt(int frame);
    virtual void renderFrame();
};

REGISTER_EFFECTS(SlideEffect_Fade)

SlideEffect_Fade::SlideEffect_Fade()
    :SlideEffect()
{
    effect_type = kFade;
}

bool SlideEffect_Fade::prepare()
{
    SlideEffect::prepare();
	next_clip_region = QRegion();
	current_clip_region = QRegion(0, 0, width, height);// - next_clip_region;
	next_rect = QRect();
	if (frame_image) {
		qDebug("delete frame_image");
		delete frame_image;
	}
	frame_image = current_image;
	return true;
}

bool SlideEffect_Fade::prepareFrameAt(int frame)
{
	if (isEndFrame(frame)) {
		current_image = 0;  //avoid delete current_image twice and ensure frame_image not empty.
		return false;
	}
	return true;
}

void SlideEffect_Fade::renderFrame()
{
    const qreal k = easing_.valueForProgress(progress_);
	QPainter p(current_image);
    p.setOpacity(k);
	p.drawImage(0, 0, *next_image);
}
