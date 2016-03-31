#ifndef FILTER2D_H
#define FILTER2D_H

#include <cassert>
#include <QImage>
#include <QGenericMatrix>
#include <QMatrix4x4>
#include <QVector4D>

namespace filter2d {
// Filter2D
// PT: (x,y)->(x',y')
//F=PP*CT*S(x',y')

struct PointTransform {
    virtual void operator()(int *x, int *y) = 0;
};

struct PointTransformIdentity final : public PointTransform {
    void operator ()(int*, int*) override {}
};

struct Sampler {
    virtual QRgb operator()(int x, int y, const QImage& img) = 0;
};

struct SamplerIdentity final : public Sampler {
    QRgb operator()(int x, int y, const QImage& img) override {
        return img.pixel(x, y);
    }
};

struct ColorTransform {
    enum ClampMode {
        ClampToBound,
        EnsurePositive,
    };
    virtual ClampMode clampMode() const {return EnsurePositive;}
    virtual QRgb operator()(QRgb c) = 0;
};

struct ColorTransformIdentity final : public ColorTransform {
    QRgb operator()(QRgb c) override { return c;}
};

struct PostProcessor {
    virtual QRgb operator()(QRgb c) = 0; // TODO: more parameters, like x, y, srcImage?
};

struct PostProcessorIdentity final : public PostProcessor {
    QRgb operator()(QRgb c) override { return c;}
};

class Filter2DBase {
public:
    virtual ~Filter2DBase() {}
    /*!
     * \brief intensity
     * 0~1
     */
    qreal intensity() const {return m_intensity;}
    void setIntensity(qreal value) {
        assert(value >= 0 && value <= 1);
        if (m_intensity == value)
            return;
        m_intensity = value;
        Q_EMIT intensityChanged();
    }

    virtual QImage apply(const QImage& src) = 0;
    virtual void intensityChanged() {}
private:
    qreal m_intensity = 1.0;
};
} //namespace filter2d
#endif // FILTER2D_H
