#ifndef CONVOLUTIONSAMPLER_H
#define CONVOLUTIONSAMPLER_H
#include "Filter2D.h"

namespace filter2d {

template<int N, int M>
void NormalizeMat(QGenericMatrix<N, M, float>& K)
{
    float s = 0;
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < N; ++j)
            s += K(i, j);
    if (!qFuzzyIsNull(s))
        K /= s;
}

template<int R, int C = R>
class ConvolutionSampler : public Sampler
{
public:
    virtual bool isPointWise() const { return false;}
    virtual QGenericMatrix<R, C, float> kernel() const = 0;
    virtual int shift() const { return 0;}

    QRgb operator()(int x, int y, const QImage& img) override {
        if (mDirtyKernel) {
            mK = kernel();
            NormalizeMat(mK);
            mDirtyKernel = isPointWise();
        }
        const int kS = shift();
        int r = 0, g = 0, b = 0, a = 0;
        for (int i = 0; i < R*C; ++i) {
            QRgb c = 0; // or 0xffffffff?
            const int offset_R = (i%C)-1;
            const int offset_C = (i/R)-1;
            if (x+offset_R >= 0 && y+offset_C >= 0
                    && x+offset_R < img.width() && y+offset_C < img.height())
                c = img.pixel(x+offset_R, y+offset_C);
            const float k = mK(i/C, i%R);
            r += k*(float)qRed(c);
            g += k*(float)qGreen(c);
            b += k*(float)qBlue(c);
            //a += k*(float)qAlpha(c);
        }
        a = 255;
        return qRgba(qBound<quint8>(0, 255, r+kS),
                     qBound<quint8>(0, 255, g+kS),
                     qBound<quint8>(0, 255, b+kS),
                     a);
    }
private:
    bool mDirtyKernel = true;
    QGenericMatrix<R, C, float> mK;
};
typedef ConvolutionSampler<3, 3> Convolution3x3;
typedef ConvolutionSampler<5, 5> Convolution5x5;

} //namespace filter2d
#endif // CONVOLUTIONSAMPLER_H
