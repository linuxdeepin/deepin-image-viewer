#ifndef FILTERS_H
#define FILTERS_H

#include "Samplers.h"
#include "ColorTransforms.h"

namespace filter2d {

QRgb mix(qreal k, QRgb c, QRgb c0);
QRgb mix(qreal k, QRgb c0, int r, int g, int b, int a = 255);

template<typename PP, typename CT, typename S, typename PT = PointTransformIdentity>
class Filter2D : public virtual Filter2DBase { //virtual: used ob FilterObj
public:
    QImage apply(const QImage& src) override {
        QImage out(src.size(), src.format());
        for (int y = 0; y < src.height(); ++y) {
            for (int x = 0; x < src.width(); ++x) {
                pt(&x, &y);
                QRgb c = pp(ct(s(x, y, src)));
                if (intensity() < 1.0) {
                    c = mix(intensity(), c, src.pixel(x, y));
                }
                out.setPixel(x, y, c); //img[x,y] = c;
            }
        }
        return out;
    }
    PT* pointTransform() const {return &pt;}
    S* sampler() const {return &s;}
    CT* colorTransform() const {return const_cast<CT*>(&ct);}
    PP* postProcessor() const {return &pp;}
private:
    // init here to avoid construct freequently, and supports non-const operations
    PT pt;
    S s;
    CT ct;
    PP pp;
};

//http://stackoverflow.com/questions/2996914/c-typedef-for-partial-templates
// typedef can not be a template
template<typename PT> using PositionTransformFilter = Filter2D<PostProcessorIdentity, ColorTransformIdentity, SamplerIdentity, PT>;
template<typename S> using SamplerFilter = Filter2D<PostProcessorIdentity, ColorTransformIdentity, S, PointTransformIdentity>;
template<typename CT> using ColorTransformFilter = Filter2D<PostProcessorIdentity, CT, SamplerIdentity, PointTransformIdentity>;
template<typename PP> using PostProcessorFilter = Filter2D<PP, ColorTransformIdentity, SamplerIdentity, PointTransformIdentity>;
/*
template<typename CT> class ColorTransformFilter : public Filter2D<PostProcessorIdentity, CT, SamplerIdentity, PointTransformIdentity> {};
template<typename PP> struct PostProcessorFilter { typedef Filter2D<PP, ColorTransformIdentity, SamplerIdentity, PointTransformIdentity> Type;};
*/

// TODO: filter(qobj) properties call sampler member funcs
typedef SamplerFilter<SpreadSampler> SpreadFilter;
typedef SamplerFilter<MediumBlurSampler> MediumBlurFilter;
typedef SamplerFilter<ReliefSampler> ReliefFilter;
typedef SamplerFilter<CarvingSampler> CarvingFilter;
typedef SamplerFilter<EdgeDetcetSampler> EdgeDetcetFilter;

typedef ColorTransformFilter<ColorTransform_Sepia> SepiaFilter;
typedef ColorTransformFilter<ColorTransform_Freezing> FreezingFilter;
typedef ColorTransformFilter<ColorTransform_Comic> ComicFilter;
typedef ColorTransformFilter<ColorTransform_Casting> CastingFilter;
typedef ColorTransformFilter<ColorTransform_Instagram1977> Instagram1977Filter;
typedef ColorTransformFilter<ColorTransform_Cold> ColdFilter;
typedef ColorTransformFilter<ColorTransform_Brightness> BrightnessFilter;
typedef ColorTransformFilter<ColorTransform_Contrast> ContrastFilter;
typedef ColorTransformFilter<ColorTransForm_Saturation> SaturationFilter;
typedef ColorTransformFilter<ColorTransform_Hue> HueFilter;
typedef ColorTransformFilter<ColorTransform_EQ> EQFilter;
} //namespace filter2d

#endif // FILTERS_H
