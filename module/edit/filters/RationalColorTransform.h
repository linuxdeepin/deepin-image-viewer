#ifndef RATIONALCOLORTRANSFORM_H
#define RATIONALCOLORTRANSFORM_H
#include "Filter2D.h"

namespace filter2d {

typedef QGenericMatrix<4, 5, float> MatrixRGBA;
//quadratic, rational fractional
// rational polynomial function
// Q/P

class RationalColorTransform : public ColorTransform {
public:
    bool isPointWise() const { return false;} // TODO: move to base
};

// rational polynomial function with order 1
// P1
class RationalPolynomialColorTransform1 : public RationalColorTransform {
public:
    QRgb operator()(QRgb c) override;
    // alpha: 5x4
protected:
    virtual QMatrix4x4 colorMatrix() const = 0;
    // TODO: isConst():  !point wise matrix
    virtual void postProcess1(QVector4D& pass1, QRgb orig) {
        Q_UNUSED(pass1);
        Q_UNUSED(orig);
    }
private:
    bool mDirtyC = true;
    QMatrix4x4 mC;
};
typedef RationalPolynomialColorTransform1 LinearColorTransform;


// rational polynomial function with order 2
// P2
class RationalPolynomialColorTransform2 : public RationalPolynomialColorTransform1
{
protected:
    void postProcess1(QVector4D& pass1, QRgb orig) override;
    // alpha: 5x4
    virtual QMatrix4x4 colorMatrix1() const = 0;
    virtual void postProcess2(QVector4D& pass2, QRgb orig) {
        Q_UNUSED(pass2);
        Q_UNUSED(orig);
    }
};

// Q1/P1. _1, _2,..._N means it's in denominator part
class RationalFractionalColorTransform1_1 : public RationalPolynomialColorTransform1
{
protected:
    virtual void postProcess1(QVector4D& pass1, QRgb orig);
    virtual QMatrix4x4 colorMatrix_1() const = 0;
    virtual void postProcess_2(QVector4D& pass_2, QRgb orig) {
        Q_UNUSED(pass_2);
        Q_UNUSED(orig);
    }
};
} //namespace filter2d
#endif // RATIONALCOLORTRANSFORM_H
