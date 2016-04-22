#include "slideeffect.h"
#include <time.h>
#include <cmath>

static const EffectId kTile = "tile";
class SlideEffect_Tile : public SlideEffect
{
public:
    SlideEffect_Tile();
    virtual QVector<EffectId> supportedTypes() const {
        return QVector<EffectId>() << kTile;
    }
protected:
    virtual bool prepare();
    virtual bool prepareFrameAt(int frame);

private:
    int nodes_per_frame, w, h;
    int rows, cols;
    QList<QPoint> nodes;
};

REGISTER_EFFECTS(SlideEffect_Tile)

SlideEffect_Tile::SlideEffect_Tile()
{
    effect_type = kTile;
}

bool SlideEffect_Tile::prepare()
{
    SlideEffect::prepare();
	rows = cols = 32; //
	nodes_per_frame = rows*cols/frames_total+1; //make sure that no blanks at the last frame

	w = width/rows+1; //make sure that no blanks at the last frame
	h = height/cols+1;//make sure that no blanks at the last frame
	nodes.clear();
	for(int i=0;i<rows;++i)
		for(int j=0;j<cols;++j)
			nodes.append(QPoint(i*w, j*h));
			//nodes.push_back(QPoint(i*w, j*h));

	next_clip_region = QRegion(); //Important!
	return true;
}

bool SlideEffect_Tile::prepareFrameAt(int frame)
{
	if (frame>frames_total || nodes.empty())
		return false;
	current_frame = frame;

	srand(time(0));
	int index = 0;
	for(int i=0;i<nodes_per_frame;++i) {
		if (nodes.empty())
			break;
		index = rand()%(nodes.size());
		QPoint p = nodes.takeAt(index);//nodes.at(index);
		next_clip_region += QRegion(p.x(), p.y(), w, h);
		//nodes.erase(nodes.begin()+index);
	}
	current_clip_region = QRegion(0, 0, width, height) - next_clip_region;
	return true;
}
