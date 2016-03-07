#include "FilterObj.h"
#include "Filters.h"
#include "FilterId.h"

namespace filter2d {

class EdgeDetcetFilterObj : public FilterObj, public EdgeDetcetFilter {};
class CarvingFilterObj : public FilterObj, public CarvingFilter {};
class ReliefFilterObj : public FilterObj, public ReliefFilter {};
class MediumBlurFilterObj : public FilterObj, public MediumBlurFilter {};
class SpreadFilterObj : public FilterObj, public SpreadFilter {};

#define DEFINE_PROPERTY_CT(T, READ, WRITE, SIG) \
    public: T READ() const {return colorTransform()->READ();} \
    void WRITE(T value) {colorTransform()->WRITE(value); Q_EMIT SIG();} \
    Q_SIGNAL void SIG();

class ComicFilterObj : public FilterObj, public ComicFilter {};
class FreezingFilterObj : public FilterObj, public FreezingFilter {};
class SepiaFilterObj : public FilterObj, public SepiaFilter {};
class ColdFilterObj : public FilterObj, public ColdFilter {};
class Instagram1977FilterObj : public FilterObj, public Instagram1977Filter {};
class CastingFilterObj : public FilterObj, public CastingFilter {};
class BrightnessFilterObj : public FilterObj, public BrightnessFilter
{
    Q_OBJECT
    Q_PROPERTY(qreal brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    DEFINE_PROPERTY_CT(qreal, brightness, setBrightness, brightnessChanged)
};

class ContrastFilterObj : public FilterObj, public ContrastFilter
{
    Q_OBJECT
    Q_PROPERTY(qreal contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    DEFINE_PROPERTY_CT(qreal, contrast, setContrast, contrastChanged)
};

class SaturationFilterObj : public FilterObj, public SaturationFilter
{
    Q_OBJECT
    Q_PROPERTY(qreal saturation READ saturation WRITE setSaturation NOTIFY saturationChanged)
    DEFINE_PROPERTY_CT(qreal, saturation, setSaturation, saturationChanged)
};

class HueFilterObj : public FilterObj, public HueFilter
{
    Q_OBJECT
    Q_PROPERTY(qreal hue READ hue WRITE setHue NOTIFY hueChanged)
    DEFINE_PROPERTY_CT(qreal, hue, setHue, hueChanged)
};

class EQFilterObj : public FilterObj, public EQFilter
{
    Q_OBJECT
    Q_PROPERTY(qreal contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    Q_PROPERTY(qreal contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    Q_PROPERTY(qreal hue READ hue WRITE setHue NOTIFY hueChanged)
    Q_PROPERTY(qreal saturation READ saturation WRITE setSaturation NOTIFY saturationChanged)
    DEFINE_PROPERTY_CT(qreal, brightness, setBrightness, brightnessChanged)
    DEFINE_PROPERTY_CT(qreal, contrast, setContrast, contrastChanged)
    DEFINE_PROPERTY_CT(qreal, hue, setHue, hueChanged)
    DEFINE_PROPERTY_CT(qreal, saturation, setSaturation, saturationChanged)
};

FilterObj* FilterObj::create(int id)
{
    switch (id) {
    case kBrightness: return new BrightnessFilterObj();
    case kHue: return new HueFilterObj();
    case kSaturation: return new SaturationFilterObj();
    case kContrast: return new ContrastFilterObj();
    case kEQ: return new EQFilterObj();
    case kCold: return new ColdFilterObj();
    case kCasting: return new CastingFilterObj();
    case kComic: return new ComicFilterObj();
    case kFreezing: return new FreezingFilterObj();
    case kInstagram1977: return new Instagram1977FilterObj();
    case kSepia: return new SepiaFilterObj();
    case kEdgeDetcet: return new EdgeDetcetFilterObj();
    case kCarving: return new CarvingFilterObj();
    case kRelief: return new ReliefFilterObj();
    case kSpread: return new SpreadFilterObj();
    case kMediumBlur: return new MediumBlurFilterObj();
    default:
        return NULL;
    }
    return NULL;
}
} //namespace filter2d

#include "FilterObj.moc"
