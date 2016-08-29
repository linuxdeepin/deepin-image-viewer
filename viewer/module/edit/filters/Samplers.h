#ifndef SAMPLERS_H
#define SAMPLERS_H
#include "ConvolutionSampler.h"

namespace filter2d {

class SpreadSampler : public Convolution3x3
{
protected:
    bool isPointWise() const override { return true;}
    QMatrix3x3 kernel() const override {
        const int n = qAbs(qrand()%9);
        QMatrix3x3 M;
        M.fill(0);
        M(n/3, n%3) = 1;
        return M;
    }
};

//http://blog.csdn.net/yangtrees/article/details/8740933
class MediumBlurSampler final : public Convolution3x3
{
protected:
    QMatrix3x3 kernel() const override {
        QMatrix3x3 M;
        M.fill(1);
        return M;
    }
};

class ReliefSampler final : public Convolution3x3 {
protected:
    QMatrix3x3 kernel() const override {
        QMatrix3x3 M;
        M.fill(0);
        M(0, 0) = 1;
        M(2, 2) = -1;
        return M;
    }
    int shift() const override {return 128;}
};

class CarvingSampler final : public Convolution3x3 {
protected:
    QMatrix3x3 kernel() const override {
        QMatrix3x3 M;
        M.fill(0);
        M(0, 0) = -1;
        M(2, 2) = 1;
        return M;
    }
    int shift() const override {return 128;}
};

//http://blog.csdn.net/yangtrees/article/details/8740933
class EdgeDetcetSampler final : public Convolution3x3 {
protected:
    QMatrix3x3 kernel() const override {
        QMatrix3x3 M;
        M.fill(0);
        M(0, 0) = M(0, 2) = M(2, 0) = M(2, 2) = -1;
        M(1, 1) = 4;
        return M;
    }
};

} // namespace filter2d
#endif // SAMPLERS_H
